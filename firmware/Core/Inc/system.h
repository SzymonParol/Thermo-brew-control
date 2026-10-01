#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "main.h"
#include "pid.h"
#include "inputs.h"
#include "ui.h"

typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_FINISHED
} MainState_t;

typedef enum {
    UI_SCREEN_MAIN,
    UI_SCREEN_MENU,
    UI_SCREEN_CONFIG,
    UI_SCREEN_DATA,
    UI_SCREEN_SETTINGS
} UIState_t;

typedef struct {
    uint8_t selected_item;
    uint8_t total_items;
    uint8_t is_editing;
    uint8_t edit_cursor_pos;
} UIContext_t;

extern MainState_t  main_state;
extern UIState_t    ui_state;
extern UIContext_t  ui_ctx;
extern float        current_temp;
extern uint8_t      temp_valid;
extern float        setpoint_temp;
extern float        gas_output;
extern PID_t        pid_temp;

void System_Init(void);
void System_Update(void);

#endif
