/* -----------------------------------------------
 Universidad del Valle de Guatemala
 IE3054: Electrónica Digital 2
 main.c
 Autor: Ian Anleu Rivera (Basado en el ejemplo en clase de Pablo)
 Proyecto: Proyecto 1
 Hardware: ATMEGA328P
 Creado: 05/02/2026
 Ultima modificacion: 18/02/2026
    -----------------------------------------------
*/


//HEADER FILES
#define F_CPU 16000000UL //F cpu en 16 Mhz
#include <avr/io.h> // IO regs
#include <util/delay.h> //Delays
#include <stdint.h> //Lib para enteros
#include <avr/interrupt.h> //Lib para interrupciones
#include <stdio.h> //Funciones de C

//Self made libs
#include "I2C.h"
#include "bmp280.h"
#include "lcd.h"
#include "UART.h"

// MASTER ONLY -----------------------------------
//SLAVE ADDRESSES
//Por Sensor
#define PRESSURESLAVER (0x30<<1) | 0x01
#define PRESSURESLAVEW (0x30<<1) & 0xFE
#define ULTRASONICSLAVER (0x40<<1) | 0x01
#define ULTRASONICSLAVEW (0x40<<1) & 0xFE

//Por Motor
#define STEPPERSLAVER (0x30<<1) | 0x01
#define STEPPERSLAVEW (0x30<<1) & 0xFE
#define SERVOSLAVER (0x30<<1) | 0x01
#define SERVOSLAVEW (0x30<<1) & 0xFE
#define DCSLAVER (0x40<<1) | 0x01
#define DCSLAVEW (0x40<<1) & 0xFE

//COMANDOS SLAVES
#define CMD_READ_SENSOR 1
#define CMD_WRITE_MOTORDATA_1 2
#define CMD_WRITE_MOTORDATA_2 3

//Clasificaciones de Temperatura
typedef enum{
	STABLE,
	HOT,
	HOT_CRITICAL
}temps;

//Clasificaciones de Gas
typedef enum{
	EMPTY,
	CRITICAL,
	HALF,
	FULL
}gaslevels;

//Clasificaciones de distancia
typedef enum{
	OUT_OF_RANGE,
	FAR,
	MID,
	CLOSE
}distances;

// Límites de Temperatura
#define TEMP_HIGH_TH 3500   // 35.00 °C
#define TEMP_CRITICAL_TH  4500   // 45.00 °C

// Condiciones de Stepper
#define STEPPER_LOW 0
#define STEPPER_MED 50
#define STEPPER_HIGH 100

// Condiciones de Servo
#define SERVO_OFF 0
#define SERVO_SHORT 10
#define SERVO_MID 30
#define SERVO_LONG 45

// Condiciones de DC
#define DC_OFF 0
#define DC_LOW 25
#define DC_HALF 50
#define DC_HIGH 100

// Variables recibidas
uint8_t gas_condition = 0;
uint8_t distance_condition = 0;
uint8_t temperature_condition = 0;

// Textos
char *gas_text = "";
char *distance_text = "";

//Variables manejo de temperatura
int32_t bmp_temp_x100 = 0;
uint8_t temp_state = 0;

// Variables enviadas
uint8_t DC_Command = 0;
uint8_t stepper_command = 0;
uint8_t servo_command = 0;

//Buffers de display en LCD
char displ1[17];
char displ2[17];

//Prototipos
uint8_t GetSlaveData(uint8_t SlaveAddressR, uint8_t SlaveAddressW);
void SendSlaveData(uint8_t SlaveAddressW, uint8_t COMMAND, uint8_t MotorValue);
uint8_t clasificarTemp(int32_t tempx100);
uint8_t stepper_from_temp(uint8_t temp_state);
//void UART_SendSensorData(uint8_t gas, uint8_t dist, uint8_t temp_ent, uint8_t temp_dec);

int main(void)
{
    //UART Setup
	UART_setup(9600);
	UART_writeString("BOOT\r\n"); //Revisar el boot
	//LCD Setup
	lcd_setup();
	//Setup Master
	I2C_MasterSetup(100000, I2C_PRESC_16); //Master mode 100khz, prescaler 
	//DEBUGGING
	UART_writeString("I2C OK\r\n");//Debugging
	
	UART_writeString("SDA=");
	UART_writeChar( (PINC & (1<<PC4)) ? '1':'0');

	UART_writeString(" SCL=");
	UART_writeChar( (PINC & (1<<PC5)) ? '1':'0');
	UART_writeString("\r\n");
	//DEBUGGING
	
	//I2C shenanigans
	BMP280_Init();
	UART_writeString("BMP280 OK\r\n");//Debugging
	
	//ESP32 shenanigans
	//Dr. Bingus

    while (1) 
    {
		//Recibo de Sensor de Presión (Gas level) --------------------------------------------------------------------------------------
		gas_condition = GetSlaveData(PRESSURESLAVER,PRESSURESLAVEW);

		UART_writeString("GAS=");//Debugging
		UART_writeUInt(gas_condition);//Debugging
		UART_writeString("\r\n");//Debugging

				
		//Escribo a Motor DC (Movement) ------------------------------------------------------------------------------------------------
		switch (gas_condition)
		{
			case EMPTY: DC_Command = DC_OFF; gas_text="EMP"; break;
			case CRITICAL: DC_Command = DC_LOW; gas_text="CRT"; break;
			case HALF: DC_Command = DC_HALF; gas_text="HLF"; break;
			case FULL: DC_Command = DC_HIGH; gas_text="FLL"; break;
			default: DC_Command = DC_OFF; gas_text="EMP";
		}
		UART_writeString("DC=");//Debugging
		UART_writeUInt(DC_Command);//Debugging
		UART_writeString("\r\n");//Debugging
		
		SendSlaveData(DCSLAVEW, CMD_WRITE_MOTORDATA_1, DC_Command);
		
		//Recibo de Sensor de Temperatura (Temperature) I2C ----------------------------------------------------------------------------
		bmp_temp_x100 = BMP280_ReadTemperature();
		UART_writeString("TEMP=");//Debugging
		UART_writeInt(bmp_temp_x100);//Debugging
		UART_writeString("\r\n");//Debugging
		
		int32_t raw = BMP280_ReadTempRaw();//Debugging

		UART_writeString("RAW=");//Debugging
		UART_writeInt(raw);//Debugging
		UART_writeString("\r\n");//Debugging
		
		temperature_condition = clasificarTemp(bmp_temp_x100);

		UART_writeString("TEMP_STATE=");//Debugging
		UART_writeUInt(temperature_condition);//Debugging
		UART_writeString("\r\n");//Debugging

		stepper_command = stepper_from_temp(temperature_condition);

		UART_writeString("STEP=");//Debugging
		UART_writeUInt(stepper_command);//Debugging
		UART_writeString("\r\n");//Debugging


		//Escribo a Motor Stepper (Cooler) ---------------------------------------------------------------------------------------------
		SendSlaveData(STEPPERSLAVEW, CMD_WRITE_MOTORDATA_1, stepper_command);
		
		//Recibo de Sensor Ultrasónico	(Distance) -------------------------------------------------------------------------------------
		distance_condition = GetSlaveData(ULTRASONICSLAVER,ULTRASONICSLAVEW);

		UART_writeString("DIST=");//Debugging
		UART_writeUInt(distance_condition);//Debugging
		UART_writeString("\r\n");//Debugging

		
		//Escribo a Motor Servo (Cannon) -----------------------------------------------------------------------------------------------
		switch (distance_condition)
		{
			case OUT_OF_RANGE: servo_command = SERVO_OFF; distance_text="OUT"; break;
			case FAR: servo_command = SERVO_LONG; distance_text="FAR"; break;
			case MID: servo_command = SERVO_MID; distance_text="MID"; break;
			case CLOSE: servo_command = SERVO_SHORT; distance_text="CLS"; break;
			default: servo_command = SERVO_OFF; distance_text="OUT";
		}
		
		UART_writeString("SERVO=");//Debugging
		UART_writeUInt(servo_command);//Debugging
		UART_writeString("\r\n");//Debugging
		
		
		SendSlaveData(SERVOSLAVEW, CMD_WRITE_MOTORDATA_2, servo_command);
		
		//Display en LCD (Data de Sensores) --------------------------------------------------------------------------------------------
		//Reinterpretación de Temperatura
		uint8_t temp_ent = bmp_temp_x100 / 100;
		uint8_t temp_dec = (bmp_temp_x100 % 100) / 10;
		//Buffers
        snprintf(displ1, sizeof(displ1), "GAS TEMP DIST");
		snprintf(displ2, sizeof(displ2), "%3s %2u.%1uC %3s", gas_text, temp_ent, temp_dec, distance_text);
		
		//Limpiar pantalla
		lcd_clear();
		
		//Reset Cursor
		lcd_cursor(0,0); //Origen
		//Desplegar el string de headers
		lcd_writestring(displ1);
		
		//Cursor Linea 2
		lcd_cursor(1,0); //Fila 2 para display de datos
		//Desplegar el string de datos
		lcd_writestring(displ2);
		
		
		//Cosas del ESP32 --------------------------------------------------------------------------------------------------------------
		//UART_SendSensorData(gas_condition, distance_condition, temp_ent, temp_dec);
		
		UART_writeString("\r\n");//Debugging
		_delay_ms(1000);
    }
}


//Funciones de programa
// LEER DE SLAVE
uint8_t GetSlaveData(uint8_t SlaveAddressR, uint8_t SlaveAddressW){
	//Secuencia de Master
	//Llamada a todos
	if (!I2C_MasterStart()) return 0; //Si no inicia, reinicia el loop
	//Selección de Slave
	if (!I2C_MasterWrite(SlaveAddressW)){
		I2C_MasterStop();
		return 0; //Si no recibo ACK, interrumpe todas las comunicaciones y reinicia el loop
	}
	//Comando
	I2C_MasterWrite(CMD_READ_SENSOR);
	//Reinicio de comms
	if (!I2C_MasterRepeatedStart()){
		I2C_MasterStop();
		return 0; //Si no recibo el acknowledge, interrumpe todas las comunicaciones y reinicia el loop
	}
	//Listo para recibir
	if (!I2C_MasterWrite(SlaveAddressR)){
		I2C_MasterStop();
		return 0; //Si no recibo ACK, interrumpe todas las comunicaciones y reinicia el loop
	}
	static uint8_t ReceivedData = 0;
	//Recibe la data
	I2C_MasterRead(&ReceivedData, 0);
	I2C_MasterStop();
	_delay_ms(2);
	return ReceivedData;
}

// ENVIAR A SLAVE
void SendSlaveData(uint8_t SlaveAddressW, uint8_t COMMAND, uint8_t MotorValue){
	//Secuencia de Master
	//Llamada a todos
	if (!I2C_MasterStart()) return; //Si no inicia, reinicia el loop
	//Selección de Slave
	if (!I2C_MasterWrite(SlaveAddressW)){
		I2C_MasterStop();
		return; //Si no recibo ACK, interrumpe todas las comunicaciones y reinicia el loop
	}
	//Comando
	if (!I2C_MasterWrite(COMMAND)){
		I2C_MasterStop();
		return; //Si no recibo ACK, interrumpe todas las comunicaciones y reinicia el loop
	}
	//Dato
	if (!I2C_MasterWrite(MotorValue)){
		I2C_MasterStop();
		return; //Si no recibo ACK, interrumpe todas las comunicaciones y reinicia el loop
	}
	I2C_MasterStop();
	_delay_ms(2);
}

//CLASIFICADORES
//Temperatura
uint8_t clasificarTemp(int32_t tempx100){
	if (tempx100 >= TEMP_CRITICAL_TH)
		return HOT_CRITICAL;
	else if (tempx100 >= TEMP_HIGH_TH)
		return HOT;
	else
		return STABLE;
}
//Stepper desde Temperatura
uint8_t stepper_from_temp(uint8_t temp_state)
{
	switch(temp_state)
	{
		case STABLE: return STEPPER_LOW;
		case HOT: return STEPPER_MED;
		case HOT_CRITICAL: return STEPPER_HIGH;
		default: return STEPPER_LOW;
	}
}


////ESP32 SHENANIGANS
//void UART_SendSensorData(uint8_t gas, uint8_t dist, uint8_t temp_ent, uint8_t temp_dec){
	//char buffer[32];
//
	//snprintf(buffer, sizeof(buffer), "G:%u,D:%u,T:%u.%u\n", gas,	dist, temp_ent, temp_dec);
//
	//UART_writeString(buffer);
//}


//Bingus