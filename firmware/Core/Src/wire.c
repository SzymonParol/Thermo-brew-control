#include "wire.h"
#include "gpio.h"
#include "tim.h"

/* Helper: delay in microseconds using TIM1 */
static void delay_us(uint32_t us)
{
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    while (__HAL_TIM_GET_COUNTER(&htim1) < us) {}
}

/* Helper: set DS pin as output (push-pull) */
static void ds_set_output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DS_GPIO_Port, &GPIO_InitStruct);
}

/* Helper: set DS pin as input (pull-up) */
static void ds_set_input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DS_GPIO_Port, &GPIO_InitStruct);
}

HAL_StatusTypeDef wire_init(void)
{
    /* Start TIM1 for microsecond delays */
    return HAL_TIM_Base_Start(&htim1);
}

HAL_StatusTypeDef wire_reset(void)
{
    uint8_t response = 0;

    ds_set_output();
    HAL_GPIO_WritePin(DS_GPIO_Port, DS_Pin, GPIO_PIN_RESET);
    delay_us(480);

    ds_set_input();
    delay_us(80);

    if (HAL_GPIO_ReadPin(DS_GPIO_Port, DS_Pin) == GPIO_PIN_RESET)
        response = 1;  /* presence detected */
    else
        response = 0;

    delay_us(400);

    return (response == 1) ? HAL_OK : HAL_ERROR;
}

static uint8_t read_bit(void)
{
    uint8_t bit = 0;

    ds_set_output();
    HAL_GPIO_WritePin(DS_GPIO_Port, DS_Pin, GPIO_PIN_RESET);
    delay_us(5);

    ds_set_input();
    delay_us(15);

    if (HAL_GPIO_ReadPin(DS_GPIO_Port, DS_Pin) != GPIO_PIN_RESET)
        bit = 1;

    delay_us(40);
    return bit;
}

uint8_t wire_read(void)
{
    uint8_t value = 0;
    for (int i = 0; i < 8; i++)
    {
        if (read_bit())
            value |= (1 << i);
    }
    return value;
}

static void write_bit(uint8_t bit)
{
    ds_set_output();

    if (bit)
    {
        /* Write 1 */
        HAL_GPIO_WritePin(DS_GPIO_Port, DS_Pin, GPIO_PIN_RESET);
        delay_us(5);
        ds_set_input();
        delay_us(60);
    }
    else
    {
        /* Write 0 */
        HAL_GPIO_WritePin(DS_GPIO_Port, DS_Pin, GPIO_PIN_RESET);
        delay_us(60);
        ds_set_input();
    }
}

void wire_write(uint8_t byte)
{
    for (int i = 0; i < 8; i++)
        write_bit((byte >> i) & 1);
}

/* CRC-8 (Dallas/Maxim) – taka sama jak u Ciebie */
static uint8_t byte_crc(uint8_t crc, uint8_t byte)
{
    for (int i = 0; i < 8; i++)
    {
        uint8_t b = crc ^ byte;
        crc >>= 1;
        if (b & 0x01)
            crc ^= 0x8C;
        byte >>= 1;
    }
    return crc;
}

uint8_t wire_crc(const uint8_t* data, int len)
{
    uint8_t crc = 0;
    for (int i = 0; i < len; i++)
        crc = byte_crc(crc, data[i]);
    return crc;
}
