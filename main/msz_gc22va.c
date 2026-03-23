#include "msz_gc22va.h"
#include <assert.h>
#include "esp_log.h"

#define ENCODING_START_SYMBOL_LVL0_DUR 3500
#define ENCODING_START_SYMBOL_LVL1_DUR 1700

#define ENCODING_SHORT_SYMBOL_LVL0_DUR 450
#define ENCODING_SHORT_SYMBOL_LVL1_DUR 400

#define ENCODING_LONG_SYMBOL_LVL0_DUR 450
#define ENCODING_LONG_SYMBOL_LVL1_DUR 1250

#define ENCODING_GAP_SYMBOL_LVL0_DUR 450
#define ENCODING_GAP_SYMBOL_LVL1_DUR 17000

#define ENCODING_TERM_SYMBOL_LVL0_DUR 465
#define ENCODING_TERM_SYMBOL_LVL1_DUR 0

#define NUM_SYMBOLS 5

typedef enum symbol_type {
  SYMBOL_TYPE_START,
  SYMBOL_TYPE_SHORT,
  SYMBOL_TYPE_LONG,
  SYMBOL_TYPE_GAP,
  SYMBOL_TYPE_TERM,
} symbol_type_t;

static const char* TAG = "msz_gc22va";

static rmt_symbol_word_t get_symbol(symbol_type_t type) {
  static const uint16_t durations[NUM_SYMBOLS * 2] = {
    ENCODING_START_SYMBOL_LVL0_DUR, ENCODING_START_SYMBOL_LVL1_DUR,
    ENCODING_SHORT_SYMBOL_LVL0_DUR, ENCODING_SHORT_SYMBOL_LVL1_DUR,
    ENCODING_LONG_SYMBOL_LVL0_DUR, ENCODING_LONG_SYMBOL_LVL1_DUR,
    ENCODING_GAP_SYMBOL_LVL0_DUR, ENCODING_GAP_SYMBOL_LVL1_DUR,
    ENCODING_TERM_SYMBOL_LVL0_DUR, ENCODING_TERM_SYMBOL_LVL1_DUR
  };
  int typeId = (int)type;
  return (rmt_symbol_word_t) {
    .duration0 = durations[typeId * 2],
    .level0 = 1,
    .duration1 = durations[typeId * 2 + 1],
    .level1 = 0
  };
}

mszgc22va_frame_t mszgc22va_frame_create(mszgc22va_frame_params_t params) {
  mszgc22va_frame_t frame = {0};
  frame.device_identification = params.deviceId;
  frame.power_status_reg = params.powerStatus ? ((uint16_t)1) << 13 : 0;
  frame.hvac_control_reg = 0;
  frame.hvac_control_reg |= ((uint8_t)params.fanSpeed) | (((uint8_t)params.vaneAngle) << 3) | (((uint8_t)(params.fvAutoModeSet ? 2 : 1)) << 6);
  frame.hvac_control_reg <<= 8;
  frame.hvac_control_reg |= 0x30 | (((uint8_t)(params.dewPreventionEnable ? 0 : 1)) << 1) | (((uint8_t)(params.coldAirPreventionEnable ? 0 : 1)) << 2);
  frame.hvac_control_reg <<= 8;
  frame.hvac_control_reg |= params.hvacTemperature;
  frame.hvac_control_reg <<= 8;
  frame.hvac_control_reg |= ((uint8_t)params.operatingMode) << 3;
  frame.timer_config_reg = 0;
  frame.timer_config_reg |= params.timeNow;
  frame.timer_config_reg |= ((uint32_t)params.schedTimeStop) << 8;
  frame.timer_config_reg |= ((uint32_t)params.schedTimeStart) << 16;
  frame.timer_config_reg |= ((uint32_t)(((uint8_t)(params.timerSchedSet ? 1 : 0)) | ((uint8_t)(params.timerStopEnable ? 0x02 : 0)) | ((uint8_t)(params.timerStartEnable ? 0x04 : 0)))) << 24;
  frame.econo_cool_reg = params.econoCoolOperationEnable ? 0x20 : 0x0;
  frame.ext_control_reg = params.horizontalVanePositionEnable ? ((uint16_t)1) << 7 : 0;
  frame.cksum = 0;
  for (int i = 0; i < MSZGC22VA_FRAME_LEN - 1; i++) {
    frame.cksum += frame.data[i];
  }
  // if (frame.data[6] == 0) {
  //   // debug print hvac_control_reg
  //   ESP_LOGI(TAG, "hvac_control_reg: 0x%08x", frame.hvac_control_reg);
  // }
  return frame;
}

mszgc22va_frame_rmt_symbols_t mszgc22va_encode_frame(mszgc22va_frame_t frame) {
  mszgc22va_frame_rmt_symbols_t txSig = {0};
  txSig.symbols[0] = get_symbol(SYMBOL_TYPE_START);
  txSig.symbols[1 + MSZGC22VA_FRAME_LEN * 8] = get_symbol(SYMBOL_TYPE_GAP);
  txSig.symbols[1 + MSZGC22VA_FRAME_LEN * 8 + 1] = get_symbol(SYMBOL_TYPE_START);
  assert(1 + MSZGC22VA_FRAME_LEN * 8 + 1 == 146);
  txSig.symbols[1 + MSZGC22VA_FRAME_LEN * 8 + 1 + MSZGC22VA_FRAME_LEN * 8] = get_symbol(SYMBOL_TYPE_TERM);
  for (int i = 0; i < MSZGC22VA_FRAME_LEN; i++) {
    for (int j = 0; j < 8; j++) {
      rmt_symbol_word_t symbol = ((frame.data[i] >> j) & 1) ? get_symbol(SYMBOL_TYPE_LONG) : get_symbol(SYMBOL_TYPE_SHORT);
      assert(1 + i * 8 + j != 146);
      assert(1 + MSZGC22VA_FRAME_LEN * 8 + 2 + i * 8 + j != 146);
      txSig.symbols[1 + i * 8 + j] = symbol;
      txSig.symbols[1 + MSZGC22VA_FRAME_LEN * 8 + 2 + i * 8 + j] = symbol;
    }
  }
  return txSig;
}