#include "driver/rmt_rx.h"

typedef struct ies_resources {
  rmt_channel_handle_t rx_chan;
} ies_resources_t;

int ies0_run(ies_resources_t* pResources);
void ies0_dispose(ies_resources_t* pResources);