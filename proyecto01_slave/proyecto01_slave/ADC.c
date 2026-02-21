/*
 * ADC.c
 *
 * Created: 29/01/2026 19:36:51
 *  Author: Usuario Dell
 */ 

#include "ADC.h"

void init_ADC(int referencia,int justificacion,int preescaler){
	ADMUX=0;
	ADCSRA=0;
	//referencia del ADC - 0
	if (referencia==0){
		//el voltaje de referencia es de 5 v
		ADMUX |= (1<<REFS0);
		} else if (referencia==1){
		//el voltaje de referencia es de 1.1 v
		ADMUX |= (1<<REFS0)|(1<<REFS1);
	}
	
	if (justificacion==0){
		//izquierda
		ADMUX |= (1<<ADLAR);
		} else if (justificacion==1){
		//derecha
		ADMUX &= ~(1<<ADLAR);
	}
	
	//preescaler
	if (preescaler==2){
		//Preescaler 2 -> frecuencia del adc = Foscilador/2
		ADCSRA |= (1<<ADPS0);
		} else if (preescaler==4){
		//Preescaler 2 -> frecuencia del adc = Foscilador/2
		ADCSRA |=(1<<ADPS1);
		}else if (preescaler==8){
		//Preescaler 2 -> frecuencia del adc = Foscilador/8
		ADCSRA |=(1<<ADPS1) | (1<<ADPS0);
		}else if (preescaler==16){
		//Preescaler 2 -> frecuencia del adc = Foscilador/16
		ADCSRA |= (1<<ADPS2);
		}else if (preescaler==32){
		//Preescaler 2 -> frecuencia del adc = Foscilador/32
		ADCSRA |= (1<<ADPS2)|(1<<ADPS0);
		}else if (preescaler==64){
		//Preescaler 2 -> frecuencia del adc = Foscilador/64
		ADCSRA |= (1<<ADPS2) | (1<<ADPS1);
		} else if (preescaler==128){
		//Preescaler 2 -> frecuencia del adc = Foscilador/128
		ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	}
	
	//Enciendo el ADC
	ADCSRA |= (1 << ADEN);
}

uint8_t readADC(int canal){
	
	//deshabilitando la interrupciOn del ADC
	//ADCSRA &= ~(1<<ADIE);
	
	//Borramos el MUX DEL ADC
	ADMUX &= ~(1<<MUX3)& ~(1<<MUX2) & ~(1<<MUX1) & ~(1<<MUX0);
	
	//usar el puerto deseado
	if (canal==0){
		//A0
		ADMUX &= ~(1<<MUX3)& ~(1<<MUX2) & ~(1<<MUX1) & ~(1<<MUX0);
		}else if (canal==1){
		//A1
		ADMUX |= (1<<MUX0);
		}else if (canal==2){
		//A2
		ADMUX |= (1<<MUX1);
		}else if (canal==3){
		//A3
		ADMUX |= (1<<MUX1)|(1<<MUX0);
		}else if (canal==4){
		//A4
		ADMUX |= (1<<MUX2);
		} else if (canal==5){
		//A5
		ADMUX |= (1<<MUX2)|(1<<MUX0);
		} else if (canal==6){
		//A6
		ADMUX |= (1<<MUX2)|(1<<MUX1);
	}
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC)) {
	}
	uint8_t valorADC_H = ADCH;
	return valorADC_H;
}


void readADC_ISR(int canal){
	
	ADCSRA |= (1<<ADIE);
	
	ADMUX &= ~(1<<MUX3)& ~(1<<MUX2) & ~(1<<MUX1) & ~(1<<MUX0);
	
	//usamos el puerto deseado
	if (canal==0){
		//A0
		ADMUX &= ~(1<<MUX3)& ~(1<<MUX2) & ~(1<<MUX1) & ~(1<<MUX0);
		}else if (canal==1){
		//A1
		ADMUX |= (1<<MUX0);
		}else if (canal==2){
		//A2
		ADMUX |= (1<<MUX1);
		}else if (canal==3){
		//A3
		ADMUX |= (1<<MUX1)|(1<<MUX0);
		}else if (canal==4){
		//A4
		ADMUX |= (1<<MUX2);
		} else if (canal==5){
		//A5
		ADMUX |= (1<<MUX2)|(1<<MUX0);
		} else if (canal==6){
		//A6
		ADMUX |= (1<<MUX2)|(1<<MUX1);
	}
	
	//INICIAMOS LECTURA
	ADCSRA |= (1 << ADSC);
}