#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <util/delay.h>

// LCD Definitions
#define LCD_RS_PIN 8
#define LCD_EN_PIN 9
#define LCD_DATA_PORT PORTC

// Function prototypes
void lcdCommand(uint8_t command);
void lcdData(uint8_t data);
void lcdInit();
void lcdClear();
void lcdSetCursor(uint8_t row, uint8_t col);
void lcdWriteString(const char *str);
void lcdWriteNumber(uint16_t num);

#endif /* LCD_H */
