#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "lcd.h"
#ifndef F_CPU
#define F_CPU 8000000UL
#endif
// Define IO pins
#define IN1_PIN 2
#define IN2_PIN 4
#define IN3_PIN 5
#define IN4_PIN 7
#define SpeedL_PIN 3
#define SpeedR_PIN 6
#define Servo_PIN 10
#define Echo_PIN 11
#define Trig_PIN 12
#define LCD_DISP_ON 0x0C
volatile int distanceFront = 0;
volatile int distanceLeft = 0;
volatile int distanceRight = 0;
volatile int furthestDirection = 0;

void setup() {
	// Set up LCD
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	
	// Set IO pins as outputs
	DDRD |= (1 << IN1_PIN) | (1 << IN2_PIN) | (1 << IN3_PIN) | (1 << IN4_PIN)
	| (1 << SpeedL_PIN) | (1 << SpeedR_PIN) | (1 << Servo_PIN) | (1 << Trig_PIN);
	
	// Enable Timer1 overflow interrupt
	TIMSK1 |= (1 << TOIE1);
	
	// Enable external interrupt for echo pin (INT0)
	EIMSK |= (1 << INT0);
	
	// Set up Timer1 for controlling servo motor
	TCCR1A = (1 << COM1A1) | (1 << WGM11); // Fast PWM mode, non-inverted output on OC1A
	TCCR1B = (1 << CS11) | (1 << WGM13); // Prescaler of 8
	
	// Set up external interrupt (INT0) for Ultrasonic sensor echo
	EICRA = (1 << ISC01); // Trigger on falling edge of INT0
	
	// Enable global interrupts
	sei();
}

ISR(TIMER1_OVF_vect) {
	// Toggle the servo motor pin
	PORTB ^= (1 << Servo_PIN);
}

ISR(INT0_vect) {
	// Calculate the pulse duration to determine distance

	static uint16_t start_time = 0;

	if (PIND & (1 << Echo_PIN)) {
		// Rising edge, start of the pulse
		start_time = TCNT1;
		} else {
		// Falling edge, end of the pulse
		uint16_t end_time = TCNT1;
		uint16_t pulse_duration = end_time - start_time;
		
		// Convert pulse duration to distance
		uint16_t distance = pulse_duration / 58; // Conversion formula for HC-SR04 Ultrasonic sensor
		
		// Update distance variables based on the direction
		if (furthestDirection == 0) {
			distanceFront = distance;
			} else if (furthestDirection == 1) {
			distanceLeft = distance;
			} else {
			distanceRight = distance;
		}

		// Determine the furthest direction to move
		if (distanceFront >= distanceLeft && distanceFront >= distanceRight) {
			furthestDirection = 0; // Forward
			} else if (distanceLeft > distanceRight) {
			furthestDirection = 1; // Left
			} else {
			furthestDirection = 2; // Right
		}
	}
}

void moveForward() {
	PORTD |= (1 << IN1_PIN) | (1 << IN3_PIN); // Set IN1 and IN3 high
	PORTD &= ~((1 << IN2_PIN) | (1 << IN4_PIN)); // Set IN2 and IN4 low
	OCR0A = 255; // Set SpeedL_PIN PWM value
	OCR0B = 255; // Set SpeedR_PIN PWM value
}

void turnLeft() {
	PORTD &= ~((1 << IN1_PIN) | (1 << IN3_PIN)); // Set IN1 and IN3 low
	PORTD |= (1 << IN2_PIN); // Set IN2 high
	PORTD &= ~(1 << IN4_PIN); // Set IN4 low
	OCR0A = 200; // Set SpeedL_PIN PWM value
	OCR0B = 200; // Set SpeedR_PIN PWM value
}

void turnRight() {
	PORTD |= (1 << IN1_PIN); // Set IN1 high
	PORTD &= ~((1 << IN2_PIN) | (1 << IN3_PIN) | (1 << IN4_PIN)); // Set IN2, IN3, and IN4 low
	OCR0A = 200; // Set SpeedL_PIN PWM value
	OCR0B = 200; // Set SpeedR_PIN PWM value
}

int main() {
	setup();
	
	char buffer[16];
	
	while (1) {
		// Update LCD with distance readings
		lcd_clrscr();

		lcd_gotoxy(0, 0);
		sprintf(buffer, "Front: %d ", distanceFront);
		lcd_puts(buffer);
		
		lcd_gotoxy(0, 1);
		sprintf(buffer, "Left: %d ", distanceLeft);
		lcd_puts(buffer);

		lcd_gotoxy(10, 0);
		sprintf(buffer, "Right: %d ", distanceRight);
		lcd_puts(buffer);

		_delay_ms(500); // Delay for half a second
	}
}
