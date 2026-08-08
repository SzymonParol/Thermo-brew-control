#ifndef ESP8266_H
#define ESP8266_H

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* Konfiguracja biblioteki */
#define ESP8266_RX_BUFFER_SIZE 1024
#define ESP8266_TX_BUFFER_SIZE 512
#define ESP8266_MAX_IP_LENGTH 16
#define ESP8266_MAX_SSID_LENGTH 32
#define ESP8266_MAX_PASSWORD_LENGTH 64

/* Typy danych */
typedef enum {
    ESP8266_OK = 0,
    ESP8266_ERROR = 1,
    ESP8266_TIMEOUT = 2,
    ESP8266_BUSY = 3
} ESP8266_Result_t;

typedef enum {
    ESP8266_MODE_STA = 1,
    ESP8266_MODE_AP = 2,
    ESP8266_MODE_STA_AP = 3
} ESP8266_Mode_t;

typedef struct {
    UART_HandleTypeDef *huart_esp;      // UART do ESP8266
    UART_HandleTypeDef *huart_debug;    // UART do debugowania
    uint8_t rx_buffer[ESP8266_RX_BUFFER_SIZE];
    uint8_t tx_buffer[ESP8266_TX_BUFFER_SIZE];
    uint8_t response_buffer[ESP8266_RX_BUFFER_SIZE];
    uint8_t debug_buffer[ESP8266_RX_BUFFER_SIZE + 100];
    volatile uint16_t old_pos;
    char device_ip[ESP8266_MAX_IP_LENGTH];
    uint8_t ip_found;
    uint8_t initialized;
} ESP8266_Handle_t;

/* Funkcje publiczne */
ESP8266_Result_t ESP8266_Init(ESP8266_Handle_t *hesp, UART_HandleTypeDef *huart_esp, UART_HandleTypeDef *huart_debug);
ESP8266_Result_t ESP8266_SetMode(ESP8266_Handle_t *hesp, ESP8266_Mode_t mode);
ESP8266_Result_t ESP8266_ConnectToWiFi(ESP8266_Handle_t *hesp, const char *ssid, const char *password);
ESP8266_Result_t ESP8266_GetIP(ESP8266_Handle_t *hesp, char *ip_buffer);
ESP8266_Result_t ESP8266_StartServer(ESP8266_Handle_t *hesp, uint16_t port);
ESP8266_Result_t ESP8266_SendCommand(ESP8266_Handle_t *hesp, const char *cmd);
ESP8266_Result_t ESP8266_SendData(ESP8266_Handle_t *hesp, const char *data);
ESP8266_Result_t ESP8266_ScanNetworks(ESP8266_Handle_t *hesp);

/* Funkcje callback - do implementacji przez użytkownika */
void ESP8266_DataReceived_Callback(ESP8266_Handle_t *hesp, const char *data);
void ESP8266_Debug_Callback(ESP8266_Handle_t *hesp, const char *prefix, const char *message);

/* Funkcje obsługi przerwań - do wywołania w interrupt handlers */
void ESP8266_UART_IRQHandler(ESP8266_Handle_t *hesp);
void ESP8266_RxEventCallback(ESP8266_Handle_t *hesp, uint16_t size);

/* Funkcje pomocnicze */
void ESP8266_ProcessData(ESP8266_Handle_t *hesp, uint16_t size);
void ESP8266_SendDebugMessage(ESP8266_Handle_t *hesp, const char *prefix, const char *message);

#endif /* ESP8266_H */
