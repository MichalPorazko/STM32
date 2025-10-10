#pragma once

#define ESP_UART       huart3   // UART connected to ESP8266

// Enable/Disable logs
#define ENABLE_USER_LOG   1
#define ENABLE_DEBUG_LOG  0
/* -------------------------------------- */

extern UART_HandleTypeDef ESP_UART;

/* ------------ LOG MACROS ------------- */
#if ENABLE_USER_LOG
  #define USER_LOG(fmt, ...) printf("[USER] " fmt "\r\n", ##__VA_ARGS__)
#else
  #define USER_LOG(fmt, ...)
#endif

#if ENABLE_DEBUG_LOG
  #define DEBUG_LOG(fmt, ...) printf("[DEBUG] " fmt "\r\n", ##__VA_ARGS__)
#else
  #define DEBUG_LOG(fmt, ...)
#endif
/* ------------------------------------- */

/* ------------ ENUMS & STATES --------- */
typedef enum {
    ESP8266_OK = 0,
    ESP8266_ERROR,
    ESP8266_TIMEOUT,
    ESP8266_NO_RESPONSE,
} ESP8266_Status;

typedef enum {
    ESP8266_DISCONNECTED = 0,
    ESP8266_CONNECTED_NO_IP,
    ESP8266_CONNECTED_IP
} ESP8266_ConnectionState;

extern ESP8266_ConnectionState ESP_ConnState;

/* ------------------------------------- */

/* ------------ API FUNCTIONS ---------- */
ESP8266_Status ESP_Init(void);
ESP8266_Status ESP_ConnectWiFi(const char *ssid, const char *password, char *ip_buffer, uint16_t buffer_len);
ESP8266_ConnectionState ESP_GetConnectionState(void);
ESP8266_Status ESP_CheckTCPConnection(void);

ESP8266_Status ESP_SendToThingSpeak(const char *apiKey, float val1, float val2, float val3);

ESP8266_Status ESP_MQTT_Connect(const char *broker, uint16_t port, const char *clientID, const char *username, const char *password, uint16_t keepalive);
ESP8266_Status ESP_MQTT_Publish(const char *topic, const char *message, uint8_t qos);
ESP8266_Status ESP_MQTT_Ping(void);
/* ------------------------------------- */
