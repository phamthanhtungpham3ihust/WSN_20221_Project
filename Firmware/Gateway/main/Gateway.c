#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "uart_t.h"
#include "app_config.h"
#include "lwip/sockets.h"
#include "json_generator.h"
#include "esp_http_client.h"
#include "driver/gpio.h"
#include "app_mqtt.h"

bool isChange= false;
float sample_time = 6000;
float new_sample_time =0;
static const char *TAG = "Gateway: ";
float time_delay1;
float time_delay2;
#define WEB_SERVER "industrial.api.ubidots.com"
#define URL_POST "http://industrial.api.ubidots.com/api/v1.6/devices/ESP32_NODE?token=BBFF-AB1jEA8iDwvOAZBeEy2wy8xYbV0k7T"

#define EX_UART_NUM UART_NUM_2
#define PATTERN_CHR_NUM    (3)      

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
char REQUEST[352];
json_gen_test_result_t result;
static const int NUM_NODES =2;


char data_uart[3];
char data_convert[5]="....";
int flag=0;
uint8_t node1[]={0x00, 0x01, 0x17,'N','O','1'};
uint8_t node2[]={0x00, 0x02, 0x17,'N','O','2'};
int check=0;
#define GPIO_INPUT_IO_0     4
#define ESP_INTR_FLAG_DEFAULT 0
TaskHandle_t ISR = NULL;
wifi_config_t wifi_config;

// Callback function when receiving response of Clound
esp_err_t client_event_post_handler_node1(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}

// Function that post temperature value of Node sensor 2 to Clound
static void post_rest_function_node1(void)
{
    esp_http_client_config_t config_post = {
        .url = URL_POST,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler_node1};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);
    json_gen_string_temp(&result, "temperature_1", data_convert, REQUEST);
    esp_http_client_set_post_field(client, REQUEST, strlen(REQUEST));
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

// Callback function when receiving response of Clound
esp_err_t client_event_post_handler_node2(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}

// Function that post temperature value of Node sensor 2 to Clound
static void post_rest_function_node2(void)
{
    esp_http_client_config_t config_post = {
        .url = URL_POST,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler_node2};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);
    json_gen_string_temp(&result, "temperature_2", data_convert, REQUEST);
    esp_http_client_set_post_field(client, REQUEST, strlen(REQUEST));
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

// Callback function when receiving response of Clound
esp_err_t client_event_post_handler_canhbao1(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        //printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}

// Function that post status of Node sensor 1 to Clound
static void post_rest_function_canhbao1(float trangthai)
{
    esp_http_client_config_t config_post = {
        .url = URL_POST,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler_canhbao1};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);
    json_gen_float_button(&result, "node_1", trangthai , REQUEST);
    esp_http_client_set_post_field(client, REQUEST, strlen(REQUEST));
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

// Callback function when receiving response of Clound
esp_err_t client_event_post_handler_canhbao2(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        break;

    default:
        break;
    }
    return ESP_OK;
}

// Function that post status of Node sensor 2 to Clound
static void post_rest_function_canhbao2(int trangthai)
{
    esp_http_client_config_t config_post = {
        .url = URL_POST,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler_canhbao2};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);
    json_gen_float_button(&result, "node_2", trangthai , REQUEST);
    esp_http_client_set_post_field(client, REQUEST, strlen(REQUEST));
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IRAM_ATTR button_isr_handler(void* arg) {
     xTaskResumeFromISR(ISR);
}
// task that will react to button clicks
void button_task(void *arg)
{
     while(1){  
     vTaskSuspend(NULL);
     wifi_config.sta.ssid[0] = 0x00;
     ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
     ESP_ERROR_CHECK(nvs_flash_init());
     ESP_ERROR_CHECK(esp_netif_init());
     ESP_ERROR_CHECK(esp_event_loop_create_default());
     app_config();
     
    }
}

// Function that handle the messages from Node sensor
void UART_PROCESS(char data[3], uint16_t length){
     strcpy(data_uart,data);
     data_convert[0]=data_uart[0];
     data_convert[1]=data_uart[1];
     data_convert[2]='.';
     data_convert[3]=data_uart[2];
     printf("%s\n", data_convert);
     check=1;
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    gpio_pad_select_gpio(GPIO_INPUT_IO_0);
    // set the correct direction
    gpio_set_direction(GPIO_INPUT_IO_0, GPIO_MODE_INPUT);
    // enable interrupt on falling (1->0) edge for button pin
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_NEGEDGE);
    // install ISR service with default configuration
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // attach the interrupt service routine
    gpio_isr_handler_add(GPIO_INPUT_IO_0, button_isr_handler, NULL);
    xTaskCreate( button_task, "button_task", 4096, NULL ,31,&ISR );

    gpio_pad_select_gpio(14); // Dùng cho chân M1
    gpio_set_direction(14, GPIO_MODE_OUTPUT); //SET MODE
    gpio_pad_select_gpio(26); // Dùng cho chân M2
    gpio_set_direction(26, GPIO_MODE_OUTPUT); //SET MODE
    gpio_set_level(14, 0);
    gpio_set_level(26, 0);

    app_config();
    app_mqtt_init();
    app_mqtt_start();
    uart_set_callback(UART_PROCESS);
    uart_init_cmd();
   while(1){
        // Send request to Node sensor 1
        uart_put((const char*)node1, sizeof(node1));
        // Wait for time-out
        vTaskDelay((time_delay1/2)/portTICK_PERIOD_MS);
        // Not receiving response form Node sensor 1
        if(check==0)
        {
        uart_put((const char*)node1, sizeof(node1));
        }
        // Receiving response form Node sensor 1
        else if(check==1)
        {
            ESP_LOGI(TAG, "NODE 1 CONNECTED: %soC\n", data_convert);
            int time_realeas = (xTaskGetTickCount()*portTICK_PERIOD_MS)/1000;
            // Print the time that receiving response of Node sensor 1
            printf("time node 1: %d\n", time_realeas);
            // Post status and data of Node sensor 1 to Clound
            post_rest_function_canhbao1(1.0);
            post_rest_function_node1();
            // Delay
            vTaskDelay(time_delay2/portTICK_PERIOD_MS);
            check=2;
        }
        vTaskDelay((time_delay2/2)/portTICK_PERIOD_MS);
        // Rceiving response form Node sensor 1 after the second request
        if(check==1)
        {
            ESP_LOGI(TAG, "NODE 1 CONNECTED: %soC\n", data_convert);
            int time_realeas = (xTaskGetTickCount()*portTICK_PERIOD_MS)/1000;
            // Print the time that receiving response of Node sensor 1
            printf("time node 1: %d\n", time_realeas);
            // Post status and data of Node sensor 1 to Clound
            post_rest_function_canhbao1(1.0);
            post_rest_function_node1();
        }
        // Not Rceiving response form Node sensor 1 after the second request
        else if(check==0)
        {
            ESP_LOGI(TAG, "NODE 1 DISCONNECTED\n");
            // Post disconnected status of Node sensor 1 to Clound
            post_rest_function_canhbao1(0.0);
        }
        check=0;

        // Send request to Node sensor 2
        uart_put((const char*)node2, sizeof(node2));
        // Wait for time-out
        vTaskDelay((time_delay1/2)/portTICK_PERIOD_MS);
        // Not receiving response form Node sensor 2
        if(check==0)
        {
        uart_put((const char*)node2, sizeof(node2));
        }
        // Receiving response form Node sensor 2
        else if(check==1){
            ESP_LOGI(TAG, "NODE 2 CONNECTED: %soC\n", data_convert);
            int time_realeas = (xTaskGetTickCount()*portTICK_PERIOD_MS)/1000;
            // Print the time that receiving response of Node sensor 2
            printf("time node 2: %d\n", time_realeas);
            // Post status and data of Node sensor 2 to Clound
            post_rest_function_canhbao2(1.0);
            post_rest_function_node2();
            // Delay
            vTaskDelay(time_delay2/portTICK_PERIOD_MS);
            check=2;
        }
        vTaskDelay((time_delay2/2)/portTICK_PERIOD_MS);
        // Rceiving response form Node sensor 2 after the second request
        if(check==1)
        {
            ESP_LOGI(TAG, "NODE 2 CONNECTED: %soC\n", data_convert);
            int time_realeas = (xTaskGetTickCount()*portTICK_PERIOD_MS)/1000;
            // Print the time that receiving response of Node sensor 2
            printf("time node 2: %d\n", time_realeas);
            // Post status and data of Node sensor 2 to Clound
            post_rest_function_canhbao2(1.0);
            post_rest_function_node2();
        }
        // Not Rceiving response form Node sensor 2 after the second request
        else if(check==0)
        {
           ESP_LOGI(TAG, "NODE 2 DISCONNECTED\n");
           post_rest_function_canhbao2(0.0);
        }
        check=0;
        // Check if having the change of sample time
        if(isChange){
            // change the sample time
            sample_time = new_sample_time;
            printf("sample_time: %f\n", sample_time);
            time_delay1 = new_sample_time*(2.0/3)*(1.0/NUM_NODES);
            time_delay2 = new_sample_time*(1.0/3)*(1.0/NUM_NODES);
            isChange=false;
        }
     }
}
