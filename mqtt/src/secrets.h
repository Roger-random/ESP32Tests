/*
 * Header file for all the stuff that shouldn't be publicly visible
 * on GitHub. After pulling this file down, run the following
 * command so further updates are ignored:
 *
 * git update-index --skip-worktree secrets.h
 *
 */
#ifndef INC_SECRETS_H
#define INC_SECRETS_H

#define ESP_WIFI_SSID "your ssid here"
#define ESP_WIFI_PASS "your password here"

#define MQTT_URI      "mqtt broker uri"
#define MQTT_USERNAME "mqtt broker username"
#define MQTT_PASSWORD "mqtt broker password"

#endif // INC_SECRETS_H