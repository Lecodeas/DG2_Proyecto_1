/*
 * l298n.h
 *
 * Created: 15/02/2026 22:46:49
 *  Author: Usuario Dell
 */ 


#ifndef L298N_H_
#define L298N_H_

#pragma once
#include <stdint.h>

void L298N_Init(void);
void L298N_ForwardPWM(uint8_t pwm);

#endif /* L298N_H_ */