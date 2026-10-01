#ifndef A4988_H_
#define A4988_H_

#include "main.h"

void A4988_Init(void);
void A4988_SetSpeed(float pct);
void A4988_SetDirection(uint8_t cw);
void A4988_Stop(void);
void A4988_Update(void);

#endif
