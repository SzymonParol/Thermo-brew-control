#include "system.h"
#include "inputs.h"
#include "ui.h"
#include "tim.h"
#include "ds18b20.h"
#include "a4988.h"
#include "ili9341.h"

MainState_t main_state = STATE_IDLE;
UIState_t ui_state = UI_SCREEN_MAIN;
UIContext_t ui_ctx = {0};

float current_temp = 0.0f;
uint8_t temp_valid = 0;
float setpoint_temp = 68.0f;
float gas_output = 0.0f;

PID_t pid_temp = {0};

static uint32_t last_pid_time = 0;

void System_Init(void)
{
    main_state = STATE_IDLE;
    ui_state = UI_SCREEN_MAIN;

    ui_ctx.selected_item = 0U;
    ui_ctx.total_items = 4U;
    ui_ctx.is_editing = 0U;
    ui_ctx.edit_cursor_pos = 0U;

    current_temp = 0.0f;
    temp_valid = 0U;
    setpoint_temp = 68.0f;
    gas_output = 0.0f;

    PID_Init(&pid_temp, 2.0f, 0.1f, 0.0f, 0.0f, 100.0f);

    Config_Init();

    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);

    ds18b20_init();
    A4988_Init();

    ILI9341_Init();
    ILI9341_FillScreen(ILI9341_BLACK);

    UI_RequestRedraw();
}

void System_Update(void)
{
    uint32_t now = HAL_GetTick();

    MainState_t previous_main_state = main_state;
    UIState_t previous_ui_state = ui_state;
    uint8_t previous_selected_item = ui_ctx.selected_item;

    System_HandleInputs();

    if (enc_btn_pressed != 0U)
    {
        enc_btn_pressed = 0U;

        if (main_state == STATE_IDLE)
        {
            if (ui_state == UI_SCREEN_MAIN)
            {
                if (ui_ctx.selected_item == 0U)
                {
                    main_state = STATE_RUNNING;
                    ui_state = UI_SCREEN_MAIN;
                    ui_ctx.selected_item = 0U;
                    ui_ctx.total_items = 5U;
                }
                else if (ui_ctx.selected_item == 1U)
                {
                    Config_RefreshList();

                    ui_state = UI_SCREEN_CONFIG_LIST;
                    ui_ctx.selected_item = 0U;
                    ui_ctx.total_items = (uint8_t)(1U + config_count);
                }
                else if (ui_ctx.selected_item == 2U)
                {
                    ui_state = UI_SCREEN_DATA;
                    ui_ctx.selected_item = 0U;
                    ui_ctx.total_items = 1U;
                }
                else if (ui_ctx.selected_item == 3U)
                {
                    ui_state = UI_SCREEN_SETTINGS;
                    ui_ctx.selected_item = 0U;
                    ui_ctx.total_items = 1U;
                }
            }
            else if (ui_state == UI_SCREEN_CONFIG_LIST)
            {
                uint8_t result;

                if (ui_ctx.selected_item == 0U)
                {
                    result = Config_SaveNew();
                }
                else
                {
                    result = Config_Load((uint8_t)(ui_ctx.selected_item - 1U));
                }

                if (result == 1U)
                {
                    if (ui_ctx.selected_item == 0U)
                    {
                        ui_state = UI_SCREEN_CONFIG_NEW;
                        ui_ctx.selected_item = 0U;
                        ui_ctx.total_items = 1U;
                    }
                    else
                    {
                        ui_state = UI_SCREEN_MAIN;
                        ui_ctx.selected_item = 0U;
                        ui_ctx.total_items = 4U;
                    }
                }
                else if (result == 2U)
                {
                    ui_state = UI_SCREEN_MAIN;
                    ui_ctx.selected_item = 0U;
                    ui_ctx.total_items = 4U;

                }
                else
                {
                    ;
                }
            }
            else if (ui_state == UI_SCREEN_CONFIG_NEW)
            {
                ui_state = UI_SCREEN_CONFIG_LIST;
                ui_ctx.selected_item = 0U;
                ui_ctx.total_items = (uint8_t)(1U + config_count);
            }
        }
        else if (main_state == STATE_RUNNING)
        {
            if (ui_state == UI_SCREEN_MAIN)
            {
                ui_state = UI_SCREEN_MENU;
                ui_ctx.selected_item = 0U;
                ui_ctx.total_items = 4U;
            }
            else if (ui_state == UI_SCREEN_MENU)
            {
                if (ui_ctx.selected_item == 0U)
                {
                    main_state = STATE_FINISHED;
                    ui_state = UI_SCREEN_MAIN;
                    ui_ctx.selected_item = 0U;
                    ui_ctx.total_items = 1U;

                    A4988_Stop();
                }
            }
        }
        else if (main_state == STATE_FINISHED)
        {
            if (ui_ctx.selected_item == 0U)
            {
                main_state = STATE_IDLE;
                ui_state = UI_SCREEN_MAIN;
                ui_ctx.selected_item = 0U;
                ui_ctx.total_items = 4U;
            }
        }
    }

    if (main_state == STATE_RUNNING)
    {
        if (ds18b20_start_measure(NULL) == HAL_OK)
        {
            HAL_Delay(750U);

            current_temp = ds18b20_get_temp(NULL);
            temp_valid = 1U;

            if ((now - last_pid_time) >= 100U)
            {
                float error = setpoint_temp - current_temp;
                float dt = 0.1f;
                float pid_out = PID_Calculate(&pid_temp, error, dt);

                if (pid_out > 0.0f)
                {
                    A4988_SetDirection(1U);
                    A4988_SetSpeed(pid_out);
                }
                else
                {
                    A4988_Stop();
                }

                gas_output = pid_out;
                last_pid_time = now;
            }
        }
        else
        {
            temp_valid = 0U;
            A4988_Stop();
            gas_output = 0.0f;
        }
    }
    else
    {
        temp_valid = 0U;
        A4988_Stop();
        gas_output = 0.0f;
    }

    A4988_Update();

    if ((previous_main_state != main_state) ||
        (previous_ui_state != ui_state))
    {
        UI_RequestRedraw();
    }
    else if (previous_selected_item != ui_ctx.selected_item)
    {
        UI_MoveCursor(previous_selected_item, ui_ctx.selected_item);
    }
}
