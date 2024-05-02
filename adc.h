#ifndef _ADC_H_
#define _ADC_H_
#include "stm32f10x.h"                  // Device header

void adc_1_init();
void adc_2_init();
uint16_t adc_1_get_value(void);
uint16_t adc_2_get_value(void);
int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);

#endif