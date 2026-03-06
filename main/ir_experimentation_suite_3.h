#ifndef IR_EXPERIMENTATION_SUITE_3_H
#define IR_EXPERIMENTATION_SUITE_3_H

#include "driver/rmt_tx.h"

typedef struct ies3_resources {
  rmt_channel_handle_t tx_chan;
  rmt_encoder_handle_t encoder;
} ies3_resources_t;

int ies3_run(ies3_resources_t* pResources);
void ies3_dispose(ies3_resources_t* pResources);

#endif //IR_EXPERIMENTATION_SUITE_3_H