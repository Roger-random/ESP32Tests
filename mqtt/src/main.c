#include "driver/adc.h"
#include "soc/adc_channel.h"

#include "esp_log.h"
#include "esp_system.h"

#include "mqtt_client.h"

#include "station_start.h"

static const char *TAG = "MQTT test";

static void adc_setup()
{
  ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO32_CHANNEL, ADC_ATTEN_DB_11));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO33_CHANNEL, ADC_ATTEN_DB_11));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO34_CHANNEL, ADC_ATTEN_DB_11));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO35_CHANNEL, ADC_ATTEN_DB_11));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO36_CHANNEL, ADC_ATTEN_DB_11));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO39_CHANNEL, ADC_ATTEN_DB_11));
}

static int adc_publish(esp_mqtt_client_handle_t client)
{
  char payload[1024];
  int messageId;

  memset(payload, 0, 1024);
  snprintf(payload, 1024,
    "{\"GPIO32\":%d, \"GPIO33\":%d, \"GPIO34\":%d, \"GPIO35\":%d, \"GPIO36\":%d, \"GPIO39\":%d}",
    adc1_get_raw(ADC1_GPIO32_CHANNEL),
    adc1_get_raw(ADC1_GPIO33_CHANNEL),
    adc1_get_raw(ADC1_GPIO34_CHANNEL),
    adc1_get_raw(ADC1_GPIO35_CHANNEL),
    adc1_get_raw(ADC1_GPIO36_CHANNEL),
    adc1_get_raw(ADC1_GPIO39_CHANNEL));
  messageId = esp_mqtt_client_publish(client, MQTT_PUB, payload, 0, 0, false);
  ESP_LOGI(TAG, "adc_publish %d", messageId);

  return messageId;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
      adc_publish(client);
      msg_id = esp_mqtt_client_subscribe(client, MQTT_SUB, 0);
      ESP_LOGI(TAG, "Subscribing %d", msg_id);
      break;
    case MQTT_EVENT_DATA:
      ESP_LOGI(TAG, "MQTT_EVENT_DATA");
      printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
      printf("DATA=%.*s\r\n", event->data_len, event->data);
      break;
    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_ERROR:
      ESP_LOGI(TAG, "MQTT_EVENT_ERROR type %d", event->error_handle->error_type);
      ESP_LOGI(TAG, "MQTT_EVENT_ERROR esp_tls_last_esp_err %d", event->error_handle->esp_tls_last_esp_err);
      ESP_LOGI(TAG, "MQTT_EVENT_ERROR esp_tls_stack_err %d", event->error_handle->esp_tls_stack_err);
      ESP_LOGI(TAG, "MQTT_EVENT_ERROR esp_mqtt_connect_return_code_t %d", event->error_handle->connect_return_code);
      break;
    default:
      ESP_LOGI(TAG, "Other event id:%d", event->event_id);
      break;
  }
}

void app_main()
{
  adc_setup();

  ESP_LOGI(TAG, "Startup");
  station_start();
  ESP_LOGI(TAG, "Station started");

  esp_mqtt_client_config_t mqtt_cfg = {
    .uri = MQTT_URI,
  };

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
  ESP_ERROR_CHECK(esp_mqtt_client_start(client));

  while(true)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

}
