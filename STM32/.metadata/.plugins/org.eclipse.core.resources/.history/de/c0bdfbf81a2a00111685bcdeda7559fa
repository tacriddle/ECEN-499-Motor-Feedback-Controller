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
//#define K_FF 0.0375f YES  // Feed Forward
//#define K_P  0.02f  Maybe, 0.04 cut in half   // Proportional gain
//#define K_I  0.015f       // Integral gain
//#define K_D  0.f       // Derivative gain

//#define K_P  0.15f   // Proportional gain
//#define K_I  0.01f   // Integral gain
//#define K_D  0.001f  // Derivative gain
//#define K_FF 0.01f   // Feed Forward


#define PID_DT 0.01f  // 10ms expressed in seconds
#define MAX_RPM 2000.0f
#define MAX_PWM 0.715f

//#define MAX_PWM 100.0
//#define MIN_PWM 0.0
//#define MAX_INTEGRAL 500.0 // Anti-windup limit

void Motor_Init(void);
void Set_Motor_Duty(TIM_HandleTypeDef *htim, uint32_t Channel, float percent);
void Motor_Update_PID(void);

#endif /* INC_MOTOR_CONTROL_H_ */
