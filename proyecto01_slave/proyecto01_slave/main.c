//********************************************************************************
//Universidad del Valle de Guatemala
//IE2023: Programacion de Microcontroladores
//Autor: Fernando Gabriel Caballeros Cu
// proyecto01_slave.c
//Archivo: main.c
//Hardware: ATMega328P
//Created: 5/02/2026 19:14:05
//********************************************************************************
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/twi.h>
#include <stdint.h>

#include "I2C.h"
#include "servo_t1.h"
#include "stepper.h"

#define SLAVE_ADDR_B 0x30

// Comandos del master
#define CMD_READ_SENSOR         1
#define CMD_WRITE_MOTORDATA_1   2
#define CMD_WRITE_MOTORDATA_2   3

// Servo (grados)
#define SERVO_OFF   0
#define SERVO_SHORT 10
#define SERVO_MID   30
#define SERVO_LONG  45

// Stepper (velocidad)
#define STEPPER_LOW   0
#define STEPPER_MED  50
#define STEPPER_HIGH 100

// GasState enum (1 byte al master)
typedef enum {
	EMPTY = 0,
	CRITICAL = 1,
	HALF = 2,
	FULL = 3
} GasState;

// I2C buffers
// RX: cmd + servo + stepper  => 3 bytes max
#define RX_LEN_B 3
#define TX_LEN_B 1

volatile uint8_t rx_buf[RX_LEN_B];
volatile uint8_t tx_buf[TX_LEN_B];
volatile uint8_t rx_idx = 0;
volatile uint8_t tx_idx = 0;

volatile uint8_t last_cmd   = 0;
volatile uint8_t servo_cmd  = SERVO_OFF;
volatile uint8_t step_cmd   = STEPPER_LOW;

// ADC (A0)
static void adc_init(void){
	ADMUX  = (1<<REFS0); // AVcc
	ADCSRA = (1<<ADEN) | (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); // prescaler 128
}
static uint16_t adc_read_a0(void){
	ADMUX = (ADMUX & 0xF0) | 0; // CH0 = A0
	ADCSRA |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC));
	return ADC;
}

// Umbrales combustible (ajustables)
// fuel_pct: 0..100
#define TH_EMPTY     10
#define TH_CRITICAL  25
#define TH_HALF      60

static GasState fuel_classify(uint8_t fuel_pct){
	if (fuel_pct <= TH_EMPTY) return EMPTY;
	if (fuel_pct <= TH_CRITICAL) return CRITICAL;
	if (fuel_pct <= TH_HALF) return HALF;
	return FULL;
}

// Velocidad stepper: delay entre pasos (ms)
static uint8_t step_delay_ms_from_cmd(uint8_t cmd){
	if (cmd >= STEPPER_HIGH) return 3;   // rápido
	if (cmd >= STEPPER_MED)  return 8;   // medio
	return 0;                            // apagado
}

ISR(TWI_vect){
	uint8_t estado = TWSR & 0xF8;

	switch(estado){
		// SLA+W recibido
		case 0x60:
		case 0x70:
		rx_idx = 0;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;

		// Data recibido
		case 0x80:
		case 0x90:
		if (rx_idx < RX_LEN_B) rx_buf[rx_idx++] = TWDR;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;

		// STOP / repeated STOP: procesar comando
		case 0xA0:
		if (rx_idx >= 1){
			last_cmd = rx_buf[0];

			if (last_cmd == CMD_WRITE_MOTORDATA_2 && rx_idx >= 3){
				servo_cmd = rx_buf[1];   // 0/10/30/45
				step_cmd  = rx_buf[2];   // 0/50/100

				// mover servo inmediatamente (en grados)
				ServoT1_SetDeg(servo_cmd);
			}
			// CMD_READ_SENSOR no necesita payload
		}
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;

		// SLA+R recibido: mandar 1 byte
		case 0xA8:
		tx_idx = 0;
		TWDR = tx_buf[tx_idx++]; // GasState
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;

		// Si el master pidiera más, mandamos lo mismo
		case 0xB8:
		TWDR = tx_buf[0];
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;

		// Fin de transmisión
		case 0xC0:
		case 0xC8:
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;

		default:
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;
	}
}

int main(void){
	adc_init();
	ServoT1_Init();
	Stepper_Init();

	I2C_Slave_Init(SLAVE_ADDR_B);
	sei();

	uint8_t step_delay = 0;

	while (1){
		//Combustible
		uint16_t adc = adc_read_a0(); // 0..1023
		uint8_t fuel_pct = (uint8_t)(((uint32_t)adc * 100UL) / 1023UL);
		GasState gas = fuel_classify(fuel_pct);

		//Stepper
		step_delay = step_delay_ms_from_cmd(step_cmd);
		if (step_delay == 0){
			Stepper_CoilsOff();
			_delay_ms(10);
			} else {
			Stepper_StepOnce();
			for (uint8_t i = 0; i < step_delay; i++) {
				_delay_ms(1);
			}

		}

		//Preparar TX
		tx_buf[0] = (uint8_t)gas;
	}
}
