/**
 * Performing a Mitsubishi MSZ-GC22VA control sequence over an IR transmission channel
 */

#ifndef IR_EXPERIMENTATION_SUITE_4_H
#define IR_EXPERIMENTATION_SUITE_4_H

#include "driver/rmt_tx.h"

typedef struct ies4_resources {
  rmt_channel_handle_t tx_chan;
  rmt_encoder_handle_t encoder;
} ies4_resources_t;

int ies4_run(ies4_resources_t* pResources);
void ies4_dispose(ies4_resources_t* pResources);

#endif //IR_EXPERIMENTATION_SUITE_4_H