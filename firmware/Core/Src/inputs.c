#include "inputs.h"
#include "system.h"
#include "ui.h"
#include "tim.h"

volatile uint8_t enc_btn_pressed = 0U;
volatile uint8_t esc_btn_pressed = 0U;
int16_t enc_delta = 0;

static int16_t enc_last_value = 0;
static int16_t enc_accumulator = 0;
static uint32_t last_enter_time = 0;
static uint32_t last_esc_time = 0;

#define ENCODER_COUNTS_PER_STEP 4

void System_HandleInputs(void)
{
    int16_t enc_current = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
    int16_t raw_delta = (int16_t)(enc_current - enc_last_value);

    uint8_t menu_is_visible = 0U;

    if (ui_ctx.selected_item >= ui_ctx.total_items)
    {
        ui_ctx.selected_item = 0U;
    }

    if ((main_state == STATE_IDLE) &&
        ((ui_state == UI_SCREEN_MAIN) ||
         (ui_state == UI_SCREEN_CONFIG_LIST)))
    {
        menu_is_visible = 1U;
    }
    else if ((main_state == STATE_RUNNING) &&
             (ui_state == UI_SCREEN_MENU))
    {
        menu_is_visible = 1U;
    }
    else if (main_state == STATE_FINISHED)
    {
        menu_is_visible = 1U;
    }

    if (raw_delta != 0)
    {
        enc_last_value = enc_current;

        if (menu_is_visible != 0U)
        {
            enc_accumulator = (int16_t)(enc_accumulator + raw_delta);

            if (enc_accumulator >= ENCODER_COUNTS_PER_STEP)
            {
                if (ui_ctx.selected_item == 0U)
                {
                    ui_ctx.selected_item = (uint8_t)(ui_ctx.total_items - 1U);
                }
                else
                {
                    ui_ctx.selected_item--;
                }

                enc_accumulator = 0;
            }
            else if (enc_accumulator <= -ENCODER_COUNTS_PER_STEP)
            {
                if (ui_ctx.selected_item >= (ui_ctx.total_items - 1U))
                {
                    ui_ctx.selected_item = 0U;
                }
                else
                {
                    ui_ctx.selected_item++;
                }

                enc_accumulator = 0;
            }
        }
        else
        {
            enc_accumulator = 0;
        }
    }

    if (esc_btn_pressed != 0U)
    {
        esc_btn_pressed = 0U;

        if (main_state == STATE_IDLE)
        {
            if ((ui_state == UI_SCREEN_CONFIG_LIST) ||
                (ui_state == UI_SCREEN_CONFIG_NEW))
            {
                ui_state = UI_SCREEN_MAIN;
                ui_ctx.selected_item = 0U;
                ui_ctx.total_items = 4U;
            }
            else if ((ui_state == UI_SCREEN_DATA) ||
                     (ui_state == UI_SCREEN_SETTINGS))
            {
                ui_state = UI_SCREEN_MAIN;
                ui_ctx.selected_item = 0U;
                ui_ctx.total_items = 4U;
            }
        }
        else if ((main_state == STATE_RUNNING) &&
                 (ui_state == UI_SCREEN_MENU))
        {
            ui_state = UI_SCREEN_MAIN;
            ui_ctx.selected_item = 0U;
            ui_ctx.total_items = 5U;
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t now = HAL_GetTick();

    if (GPIO_Pin == ENTER_button_Pin)
    {
        if ((now - last_enter_time) >= 100U)
        {
            enc_btn_pressed = 1U;
            last_enter_time = now;
        }
    }
    else if (GPIO_Pin == ESC_button_Pin)
    {
        if ((now - last_esc_time) >= 100U)
        {
            esc_btn_pressed = 1U;
            last_esc_time = now;
        }
    }
}
