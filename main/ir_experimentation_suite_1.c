#include "ir_experimentation_suite_1.h"
#include "sony.h"
#include "esp_log.h"

static const char *TAG = "ies1";

int ies1_run(ies1_resources_t* pResources) {
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

  rmt_encoder_handle_t encoder = NULL;
  rmt_copy_encoder_config_t encoder_config = {};
  ESP_ERROR_CHECK(rmt_new_copy_encoder(&encoder_config, &encoder));
  pResources->encoder = encoder;

  ESP_ERROR_CHECK(rmt_enable(tx_chan));

  sony_frame_t frame = SONY_FRAME_POWER;
  const int n = 3;

  sony_frame_rmt_symbols_t frame_symbols = sony_encode_frame(frame, 1);
  rmt_symbol_word_t tx_symbols[sizeof(sony_frame_rmt_symbols_t) * n];
  sony_repeat(frame_symbols, tx_symbols, n);

  ESP_LOGI(TAG, "Transmitting...");
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, tx_symbols, sizeof(tx_symbols), &transmit_config));
  ESP_LOGI(TAG, "Done transmitting");

  return 0;
}

void ies1_dispose(ies1_resources_t* pResources) {
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