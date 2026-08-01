/*
 * esp8266.c
 *
 *  Created on: Jun 20, 2025
 *      Author: Szym
 */
#include "esp8266.h"

/* Funkcje prywatne */
static void ESP8266_ExtractIP(ESP8266_Handle_t *hesp, const char *response);
static ESP8266_Result_t ESP8266_WaitForResponse(ESP8266_Handle_t *hesp, const char *expected, uint32_t timeout);

/* Implementacja funkcji publicznych */
ESP8266_Result_t ESP8266_Init(ESP8266_Handle_t *hesp, UART_HandleTypeDef *huart_esp, UART_HandleTypeDef *huart_debug) {
    if (!hesp || !huart_esp) {
        return ESP8266_ERROR;
    }

    // Inicjalizacja struktury
    hesp->huart_esp = huart_esp;
    hesp->huart_debug = huart_debug;
    hesp->old_pos = 0;
    hesp->ip_found = 0;
    hesp->initialized = 0;

    memset(hesp->rx_buffer, 0, ESP8266_RX_BUFFER_SIZE);
    memset(hesp->tx_buffer, 0, ESP8266_TX_BUFFER_SIZE);
    memset(hesp->response_buffer, 0, ESP8266_RX_BUFFER_SIZE);
    memset(hesp->device_ip, 0, ESP8266_MAX_IP_LENGTH);

    // Uruchomienie DMA
    if (HAL_UART_Receive_DMA(hesp->huart_esp, hesp->rx_buffer, ESP8266_RX_BUFFER_SIZE) != HAL_OK) {
        return ESP8266_ERROR;
    }

    // Włączenie przerwania IDLE
    __HAL_UART_ENABLE_IT(hesp->huart_esp, UART_IT_IDLE);

    ESP8266_SendDebugMessage(hesp, "ESP8266", "Biblioteka zainicjalizowana");

    // Test komunikacji
    ESP8266_SendCommand(hesp, "AT");
    HAL_Delay(1000);

    hesp->initialized = 1;
    return ESP8266_OK;
}

ESP8266_Result_t ESP8266_SetMode(ESP8266_Handle_t *hesp, ESP8266_Mode_t mode) {
    if (!hesp || !hesp->initialized) {
        return ESP8266_ERROR;
    }

    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CWMODE=%d", mode);

    ESP8266_SendCommand(hesp, cmd);
    HAL_Delay(2000);

    return ESP8266_OK;
}

ESP8266_Result_t ESP8266_ConnectToWiFi(ESP8266_Handle_t *hesp, const char *ssid, const char *password) {
    if (!hesp || !hesp->initialized || !ssid || !password) {
        return ESP8266_ERROR;
    }

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);

    ESP8266_SendDebugMessage(hesp, "WIFI", "Łączenie z siecią...");
    ESP8266_SendCommand(hesp, cmd);
    HAL_Delay(8000);

    return ESP8266_OK;
}

ESP8266_Result_t ESP8266_GetIP(ESP8266_Handle_t *hesp, char *ip_buffer) {
    if (!hesp || !hesp->initialized || !ip_buffer) {
        return ESP8266_ERROR;
    }

    ESP8266_SendCommand(hesp, "AT+CIFSR");
    HAL_Delay(2000);

    if (hesp->ip_found) {
        strcpy(ip_buffer, hesp->device_ip);
        return ESP8266_OK;
    }

    return ESP8266_ERROR;
}

ESP8266_Result_t ESP8266_StartServer(ESP8266_Handle_t *hesp, uint16_t port) {
    if (!hesp || !hesp->initialized) {
        return ESP8266_ERROR;
    }

    // Włącz tryb wielokanalowy
    ESP8266_SendCommand(hesp, "AT+CIPMUX=1");
    HAL_Delay(1000);

    // Uruchom serwer
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CIPSERVER=1,%d", port);
    ESP8266_SendCommand(hesp, cmd);
    HAL_Delay(1000);

    ESP8266_SendDebugMessage(hesp, "SERVER", "Serwer uruchomiony");

    return ESP8266_OK;
}

ESP8266_Result_t ESP8266_SendCommand(ESP8266_Handle_t *hesp, const char *cmd) {
    if (!hesp || !cmd) {
        return ESP8266_ERROR;
    }

    // Wyczyść bufor odpowiedzi
    memset(hesp->response_buffer, 0, ESP8266_RX_BUFFER_SIZE);

    // Przygotuj komendę
    uint16_t cmd_len = strlen(cmd);
    snprintf((char*)hesp->tx_buffer, ESP8266_TX_BUFFER_SIZE, "%s\r\n", cmd);

    // Debug: pokaż wysyłaną komendę
    ESP8266_SendDebugMessage(hesp, "STM>>", cmd);

    // Wyślij przez DMA
    if (HAL_UART_Transmit_DMA(hesp->huart_esp, hesp->tx_buffer, cmd_len + 2) != HAL_OK) {
        return ESP8266_ERROR;
    }

    return ESP8266_OK;
}

ESP8266_Result_t ESP8266_SendData(ESP8266_Handle_t *hesp, const char *data) {
    if (!hesp || !data) {
        return ESP8266_ERROR;
    }

    uint16_t data_len = strlen(data);
    if (HAL_UART_Transmit_DMA(hesp->huart_esp, (uint8_t*)data, data_len) != HAL_OK) {
        return ESP8266_ERROR;
    }

    return ESP8266_OK;
}

void ESP8266_SendDebugMessage(ESP8266_Handle_t *hesp, const char *prefix, const char *message) {
    if (!hesp || !prefix || !message) {
        return;
    }

    if (hesp->huart_debug) {
        snprintf((char*)hesp->debug_buffer, sizeof(hesp->debug_buffer), "[%s] %s\r\n", prefix, message);
        HAL_UART_Transmit(hesp->huart_debug, hesp->debug_buffer, strlen((char*)hesp->debug_buffer), 1000);
    }

    // Wywołaj callback jeśli zdefiniowany
    ESP8266_Debug_Callback(hesp, prefix, message);
}

void ESP8266_ProcessData(ESP8266_Handle_t *hesp, uint16_t size) {
    if (!hesp || size == 0) {
        return;
    }

    // Skopiuj dane z bufora DMA
    memcpy(hesp->response_buffer, hesp->rx_buffer + hesp->old_pos, size);
    hesp->response_buffer[size] = '\0';

    // Sanityzacja danych
    char *src = (char*)hesp->response_buffer, *dst = (char*)hesp->response_buffer;
    while (*src) {
        if ((*src >= 32 && *src <= 126) || *src == '\r' || *src == '\n') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';

    // Wyświetl odpowiedź ESP w konsoli
    ESP8266_SendDebugMessage(hesp, "ESP>>", (char*)hesp->response_buffer);

    // Przetwórz odpowiedź
    if (strstr((char*)hesp->response_buffer, "STAIP")) {
        ESP8266_ExtractIP(hesp, (char*)hesp->response_buffer);
    }

    // Wywołaj callback użytkownika
    ESP8266_DataReceived_Callback(hesp, (char*)hesp->response_buffer);
}

void ESP8266_UART_IRQHandler(ESP8266_Handle_t *hesp) {
    if (!hesp) {
        return;
    }

    if (__HAL_UART_GET_FLAG(hesp->huart_esp, UART_FLAG_IDLE)) {
        __HAL_UART_CLEAR_IDLEFLAG(hesp->huart_esp);

        uint16_t current_pos = ESP8266_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(hesp->huart_esp->hdmarx);

        if (current_pos != hesp->old_pos) {
            if (current_pos > hesp->old_pos) {
                ESP8266_ProcessData(hesp, current_pos - hesp->old_pos);
            } else {
                // Obsługa zawijania bufora cyklicznego
                ESP8266_ProcessData(hesp, ESP8266_RX_BUFFER_SIZE - hesp->old_pos);
                if (current_pos > 0) {
                    hesp->old_pos = 0;
                    ESP8266_ProcessData(hesp, current_pos);
                }
            }
            hesp->old_pos = current_pos;
        }
    }
}

void ESP8266_RxEventCallback(ESP8266_Handle_t *hesp, uint16_t size) {
    if (!hesp) {
        return;
    }

    ESP8266_ProcessData(hesp, size);
    hesp->old_pos = size;
}

/* Funkcje prywatne */
static void ESP8266_ExtractIP(ESP8266_Handle_t *hesp, const char *response) {
    char* staip_pos = strstr(response, "STAIP,\"");
    if (staip_pos != NULL) {
        staip_pos += 7;
        char* end_quote = strchr(staip_pos, '"');
        if (end_quote != NULL) {
            int ip_len = end_quote - staip_pos;
            if (ip_len < ESP8266_MAX_IP_LENGTH) {
                strncpy(hesp->device_ip, staip_pos, ip_len);
                hesp->device_ip[ip_len] = '\0';
                hesp->ip_found = 1;

                char ip_msg[100];
                snprintf(ip_msg, sizeof(ip_msg), "ADRES IP URZADZENIA: %s", hesp->device_ip);
                ESP8266_SendDebugMessage(hesp, "IP", ip_msg);
            }
        }
    }
}

/* Słabe definicje callback'ów - użytkownik może je przedefiniować */
__weak void ESP8266_DataReceived_Callback(ESP8266_Handle_t *hesp, const char *data) {
    // Domyślnie nic nie rób
    UNUSED(hesp);
    UNUSED(data);
}

__weak void ESP8266_Debug_Callback(ESP8266_Handle_t *hesp, const char *prefix, const char *message) {
    // Domyślnie nic nie rób
    UNUSED(hesp);
    UNUSED(prefix);
    UNUSED(message);
}




