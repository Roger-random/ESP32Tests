#include "http_file_server.h"

static httpd_handle_t server_handle;

/* Waiting for certain WiFi events before continuing */
static EventGroupHandle_t s_wifi_event_group;

static const char *TAG = "http file server";
static const char *joy_msg_send = "joy_msg_send";
static const char *joy_msg_receive = "joy_msg_receive";

// We can spend effort making code memory-efficient, or we can just blow
// a chunk of RAM to make code trivial.
#define readBufSize 1024*16
static char readBuf[readBufSize];

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

void spiffs_init()
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/static",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = false
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}

static uint32_t read_spiff_buffer(const char *file_name)
{
  uint32_t readSize = 0;
  FILE* f = fopen(file_name, "r");
  if (f == NULL) {
      ESP_LOGE(TAG, "Failed to open file for reading");
      return 0;
  }

  readSize = fread(readBuf, sizeof(char), readBufSize, f);
  fclose(f);

  return readSize;
}

static esp_err_t index_html_get_handler(httpd_req_t *req)
{
  uint32_t readSize = 0;
  ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "cache-control", "max-age=1")); // For development
  ESP_ERROR_CHECK(httpd_resp_set_type(req, "text/html"));
  readSize = read_spiff_buffer("/static/index.html");
  ESP_ERROR_CHECK(httpd_resp_send(req, readBuf, readSize));

  return ESP_OK;
}

static const httpd_uri_t index_html = {
  .uri      = "/index.html",
  .method   = HTTP_GET,
  .handler  = index_html_get_handler,
  .user_ctx = NULL
};

static esp_err_t wsplay_css_get_handler(httpd_req_t *req)
{
  uint32_t readSize = 0;
  ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "cache-control", "max-age=1")); // For development
  ESP_ERROR_CHECK(httpd_resp_set_type(req, "text/css"));
  readSize = read_spiff_buffer("/static/wsplay.css");
  ESP_ERROR_CHECK(httpd_resp_send(req, readBuf, readSize));

  return ESP_OK;
}

static const httpd_uri_t wsplay_css = {
  .uri      = "/wsplay.css",
  .method   = HTTP_GET,
  .handler  = wsplay_css_get_handler,
  .user_ctx = NULL
};

static esp_err_t wsplay_js_get_handler(httpd_req_t *req)
{
  uint32_t readSize = 0;
  ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "cache-control", "max-age=1")); // For development
  ESP_ERROR_CHECK(httpd_resp_set_type(req, "application/javascript"));
  readSize = read_spiff_buffer("/static/wsplay.js");
  ESP_ERROR_CHECK(httpd_resp_send(req, readBuf, readSize));

  return ESP_OK;
}

static const httpd_uri_t wsplay_js = {
  .uri      = "/wsplay.js",
  .method   = HTTP_GET,
  .handler  = wsplay_js_get_handler,
  .user_ctx = NULL
};

static esp_err_t websocket_root_get_handler(httpd_req_t *req)
{
  if (req->method == HTTP_GET) {
    ESP_LOGI(TAG, "Handshake done, the new connection was opened");
    return ESP_OK;
  }
  httpd_ws_frame_t ws_pkt;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;
  ws_pkt.payload = (uint8_t*)readBuf;
  ESP_ERROR_CHECK(httpd_ws_recv_frame(req, &ws_pkt, readBufSize));
  if (ws_pkt.len < readBufSize) {
    readBuf[ws_pkt.len] = 0; // null termination
    if (0 == strcmp(readBuf, joy_msg_send)) {
      ws_pkt.payload = (uint8_t*)joy_msg_receive;
      ws_pkt.len = strlen(joy_msg_receive);
      return httpd_ws_send_frame(req, &ws_pkt);
    }
    cJSON *root = cJSON_Parse(readBuf);
    cJSON *axes = cJSON_GetObjectItem(root,"axes");
    cJSON *axes0 = cJSON_GetArrayItem(axes, 0);
    cJSON *axes1 = cJSON_GetArrayItem(axes, 1);
    ESP_LOGI(TAG, "Steer %+.2f speed %+.2f", axes0->valuedouble, axes1->valuedouble);
    cJSON_Delete(root);
  } else {
    ESP_LOGE(TAG, "Ignoring oversized WebSocket packet");
  }
  return ESP_OK;
}

static const httpd_uri_t websocket_root = {
  .uri      = "/",
  .method   = HTTP_GET,
  .handler  = websocket_root_get_handler,
  .user_ctx = NULL,
  .is_websocket = true
};

void http_file_server_task(void* pvParameters)
{
  spiffs_init();

  wait_for_wifi_ready();

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;

  // Start the httpd server
  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
  if (httpd_start(&server_handle, &config) == ESP_OK) {
      // Set URI handlers
      ESP_LOGI(TAG, "Registering URI handlers");
      httpd_register_uri_handler(server_handle, &index_html);
      httpd_register_uri_handler(server_handle, &wsplay_css);
      httpd_register_uri_handler(server_handle, &wsplay_js);
      httpd_register_uri_handler(server_handle, &websocket_root);
      // TODO: Wait for something to shut down server. Right now we just spin
      while(true) {
        vTaskDelay(portMAX_DELAY);
      }
  }

  ESP_LOGI(TAG, "Error starting server!");
  vTaskDelete(NULL);
}
