/*
 * hcsr04.c
 *
 * Created: 15/02/2026 22:48:29
 *  Author: Usuario Dell
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

// Pines fijos
#define TRIG_PD PD3
#define ECHO_PD PD2

void HCSR04_Init(void) {
	DDRD |= (1<<TRIG_PD);   // TRIG output
	DDRD &= ~(1<<ECHO_PD);  // ECHO input
	PORTD &= ~(1<<TRIG_PD);
}

uint16_t HCSR04_ReadCm(void) {
	// Trigger pulse 10us
	PORTD &= ~(1<<TRIG_PD);
	_delay_us(2);
	PORTD |=  (1<<TRIG_PD);
	_delay_us(10);
	PORTD &= ~(1<<TRIG_PD);

	// Wait echo high (timeout)
	uint32_t to = 60000UL;
	while (!(PIND & (1<<ECHO_PD))) {
		if (--to == 0) return 0;
		_delay_us(1);
	}

	// Measure high width in us-ish (timeout)
	uint32_t width = 0;
	to = 30000UL;
	while (PIND & (1<<ECHO_PD)) {
		if (--to == 0) break;
		width++;
		_delay_us(1);
	}

	
	return (uint16_t)(width / 58UL);
}
