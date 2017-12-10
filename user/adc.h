#ifndef _ADC_H_
#define _ADC_H_


#include "stm32f4xx.h"

extern vs16 ADC2ConvertedValue[4];
extern vs16 ADC3ConvertedValue[4];

void Adc1Init(void);//可调电阻电压采样
void Adc2Init(void);//电机A电流采样
void Adc3Init(void);//电机B电流采样

#endif
