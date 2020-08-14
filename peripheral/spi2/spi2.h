#ifndef __SPI_H
#define __SPI_H

#include "sys.h"

#define SPI2_PORT	     GPIOB
#define RCC_SPI2    	 RCC_APB2Periph_GPIOB
#define SCK_PIN        GPIO_Pin_13
#define MISO_PIN       GPIO_Pin_14
#define MOSI_PIN       GPIO_Pin_15
#define NSS_PIN        GPIO_Pin_12

void SPI1_Init(void);
void SPI2_Init(void);
void SPI1_Speed(u8 speed);
void SPI2_Speed(u8 speed);
void SPI1_ReadWriteByte(u8 dat);
u8 SPI2_ReadWriteByte(u8 dat);
		 
#endif

