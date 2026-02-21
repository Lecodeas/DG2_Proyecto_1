/*
 * ADC.h
 *
 * Created: 29/01/2026 19:36:38
 *  Author: Usuario Dell
 */ 

#ifndef ADC_H_
#define ADC_H_
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

void init_ADC(int referencia,int justificacion,int preescaler);
uint8_t readADC(int canal);
void readADC_ISR(int canal);


#endif /* ADC_H_ */