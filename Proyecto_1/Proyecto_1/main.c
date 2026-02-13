/* -----------------------------------------------
 Universidad del Valle de Guatemala
 IE3054: Electrónica Digital 2
 main.c
 Autor: Ian Anleu Rivera (Basado en el ejemplo en clase de Pablo)
 Proyecto: Proyecto 1
 Hardware: ATMEGA328P
 Creado: 05/02/2026
 Ultima modificacion: 12/02/2026
    -----------------------------------------------
*/


//HEADER FILES
#define F_CPU 16000000UL //F cpu en 16 Mhz
#include <avr/io.h> // IO regs
#include <util/delay.h> //Delays
#include <stdint.h> //Lib para enteros
#include <avr/interrupt.h> //Lib para interrupciones

//Self made libs
#include "I2C.h"

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
//SENSOR I2C


//COMANDOS SLAVES
#define CMD_READ_SENSOR 1
#define CMD_WRITE_MOTORDATA_1 2
#define CMD_WRITE_MOTORDATA_2 3

//COMANDOS I2C


//Clasificaciones de Temperatura
typedef enum{
	COLD_CRITICAL,
	COLD,
	STABLE,
	HOT,
	HOT_CRITICAL,
	};

//Clasificaciones de Gas
typedef enum{
	EMPTY,
	CRITICAL,
	HALF,
	FULL
	};

//Clasificaciones de distancia
typedef enum{
	OUT_OF_RANGE,
	FAR,
	MID,
	CLOSE
	};

// Variables recibidas
uint8_t gas_condition = 0;
uint8_t distance_condition = 0;
uint8_t temperature_condition = 0;

// Variables enviadas
uint8_t Speed_Command = 0;
uint8_t Cooling_command = 0;
uint8_t Aim_command = 0;

//Buffers de display en LCD
char displ1[17];
char displ2[17];

//Prototipos
uint8_t GetSlaveData(uint8_t SlaveAddressR, uint8_t SlaveAddressW);
void SendSlaveData(uint8_t SlaveAddressR, uint8_t SlaveAddressW, uint8_t COMMAND, uint8_t MotorValue);

int main(void)
{
    //UART Setup
	
	//LCD Setup
	lcd_setup();
	//Setup Master
	I2C_MasterSetup(100000, I2C_PRESC_16); //Master mode 100khz, prescaler 
	
	//I2C shenanigans
	
	//ESP32 shenanigans
	
    while (1) 
    {
		//Recibo de Sensor de Presión (Gas level) --------------------------------------------------------------------------------------
		gas_condition = GetSlaveData(PRESSURESLAVER,PRESSURESLAVEW);
				
		//Escribo a Motor DC (Movement) ------------------------------------------------------------------------------------------------
		SendSlaveData(DCSLAVER, DCSLAVEW, CMD_WRITE_MOTORDATA_1, DC_Data);
		
		//Recibo de Sensor de Temperatura (Temperature) I2C ----------------------------------------------------------------------------
		

		//Escribo a Motor Stepper (Cooler) ---------------------------------------------------------------------------------------------
		SendSlaveData(STEPPERSLAVER, STEPPERSLAVEW, CMD_WRITE_MOTORDATA_1, Stepper_Data);
		
		//Recibo de Sensor Ultrasónico	(Distance) -------------------------------------------------------------------------------------
		Sens_ultrasonic = GetSlaveData(ULTRASONICSLAVER,ULTRASONICSLAVEW);
		
		//Escribo a Motor Servo (Cannon) -----------------------------------------------------------------------------------------------
		SendSlaveData(SERVOSLAVER, SERVOSLAVEW, CMD_WRITE_MOTORDATA_2, Servo_Data);
		
		//Display en LCD (Data de Sensores) --------------------------------------------------------------------------------------------
		//Buffers
		snprintf(displ1, sizeof(displ1),"GAS: TEMP: DIST:");
		snprintf(displ2, sizeof(displ2),"%u.%01uL %2u.%01u° %3ucm",pressureENT,pressureDEC,temperatureENT,temperatureDEC,distance);
		
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
void SendSlaveData(uint8_t SlaveAddressR, uint8_t SlaveAddressW, uint8_t COMMAND, uint8_t MotorValue){
	//Secuencia de Master
	//Llamada a todos
	if (!I2C_MasterStart()) return; //Si no inicia, reinicia el loop
	//Selección de Slave
	if (!I2C_MasterWrite(SlaveAddressW)){
		I2C_MasterStop();
		return; //Si no recibo ACK, interrumpe todas las comunicaciones y reinicia el loop
	}
	//Comando
	I2C_MasterWrite(COMMAND);
	//Reinicio de comms
	if (!I2C_MasterRepeatedStart()){
		I2C_MasterStop();
		return; //Si no recibo el acknowledge, interrumpe todas las comunicaciones y reinicia el loop
	}
	I2C_MasterWrite(MotorValue);
	I2C_MasterStop();
	_delay_ms(2);
}