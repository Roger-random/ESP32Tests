#include "http_file_server.h"

static httpd_handle_t server_handle;

/* Waiting for certain WiFi events before continuing */
static EventGroupHandle_t s_wifi_event_group;

static const char *TAG = "http file server";

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

static esp_err_t hello_get_handler(httpd_req_t *req)
{
  const char* strTest = "HTTP File Server Test poombah\n";
  //ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "cache-control", "max-age=1"));
  ESP_ERROR_CHECK(httpd_resp_set_type(req, "text/html"));
  ESP_ERROR_CHECK(httpd_resp_send(req, strTest, strlen(strTest)));

  return ESP_OK;
}

static const httpd_uri_t hello = {
  .uri      = "/hello",
  .method   = HTTP_GET,
  .handler  = hello_get_handler,
  .user_ctx = NULL
};

void http_file_server_task(void* pvParameters)
{
  wait_for_wifi_ready();

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;

  // Start the httpd server
  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
  if (httpd_start(&server_handle, &config) == ESP_OK) {
      // Set URI handlers
      ESP_LOGI(TAG, "Registering URI handlers");
      httpd_register_uri_handler(server_handle, &hello);
      // TODO: Wait for something to shut down server. Right now we just spin
      while(true) {
        vTaskDelay(portMAX_DELAY);
      }
  }

  ESP_LOGI(TAG, "Error starting server!");
  vTaskDelete(NULL);
}
