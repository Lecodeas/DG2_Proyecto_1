/* -----------------------------------------------
 Universidad del Valle de Guatemala
 IE3054: Electrónica Digital 2
 UART.h
 Autor: Ian Anleu Rivera
 Proyecto: Lab 2 Digital 2
 Hardware: ATMEGA328P
 Creado: 29/01/2024
 Ultima modificacion: 17/02/2026
    ----------------------------------------------- Basado en librería personal de Micros 1
*/


#ifndef UART_H_
#define UART_H_

#ifndef F_CPU
#define F_CPU 16000000UL //F cpu en 16 Mhz
#endif

#include <avr/io.h>//IO registros
#include <stdint.h> //Lib para enteros

void UART_setup(uint32_t baudrate);

void UART_writeChar(char car);
void UART_writeString(const char *str);

char UART_readChar(void);
uint8_t UART_listo(void);

//DEBUGGING
void UART_writeInt(int32_t v);
void UART_writeUInt(uint32_t v);


#endif /* UART_H_ */