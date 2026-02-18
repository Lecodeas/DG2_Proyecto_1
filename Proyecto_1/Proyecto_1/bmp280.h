/* -----------------------------------------------
 Universidad del Valle de Guatemala
 IE3054: Electrónica Digital 2
 bmp280.h
 Autor: Ian Anleu Rivera (con ayuda de ChatGPT)
 Proyecto: Lib para manejo de I2C con BMP280
 Hardware: ATMEGA328P
 Creado: 17/02/2026
 Ultima modificacion: 18/02/2026
    -----------------------------------------------
*/

#ifndef BMP280_H_
#define BMP280_H_

#ifndef F_CPU
#define F_CPU 16000000UL //F cpu en 16 Mhz
#endif

#include <avr/io.h>
#include <stdint.h>
#include "I2C.h"

// Dirección I2C BMP280
#define BMP280_ADDR 0x76


// Estructura de calibración
typedef struct {
	uint16_t dig_T1;
	int16_t  dig_T2;
	int16_t  dig_T3;
} BMP280_CalibData;

// Variables globales
extern BMP280_CalibData bmp280_cal;
extern int32_t bmp280_t_fine;

// API
uint8_t BMP280_Init(void);
int32_t BMP280_ReadTempRaw(void);
int32_t BMP280_ReadTemperature(void); // devuelve °C x100




#endif /* BMP280_H_ */