#include "driver/adc.h"
#include "soc/adc_channel.h"

#include "esp_log.h"
#include "esp_system.h"

#include "mqtt_client.h"

#include "cJSON.h"

#include "station_start.h"

static const char *TAG = "MQTT test";

static void analog_input_setup()
{
  ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO32_CHANNEL, ADC_ATTEN_DB_11));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO33_CHANNEL, ADC_ATTEN_DB_11));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO34_CHANNEL, ADC_ATTEN_DB_11));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO35_CHANNEL, ADC_ATTEN_DB_11));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO36_CHANNEL, ADC_ATTEN_DB_11));
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO39_CHANNEL, ADC_ATTEN_DB_11));
}

static void digital_output_setup()
{
  gpio_config_t io_conf = {
      .mode = GPIO_MODE_OUTPUT,
      .intr_type = GPIO_INTR_DISABLE,
      .pull_down_en = 0,
      .pull_up_en = 0,
      .pin_bit_mask = (1ULL<<GPIO_NUM_2) |
                      (1ull<<GPIO_NUM_25) |
                      (1ull<<GPIO_NUM_26) |
                      (1ull<<GPIO_NUM_27),
  };
  ESP_ERROR_CHECK(gpio_config(&io_conf));
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

static void update_digital_out(cJSON *json_root, char* label, gpio_num_t pin)
{
  cJSON *pinUpdate = cJSON_GetObjectItem(json_root, label);

  if (NULL == pinUpdate)
  {
    ESP_LOGI(TAG, "No pin specification for %s", label);
  }
  else if (cJSON_IsTrue(pinUpdate))
  {
    ESP_ERROR_CHECK(gpio_set_level(pin, true));
  }
  else if (cJSON_IsFalse(pinUpdate))
  {
    ESP_ERROR_CHECK(gpio_set_level(pin, false));
  }
  else
  {
    ESP_LOGI(TAG, "Expected true or false to update pin %s", label);
  }
}

static void parse_mqtt_event(esp_mqtt_event_handle_t event)
{
  if(strlen(MQTT_SUB) == event->topic_len)
  {
    if(0 == strncmp(MQTT_SUB, event->topic, strlen(MQTT_SUB)))
    {
      ESP_LOGI(TAG, "Processing data for subscribed event");

      // cJSON failure mode is to return NULL. If NULL is passed in as first
      // parameter, NULL will be returned. So we don't really need to check
      // intermediate values for NULL, checking leaf objects are sufficient.
      cJSON *json_root = cJSON_Parse(event->data);
      update_digital_out(json_root, "gpio2", GPIO_NUM_2);
      update_digital_out(json_root, "gpio25", GPIO_NUM_25);
      update_digital_out(json_root, "gpio26", GPIO_NUM_26);
      update_digital_out(json_root, "gpio27", GPIO_NUM_27);
      cJSON_Delete(json_root);
    }
    else
    {
      ESP_LOGI(TAG, "MQTT event topic does not match. (want %s got %s) ignoring", MQTT_SUB, event->topic);
    }
  }
  else
  {
    ESP_LOGI(TAG, "MQTT event topic isn't even the right length (want %d got %d) ignoring.", strlen(MQTT_SUB), event->topic_len);
  }
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
      parse_mqtt_event(event);
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
  analog_input_setup();
  digital_output_setup();

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
