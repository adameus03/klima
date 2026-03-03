#include "ir_experimentation_suite_0.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "ies0";

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

int ies0_run(ies_resources_t* pResources) {
  // rmt_channel_handle_t tx_chan = NULL;
  // rmt_tx_channel_config_t tx_chan_config = {
  //   .clk_src = RMT_CLK_SRC_DEFAULT,   // select source clock
  //   .gpio_num = 14,                    // GPIO number
  //   .mem_block_symbols = 64,          // memory block size, 64 * 4 = 256 Bytes
  //   .resolution_hz = 1 * 1000 * 1000, // 1 MHz tick resolution, i.e., 1 tick = 1 µs
  //   .trans_queue_depth = 4,           // set the number of transactions that can pend in the background
  //   .flags.invert_out = false,        // do not invert output signal
  //   .flags.with_dma = false,          // do not need DMA backend
  // };
  // ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &tx_chan));
  // rmt_transmit_config_t transmit_config = {
  //   .loop_count = 0, // no loop
  // };

  // rmt_encoder_handle_t encoder = NULL;
  // rmt_copy_encoder_config_t encoder_config = {0};
  // ESP_ERR_CHECK(rmt_new_copy_encoder(&encoder_config, &encoder));

  
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


  //ESP_ERROR_CHECK(rmt_enable(tx_chan));
  ESP_ERROR_CHECK(rmt_enable(rx_chan));

  rmt_symbol_word_t raw_symbols[256];
  rmt_rx_done_event_data_t rx_data;

  ESP_ERROR_CHECK(rmt_receive(rx_chan, raw_symbols, sizeof(raw_symbols), &receive_config));

  int counter = 0;

  while (counter++ < 100) {
    ESP_LOGI(TAG, "Receive loop iteration %d", counter);
    rx_data = (rmt_rx_done_event_data_t){0};
    // wait for RX done signal
    if (xQueueReceive(receive_queue, &rx_data, pdMS_TO_TICKS(1000)) == pdPASS) {
      // parse the receive symbols and print the result

      ies0_print_rmt_rx_done_event_data(rx_data);

      //example_parse_nec_frame(rx_data.received_symbols, rx_data.num_symbols);
      // start receive again
      ESP_ERROR_CHECK(rmt_receive(rx_chan, raw_symbols, sizeof(raw_symbols), &receive_config));
    } else {
      // timeout, transmit predefined IR NEC packets
      // const ir_nec_scan_code_t scan_code = {
      //     .address = 0x0440,
      //     .command = 0x3003,
      // };
      // ESP_ERROR_CHECK(rmt_transmit(tx_channel, nec_encoder, &scan_code, sizeof(scan_code), &transmit_config));
      
      ESP_LOGI(TAG, "Receive timeout, iterating");
    }
    
  }
  ESP_LOGI(TAG, "Exited receive loop");
  return 0;
}

void ies0_dispose(ies_resources_t* pResources) {
  if (pResources->rx_chan != NULL) {
    ESP_ERROR_CHECK(rmt_disable(pResources->rx_chan));
    ESP_ERROR_CHECK(rmt_del_channel(pResources->rx_chan));
  }
}