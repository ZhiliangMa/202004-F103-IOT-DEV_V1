#ifndef __SPI1_H
#define __SPI1_H
#include "sys.h"

void SPI1_Init(void);
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler);
u8 SPI1_ReadWriteByte(u8 TxData);

#endif
