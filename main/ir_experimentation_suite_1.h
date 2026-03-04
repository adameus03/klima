/**
 * Experiment with IR transmission (simulating a Sony TV power on signal) 
 */

#ifndef IR_EXPERIMENTATION_SUITE_1_H
#define IR_EXPERIMENTATION_SUITE_1_H

#include "driver/rmt_tx.h"

typedef struct ies1_resources {
  rmt_channel_handle_t tx_chan;
  rmt_encoder_handle_t encoder;
} ies1_resources_t;

int ies1_run(ies1_resources_t* pResources);
void ies1_dispose(ies1_resources_t* pResources);

#endif //IR_EXPERIMENTATION_SUITE_1_H