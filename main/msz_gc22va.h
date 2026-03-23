#ifndef MSZG22VA_H
#define MSZG22VA_H

#include <stdint.h>
#include "driver/rmt_tx.h"

#define MSZGC22VA_FRAME_LEN 18
#define MSZGC22VA_RMT_SYMBOLS_PER_FRAME (2 * (MSZGC22VA_FRAME_LEN * 8 + 2))

#define mszgc22vaENABLE 1
#define mszgc22vaDISABLE 0
#define mszgc22vaTRUE 1
#define mszgc22vaFALSE 0

#define MSZGC22VA_DEVICE_ID 0x0126cb23

typedef union mszgc22va_frame {
  struct {
    /* Device identification */
    uint32_t device_identification;

    /* Configuration registers */

    uint16_t power_status_reg;
    //uint8_t mode_config_reg;
    uint32_t hvac_control_reg;
    uint32_t timer_config_reg;
    uint8_t econo_cool_reg;

    /* Extended control register */
    uint16_t ext_control_reg;
    /* Checksum */
    uint8_t cksum;
  } __attribute__((__packed__));
  uint8_t data[MSZGC22VA_FRAME_LEN];
} __attribute__((__packed__)) mszgc22va_frame_t;

typedef enum mszgc22va_power_status {
  mszgc22vaPOWER_OFF,
  mszgc22vaPOWER_ON
} mszgc22va_power_status_t;

typedef uint8_t mszgc22va_temperature_value_t;

typedef enum mszgc22va_fan_speed {
  mszgc22vaFAN_SPEED_AUTO,
  mszgc22vaFAN_SPEED_SLOW,
  mszgc22vaFAN_SPEED_MEDIUM,
  mszgc22vaFAN_SPEED_HIGH,
  mszgc22vaFAN_SPEED_VERY_HIGH
} mszgc22va_fan_speed_t;

typedef enum mszgc22va_vane_angle {
  mszgc22vaVANE_ANGLE_AUTO,
  mszgc22vaVANE_ANGLE_P1,
  mszgc22vaVANE_ANGLE_P2,
  mszgc22vaVANE_ANGLE_P3,
  mszgc22vaVANE_ANGLE_P4,
  mszgc22vaVANE_ANGLE_SWING = 7
} mszgc22va_vane_angle_t;

typedef enum mszgc22va_operation_mode {
  mszgc22vaMODE_HEATING = 1,
  mszgc22vaMODE_DRYING,
  mszgc22vaMODE_COOLING,
  mszgc22vaMODE_AUTO
} mszgc22va_operation_mode_t;

typedef struct mszgc22va_frame_rmt_symbols {
  rmt_symbol_word_t symbols[MSZGC22VA_RMT_SYMBOLS_PER_FRAME];
} mszgc22va_frame_rmt_symbols_t;

typedef struct mszgc22va_frame_params {
  uint32_t deviceId;
  mszgc22va_power_status_t powerStatus;
  mszgc22va_operation_mode_t operatingMode;
  uint8_t hvacTemperature;
  mszgc22va_fan_speed_t fanSpeed;
  mszgc22va_vane_angle_t vaneAngle;
  int fvAutoModeSet;
  int dewPreventionEnable;
  int coldAirPreventionEnable;
  uint8_t timeNow;
  uint8_t schedTimeStop;
  uint8_t schedTimeStart;
  int timerSchedSet;
  int timerStopEnable;
  int timerStartEnable;
  int econoCoolOperationEnable;
  int horizontalVanePositionEnable;
} mszgc22va_frame_params_t;

mszgc22va_frame_t mszgc22va_frame_create(mszgc22va_frame_params_t params);

mszgc22va_frame_rmt_symbols_t mszgc22va_encode_frame(mszgc22va_frame_t frame);

#endif //MSZG22VA_H