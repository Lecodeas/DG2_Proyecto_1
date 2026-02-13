/* -----------------------------------------------
 Universidad del Valle de Guatemala
 IE3054: Electrónica Digital 2
 I2C.c
 Autor: Ian Anleu Rivera
 Proyecto: Librería para comunicación I2C
 Hardware: ATMEGA328P
 Creado: 05/02/2026
 Ultima modificacion: 05/02/2026
    -----------------------------------------------
*/

#include "I2C.h"

// MASTER SETUP
void I2C_MasterSetup(unsigned long SCL_CLK, I2C_PRESC Prescaler){
	DDRC &= ~((1<<DDC4)|(1<<DDC5)); //I2C entradas en SDA y SCL
	
	//Prescaler (dependiendo del enum se configura ese prescaler en TWSR
	TWSR &= ~((1<<TWPS1)|(1<<TWPS0)); //Limpiar prescaler
	static uint8_t prescval = 0;
	switch(Prescaler){
		case I2C_PRESC_1:
			prescval = 1;
			break;
		case I2C_PRESC_4:
			TWSR |= (1<<TWPS0);
			prescval = 4;
			break;
		case I2C_PRESC_16:
			TWSR |= (1<<TWPS1);
			prescval = 16;
			break;
		case I2C_PRESC_64:
			TWSR |= (1<<TWPS1)|(1<<TWPS0);
			prescval = 64;
			break;
		default:
			TWSR &= ~((1<<TWPS1)|(1<<TWPS0)); //Limpiar prescaler
			Prescaler = I2C_PRESC_1;
			break;
	}
	
	//Velocidad
	TWBR = ((F_CPU/SCL_CLK)-16)/(2*prescval); 
	
	//Interfase I2C (TWI)
	TWCR |= (1<<TWEN);
}

// MASTER START
uint8_t I2C_MasterStart(void){
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); //Clear interrupt, Master Transmit y TWI
	while (!(TWCR & (1<<TWINT))); //Espera hasta que encienda la bandera
	
	return ((TWSR & 0xF8)==0x08); // Solo interesan los bits de estado
}

// MASTER REPEATED START
uint8_t I2C_MasterRepeatedStart(void){
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); //Clear interrupt, Master Transmit y TWI
	while (!(TWCR & (1<<TWINT))); //Espera hasta que encienda la bandera
	
	return ((TWSR & 0xF8)==0x10); // Solo interesan los bits de estado
}

// MASTER STOP
void I2C_MasterStop(void){
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); //Clear interrupt, TWI, secuencia STOP
	while (TWCR & (1<<TWSTO)); //Espera hasta que se limpie el STOP
}

// MASTER WRITE
uint8_t I2C_MasterWrite(uint8_t dato){
	static uint8_t state; // Que dato se transmitio
	
	TWDR = dato; // Cargar el dato al buffer
	TWCR = (1<<TWEN)|(1<<TWINT); //TWI, Clear interrupt
	
	while (!(TWCR & (1<<TWINT))); //Espera hasta que encienda la bandera
	
	state = TWSR & 0xF8; //Interesa solo lo que es el TWI status
	
	if ((state==0x18)||(state==0x28)){ //0x18 es SLA+W, data+W (IMPORTANTE SIEMPRE TENGO QUE RECIBIR ACK)
		return 1;
		}else{
		return state; //Estado error
	}
	
}

// MASTER READ
uint8_t I2C_MasterRead(uint8_t *buffer, uint8_t acknowledge){
	static uint8_t state; // Que dato se transmitio
	
	if (acknowledge){
		//Más datos
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA); //Clear interrupt, TWI, Envío de ACK
	}else{
		//Último dato
		TWCR = (1<<TWINT)|(1<<TWEN); //Clear interrupt, TWI, Sin ACK
	}
	
	while (!(TWCR & (1<<TWINT))); //Espera hasta que encienda la bandera
	
	state = TWSR & 0xF8; //Interesa solo lo que es el TWI status
	
	//Comprobación con Ack o sin Ack
	if (acknowledge && state != 0x50) return 0; //Data recibida con Ack
	if (!acknowledge && state != 0x58) return 0; //Data recibida sin Ack
	
	
	*buffer = TWDR; //Valor en un puntero
	return 1; //Exitoso
}

// SLAVE SETUP
void I2C_SlaveSetup(uint8_t address){
	DDRC &= ~((1<<DDC4)|(1<<DDC5)); //I2C como entradas
	
	TWAR = address<<1; //Asignar dirección (nombre en 7 bits)
	//TWAR  = (address<<1 | 0x01); //Dirección + enable de llamada general
	
	TWCR = (1<<TWEA)|(1<<TWEN)|(1<<TWIE); //Acknowledge automático, TWI, Interrupt	
}