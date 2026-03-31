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

    // 5. Calculate total PID output
    needed_pwm = ((target_rpm / MAX_RPM) * MAX_PWM) * 100.0f;
    output_pwm = needed_pwm + P_out;

    // 6. Final safety clamp
    if (output_pwm > 100.0f) output_pwm = 100.0f;
    if (output_pwm < 0.0f) output_pwm = 0.0f;

    // 7. Apply the new duty cycle
    Set_Motor_Duty(&htim1, TIM_CHANNEL_1, output_pwm);
}
