#ifndef INC_MAX6675_H_
#define INC_MAX6675_H_

#include "main.h"

void MAX6675_Init(void);
float MAX6675_ReadCelsius(void);
uint8_t MAX6675_IsOpenCircuit(void);

#endif
