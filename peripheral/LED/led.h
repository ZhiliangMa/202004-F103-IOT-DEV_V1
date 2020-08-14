#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
/************************************************

************************************************/
//#define LED0 PBout(5)// PB5
//#define LED1 PEout(5)// PE5	

//#define LED1_ON       GPIO_ResetBits(GPIOC, GPIO_Pin_13)         //共阳极，拉低PC13电平，点亮LED1
//#define LED1_OFF      GPIO_SetBits(GPIOC, GPIO_Pin_13)           //共阳极，拉高PC13电平，熄灭LED1

#define LED1_ON       GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define LED1_OFF      GPIO_SetBits(GPIOB, GPIO_Pin_8)

#define LED2_ON       GPIO_ResetBits(GPIOB, GPIO_Pin_5)
#define LED2_OFF      GPIO_SetBits(GPIOB, GPIO_Pin_5)

#define LED3_ON       GPIO_SetBits(GPIOB, GPIO_Pin_4)
#define LED3_OFF      GPIO_ResetBits(GPIOB, GPIO_Pin_4)

#define LED4_ON       GPIO_SetBits(GPIOB, GPIO_Pin_3)
#define LED4_OFF      GPIO_ResetBits(GPIOB, GPIO_Pin_3)

void LED_Init(void);//初始化

		 				    
#endif
