/*
 * motor_control.c
 *
 *  Created on: Mar 15, 2026
 *      Author: young
 */

#include "motor_control.h"

// Used for "Soft Start" so drill ramps up smoothly rather than jerking to life instantly
#define RAMP_STEP 10 // Define how many RPM to increase per PID cycle (e.g., 10 RPM per update)
static float ramped_target_rpm = 0; // The "moving" target for the PID

extern TIM_HandleTypeDef htim1;

float current_pwm = 0;
int target_rpm = 0;
extern float current_rpm; // From rpm_sensor.c

uint32_t last_PID_run = 0; // Track the last time PID ran

// PID internal variables
static float error_integral = 0;
static float last_error = 0;

float kp_val = 0.05;

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
    // 1. Kill switch / Reset
    if (target_rpm <= 0) {
        Set_Motor_Duty(&htim1, TIM_CHANNEL_1, 0);
        ramped_target_rpm = 0;
        error_integral = 0;
        last_error = 0;
        return;
    }

    // 2. Feed-Forward Calculation
    // Use the 0-2000 RPM -> 0-75% PWM mapping as our baseline baseline
    // 75 / 2000 = 0.0375
    float base_pwm = target_rpm * 0.0375f;

    // 3. Calculate Error
    // You can swap target_rpm for ramped_target_rpm if you re-enable soft start
    float error = target_rpm - current_rpm;

    // 4. PID Math
    float p_term = K_P * error;

    error_integral += error;

    // ANTI-WINDUP FIX:
    // Previously, you limited error_integral to 500.
    // With K_I at 0.01, your max i_term was (500 * 0.01) = 5% PWM.
    // This limits the integral's ability to correct heavy loads.
    // It is usually better to limit the actual i_term output, rather than the raw error sum.
    float i_term = K_I * error_integral;

    // Clamp the i_term so it can only add or subtract a maximum of, say, 25% PWM
    if (i_term > 25.0f) {
        i_term = 25.0f;
        error_integral = 25.0f / K_I; // Keep the accumulator in sync
    } else if (i_term < -25.0f) {
        i_term = -25.0f;
        error_integral = -25.0f / K_I;
    }

    // 5. Sum Base PWM + PID Corrections
    // If the drill hits a heavy load, RPM drops, error goes up, P and I add to the base_pwm.
    float output_pwm = base_pwm + p_term + i_term;

    // 6. Final safety clamp to ensure we don't exceed your 75% hardware limit
    if (output_pwm > 75.0f) output_pwm = 75.0f;
    if (output_pwm < 0.0f) output_pwm = 0.0f;

    Set_Motor_Duty(&htim1, TIM_CHANNEL_1, output_pwm);

    last_PID_run = HAL_GetTick();
}

//void Motor_Update_PID(void) {
////	// 1. Soft Start Logic: Gradually move ramped_target toward target_rpm
////	if (ramped_target_rpm < target_rpm) {
////		ramped_target_rpm += RAMP_STEP;
////		if (ramped_target_rpm > target_rpm) ramped_target_rpm = target_rpm;
////	}
////	else if (ramped_target_rpm > target_rpm) {
////		// Optional: Soft stop/ramp down
////		ramped_target_rpm -= (RAMP_STEP * 2); // Ramp down a bit faster for safety
////		if (ramped_target_rpm < target_rpm) ramped_target_rpm = target_rpm;
////	}
//
//
//	// 2. Kill switch / Reset
//	if (target_rpm <= 0) {
//		Set_Motor_Duty(&htim1, TIM_CHANNEL_1, 0);
//		ramped_target_rpm = 0;
//		error_integral = 0;
//		last_error = 0;
//		return;
//	}
//
//
//    // 3. Calculate Error using the RAMPED target, not the final target
//    float error = target_rpm - current_rpm;
//
//
//    // 4. PID math
//    // Proportional term
////    float p_term = K_P * error;
//    float p_term = kp_val * error;
//
//    // Integral term (Accumulates over time to fix steady-state error)
//    error_integral += error;
//    // Anti-windup: Limit the integral so it doesn't grow infinitely
//    if (error_integral > 500) error_integral = 500;
//    if (error_integral < -500) error_integral = -500;
//    float i_term = K_I * error_integral;
//
////    // Derivative term (Reacts to how fast the error is changing)
////    float d_term = K_D * (error - last_error);
////    last_error = error;
//
//
//    // 5. Sum and apply total new PWM
////    float output_pwm = p_term + i_term + d_term;
//    float output_pwm = p_term + i_term;
//    Set_Motor_Duty(&htim1, TIM_CHANNEL_1, output_pwm);
//
//    last_PID_run = HAL_GetTick();
//}
