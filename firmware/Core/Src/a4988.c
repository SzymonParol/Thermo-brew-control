#include "a4988.h"
#include "main.h"

static uint8_t motor_running = 0;
static uint8_t motor_direction = 0;
static uint32_t step_interval_ms = 0;
static uint32_t last_step_tick = 0;

void A4988_Init(void)
{
    HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_RESET);

    motor_running = 0;
    motor_direction = 0;
    step_interval_ms = 0;
    last_step_tick = HAL_GetTick();
}

void A4988_SetDirection(uint8_t direction)
{
    motor_direction = direction ? 1U : 0U;

    HAL_GPIO_WritePin(DIR_GPIO_Port,
                      DIR_Pin,
                      motor_direction ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void A4988_SetSpeed(float speed_percent)
{
    if (speed_percent <= 0.0f)
    {
        A4988_Stop();
        return;
    }

    if (speed_percent > 100.0f)
    {
        speed_percent = 100.0f;
    }


    step_interval_ms = (uint32_t)(101.0f - speed_percent);

    if (step_interval_ms < 1U)
    {
        step_interval_ms = 1U;
    }

    motor_running = 1U;
}

void A4988_Stop(void)
{
    motor_running = 0U;

    HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_RESET);
}

void A4988_Update(void)
{
    uint32_t now;

    if (motor_running == 0U)
    {
        return;
    }

    now = HAL_GetTick();

    if ((now - last_step_tick) >= step_interval_ms)
    {
        HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_SET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_RESET);

        last_step_tick = now;
    }
}
