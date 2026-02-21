/*
 * hcsr04.h
 *
 * Created: 15/02/2026 22:48:15
 *  Author: Usuario Dell
 */ 


#ifndef HCSR04_H_
#define HCSR04_H_

#pragma once
#include <stdint.h>

void     HCSR04_Init(void);
uint16_t HCSR04_ReadCm(void);


#endif /* HCSR04_H_ */