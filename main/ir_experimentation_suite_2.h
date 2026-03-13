/*
  Experiment where we receive the signal that we send ourselves
*/

#ifndef IR_EXPERIMENTATION_SUITE_2_H
#define IR_EXPERIMENTATION_SUITE_2_H

#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"

typedef struct ies2_resources {
  rmt_channel_handle_t rx_chan;
  rmt_channel_handle_t tx_chan;
  rmt_encoder_handle_t encoder;
} ies2_resources_t;

int ies2_run(ies2_resources_t* pResources);
void ies2_dispose(ies2_resources_t* pResources);

#endif //IR_EXPERIMENTATION_SUITE_2_H