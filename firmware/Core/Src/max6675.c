#include "max6675.h"
#include "spi.h"
#include "gpio.h"


static void MAX6675_CS_Low(void)
{
    HAL_GPIO_WritePin(MAX6675_CS_GPIO_Port, MAX6675_CS_Pin, GPIO_PIN_RESET);
}

static void MAX6675_CS_High(void)
{
    HAL_GPIO_WritePin(MAX6675_CS_GPIO_Port, MAX6675_CS_Pin, GPIO_PIN_SET);
}

void MAX6675_Init(void)
{
    MAX6675_CS_High();
}

static uint16_t MAX6675_ReadRaw(void)
{
    uint8_t rx[2] = {0, 0};

    MAX6675_CS_Low();
    HAL_Delay(1);
    HAL_SPI_Receive(&hspi2, rx, 2, 100);
    MAX6675_CS_High();

    return ((uint16_t)rx[0] << 8) | rx[1];
}

uint8_t MAX6675_IsOpenCircuit(void)
{
    uint16_t raw = MAX6675_ReadRaw();
    return (raw & 0x0004) ? 1 : 0;
}

float MAX6675_ReadCelsius(void)
{
    uint16_t raw = MAX6675_ReadRaw();

    if (raw & 0x0004)
        return -127.0f;

    raw >>= 3;
    return raw * 0.25f;
}
