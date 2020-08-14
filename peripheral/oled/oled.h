#ifndef __OLED_H
#define __OLED_H
//#include "sys.h"
//#include "stdlib.h"
#include "stm32f10x.h"
#include "delay.h"
#include "spi1.h"
#include "stdlib.h"
#include "structure.h"
#include "stdio.h"

extern uint8_t OLED_GRAM[][128];
extern uint8_t switchIFS[][128];
extern uint8_t weatherIFS[][128];
extern uint8_t blendIFS[][128];

/*
 * OLED_RST -> PA6
 * DC 		-> PB0
 * CS 		-> PB1
 */
#define OLED_RST_Clr() GPIO_ResetBits(GPIOA,GPIO_Pin_6)//RST	PA6
#define OLED_RST_Set() GPIO_SetBits(GPIOA,GPIO_Pin_6)

#define OLED_RS_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_0)//A0/DC	PB0
#define OLED_RS_Set() GPIO_SetBits(GPIOB,GPIO_Pin_0)

#define OLED_CS_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_1)//CS		PB1
#define OLED_CS_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_1)

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据


// OLED控制用函数
void OLED_WR_Byte(unsigned char  dat,unsigned char  cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);

void ma_OLED_Gram_Clear(uint8_t (*ram)[128]);
void load_Img(uint8_t x, uint8_t y, uint8_t (*ram)[128], const unsigned char *p, uint8_t imgWidth, uint8_t imgHeight);
void ma_OLED_Refresh_Gram(uint8_t (*ram)[128]);
void blend_Gram_H(uint8_t x, uint8_t (*ram1)[128], uint8_t (*ram2)[128], uint8_t (*blend)[128]);
void blend_Gram_V(uint8_t y, uint8_t d, uint8_t w, uint8_t h, uint8_t *img1, uint8_t *img2, uint8_t *blend);
void ma_OLED_Gram_Test(uint8_t x, uint8_t y, uint8_t xram);
void OLED_show1608Num(uint8_t x, uint8_t y, uint8_t (*ram)[128], uint16_t num, uint8_t len);
void OLED_show1608String(uint8_t x, uint8_t y, uint8_t (*ram)[128], char* p, uint8_t len);

void OLED_Init(void);

#endif  
	 



