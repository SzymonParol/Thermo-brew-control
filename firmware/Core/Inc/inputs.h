#ifndef INPUTS_H_
#define INPUTS_H_

#include <stdint.h>

extern volatile uint8_t enc_btn_pressed;
extern volatile uint8_t esc_btn_pressed;
extern int16_t enc_delta;

void System_HandleInputs(void);

#endif /* INPUTS_H_ */
