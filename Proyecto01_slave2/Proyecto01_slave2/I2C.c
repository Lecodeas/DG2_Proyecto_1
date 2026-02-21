/*
 * I2C.c
 *
 * Created: 5/02/2026 19:29:08
 *  Author: Usuario Dell
 */ 

#include "I2C.h"

//Funcion para inicializar I2C Maestro
void I2C_Master_Init(unsigned long SCL_Clock, uint8_t Prescaler){
	DDRC &= ~((1<<DDC4)|(1<<DDC5));
	//Seleccionamos el valor de lo bits para el prescaler del registro TWSR
	switch (Prescaler){
		case 1:
			TWSR &= ~((1<<TWPS1)|(1<<TWPS0));
		break;
		case 4:
			TWSR &= ~(1<<TWPS1);
			TWSR |= (1<<TWPS0);
		break;
		case 16:
			TWSR &= ~(1<<TWPS0);
			TWSR |= (1<<TWPS1);
		break;
		case 64:
			TWSR |= (1<<TWPS1)|(1<<TWPS0);
		break;
		default:
			TWSR &= ~((1<< TWPS1)|(1<<TWPS0));
			Prescaler = 1;
		break;
	}
	TWBR = ((F_CPU/SCL_Clock)-16)/(2*Prescaler); //Calcula la velocidad
	TWCR |= (1<<TWEN); //Activa la interfase (TWI) I2C
}
//Funcion de inicio de la comunicacion I2C
uint8_t I2C_Master_Start(void){
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); //Master, Reiniciar bandera de Int, Condicion de start
	while (!(TWCR & (1<<TWINT))); //Esperamos a que se encienda la bandera
	return ((TWSR & 0xF8) == 0x08); //Nos quedamos con los bits de estado TWI Status y revisamos
}
//Funcion de reinicio de la comunicacion I2C
uint8_t I2C_Master_RepeatedStart(void){
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); //Master, Reiniciar bandera de Int, Condicion de start
	while (!(TWCR & (1<<TWINT))); //Esperamos a que se encienda la bandera
	return ((TWSR & 0xF8) == 0x10); //Nos quedamos con los bits de estado TWI Status y revisamos
}
//Funcion de parada de la comunicacion I2C
void I2C_Master_Stop(void){
	TWCR =(1<<TWEN)|(1<<TWINT)|(1<<TWSTO); //Inicia el envio secuencia parada STOP
	while(TWCR &(1<<TWSTO)); //Esperamos a que el bit se limpie	
}
//Funcion de transmision de datos del maestro al esclavo
//esta funcion devolvera un 0 si el esclavo a recibido el dato
uint8_t I2C_Master_Write(uint8_t dato){
	 uint8_t estado;
	 TWDR = dato; //Cargar el dato
	 TWCR = (1<<TWEN)|(1<<TWINT); //Nos quedamos unicamente con los bits de estado TWI Status
	 
	 while(!(TWCR & (1<< TWINT))); //espera al flag TWINT
	 estado = TWSR & 0xF8; //Nos quedamos unicamente con los bits de estado TWI Status
	 //Verificar si se transmitio una SLA + W con ACK, o un dato con ACK
	// 0x18: SLA+W ACK
	// 0x28: DATA ACK
	// 0x40: SLA+R ACK
	if(estado == 0x18 || estado == 0x28 || estado == 0x40){
		return 1;
	}else{
		return estado;
	}
}
//Funcion de recepcion de datos enviados por el esclavo al maestro
//esta funcion es para leer los datos que estan en el esclavo
uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack){
	uint8_t estado;
	
	if(ack){
		//ACK: quiero mas datos
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA); //habilitamos interfase I2C con envio ACK
	}else{
		//NACK: ultimo byte
		TWCR = (1<<TWINT)|(1<<TWEN); //habilitamos interfase I2C sin envio de ACK (NACK)
	}
	
	while (!(TWCR & (1<<TWINT))); //Espera al flag TWINT
	
	estado = TWSR & 0xF8; //nos quedamos unicamente con los bits de estado TWI Status
	//Verificar si se recibio Dato con ACK o sin ACK
	if(ack && estado != 0x50) return 0; //Data recibida, ACK
	if(!ack && estado != 0x58) return 0; //Data recibida, NACK
	
	*buffer = TWDR; //obtenemos el resultado en el registro de datos
	return 1;
}
//Funcion para inicializar I2C Esclavo
void I2C_Slave_Init(uint8_t address){
	DDRC &= ~((1<<DDC4)|(1<<DDC5)); //pines de I2C como entradas
	TWAR = (address << 1 | 0x01); //se asigna la direccion que tendra y habilita llamada general
	
	//Se habilita la interfaz, ACK automatico, se habilita la ISR
	TWCR = (1<< TWEA)|(1<<TWEN)|(1<<TWIE);
}