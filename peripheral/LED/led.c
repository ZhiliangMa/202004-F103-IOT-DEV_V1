#include "led.h"

/************************************************

************************************************/  

/*	LED IO��ʼ��
 *	����ʹ����PA3/4�˿ڣ���Ҫ�ر�JTAG����
 *
 *	SCR1  -  PB8
 *	SCR2  -  PB5
 *	JK1   -  PB4
 *	JK2   -  PB3
 *
 *	�ο����£�https://blog.csdn.net/qhw5279/article/details/72630637
 *	https://blog.csdn.net/bobbat/article/details/50910559
 *	https://blog.csdn.net/euxnijuoh/article/details/74939446
 */
//LED IO��ʼ��
void LED_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��GPIOB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_5;	//�˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//�����趨������ʼ��
	
	GPIO_SetBits(GPIOB,GPIO_Pin_8);		//PB.8 �����
	GPIO_SetBits(GPIOB,GPIO_Pin_5);		//PB.5 �����
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |  RCC_APB2Periph_AFIO, ENABLE);//ʹ��PORTB��ʱ�ӡ���������PB3��PB4����
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//�ر�JTAG����(PB3/4)��ֻʹ��SWD(PA13/14)����
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_3;	//�˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//�����趨������ʼ��
	
	GPIO_ResetBits(GPIOB,GPIO_Pin_4);	//PB.4 �����
	GPIO_ResetBits(GPIOB,GPIO_Pin_3);	//PB.3 �����
}
