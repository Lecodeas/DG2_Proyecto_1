/* -----------------------------------------------
 Universidad del Valle de Guatemala
 IE3054: Electrónica Digital 2
 bmp280.c
 Autor: Ian Anleu Rivera (con ayuda de ChatGPT)
 Proyecto: Lib para manejo de I2C con BMP280
 Hardware: ATMEGA328P
 Creado: 17/02/2026
 Ultima modificacion: 18/02/2026
    -----------------------------------------------
*/

#include "bmp280.h"
#include "UART.h"

BMP280_CalibData bmp280_cal;
int32_t bmp280_t_fine;

// --------- Funciones internas I2C ----------
static uint8_t bmp280_write8(uint8_t reg, uint8_t val) //Comandos de 1 byte
{
	if (!I2C_MasterStart()) return 0;
	if (!I2C_MasterWrite((BMP280_ADDR<<1) | 0)) return 0;
	if (!I2C_MasterWrite(reg)) return 0;
	if (!I2C_MasterWrite(val)) return 0;
	I2C_MasterStop();
	return 1;
}

static uint8_t bmp280_read8(uint8_t reg, uint8_t *val)  //Lecturas de 1 byte
{
	if (!I2C_MasterStart()) return 0;
	if (!I2C_MasterWrite((BMP280_ADDR<<1) | 0)) return 0;
	if (!I2C_MasterWrite(reg)) return 0;

	if (!I2C_MasterRepeatedStart()) return 0;
	if (!I2C_MasterWrite((BMP280_ADDR<<1) | 1)) return 0;

	I2C_MasterRead(val, 0);
	I2C_MasterStop();
	return 1;
}

static uint8_t bmp280_read16(uint8_t reg, uint16_t *val) //Lecturas de 2 bytes
{
	uint8_t msb, lsb;

	if (!bmp280_read8(reg, &lsb)) return 0; //Low byte
	if (!bmp280_read8(reg+1, &msb)) return 0; //High byte

	*val = (msb<<8) | lsb; //Almacena el dato en val
	return 1;
}

static uint8_t bmp280_readS16(uint8_t reg, int16_t *val) //Lecturas (signed) 2 bytes
{
	uint16_t tmp;
	if (!bmp280_read16(reg, &tmp)) return 0;  //Lo mismo pero para signed
	*val = (int16_t)tmp;
	return 1;
}

// --------- Inicialización ----------
uint8_t BMP280_Init(void)
{
	uint8_t id;

	UART_writeString("BMP:init\r\n");

	if (!bmp280_read8(0xD0, &id)) {
		UART_writeString("BMP:read8 fail\r\n");
		return 0;
	}

	UART_writeString("BMP:id=");
	UART_writeUInt(id);
	UART_writeString("\r\n");

	if (id != 0x58) {
		UART_writeString("BMP:id wrong\r\n");
		return 0;
	}

	UART_writeString("BMP:cal\r\n");

	bmp280_read16(0x88, &bmp280_cal.dig_T1);
	bmp280_readS16(0x8A, &bmp280_cal.dig_T2);
	bmp280_readS16(0x8C, &bmp280_cal.dig_T3);

	//DEBUGGING
	UART_writeString("BMP:cfg\r\n");
	
	UART_writeString("T1=");
	UART_writeUInt(bmp280_cal.dig_T1);
	UART_writeString(" T2=");
	UART_writeInt(bmp280_cal.dig_T2);
	UART_writeString(" T3=");
	UART_writeInt(bmp280_cal.dig_T3);
	UART_writeString("\r\n");

	bmp280_write8(0xF4, 0x27);
	bmp280_write8(0xF5, 0x00);

	UART_writeString("BMP:done\r\n");
	//DEBUGGING

	return 1;
}


// --------- Lectura RAW ----------
int32_t BMP280_ReadTempRaw(void)
{
	uint8_t buf[3];

	if (!I2C_MasterStart()) return 0;
	if (!I2C_MasterWrite((BMP280_ADDR<<1) | 0)) return 0; //Address del BMP
	if (!I2C_MasterWrite(0xFA)) return 0; //Comando

	if (!I2C_MasterRepeatedStart()) return 0;
	if (!I2C_MasterWrite((BMP280_ADDR<<1) | 1)) return 0;

	I2C_MasterRead(&buf[0], 1); // MSB
	I2C_MasterRead(&buf[1], 1); // LSB
	I2C_MasterRead(&buf[2], 0); // XLSB (NACK)
	I2C_MasterStop();

	int32_t raw = ((int32_t)buf[0]<<12) | ((int32_t)buf[1]<<4) | (buf[2]>>4); //Todos los datos se combinan para el dato bruto

	return raw;
}

// --------- Temperatura compensada ----------
int32_t BMP280_ReadTemperature(void) //Fórmula Bosch
{
	int32_t adc_T = BMP280_ReadTempRaw(); //Temperatura raw

	int32_t var1 = ((((adc_T>>3) - ((int32_t)bmp280_cal.dig_T1<<1))) * //Offset de sensor
	((int32_t)bmp280_cal.dig_T2)) >> 11;

	int32_t var2 = (((((adc_T>>4) - ((int32_t)bmp280_cal.dig_T1)) * //No linealidad térmica
	((adc_T>>4) - ((int32_t)bmp280_cal.dig_T1))) >> 12) *
	((int32_t)bmp280_cal.dig_T3)) >> 14;

	bmp280_t_fine = var1 + var2; //valor proporcional a temperatura real

	int32_t T = (bmp280_t_fine * 5 + 128) >> 8; // °C x100

	return T; //Retorno de decimal a 4 digitos enteros
}