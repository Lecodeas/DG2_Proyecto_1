/* -----------------------------------------------
 Universidad del Valle de Guatemala
 IE3054: Electrónica Digital 2
 lcd.h
 Autor: Ian Anleu Rivera (basado en ejemplo de Pablo y ayuda de ChatGPT)
 Proyecto: Lib para manejo de LCD
 Hardware: ATMEGA328P
 Creado: 22/01/2026
 Ultima modificacion: 22/01/2026
    -----------------------------------------------
*/

#ifndef LCD_H_
#define LCD_H_

//HEADER FILES
#define F_CPU 16000000UL //F cpu en 16 Mhz
#include <avr/io.h> // IO regs
#include <util/delay.h> //Delays
#include <stdint.h> //Lib para enteros

//TODOS LOS PUERTOS A USAR
#define RS PD2
#define ENABLE PD3

#define LDATAPORT PORTD
#define LDATADDR DDRD
#define CONTROLPORT PORTD
#define CONTROLDDR DDRD

#define HDATAPORT PORTB
#define HDATADDR DDRB

//PROTOTIPOS DE FUNCIÓN
void lcd_setup(void);
void lcd_command(uint8_t cmd);
void lcd_write(uint8_t data);

void lcd_clear(void);
void lcd_cursor(uint8_t fila, uint8_t columna);
void lcd_writechar(char caracter);
void lcd_writestring(const char *string);


#endif /* LCD_H_ */