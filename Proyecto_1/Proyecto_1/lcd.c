/* -----------------------------------------------
 Universidad del Valle de Guatemala
 IE3054: Electrónica Digital 2
 lcd.c
 Autor: Ian Anleu Rivera (basado en ejemplo de Pablo y ayuda de ChatGPT)
 Proyecto: Lib para manejo de LCD
 Hardware: ATMEGA328P
 Creado: 22/01/2026
 Ultima modificacion: 22/01/2026
    -----------------------------------------------
*/

#include "lcd.h" //El "self"

//SETUP DE LCD
void lcd_setup(void){
	CONTROLDDR |= (1<<RS) | (1<<ENABLE); //Enable de LCD y modo de comando
	LDATADDR |= 0xF0;
	HDATADDR |= 0x0F; //Datapins como outputs (recordar no hacer en el setup de main.c)
	
	_delay_ms(40); //requerimiento para incialización de LCD
	
	lcd_command(0x38); //MODO 8 BITS, 2 LINEAS
	lcd_command(0x0C); //Display ON
	lcd_command(0x06); //Auto incrementar cursor
	lcd_clear();
}

//FUNCIONES ESPECIALES DE LCD
static void pulsos(void){  //Permite al LCD leer almacenar los datos en ciclos que requiere
	CONTROLPORT |= (1<<ENABLE);
	_delay_us(1);
	CONTROLPORT &= ~(1<<ENABLE);
	_delay_us(100);
}

static void write_bus(uint8_t val){
	LDATAPORT &= 0x0F;
	LDATAPORT |= (val<<4); //Shift del valor para el Low Nibble
	
	HDATAPORT &= 0xF0;
	HDATAPORT |= (val>>4); //Shift del valor para el High Nibble
	
	pulsos(); //Procesamiento de datos
}


//ACCIONES EJECUTABLES PARA ESCRITURA DE DATOS
void lcd_command(uint8_t cmd){ //Manejo de comandos especializados para el LCD
	CONTROLPORT &= ~(1<<RS); //Registro de comandos
	write_bus(cmd);
	_delay_ms(2); //cubre el tiempo que requiere el LCD para procesar el comando
}

void lcd_write(uint8_t data){
	CONTROLPORT |= (1<<RS); //Registro de Data
	write_bus(data); //escribir la data
	_delay_ms(2); //Tiempo para procesamiento
}

void lcd_clear(void){ //Limpiar el LCD de datos
	lcd_command(0x01); //Comando para limpiar el Display y reset de cursor
	_delay_ms(3); //Tiempo muerto
}

void lcd_cursor(uint8_t fila, uint8_t columna){
	uint8_t direccion = (fila==0) ? 0x00:0x40; //Si es fila 0, posicionamiento en el primer registro. Si es fila 1 posicionamiento hasta el 64.
	lcd_command(0x80 | (direccion+columna)); //el comando de posicionamiento de DDRAM se le añade la dirección + la columna
}

void lcd_writechar(char caracter){
	lcd_write(caracter); //Literalmente solo escribo el caracter que recibo
}

void lcd_writestring(const char *string){
	while (*string){ //Por cada elemento en el string,                         -> Hasta llegar al final \0
		lcd_write(*string++); //escribe el caracter e incrementa el puntero
	}
}