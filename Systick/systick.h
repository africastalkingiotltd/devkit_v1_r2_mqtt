/*
 * gsm_systick.h
 *
 *  Created on: Dec 13, 2019
 *      Author: kennedyotieno
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "main.h"

extern volatile uint32_t milliSecondsTick;
extern volatile uint32_t gsmTick;
extern volatile unsigned long mqttTick;

void delay(uint32_t delay_milliseconds);
void systickHandler();

#endif /* SYSTICK_H_ */
