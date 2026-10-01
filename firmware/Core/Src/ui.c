#include "ui.h"
#include "system.h"
#include "ili9341.h"
#include <stdio.h>

#define UI_FONT_SCALE  2U
#define UI_LINE_HEIGHT 18U

static void UI_DrawText(
    uint16_t x,
    uint16_t y,
    const char *text,
    uint16_t text_color,
    uint16_t background_color
)
{
    ILI9341_DrawString(
        x,
        y,
        text,
        text_color,
        background_color,
        UI_FONT_SCALE
    );
}

static void draw_header(void)
{
    char buffer[32];

    ILI9341_FillRect(
        0U,
        0U,
        ILI9341_WIDTH,
        20U,
        ILI9341_BLUE
    );

    if (temp_valid != 0U)
    {
        snprintf(buffer, sizeof(buffer), "T: %.1f C", current_temp);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "T: --.- C");
    }

    UI_DrawText(
        4U,
        3U,
        buffer,
        ILI9341_WHITE,
        ILI9341_BLUE
    );
}

static void draw_process_screen(void)
{
    char buffer[32];
    const uint16_t y = 32U;

    UI_DrawText(10U, y, "PROCES TRWA", ILI9341_GREEN, ILI9341_BLACK);

    snprintf(buffer, sizeof(buffer), "SET: %.1f C", setpoint_temp);
    UI_DrawText(10U, y + UI_LINE_HEIGHT, buffer, ILI9341_WHITE, ILI9341_BLACK);

    snprintf(buffer, sizeof(buffer), "GAS: %.0f %%", gas_output);
    UI_DrawText(10U, y + (2U * UI_LINE_HEIGHT), buffer, ILI9341_WHITE, ILI9341_BLACK);

    UI_DrawText(10U, y + (3U * UI_LINE_HEIGHT), "ETA: --:--", ILI9341_WHITE, ILI9341_BLACK);
    UI_DrawText(10U, y + (4U * UI_LINE_HEIGHT), "ETAP: -/-", ILI9341_WHITE, ILI9341_BLACK);
    UI_DrawText(10U, y + (6U * UI_LINE_HEIGHT), "[ENTER] MENU", ILI9341_CYAN, ILI9341_BLACK);
}

static void draw_menu_idle(void)
{
    static const char *items[] =
    {
        "START PROCESU",
        "KONFIGURACJE",
        "DANE PROCESU",
        "USTAWIENIA"
    };

    uint8_t index;
    uint16_t y;

    for (index = 0U; index < 4U; index++)
    {
        y = (uint16_t)(32U + (index * UI_LINE_HEIGHT));

        if (index == ui_ctx.selected_item)
        {
            UI_DrawText(2U, y, ">", ILI9341_YELLOW, ILI9341_BLACK);
        }
        else
        {
            UI_DrawText(2U, y, " ", ILI9341_WHITE, ILI9341_BLACK);
        }

        UI_DrawText(16U, y, items[index], ILI9341_WHITE, ILI9341_BLACK);
    }
}

static void draw_menu_running(void)
{
    static const char *items[] =
    {
        "ZAKONCZ PROCES",
        "KONFIGURACJE",
        "DANE PROCESU",
        "USTAWIENIA",
        "POWROT"
    };

    uint8_t index;
    uint16_t y;

    for (index = 0U; index < 5U; index++)
    {
        y = (uint16_t)(32U + (index * UI_LINE_HEIGHT));

        if (index == ui_ctx.selected_item)
        {
            UI_DrawText(2U, y, ">", ILI9341_YELLOW, ILI9341_BLACK);
        }
        else
        {
            UI_DrawText(2U, y, " ", ILI9341_WHITE, ILI9341_BLACK);
        }

        UI_DrawText(16U, y, items[index], ILI9341_WHITE, ILI9341_BLACK);
    }
}

static void draw_finished_screen(void)
{
    UI_DrawText(10U, 32U, "PROCES ZAKONCZONY", ILI9341_GREEN, ILI9341_BLACK);
    UI_DrawText(16U, 56U, "NOWY PROCES", ILI9341_WHITE, ILI9341_BLACK);

    if (ui_ctx.selected_item == 0U)
    {
        UI_DrawText(2U, 56U, ">", ILI9341_YELLOW, ILI9341_BLACK);
    }
}

void System_Draw(void)
{
    ILI9341_FillScreen(ILI9341_BLACK);

    draw_header();

    switch (main_state)
    {
        case STATE_IDLE:
            draw_menu_idle();
            break;

        case STATE_RUNNING:
            if (ui_state == UI_SCREEN_MAIN)
            {
                draw_process_screen();
            }
            else
            {
                draw_menu_running();
            }
            break;

        case STATE_FINISHED:
            draw_finished_screen();
            break;

        default:
            break;
    }
}
