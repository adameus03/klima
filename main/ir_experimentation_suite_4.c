#include "ir_experimentation_suite_4.h"
#include "msz_gc22va.h"
#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

static const char *TAG = "ies4";

int ies4_run(ies4_resources_t* pResources) {
  /*
    Prepare tx
  */

  rmt_channel_handle_t tx_chan = NULL;
  rmt_tx_channel_config_t tx_chan_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,   // select source clock
    .gpio_num = 14,                    // GPIO number
    .mem_block_symbols = 512,          // memory block size
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

  // Wait 10s before turning on the AC
  ESP_LOGI(TAG, "Waiting 10s before turning on the AC");
  vTaskDelay(10000 / portTICK_PERIOD_MS);
  
  // turn on the AC with extreme heating settings
  ESP_LOGI(TAG, "Turning on the AC with extreme heating settings");
  mszgc22va_frame_params_t turn_on_prms = {
    .deviceId = 0x0126cb23,
    .powerStatus = mszgc22vaPOWER_ON,
    .operatingMode = mszgc22vaMODE_HEATING,
    .hvacTemperature = 0xf,
    .coldAirPreventionEnable = mszgc22vaENABLE,
    .dewPreventionEnable = mszgc22vaENABLE,
    .fanSpeed = mszgc22vaFAN_SPEED_VERY_HIGH,
    .vaneAngle = mszgc22vaVANE_ANGLE_SWING,
    .fvAutoModeSet = mszgc22vaFALSE,
    .timeNow =  0b01011011,  //11011010,
    .schedTimeStop = 0,
    .schedTimeStart = 0,
    .timerSchedSet = mszgc22vaDISABLE,
    .timerStopEnable = mszgc22vaDISABLE,
    .timerStartEnable = mszgc22vaDISABLE,
    .econoCoolOperationEnable = mszgc22vaDISABLE,
    .horizontalVanePositionEnable = mszgc22vaDISABLE
  };
  mszgc22va_frame_t ac_frame = mszgc22va_frame_create(turn_on_prms);
  mszgc22va_frame_rmt_symbols_t ac_control_signal = mszgc22va_encode_frame(ac_frame);

  // //debug print the transmitted symbols
  // for (int i = 0; i < sizeof(ac_control_signal.symbols) / sizeof(ac_control_signal.symbols[0]); i++) {
  //   ESP_LOGI(TAG, "tx_symbol[%d]: duration0=%hu, level0=%hu, duration1=%hu, level1=%hu",
  //            i, ac_control_signal.symbols[i].duration0, ac_control_signal.symbols[i].level0,
  //            ac_control_signal.symbols[i].duration1, ac_control_signal.symbols[i].level1);
  // }
  // ESP_LOGI(TAG, "Transmitting...");
  // ESP_LOGI(TAG, "Payload bytes: %d", sizeof(ac_control_signal.symbols));

  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, ac_control_signal.symbols, sizeof(ac_control_signal.symbols), &transmit_config));

  // wait 15s for the AC to turn on
  ESP_LOGI(TAG, "Waiting 15s");
  vTaskDelay(15000 / portTICK_PERIOD_MS);

  // Set vane angle to P1
  ESP_LOGI(TAG, "Setting vane angle to P1");
  mszgc22va_frame_params_t vane_p1_prms = turn_on_prms;
  vane_p1_prms.vaneAngle = mszgc22vaVANE_ANGLE_P1;
  ac_frame = mszgc22va_frame_create(vane_p1_prms);
  ac_control_signal = mszgc22va_encode_frame(ac_frame);
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, ac_control_signal.symbols, sizeof(ac_control_signal.symbols), &transmit_config));

  ESP_LOGI(TAG, "Waiting 20s");
  vTaskDelay(20000 / portTICK_PERIOD_MS);

  // Set vane angle to P2
  ESP_LOGI(TAG, "Setting vane angle to P2");
  mszgc22va_frame_params_t vane_p2_prms = turn_on_prms;
  vane_p2_prms.vaneAngle = mszgc22vaVANE_ANGLE_P2;
  ac_frame = mszgc22va_frame_create(vane_p2_prms);
  ac_control_signal = mszgc22va_encode_frame(ac_frame);
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, ac_control_signal.symbols, sizeof(ac_control_signal.symbols), &transmit_config));

  ESP_LOGI(TAG, "Waiting 20s");
  vTaskDelay(20000 / portTICK_PERIOD_MS);

  // Set vane angle to P3
  ESP_LOGI(TAG, "Setting vane angle to P3");
  mszgc22va_frame_params_t vane_p3_prms = turn_on_prms;
  vane_p3_prms.vaneAngle = mszgc22vaVANE_ANGLE_P3;
  ac_frame = mszgc22va_frame_create(vane_p3_prms);
  ac_control_signal = mszgc22va_encode_frame(ac_frame);
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, ac_control_signal.symbols, sizeof(ac_control_signal.symbols), &transmit_config));

  ESP_LOGI(TAG, "Waiting 20s");
  vTaskDelay(20000 / portTICK_PERIOD_MS);

  // Set vane angle to P4
  ESP_LOGI(TAG, "Setting vane angle to P4");
  mszgc22va_frame_params_t vane_p4_prms = turn_on_prms;
  vane_p4_prms.vaneAngle = mszgc22vaVANE_ANGLE_P4;
  ac_frame = mszgc22va_frame_create(vane_p4_prms);
  ac_control_signal = mszgc22va_encode_frame(ac_frame);
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, ac_control_signal.symbols, sizeof(ac_control_signal.symbols), &transmit_config));

  ESP_LOGI(TAG, "Waiting 20s");
  vTaskDelay(20000 / portTICK_PERIOD_MS);

  // Set vane angle to swing
  ESP_LOGI(TAG, "Setting vane angle to swing");
  mszgc22va_frame_params_t vane_swing_prms = turn_on_prms;
  vane_swing_prms.vaneAngle = mszgc22vaVANE_ANGLE_SWING;
  ac_frame = mszgc22va_frame_create(vane_swing_prms);
  ac_control_signal = mszgc22va_encode_frame(ac_frame);
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, ac_control_signal.symbols, sizeof(ac_control_signal.symbols), &transmit_config));

  ESP_LOGI(TAG, "Waiting 20s");
  vTaskDelay(20000 / portTICK_PERIOD_MS);

  // Set vane angle to P1
  ESP_LOGI(TAG, "Setting vane angle to P1");
  ac_frame = mszgc22va_frame_create(vane_p1_prms);
  ac_control_signal = mszgc22va_encode_frame(ac_frame);
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, ac_control_signal.symbols, sizeof(ac_control_signal.symbols), &transmit_config));

  ESP_LOGI(TAG, "Waiting 20s");
  vTaskDelay(20000 / portTICK_PERIOD_MS);

  // Turn off the AC
  ESP_LOGI(TAG, "Turning off the AC");
  mszgc22va_frame_params_t turn_off_prms = turn_on_prms;
  turn_off_prms.powerStatus = mszgc22vaPOWER_OFF;
  ac_frame = mszgc22va_frame_create(turn_off_prms);
  ac_control_signal = mszgc22va_encode_frame(ac_frame);
  ESP_ERROR_CHECK(rmt_transmit(tx_chan, encoder, ac_control_signal.symbols, sizeof(ac_control_signal.symbols), &transmit_config));

  // wait 15s for the AC to turn off
  ESP_LOGI(TAG, "Waiting 15s");
  vTaskDelay(15000 / portTICK_PERIOD_MS);

  ESP_LOGI(TAG, "Done transmitting, AC should be off now");
  
  return 0;
}

void ies4_dispose(ies4_resources_t* pResources) {
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