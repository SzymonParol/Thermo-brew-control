#include "ui.h"
#include "system.h"
#include "ili9341.h"
#include <stdio.h>
#include <string.h>

volatile uint8_t ui_needs_redraw = 1U;

void UI_RequestRedraw(void)
{
    ui_needs_redraw = 1U;
}

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
        "USTAWIENIA"
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
    UI_DrawText(
        10U,
        32U,
        "PROCES ZAKONCZONY",
        ILI9341_GREEN,
        ILI9341_BLACK
    );

    UI_DrawText(
        16U,
        56U,
        "NOWY PROCES",
        ILI9341_WHITE,
        ILI9341_BLACK
    );

    UI_DrawText(
        2U,
        56U,
        ">",
        ILI9341_YELLOW,
        ILI9341_BLACK
    );
}

static void draw_config_list(void)
{
    static const char *items[] =
    {
        "NOWA KONFIGURACJA"
    };

    uint8_t index;
    uint16_t y;
    char display_name[CONFIG_NAME_LENGTH];

    for (index = 0U; index < 1U; index++)
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

    for (index = 0U; index < config_count; index++)
    {
        y = (uint16_t)(32U + ((index + 1U) * UI_LINE_HEIGHT));

        strncpy(display_name, config_list[index].name, sizeof(display_name) - 1U);
        display_name[sizeof(display_name) - 1U] = '\0';

        if ((index + 1U) == ui_ctx.selected_item)
        {
            UI_DrawText(2U, y, ">", ILI9341_YELLOW, ILI9341_BLACK);
        }
        else
        {
            UI_DrawText(2U, y, " ", ILI9341_WHITE, ILI9341_BLACK);
        }

        UI_DrawText(16U, y, display_name, ILI9341_WHITE, ILI9341_BLACK);
    }

    if (config_count == 0U)
    {
        UI_DrawText(
            16U,
            (uint16_t)(32U + UI_LINE_HEIGHT),
            "BRAK KONFIGURACJI",
            ILI9341_WHITE,
            ILI9341_BLACK
        );
    }
}

static void draw_config_new(void)
{
    char buffer[32];

    UI_DrawText(
        10U,
        32U,
        "NOWA KONFIGURACJA",
        ILI9341_CYAN,
        ILI9341_BLACK
    );

    snprintf(buffer, sizeof(buffer), "SET: %.1f C", active_config.setpoint);

    UI_DrawText(
        10U,
        56U,
        buffer,
        ILI9341_WHITE,
        ILI9341_BLACK
    );

    UI_DrawText(
        10U,
        100U,
        "[ESC] POWROT",
        ILI9341_YELLOW,
        ILI9341_BLACK
    );
}

static void draw_config_screen(void)
{
    UI_DrawText(
        10U,
        32U,
        "KONFIGURACJE",
        ILI9341_CYAN,
        ILI9341_BLACK
    );

    UI_DrawText(
        10U,
        56U,
        "BRAK OPCJI",
        ILI9341_WHITE,
        ILI9341_BLACK
    );

    UI_DrawText(
        10U,
        100U,
        "[ESC] POWROT",
        ILI9341_YELLOW,
        ILI9341_BLACK
    );
}

static void draw_data_screen(void)
{
    UI_DrawText(
        10U,
        32U,
        "DANE PROCESU",
        ILI9341_CYAN,
        ILI9341_BLACK
    );

    UI_DrawText(
        10U,
        56U,
        "BRAK DANYCH",
        ILI9341_WHITE,
        ILI9341_BLACK
    );

    UI_DrawText(
        10U,
        100U,
        "[ESC] POWROT",
        ILI9341_YELLOW,
        ILI9341_BLACK
    );
}

static void draw_settings_screen(void)
{
    UI_DrawText(
        10U,
        32U,
        "USTAWIENIA",
        ILI9341_CYAN,
        ILI9341_BLACK
    );

    UI_DrawText(
        10U,
        56U,
        "BRAK OPCJI",
        ILI9341_WHITE,
        ILI9341_BLACK
    );

    UI_DrawText(
        10U,
        100U,
        "[ESC] POWROT",
        ILI9341_YELLOW,
        ILI9341_BLACK
    );
}

void UI_MoveCursor(uint8_t old_item, uint8_t new_item)
{
    uint16_t old_y;
    uint16_t new_y;
    uint8_t cursor_can_be_drawn = 0U;

    if ((main_state == STATE_IDLE) &&
        (ui_state == UI_SCREEN_MAIN))
    {
        cursor_can_be_drawn = 1U;
    }
    else if ((main_state == STATE_RUNNING) &&
             (ui_state == UI_SCREEN_MENU))
    {
        cursor_can_be_drawn = 1U;
    }
    else if ((main_state == STATE_IDLE) &&
             (ui_state == UI_SCREEN_CONFIG_LIST))
    {
        cursor_can_be_drawn = 1U;
    }
    else if (main_state == STATE_FINISHED)
    {
        cursor_can_be_drawn = 1U;
    }

    if (cursor_can_be_drawn == 0U)
    {
        return;
    }

    if (old_item == new_item)
    {
        return;
    }

    old_y = (uint16_t)(32U + (old_item * UI_LINE_HEIGHT));
    new_y = (uint16_t)(32U + (new_item * UI_LINE_HEIGHT));

    UI_DrawText(2U, old_y, " ", ILI9341_WHITE, ILI9341_BLACK);
    UI_DrawText(2U, new_y, ">", ILI9341_YELLOW, ILI9341_BLACK);
}

void System_Draw(void)
{
    if (ui_needs_redraw == 0U)
    {
        return;
    }

    ui_needs_redraw = 0U;

    ILI9341_FillScreen(ILI9341_BLACK);

    draw_header();

    switch (main_state)
    {
    case STATE_IDLE:
        if (ui_state == UI_SCREEN_MAIN)
        {
            draw_menu_idle();
        }
        else if (ui_state == UI_SCREEN_CONFIG_LIST)
        {
            draw_config_list();
        }
        else if (ui_state == UI_SCREEN_CONFIG_NEW)
        {
            draw_config_new();
        }
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
