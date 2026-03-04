/**
 * In this experimentation suite, we receive signal from a Sony TV remote and log raw RMT symbols 
 */

#ifndef IR_EXPERIMENTATION_SUITE_0_H
#define IR_EXPERIMENTATION_SUITE_0_H

#include "driver/rmt_rx.h"

typedef struct ies0_resources {
  rmt_channel_handle_t rx_chan;
} ies0_resources_t;

int ies0_run(ies0_resources_t* pResources);
void ies0_dispose(ies0_resources_t* pResources);

#endif //IR_EXPERIMENTATION_SUITE_0_H