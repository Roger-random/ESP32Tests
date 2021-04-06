#include "station_start.h"

void station_start_task(void* pvParameter)
{
  while(true)
  {
    printf("Station start task running.\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
