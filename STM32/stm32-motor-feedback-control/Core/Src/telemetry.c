/*
 * telemetry.c
 *
 *  Created on: Mar 15, 2026
 *      Author: young
 */

#include "telemetry.h"
#include "rpm_sensor.h"
#include "motor_control.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim1;


// ESP32
uint32_t last_ESP32_transmit = 0; 	// transmit to ESP32 every 100ms
char esp32_tx_buffer[20]; 			// buffer for rpm string to be sent to ESP32
char esp32_rx_buffer[10];   		// To hold the incoming string (e.g., "3000\n") from ESP32
uint8_t esp32_rx_index = 0;			// Current position in buffer
uint8_t esp32_rx_char;     			// Temp storage for 1 byte


void Telemetry_Init(void) {
    HAL_UART_Receive_IT(&huart1, &esp32_rx_char, 1);
}

// 1. Define a packed structure for the telemetry packet
// Using __attribute__((packed)) ensures the compiler doesn't add padding bytes
typedef struct __attribute__((packed)) {
    uint8_t  header; // Set this to 0x7E
    uint16_t rpm;
    uint16_t pwm;
    uint8_t  footer; // Set this to 0xAA
} TelemetryPacket_t;

void Telemetry_Transmit_ESP32(void) {
    static TelemetryPacket_t packet;

    // Check huart1 (the ESP32 link) state
    if (huart1.gState == HAL_UART_STATE_READY) {

        // 1. PACK the data
    	packet.header = 0x7E;
        packet.rpm = (uint16_t)current_rpm;
        packet.pwm = (uint16_t)current_pwm;
        packet.footer = 0xAA;

        // 2. TRANSMIT the raw binary bytes to the ESP32 (huart1)
        HAL_UART_Transmit_DMA(&huart1, (uint8_t*)&packet, sizeof(packet));

        last_ESP32_transmit = HAL_GetTick();
    }
}




void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
    	// Triggered on UART target rpm input from ESP32
    	// Using example of "1500\n" sent from ESP32:
    	// 	 Each digit (1, 5, 0, 0) triggers the RxCpltCallback.
    	//   When the \n arrives, it runs atoi(), and target_rpm instantly becomes the integer 1500

        // 1. Check for end-of-line characters
        if (esp32_rx_char == '\n' || esp32_rx_char == '\r') {

            // Only process if we actually collected characters (prevents \r\n double-trigger)
            if (esp32_rx_index > 0) {
                esp32_rx_buffer[esp32_rx_index] = '\0'; // Null-terminate
                target_rpm = atoi(esp32_rx_buffer);     // Convert to integer
                esp32_rx_index = 0; // Reset index ONLY after a successful parse
            }
        }
        // 2. Add numeric characters to the buffer
        else if (esp32_rx_index < sizeof(esp32_rx_buffer) - 1) {

            // Only accept numbers 0-9 to filter out potential serial noise/trash
            if (esp32_rx_char >= '0' && esp32_rx_char <= '9') {
                esp32_rx_buffer[esp32_rx_index++] = esp32_rx_char;
            }
        }

        // 3. Re-prime the interrupt to catch the next byte
        HAL_UART_Receive_IT(huart, &esp32_rx_char, 1);
    }

}
