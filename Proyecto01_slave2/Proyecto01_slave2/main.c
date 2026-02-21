//********************************************************************************
//Universidad del Valle de Guatemala
//IE2023: Programacion de Microcontroladores
//Autor: Fernando Gabriel Caballeros Cu
//Proyecto01_slave2.c
//Archivo: main.c
//Hardware: ATMega328P
//Created: 12/02/2026 19:24:09
//********************************************************************************

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/twi.h>
#include <stdint.h>

#include "I2C.h"
#include "hcsr04.h"
#include "l298n.h"

#define SLAVE_ADDR 0x40

// LEDs
#define LED_OUT_PB  PB2
#define LED_FAR_PB  PB3
#define LED_MID_PB  PB4
#define LED_NEAR_PB PB5

// Rangos
#define RANGE_MAX_CM 50
#define NEAR_MAX_CM  10
#define MID_MAX_CM   30
#define MIN_VALID_CM 2

// Protocolo master
#define CMD_READ_SENSOR         1
#define CMD_WRITE_MOTORDATA_1   2
#define CMD_WRITE_MOTORDATA_2   3

// DC niveles
#define DC_OFF  0
#define DC_LOW  25
#define DC_HALF 50
#define DC_HIGH 100

// RX: cmd + (opcional) dato
#define RX_LEN 2
#define TX_LEN 1

volatile uint8_t rx_buf[RX_LEN];
volatile uint8_t tx_buf[TX_LEN];
volatile uint8_t rx_idx = 0;
volatile uint8_t tx_idx = 0;

volatile uint8_t last_cmd = 0;
volatile uint8_t dc_level = DC_HIGH;  // default

static uint8_t pctToPwm(uint8_t pct){
	if(pct >= 100) return 255;
	return (uint8_t)((uint16_t)pct * 255 / 100);
}

static void leds_init(void){
	DDRB |= (1<<LED_OUT_PB)|(1<<LED_FAR_PB)|(1<<LED_MID_PB)|(1<<LED_NEAR_PB);
	PORTB &= ~((1<<LED_OUT_PB)|(1<<LED_FAR_PB)|(1<<LED_MID_PB)|(1<<LED_NEAR_PB));
}
static void leds_set(uint8_t s){
	PORTB &= ~((1<<LED_OUT_PB)|(1<<LED_FAR_PB)|(1<<LED_MID_PB)|(1<<LED_NEAR_PB));
	if (s==0) PORTB |= (1<<LED_OUT_PB);
	else if (s==1) PORTB |= (1<<LED_FAR_PB);
	else if (s==2) PORTB |= (1<<LED_MID_PB);
	else PORTB |= (1<<LED_NEAR_PB);
}
static uint8_t classify(uint16_t cm, uint8_t *valid){
	if (cm==0 || cm<MIN_VALID_CM){ *valid=0; return 0; }
	*valid=1;
	if (cm>RANGE_MAX_CM) return 0;
	if (cm<=NEAR_MAX_CM) return 3;
	if (cm<=MID_MAX_CM)  return 2;
	return 1;
}

ISR(TWI_vect){
	uint8_t estado = TWSR & 0xF8;

	switch(estado){
		case 0x60:
		case 0x70:
		rx_idx = 0;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;

		case 0x80:
		case 0x90:
		if (rx_idx < RX_LEN) rx_buf[rx_idx++] = TWDR;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;

		case 0xA0: // STOP o repeated STOP
		if (rx_idx >= 1) {
			last_cmd = rx_buf[0];

			if (last_cmd == CMD_WRITE_MOTORDATA_1 && rx_idx >= 2) {
				dc_level = rx_buf[1];

				// no se detiene
				uint8_t scale = (dc_level == DC_OFF) ? DC_LOW : dc_level; // 25/50/100
				L298N_ForwardPWM(pctToPwm(scale));
			}

			// CMD_READ_SENSOR no requiere acción aquí
		}
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;

		case 0xA8: // SLA+R recibido
		tx_idx = 0;
		TWDR = tx_buf[tx_idx++]; // 1 byte: state
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;

		case 0xB8:
		TWDR = tx_buf[0];
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;

		// IMPORTANTES para cuando el master hace NACK al final (1 byte)
		case 0xC0: // Data transmitted, NACK received
		case 0xC8: // Last data transmitted, ACK received
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;

		default:
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;
	}
}

int main(void){
	leds_init();
	HCSR04_Init();
	L298N_Init();
	L298N_ForwardPWM(255);   // forward 100%


	I2C_Slave_Init(SLAVE_ADDR);
	sei();

	while(1){
		uint16_t a = HCSR04_ReadCm(); _delay_ms(20);
		uint16_t b = HCSR04_ReadCm(); _delay_ms(20);
		uint16_t c = HCSR04_ReadCm();

		uint16_t cm;
		if ((a>=b && a<=c) || (a>=c && a<=b)) cm = a;
		else if ((b>=a && b<=c) || (b>=c && b<=a)) cm = b;
		else cm = c;

		uint8_t valid=0;
		uint8_t state = classify(cm, &valid);
	if (dc_level == DC_OFF) L298N_ForwardPWM(pctToPwm(DC_LOW));

		leds_set(state);

		cli();
		tx_buf[0] = state;   // SOLO 0..3 (OUT/FAR/MID/CLOSE)
		sei();

		_delay_ms(60);
	}
}

