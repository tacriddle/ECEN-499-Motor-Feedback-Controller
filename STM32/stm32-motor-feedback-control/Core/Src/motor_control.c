/*
 * motor_control.c
 *
 *  Created on: Mar 15, 2026
 *      Author: young
 */

#include "motor_control.h"
#include "rpm_sensor.h"

extern TIM_HandleTypeDef htim1;

float current_pwm = 0;
float target_rpm = 0;

// Global variables to store state between function calls
float needed_pwm = 0;
float output_pwm = 0;
float rpm_error = 0;
float error_pwm = 0;

/* =======================================================================================================
 * TODO - PID Tuning: If implementing derivative and/or integral feedback, uncomment the following lines
 *  					as well as in the 2 sections below and 1 section in motor_control.h
 * ======================================================================================================*/
// Derivative:
//float D_alpha = 0.2f; // Derivative filter factor (0.0 to 1.0). Lower = smoother.
//float previous_error = 0.0f;
//float filtered_derivative = 0.0f; // Stores the smoothed derivative

// Integral:
//float integral_sum = 0.0f;


void Motor_Init(void) {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    Set_Motor_Duty(&htim1, TIM_CHANNEL_1, 0);
}


void Set_Motor_Duty(TIM_HandleTypeDef *htim, uint32_t Channel, float percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    current_pwm = percent;

    uint32_t arr = htim->Instance->ARR;
    uint32_t pulse = (uint32_t)((percent * (arr + 1)) / 100.0f);
    __HAL_TIM_SET_COMPARE(htim, Channel, pulse);
}


void Motor_Update_PID(void) {
    if (target_rpm <= 0.0f) {
        Set_Motor_Duty(&htim1, TIM_CHANNEL_1, 0.0f);
        rpm_error = 0.0f;
        return;
    }

    // 1. Calculate the error
    rpm_error = target_rpm - current_rpm;

    // 2. Proportional Term
    float P_out = K_P * rpm_error;

/* =================================================================================================================
 * TODO - PID Tuning: If implementing derivative and/or integral feedback, uncomment the following lines
 * 						(#3 for integral, #4 for derivative) as well as in the 1 sections above, 1 section
 * 						below, and 1 section in motor_control.h
 * =================================================================================================================*/
//	// 3. Integral Term with Anti-Windup
//	integral_sum += rpm_error * PID_DT;
//	float I_out = K_I * integral_sum;
//
//	if (I_out > 100.0f) {
//		I_out = 100.0f;
//		if (K_I != 0) integral_sum = 100.0f / K_I;
//	} else if (I_out < 0.0f) {
//		I_out = 0.0f;
//		integral_sum = 0.0f;
//	}
//
//	// 4. Filtered Derivative Term
//	float raw_derivative = (rpm_error - previous_error) / PID_DT;
//	filtered_derivative = (D_alpha * raw_derivative) + ((1.0f - D_alpha) * filtered_derivative);
//	float D_out = K_D * filtered_derivative;


    // 5. Calculate total PID output
/* ===================================================================================================================
 * TODO - PID Tuning: If implementing derivative and/or integral feedback, uncomment/comment out the necessary lines
 * 						as well as in the 2 sections above and 1 section in motor_control.h
 * ===================================================================================================================*/
    // if using only proportional
    error_pwm = P_out;

    // if using proportional and derivative
//    error_pwm = P_out + D_out;

    // if using proportional and integral
//    error_pwm = P_out + I_out;

    // if using proportional, derivative, and integral
//    error_pwm = P_out + I_out + D_out;

    needed_pwm = ((target_rpm / MAX_RPM) * MAX_PWM) * 100.0f;
    output_pwm = needed_pwm + error_pwm;

    // 6. Final safety clamp
    if (output_pwm > 100.0f) output_pwm = 100.0f;
    if (output_pwm < 0.0f) output_pwm = 0.0f;

    // 7. Apply the new duty cycle
    Set_Motor_Duty(&htim1, TIM_CHANNEL_1, output_pwm);
}
