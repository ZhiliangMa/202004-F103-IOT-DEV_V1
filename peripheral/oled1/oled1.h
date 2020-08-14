#ifndef _OLED_H
#define _OLED_H

//#include "delay.h"
#include "spi1.h"
#include "stm32f10x_gpio.h"
#include "stdlib.h"

/* 引脚工作正常测试 */
//#define GPIO_DEBUG

/* OLED接口配置 */
#define HardWare_SPI 0x01
//#define SoftWare_SPI 0x02
//#define SoftWare_IIC 0x04

/* OLED引脚配置 */
#define OLED_SCL     GPIO_Pin_13    
#define OLED_SDA     GPIO_Pin_14   

#define OLED_D1      GPIO_Pin_7 //dat
#define OLED_D0      GPIO_Pin_5 //sck
#define OLED_RST     GPIO_Pin_6 //reset	-	PA6
#define OLED_DC      GPIO_Pin_0 //dat/cmd   PB0 
#define OLED_CS      GPIO_Pin_1 //chip select  PB1

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

#define OLED_RST_H()  GPIO_SetBits(GPIOB, OLED_RST) 
#define OLED_RST_L()  GPIO_ResetBits(GPIOB, OLED_RST) 	
#define OLED_D0_L()		GPIO_ResetBits(GPIOA, OLED_D0)	
#define OLED_D0_H()		GPIO_SetBits(GPIOA, OLED_D0)  
#define OLED_D1_L()		GPIO_ResetBits(GPIOA, OLED_D1)	
#define OLED_D1_H()		GPIO_SetBits(GPIOA, OLED_D1) 	
#define OLED_DC_L()		GPIO_ResetBits(GPIOB, OLED_DC)	
#define OLED_DC_H()		GPIO_SetBits(GPIOB, OLED_DC) 	  
#define OLED_CS_L()		GPIO_ResetBits(GPIOC, OLED_CS)
#define OLED_CS_H()	  GPIO_SetBits(GPIOC, OLED_CS) 	 

/* OLED函数声明 */
void OLED_Init(void);
void OLED_CLS(void);
void OLED_4num(u8 x,u8 y,int number);
void OLED_3num(u8 x,u8 y,u16 number);
void OLED_2num(u8 x,u8 y,u8 number);
void OLED_Num(u8 x,u8 y,u8 asc);
void OLED_P6x8Str(u8 x,u8 y,u8 ch[]);
void OLED_P8x16Str(u8 x,u8 y,u8 ch[]);
void OLED_P14x16Str(u8 x,u8 y,u8 ch[]);
void OLED_Print(u8 x, u8 y, u8 ch[]);
void OLED_PutPixel(u8 x,u8 y);
void OLED_Rectangle(int16_t acc_x,int16_t acc_y);

void Draw_Logo(void);
void Draw_BMP(u8 x0,u8 y0,u8 x1,u8 y1,u8 bmp[]); 


void OLED_DisOnOff(u8 d);
void OLED_BkgLight(u8 d);

void OLED_Fill(u8 dat);
void Dis_String(u8 y, u8 x, u8 ch[]);
void Dis_Char(u8 y,u8 x,u8 asc);
void Dis_Num(u8 y, u8 x, u16 num,u8 N);
void Dis_Float(u8 Y,u8 X,double real,u8 N);
void Dis_Float2(u8 Y,u8 X,double real,u8 N1,u8 N2);
void OLED_P6x8Num_8bit(u8 x,u8 y,u8 Number); 
void OLED_Num5(u8 x,u8 y,u16 number);
void OLED_WhiteLine(u8 line , u8 num); //line:0~7
void OLED_ClrLine(u8 line);

//OLED_DisOnOff(0);  //1,开启(白屏);0,关闭(黑屏)
//OLED_BkgLight(2);

#endif

