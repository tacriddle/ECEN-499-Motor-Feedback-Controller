/*
 * lcd.h
 *
 *  Created on: Mar 16, 2026
 *      Author: young
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "main.h"
#include <stdio.h>

// The default address for most I2C LCD backpacks is 0x27.
// We shift it left by 1 because HAL uses 8-bit addresses.
#define LCD_ADDR (0x27 << 1)

// Function Prototypes
void LCD_Init(void);
void LCD_Cmd(uint8_t cmd);
void LCD_Data(uint8_t data);
void LCD_Clear(void);

void LCD_Update_Display(void);

#endif /* INC_LCD_H_ */
