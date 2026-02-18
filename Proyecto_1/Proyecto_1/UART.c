/* -----------------------------------------------
 Universidad del Valle de Guatemala
 IE3054: Electrónica Digital 2
 UART.c
 Autor: Ian Anleu Rivera
 Proyecto: Lab 2 Digital 2
 Hardware: ATMEGA328P
 Creado: 29/01/2024
 Ultima modificacion: 29/012026
    ----------------------------------------------- Basado en librería personal de Micros 1
*/

#include "UART.h"

void UART_setup(uint32_t baudrate){
 	uint16_t ubrr = (F_CPU / 16UL / baudrate)-1; //Cálculo automatizado
 	UBRR0H = ubrr>>8;     //High Nibble
 	UBRR0L = ubrr;        //Low Nibble
 	
 	UCSR0A = 0;
 	
 	// Enable de RX y TX
 	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
 
 	// 8 data bits, sin pariedad, 1 stopbit
 	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
 }

void UART_writeChar(char c)
{
	while (!(UCSR0A & (1 << UDRE0))); //Esperar que esté listo
	UDR0 = c; //Enviar
}

void UART_writeString(const char *str){
	while(*str){
		UART_writeChar(*str++); //Recorre el str y escribe el caracter, luego posiciona al siguiente
	}
}

uint8_t UART_listo(void){
	return (UCSR0A & (1 << RXC0)); //Flag de completo
}

char UART_readChar(void){
	while (!UART_listo()); //Se traba hasta que detecta que se completó una transmisión
	return UDR0; // Retorno lo que está en el registro
}

//DEBUGGING
void UART_writeInt(int32_t v)
{
	char buf[12];
	ltoa(v, buf, 10);
	UART_writeString(buf);
}

void UART_writeUInt(uint32_t v)
{
	char buf[12];
	ultoa(v, buf, 10);
	UART_writeString(buf);
}
