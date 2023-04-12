#include "app_mqtt.h"
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "mqtt_client.h"

static esp_mqtt_client_handle_t client;
static const char *TAG = "app_mqtt";
static mqtt_data_handle_t mqtt_data_handle = NULL;
static mqtt_publish_handle_t mqtt_publish_handle = NULL;
extern bool isChange;
extern float new_sample_time;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            esp_mqtt_client_subscribe(client, TOPIC, 1);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            //vTaskDelay(500/portTICK_PERIOD_MS);
            int num = atoi(event->data);
            printf("num: %d\n", num);
            //printf("data: %s\n",event->data);
            isChange = true;
            new_sample_time = num*1000;
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

void app_mqtt_init(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_BROKER,
        .username = "BBFF-AB1jEA8iDwvOAZBeEy2wy8xYbV0k7T",
        .password = "BBFF-AB1jEA8iDwvOAZBeEy2wy8xYbV0k7T",
        .event_handle = mqtt_event_handler,
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    client = esp_mqtt_client_init(&mqtt_cfg);
}

void app_mqtt_start(void)
{
    esp_mqtt_client_start(client);
}

void app_mqtt_stop(void)
{
    esp_mqtt_client_stop(client);
}

void app_mqtt_publish(char *topic, char *data, uint16_t len)
{
    esp_mqtt_client_publish(client,topic, data, len, 1, 0);
}

void app_mqtt_subscribe(char * topic)
{
    esp_mqtt_client_subscribe(client, topic, 1);
}

void app_mqtt_set_cb_event(void *cb)
{
    if(cb){
        mqtt_data_handle = cb;
    }
}

void app_mqtt_set_cb_publish(void *cb)
{
    if(cb){
        mqtt_publish_handle = cb;
    }
}