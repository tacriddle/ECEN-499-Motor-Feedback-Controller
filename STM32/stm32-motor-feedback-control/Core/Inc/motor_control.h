/*
 * motor_control.h
 *
 *  Created on: Mar 15, 2026
 *      Author: young
 */

#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_

#include "main.h"

/* =======================================================================================================
 * TODO - PID Tuning: To change the gain values, change the following 3 macro definitions.
 * ======================================================================================================*/
#define K_P  0.15f  // Proportional gain PID tuning constant

// If implementing derivative and/or integral feedback, uncomment the following lines as well as in the 3 sections in motor_control.c
//#define K_D 0.0f	// Derivative gain PID tuning constant
//#define K_I 0.0f	// Integral gain PID tuning constant


#define PID_DT 0.01f  // 10ms expressed in seconds
#define MAX_RPM 2000.0f
#define MAX_PWM 0.715f

extern float current_pwm;
extern float target_rpm;
extern float needed_pwm;
extern float output_pwm;
extern float rpm_error;

void Motor_Init(void);
void Set_Motor_Duty(TIM_HandleTypeDef *htim, uint32_t Channel, float percent);
void Motor_Update_PID(void);

#endif /* INC_MOTOR_CONTROL_H_ */
