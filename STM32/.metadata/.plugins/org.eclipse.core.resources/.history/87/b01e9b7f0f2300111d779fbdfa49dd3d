/*
 * motor_control.h
 *
 *  Created on: Mar 15, 2026
 *      Author: young
 */

#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_

#include "main.h"

// PID Tuning Constants
#define K_P  0.15f   // Proportional gain
#define K_I  0.01f   // Integral gain
#define K_D  0.001f  // Derivative gain

extern float current_pwm;
extern int target_rpm;

void Motor_Init(void);
void Set_Motor_Duty(TIM_HandleTypeDef *htim, uint32_t Channel, float percent);
void Motor_Update_PID(void);

#endif /* INC_MOTOR_CONTROL_H_ */
