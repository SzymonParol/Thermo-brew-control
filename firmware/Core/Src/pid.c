#include "pid.h"

void PID_Init(PID_t *pid, float Kp, float Ki, float Kd, float min, float max)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output_min = min;
    pid->output_max = max;
}

float PID_Calculate(PID_t *pid, float error, float dt)
{
    float p_term = pid->Kp * error;
    pid->integral += error * dt;
    float i_term = pid->Ki * pid->integral;
    float derivative = (error - pid->prev_error) / dt;
    float d_term = pid->Kd * derivative;
    pid->prev_error = error;

    float output = p_term + i_term + d_term;

    if (output > pid->output_max)
        output = pid->output_max;
    if (output < pid->output_min)
        output = pid->output_min;

    return output;
}
