/* -----------------------------------------------
 Universidad del Valle de Guatemala
 IE3054: Electrónica Digital 2
 I2C.h
 Autor: Ian Anleu Rivera
 Proyecto: Librería para comunicación I2C
 Hardware: ATMEGA328P
 Creado: 05/02/2026
 Ultima modificacion: 05/02/2026
    -----------------------------------------------
*/

#ifndef I2C_H_
#define I2C_H_

#ifndef F_CPU
#define F_CPU 16000000UL //F cpu en 16 Mhz
#endif

typedef enum
{
	I2C_PRESC_1,
	I2C_PRESC_4,
	I2C_PRESC_16,
	I2C_PRESC_64
}I2C_PRESC;

#include <avr/io.h> // IO regs
#include <stdint.h> //Lib para enteros

// PROTOTIPOS -------------------
// MASTER MODE
void I2C_MasterSetup(unsigned long SCL_CLK, I2C_PRESC Prescaler); // Setup inicial (master)
uint8_t I2C_MasterStart(void); //Inicia comunicación (escritura)
uint8_t I2C_MasterRepeatedStart(void); //Inicia comunicación (lectura)
void I2C_MasterStop(void); // Secuencia de stop

// FUNCIONES GENERALES
uint8_t I2C_MasterWrite(uint8_t dato); // Escritura de dato
uint8_t I2C_MasterRead(uint8_t *buffer, uint8_t acknowledge); //Lectura de dato

// SLAVE MODE
void I2C_SlaveSetup(uint8_t address); // Setup inicial (slave)

#endif /* I2C_H_ */