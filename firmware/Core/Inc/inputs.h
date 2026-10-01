#ifndef INPUTS_H_
#define INPUTS_H_

#include "main.h"

extern uint8_t enc_btn_pressed;
extern uint8_t esc_btn_pressed;
extern int16_t enc_delta;

void System_HandleInputs(void);

#endif
