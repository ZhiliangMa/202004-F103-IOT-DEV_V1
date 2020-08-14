#ifndef __NET_TIME_H
#define __NET_TIME_H

#include "stm32f10x.h"
#include "structure.h"
#include "stdlib.h"
#include "string.h"

void GMT_ToBeijingTime(char *strP, struct xTime *Time);
void TimePlusOneSecond(struct xTime *Time);

#endif
