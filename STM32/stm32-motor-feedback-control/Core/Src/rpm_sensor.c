/*
 * rpm_sensor.c
 *
 *  Created on: Mar 15, 2026
 *      Author: young
 */


#include "rpm_sensor.h"

extern TIM_HandleTypeDef htim2;

uint32_t capture_buffer[DMA_BUF_SIZE];

// Indices for the end of the first half and total buffer
// These define our "capture windows" for the DMA interrupts
uint8_t CALC_WINDOW_MAX_IDX = CALC_WINDOW_SIZE - 1;
uint8_t DMA_BUF_MAX_IDX = DMA_BUF_SIZE - 1;

// Conversion Constant Calculation:
// We measure ticks between pulses. To get RPM:
// RPM = (60,000,000 us/min) / (Time for 1 revolution in us)
// Time for 1 rev = (Avg time between pulses) * SLOTS_ON_DISK
float conversion_factor = (60000000.0f * CALC_WINDOW_SIZE) / SLOTS_ON_DISK;

uint32_t last_pulse_timestamp = 0; 	// Stores the final timestamp of the previous DMA batch
volatile uint32_t period_ticks = 0; // Raw timer ticks over the current window
volatile uint8_t new_rpm_data = 0;  // Semaphore flag to signal main loop processing
float current_rpm = 0; 				// Resultant filtered RPM

// Sliding window for spike rejection
float median_buffer[3] = {0, 0, 0};
uint8_t median_idx = 0;

// Smoothing factor (Lower = smoother but slower response. 30% weight to new data, 70% to history)
float alpha = 0.3f;

// Calculate max allowable time between pulses before assuming motor has stopped
float timeout_ms = 60000.0f / (MINIMUM_RPM * SLOTS_ON_DISK);
uint32_t last_tick = 0; // For timeout logic


void RPM_Sensor_Init(void) {
	// This tells the DMA: "Every time TIM2 captures a value, put it in capture_buffer."
    HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, capture_buffer, DMA_BUF_SIZE);
}

float get_median_of_3(float a, float b, float c) {
    if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
    if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
    return c;
}

// Process new RPM measurement if a DMA interrupt has occurred
void RPM_Process_Data(void) {
	if (new_rpm_data) {
		new_rpm_data = 0;
		// Step 1: Calculate raw RPM from timer ticks
		float rpm_raw = conversion_factor / (float)period_ticks;

		// Step 2: Median Filter - Rejects single-point noise/glitches
		median_buffer[median_idx] = rpm_raw;
		median_idx++;
		if (median_idx >= 3) median_idx = 0;
		float rpm_filtered = get_median_of_3(median_buffer[0], median_buffer[1], median_buffer[2]);

		// Step 3: Complementary Alpha Filter - Smooths out quantization jitter
		current_rpm = (alpha * rpm_filtered) + ((1.0f - alpha) * current_rpm);

		// Update timeout timestamp
		last_tick = HAL_GetTick();
	}

	// timeout logic to detect when RPM = 0 (If no pulses seen for too long, set RPM to zero)
	if (HAL_GetTick() - last_tick > (uint32_t)timeout_ms) {
		current_rpm = 0;
		median_buffer[0] = median_buffer[1] = median_buffer[2] = 0;
	}
}


// Triggered when DMA fills the FIRST half of the buffer (Indices 0 to CALC_WINDOW_MAX_IDX)
// CALC_WINDOW_MAX_IDX = (DMA_BUF_SIZE / 2) - 1
void HAL_TIM_IC_CaptureHalfCpltCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
    	// Calculate time elapsed from the end of the previous DMA batch to the end of this half.
    	// This ensures we don't lose the "gap" between interrupts.
        period_ticks = capture_buffer[CALC_WINDOW_MAX_IDX] - last_pulse_timestamp;

        // Save last timestamp of the first half to be used for calculation of second half
        last_pulse_timestamp = capture_buffer[CALC_WINDOW_MAX_IDX];

        new_rpm_data = 1;
    }
}


// Triggered when DMA fills the SECOND half of the buffer (Indices CALC_WINDOW_SIZE to DMA_BUF_MAX_IDX)
// CALC_WINDOW_SIZE = DMA_BUF_SIZE / 2
// DMA_BUF_MAX_IDX = DMA_BUF_SIZE - 1
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
    	// Calculate time elapsed from the end of the first half to the end of the buffer
        period_ticks = capture_buffer[DMA_BUF_MAX_IDX] - last_pulse_timestamp;

        // Save last timestamp of second half to be used for calculation of first half of next buffer
        last_pulse_timestamp = capture_buffer[DMA_BUF_MAX_IDX];

        new_rpm_data = 1;
    }
}
