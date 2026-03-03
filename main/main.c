#include "esp_log.h"
#include "ir_experimentation_suite_0.h"

static const char *TAG = "main";

void app_main(void)
{
  ESP_LOGI(TAG, "Started main");
  ESP_LOGI(TAG, "Starting IR experimentation suite 0");

  ies_resources_t resources_ies0 = {0};
  int rv = ies0_run(&resources_ies0);
  ies0_dispose(&resources_ies0);
  return;
}
