#include "inputs.h"
#include "system.h"
#include "tim.h"

uint8_t enc_btn_pressed = 0;
uint8_t esc_btn_pressed = 0;
int16_t enc_delta = 0;

static int16_t enc_last_value = 0;
static uint32_t last_enter_time = 0;
static uint32_t last_esc_time = 0;

void System_HandleInputs(void)
{
    int16_t enc_current = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
    enc_delta = enc_current - enc_last_value;

    if (enc_delta != 0)
    {
        enc_last_value = enc_current;

        if (enc_delta > 0)
        {
            if (ui_ctx.selected_item < ui_ctx.total_items - 1)
                ui_ctx.selected_item++;
        }
        else
        {
            if (ui_ctx.selected_item > 0)
                ui_ctx.selected_item--;
        }

        enc_delta = 0;
    }

    if (esc_btn_pressed)
    {
        esc_btn_pressed = 0;

        if (ui_state == UI_SCREEN_CONFIG ||
            ui_state == UI_SCREEN_DATA ||
            ui_state == UI_SCREEN_SETTINGS)
        {
            ui_state = UI_SCREEN_MENU;
        }
        else if (ui_state == UI_SCREEN_MENU && main_state == STATE_RUNNING)
        {
            ui_state = UI_SCREEN_MAIN;
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t now = HAL_GetTick();

    if (GPIO_Pin == ENTER_button_Pin)
    {
        if (now - last_enter_time > 200)
        {
            enc_btn_pressed = 1;
            last_enter_time = now;
        }
    }
    else if (GPIO_Pin == ESC_button_Pin)
    {
        if (now - last_esc_time > 200)
        {
            esc_btn_pressed = 1;
            last_esc_time = now;
        }
    }
}
