#include "ir_experimentation_suite_2.h"
#include "sony.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "ies2";

static bool ies0_rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data)
{
  BaseType_t high_task_wakeup = pdFALSE;
  QueueHandle_t receive_queue = (QueueHandle_t)user_data;
  // send the received RMT symbols to the parser task
  xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
  return high_task_wakeup == pdTRUE;
}

static void ies0_print_rmt_rx_done_event_data(const rmt_rx_done_event_data_t data) {
  ESP_LOGI(TAG, "Received RMT data, num_symbols=%d, is_last=%d", data.num_symbols, data.flags.is_last);
  for (int i = 0; i < data.num_symbols; i++) {
    ESP_LOGI(TAG, "symbol[%d]: duration0=%hu, level0=%hu, duration1=%hu, level1=%hu",
             i, data.received_symbols[i].duration0, data.received_symbols[i].level0,
             data.received_symbols[i].duration1, data.received_symbols[i].level1);
    
  }
}

int ies2_run(ies2_resources_t* pResources) {
  /*
    Prepare rx
  */

  rmt_channel_handle_t rx_chan = NULL;
  rmt_rx_channel_config_t rx_chan_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,   // select source clock
    .resolution_hz = 1 * 1000 * 1000, // 1 MHz tick resolution, i.e., 1 tick = 1 µs
    //.mem_block_symbols = 64,          // memory block size, 64 * 4 = 256 Bytes
    .mem_block_symbols = 256,
    .gpio_num = 27,                    // GPIO number
    .flags.invert_in = false,         // do not invert input signal
    .flags.with_dma = false,          // do not need DMA backend
  };
  ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_chan_config, &rx_chan));
  pResources->rx_chan = rx_chan;

  QueueHandle_t receive_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
  assert(receive_queue);
  rmt_rx_event_callbacks_t cbs = {
    .on_recv_done = ies0_rmt_rx_done_callback,
  };
  
  ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_chan, &cbs, receive_queue));

  rmt_receive_config_t receive_config = {
      .signal_range_min_ns = 1000, // 1 us
      .signal_range_max_ns = 65534999 // 65.534999 ms, max allowed value
  };

  ESP_ERROR_CHECK(rmt_enable(rx_chan));

  /*
    Prepare tx
  */

  rmt_channel_handle_t tx_chan = NULL;
  rmt_tx_channel_config_t tx_chan_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,   // select source clock
    .gpio_num = 14,                    // GPIO number
    .mem_block_symbols = 64,          // memory block size, 64 * 4 = 256 Bytes
    .resolution_hz = 1 * 1000 * 1000, // 1 MHz tick resolution, i.e., 1 tick = 1 µs
    .trans_queue_depth = 4,           // set the number of transactions that can pend in the background
    .flags.invert_out = false,        // do not invert output signal
    .flags.with_dma = false,          // do not need DMA backend
  };
  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &tx_chan));
  pResources->tx_chan = tx_chan;

  rmt_transmit_config_t transmit_config = {
    .loop_count = 0, // no loop
  };

  rmt_carrier_config_t tx_carrier_cfg = {
    .duty_cycle = 0.33,                 // duty cycle 33%
    .frequency_hz = 36000,              // 36 KHz
    .flags.polarity_active_low = false, // carrier should be modulated to high level
  };
  // setup carrier modulation for TX channel
  ESP_ERROR_CHECK(rmt_apply_carrier(tx_chan, &tx_carrier_cfg));


  rmt_encoder_handle_t encoder = NULL;
  rmt_copy_encoder_config_t encoder_config = {};
  ESP_ERROR_CHECK(rmt_new_copy_encoder(&encoder_config, &encoder));
  pResources->encoder = encoder;

  ESP_ERROR_CHECK(rmt_enable(tx_chan));

  /* 
    Start receiving
  */

  rmt_symbol_word_t raw_symbols[256];
  rmt_rx_done_event_data_t rx_data;

  ESP_ERROR_CHECK(rmt_receive(rx_chan, raw_symbols, sizeof(raw_symbols), &receive_config));

  /*
    Transmit
  */
  
  sony_frame_t frame = SONY_FRAME_POWER;
  const int n = 3;

  sony_frame_rmt_symbols_t frame_symbols = sony_encode_frame(frame, 1);
  rmt_symbol_word_t tx_symbols[sizeof(sony_frame_rmt_symbols_t) / sizeof(rmt_symbol_word_t) * n];
  sony_repeat(frame_symbols, tx_symbols, n);

  //debug print the transmitted symbols
  for (int i = 0; i < sizeof(tx_symbols) / sizeof(tx_symbols[0]); i++) {
    ESP_LOGI(TAG, "tx_symbol[%d]: duration0=%hu, level0=%hu, duration1=%hu, level1=%hu",
             i, tx_symbols[i].duration0, tx_symbols[i].level0,
             tx_symbols[i].duration1, tx_symbols[i].level1);
  }

  ESP_LOGI(TAG, "Transmitting...");
  ESP_LOGI(TAG, "Payload bytes: %d", sizeof(tx_symbols));
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, tx_symbols, sizeof(tx_symbols), &transmit_config));
  ESP_LOGI(TAG, "Done transmitting");

  /*
    Run the receive loop
  */
  int counter = 0;

  while (counter++ < 100) {
    ESP_LOGI(TAG, "Receive loop iteration %d", counter);
    rx_data = (rmt_rx_done_event_data_t){0};
    // wait for RX done signal
    if (xQueueReceive(receive_queue, &rx_data, pdMS_TO_TICKS(1000)) == pdPASS) {
      ies0_print_rmt_rx_done_event_data(rx_data);

      // start receive again
      ESP_ERROR_CHECK(rmt_receive(rx_chan, raw_symbols, sizeof(raw_symbols), &receive_config));
    } else {
      ESP_LOGI(TAG, "Receive timeout, iterating");
    }
    
  }
  ESP_LOGI(TAG, "Exited receive loop");
  
  return 0;
}

void ies2_dispose(ies2_resources_t* pResources) {
  if (pResources) {
    if (pResources->encoder != NULL) {
      ESP_ERROR_CHECK(rmt_del_encoder(pResources->encoder));
    }
    if (pResources->tx_chan != NULL) {
      ESP_ERROR_CHECK(rmt_disable(pResources->tx_chan));
      ESP_ERROR_CHECK(rmt_del_channel(pResources->tx_chan));
    }
    if (pResources->rx_chan != NULL) {
      ESP_ERROR_CHECK(rmt_disable(pResources->rx_chan));
      ESP_ERROR_CHECK(rmt_del_channel(pResources->rx_chan));
    }
  }
}