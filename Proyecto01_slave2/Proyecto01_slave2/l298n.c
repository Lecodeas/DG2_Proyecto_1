/*
 * l298n.c
 *
 * Created: 15/02/2026 22:47:17
 *  Author: Usuario Dell
 */ 
#include <avr/io.h>
#include <stdint.h>

#define IN1_PD  PD4  
#define IN2_PD  PD5 
#define IN3_PD  PD7 
#define IN4_PB  PB0  
#define ENA_PD PD6
#define ENB_PB PB1 

void L298N_Init(void){
	// Pines de dirección como salida
	DDRD |= (1<<IN1_PD) | (1<<IN2_PD) | (1<<IN3_PD) | (1<<ENA_PD);
	DDRB |= (1<<IN4_PB) | (1<<ENB_PB);

	// ---- PWM1 en D6 (OC0A) Timer0 Fast PWM ----
	TCCR0A = (1<<WGM01) | (1<<WGM00) | (1<<COM0A1); // Fast PWM, clear OC0A on compare
	TCCR0B = (1<<CS01);                             // prescaler 8
	OCR0A = 0;

	// ---- PWM2 en D9 (OC1A) Timer1 Fast PWM 8-bit ----
	TCCR1A = (1<<COM1A1) | (1<<WGM10);
	TCCR1B = (1<<WGM12) | (1<<CS11);                // prescaler 8
	OCR1A = 0;
}

void L298N_ForwardPWM(uint8_t pwm){
	// Forward ambos motores:
	// Motor1: IN1=1 IN2=0
	// Motor2: IN3=1 IN4=0
	PORTD |=  (1<<IN1_PD) | (1<<IN3_PD);
	PORTD &= ~(1<<IN2_PD);
	PORTB &= ~(1<<IN4_PB);

	OCR0A = pwm; // PWM motor 1 (D6)
	OCR1A = pwm; // PWM motor 2 (D9)
}
