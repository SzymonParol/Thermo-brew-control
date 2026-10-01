#include "system.h"

#include "tim.h"
#include "ds18b20.h"
#include "a4988.h"
#include "ili9341.h"

MainState_t  main_state = STATE_IDLE;
UIState_t    ui_state   = UI_SCREEN_MAIN;
UIContext_t  ui_ctx     = {0};
float        current_temp = 0.0f;
uint8_t      temp_valid = 0;
float        setpoint_temp = 68.0f;
float        gas_output = 0.0f;

PID_t pid_temp = {0};

static uint32_t last_pid_time = 0;

void System_Init(void)
{
    main_state   = STATE_IDLE;
    ui_state     = UI_SCREEN_MAIN;
    ui_ctx.selected_item = 0;
    ui_ctx.total_items   = 4;
    ui_ctx.is_editing    = 0;
    ui_ctx.edit_cursor_pos = 0;
    current_temp = 0.0f;
    temp_valid = 0;
    setpoint_temp = 68.0f;
    gas_output = 0.0f;

    PID_Init(&pid_temp, 2.0f, 0.1f, 0.0f, 0.0f, 100.0f);

    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    ds18b20_init();
    A4988_Init();
    ILI9341_Init();
    ILI9341_FillScreen(ILI9341_BLACK);
}

void System_Update(void)
{
    uint32_t now = HAL_GetTick();

    System_HandleInputs();

    if (enc_btn_pressed)
    {
        enc_btn_pressed = 0;

        if (main_state == STATE_IDLE)
        {
            if (ui_state == UI_SCREEN_MAIN)
            {
                if (ui_ctx.selected_item == 0)
                {
                    main_state = STATE_RUNNING;
                    ui_ctx.selected_item = 0;
                }
                else if (ui_ctx.selected_item == 1)
                    ui_state = UI_SCREEN_CONFIG;
                else if (ui_ctx.selected_item == 2)
                    ui_state = UI_SCREEN_DATA;
                else if (ui_ctx.selected_item == 3)
                    ui_state = UI_SCREEN_SETTINGS;
            }
        }
        else if (main_state == STATE_RUNNING)
        {
            if (ui_state == UI_SCREEN_MAIN)
            {
                ui_state = UI_SCREEN_MENU;
                ui_ctx.total_items = 5;
            }
            else if (ui_state == UI_SCREEN_MENU)
            {
                if (ui_ctx.selected_item == 0)
                {
                    main_state = STATE_FINISHED;
                    ui_state = UI_SCREEN_MAIN;
                    ui_ctx.total_items = 4;
                    A4988_Stop();
                }
                else if (ui_ctx.selected_item == 4)
                {
                    ui_state = UI_SCREEN_MAIN;
                }
            }
        }
        else if (main_state == STATE_FINISHED)
        {
            if (ui_ctx.selected_item == 0)
            {
                main_state = STATE_IDLE;
                ui_state = UI_SCREEN_MAIN;
                ui_ctx.total_items = 4;
            }
        }
    }

    if (main_state == STATE_RUNNING)
    {
        if (ds18b20_start_measure(NULL) == HAL_OK)
        {
            HAL_Delay(750);
            current_temp = ds18b20_get_temp(NULL);
            temp_valid = 1;

            if (now - last_pid_time >= 100)
            {
                float error = setpoint_temp - current_temp;
                float dt = 0.1f;
                float pid_out = PID_Calculate(&pid_temp, error, dt);

                if (pid_out > 0.0f)
                {
                    A4988_SetDirection(1);
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
            temp_valid = 0;
            A4988_Stop();
            gas_output = 0.0f;
        }
    }
    else
    {
        temp_valid = 0;
        A4988_Stop();
        gas_output = 0.0f;
    }

    A4988_Update();
}
