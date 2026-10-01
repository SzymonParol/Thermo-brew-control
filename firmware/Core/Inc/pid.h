#ifndef PID_H_
#define PID_H_

#include "main.h"

typedef struct {
    float Kp, Ki, Kd;
    float integral;
    float prev_error;
    float output_min;
    float output_max;
} PID_t;

void PID_Init(PID_t *pid, float Kp, float Ki, float Kd, float min, float max);
float PID_Calculate(PID_t *pid, float error, float dt);

#endif
