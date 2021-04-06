#include "http_file_server.h"

/* Waiting for certain WiFi events before continuing */
static EventGroupHandle_t s_wifi_event_group;

static void wifi_connected_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

void wait_for_wifi_ready()
{
  // Used by WiFi event handler to signal we can continue
  s_wifi_event_group = xEventGroupCreate();

  // Register for WiFi event
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
    IP_EVENT_STA_GOT_IP, &wifi_connected_handler, NULL, &instance_got_ip));

  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
  assert(bits == WIFI_CONNECTED_BIT); // We only have one bit today.

  // No longer need to listen to WiFi event
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
  vEventGroupDelete(s_wifi_event_group);
}

void http_file_server_task(void* pvParameters)
{
  wait_for_wifi_ready();

  // Test loop
  wifi_ap_record_t ap_record;
  while(true) {
    ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&ap_record));
    printf("RSSI %d\n", ap_record.rssi);
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}
