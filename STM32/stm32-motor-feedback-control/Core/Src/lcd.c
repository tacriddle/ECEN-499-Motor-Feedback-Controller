/*
 * lcd.c
 *
 *  Created on: Mar 16, 2026
 *      Author: young
 */


#include "lcd.h"

extern I2C_HandleTypeDef hi2c1;

static uint8_t lcd_backlight = 0x08; // 0x08 is ON, 0x00 is OFF
uint32_t last_LCD_update = 0; 	// for LCD refresh every 200ms
extern float current_rpm;		// From rpm_sensor.c
extern float current_pwm;		// From motor_control.c


void LCD_Update_Display(void) {
    char display_buf[21];

    // Line 1: RPM
    snprintf(display_buf, sizeof(display_buf), "RPM: %-5d", (int)current_rpm);
    LCD_Set_Cursor(0, 0);
    LCD_Print(display_buf);

    // Line 2: PWM Duty Cycle
    snprintf(display_buf, sizeof(display_buf), "PWM: %d%%", (int)current_pwm);
    LCD_Set_Cursor(1, 0);
    LCD_Print(display_buf);

    // Update the timestamp so main loop knows when we last ran
    last_LCD_update = HAL_GetTick();
}



static void lcd_write_byte(uint8_t data) {
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, &data, 1, 10);
}

/**
 * @brief Pulses the Enable pin to latch data into the LCD
 */
static void lcd_pulse(uint8_t data) {
    lcd_write_byte(data | 0x04);    // En=1
    HAL_Delay(1);
    lcd_write_byte(data & ~0x04);   // En=0
    HAL_Delay(1);
}

static void lcd_write4(uint8_t nibble, uint8_t rs) {
    uint8_t data = (nibble & 0xF0) | lcd_backlight;
    if (rs) data |= 0x01;  // Explicitly set RS bit (Bit 0)
    lcd_pulse(data);
}

void LCD_Cmd(uint8_t cmd) {
    lcd_write4(cmd & 0xF0, 0);         // Upper nibble
    lcd_write4((cmd << 4) & 0xF0, 0);  // Lower nibble
}

void LCD_Data(uint8_t data) {
    lcd_write4(data & 0xF0, 1);         // Upper nibble (RS=1)
    lcd_write4((data << 4) & 0xF0, 1);  // Lower nibble (RS=1)
}

void LCD_Init(void) {
    HAL_Delay(50);
    lcd_write4(0x30, 0); HAL_Delay(5);
    lcd_write4(0x30, 0); HAL_Delay(1);
    lcd_write4(0x30, 0); HAL_Delay(1);
    lcd_write4(0x20, 0); HAL_Delay(1); // Switch to 4-bit mode

    LCD_Cmd(0x28); // 2 lines (this covers 4 lines too), 5x8 font
    LCD_Cmd(0x0C); // Display ON, Cursor OFF
    LCD_Cmd(0x01); // Clear Display
    HAL_Delay(5);  // CRITICAL: Clearing takes longer than other cmds
    LCD_Cmd(0x06); // Entry mode: Increment cursor
}
void LCD_Set_Cursor(uint8_t row, uint8_t col) {
    // 20x4 row addresses: Row 0=0x00, Row 1=0x40, Row 2=0x14, Row 3=0x54
    static const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};

    // Command 0x80 is the "Set DDRAM Address" instruction
    LCD_Cmd(0x80 | (row_offsets[row] + col));
}

void LCD_Print(const char *s) {
    while (*s) LCD_Data((uint8_t)*s++);
}

void LCD_Clear(void) {
    LCD_Cmd(0x01);
    HAL_Delay(2);
}
