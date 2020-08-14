#include "led.h"

/************************************************

************************************************/  

/*	LED IO初始化
 *	由于使用了PA3/4端口，需要关闭JTAG功能
 *
 *	SCR1  -  PB8
 *	SCR2  -  PB5
 *	JK1   -  PB4
 *	JK2   -  PB3
 *
 *	参考文章：https://blog.csdn.net/qhw5279/article/details/72630637
 *	https://blog.csdn.net/bobbat/article/details/50910559
 *	https://blog.csdn.net/euxnijuoh/article/details/74939446
 */
//LED IO初始化
void LED_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);	//使能GPIOB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_5;	//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//根据设定参数初始化
	
	GPIO_SetBits(GPIOB,GPIO_Pin_8);		//PB.8 输出高
	GPIO_SetBits(GPIOB,GPIO_Pin_5);		//PB.5 输出高
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |  RCC_APB2Periph_AFIO, ENABLE);//使能PORTB口时钟、复用引脚PB3、PB4引脚
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭JTAG功能(PB3/4)，只使用SWD(PA13/14)调试
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_3;	//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//根据设定参数初始化
	
	GPIO_ResetBits(GPIOB,GPIO_Pin_4);	//PB.4 输出低
	GPIO_ResetBits(GPIOB,GPIO_Pin_3);	//PB.3 输出低
}
