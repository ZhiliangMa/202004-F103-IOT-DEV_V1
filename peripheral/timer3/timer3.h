#ifndef __TIMER3_H
#define __TIMER3_H

#include "stm32f10x.h"

void Wifi_RxFlag_Clear(void);
void Tim3_Counter_Clear(void);
void Timer3_Configuration(uint16_t xms);

#endif
