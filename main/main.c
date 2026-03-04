#include "esp_log.h"
#include "ir_experimentation_suite_0.h"
#include "ir_experimentation_suite_1.h"
#include "ir_experimentation_suite_2.h"

static const char *TAG = "main";

void app_main(void)
{
  ESP_LOGI(TAG, "Started main");
  
  // ESP_LOGI(TAG, "Starting IR experimentation suite 0");
  // ies0_resources_t resources_ies0 = {0};
  // int rv = ies0_run(&resources_ies0);
  // ies0_dispose(&resources_ies0);

  // ESP_LOGI(TAG, "Starting IR experimentation suite 1");
  // ies1_resources_t resources_ies1 = {0};
  // int rv = ies1_run(&resources_ies1);
  // ies1_dispose(&resources_ies1);

  ESP_LOGI(TAG, "Starting IR experimentation suite 2");
  ies2_resources_t resources_ies2 = {0};
  int rv = ies2_run(&resources_ies2);
  ies2_dispose(&resources_ies2);

  return;
}
