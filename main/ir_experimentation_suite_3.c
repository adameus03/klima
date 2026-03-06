#include "ir_experimentation_suite_3.h"
#include "sony.h"
#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

static const char *TAG = "ies3";

int ies3_run(ies3_resources_t* pResources) {
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
    Transmit
  */
  
  // turn on the TV
  ESP_LOGI(TAG, "Turning on the TV");
  sony_frame_t frame = SONY_FRAME_POWER;
  const int n = 3;

  sony_frame_rmt_symbols_t frame_symbols = sony_encode_frame(frame, 1);
  rmt_symbol_word_t tx_symbols[sizeof(sony_frame_rmt_symbols_t) / sizeof(rmt_symbol_word_t) * n];
  sony_repeat(frame_symbols, tx_symbols, n);

  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, tx_symbols, sizeof(tx_symbols), &transmit_config));

  // wait 5s for the TV to turn on
  ESP_LOGI(TAG, "Waiting 5s");
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  // increase volume
  ESP_LOGI(TAG, "Increasing volume");
  frame_symbols = sony_encode_frame(SONY_FRAME_VOLUP, 1);
  sony_repeat(frame_symbols, tx_symbols, n);
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, tx_symbols, sizeof(tx_symbols), &transmit_config));

  // wait 1s between volume up commands
  ESP_LOGI(TAG, "Waiting 1s");
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // send 3 more volume up commands
  ESP_LOGI(TAG, "Increasing volume 3 more times");
  for (int i = 0; i < 3; i++) {
    ESP_LOGI(TAG, "Volume up #%d", i + 1);
    ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, tx_symbols, sizeof(tx_symbols), &transmit_config));
    ESP_LOGI(TAG, "Waiting 1s");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  // send 3 volume down commands
  ESP_LOGI(TAG, "Decreasing volume 3 times");
  frame_symbols = sony_encode_frame(SONY_FRAME_VOLDN, 1);
  sony_repeat(frame_symbols, tx_symbols, n);
  for (int i = 0; i < 3; i++) {
    ESP_LOGI(TAG, "Volume down #%d", i + 1);
    ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, tx_symbols, sizeof(tx_symbols), &transmit_config));
    ESP_LOGI(TAG, "Waiting 1s");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  // switch to next program on the TV
  ESP_LOGI(TAG, "Switching to next program");
  frame_symbols = sony_encode_frame(SONY_FRAME_PROG_NEXT, 1);
  sony_repeat(frame_symbols, tx_symbols, n);
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, tx_symbols, sizeof(tx_symbols), &transmit_config));

  // wait 5s between program changes
  ESP_LOGI(TAG, "Waiting 5s");
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  // switch to previous program on the TV
  ESP_LOGI(TAG, "Switching to previous program");
  frame_symbols = sony_encode_frame(SONY_FRAME_PROG_PREV, 1);
  sony_repeat(frame_symbols, tx_symbols, n);
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, tx_symbols, sizeof(tx_symbols), &transmit_config));

  // wait 5s before turning off the TV
  ESP_LOGI(TAG, "Waiting 5s");
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  // turn off the TV
  ESP_LOGI(TAG, "Turning off the TV");
  frame_symbols = sony_encode_frame(SONY_FRAME_POWER, 1);
  sony_repeat(frame_symbols, tx_symbols, n);
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, tx_symbols, sizeof(tx_symbols), &transmit_config));

  ESP_LOGI(TAG, "Done transmitting, TV should be off now");
  
  return 0;
}

void ies3_dispose(ies3_resources_t* pResources) {
  if (pResources) {
    if (pResources->encoder != NULL) {
      ESP_ERROR_CHECK(rmt_del_encoder(pResources->encoder));
    }
    if (pResources->tx_chan != NULL) {
      ESP_ERROR_CHECK(rmt_disable(pResources->tx_chan));
      ESP_ERROR_CHECK(rmt_del_channel(pResources->tx_chan));
    }
  }
}