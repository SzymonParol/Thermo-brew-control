/*
 * a4988.c
 *
 *  Created on: 2 lip 2023
 *      Author: Szym
 */

#include "gpio.h"
#include "a4988.h"
#include "stm32f0xx.h"

void move_step(int count_steps, int del)
{
		if(count_steps < 0)
		{
			HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_RESET);
			for(int i = 0; i > count_steps; i--)
				  {
				  HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_SET);
				  HAL_Delay(del);
				  HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_RESET);
				  HAL_Delay(del);
				  }
		}
		else
		{
			HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_SET);
			for(int i = 0; i < count_steps; i++)
				{
					HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_SET);
					HAL_Delay(del);
					HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_RESET);
					HAL_Delay(del);
				 }
		}

	}



