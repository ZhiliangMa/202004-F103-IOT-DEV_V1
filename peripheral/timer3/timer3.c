#include "timer3.h"
#include "uart2.h"
#include "structure.h"

#include "uart1.h"
#include "uart2.h"
#include "string.h"
#include "onenet_http.h"  //������Ҫ��ͷ�ļ�
#include "wifi.h"	      //������Ҫ��ͷ�ļ�

void Timer3_Configuration(uint16_t xms)
{
	/* ����Tim��ʼ���ṹ�� */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* ʹ���ڲ�����ʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//ʹ��TIM3ʱ��
	
	/* Tim3 �������� */
	TIM_DeInit(TIM3);//ʹ��ȱʡֵ��ʼ��TIM����Ĵ���
	TIM_TimeBaseStructure.TIM_Prescaler=(7200-1);//ʱ��Ԥ��Ƶ��Ϊ7200
	TIM_TimeBaseStructure.TIM_Period=(xms * 10 - 1);//20ms//�Զ���װ�ؼĴ���,10 - 1ms,10000 - 1000ms - 1s
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//������Ƶ����1
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//����ģʽ
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);//������±�־λ
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//ʹ���ж�
	//TIM_Cmd(TIM3,ENABLE);//ʹ��TIM3��ʱ��
	TIM_Cmd(TIM3,DISABLE);//�ر�TIM3��ʱ��
	
	/* TIM3 NVIC ����*/
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; 			//Tim3�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;	//��ռ���ȼ�6
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure); //����ָ���Ĳ�����ʼ��VIC�Ĵ���
}

void Tim3_Counter_Clear(void)
{
	TIM3->CNT = 0;
	TIM_Cmd(TIM3,ENABLE);//ʹ��TIM3��ʱ��
}

void Wifi_RxFlag_Clear(void)
{
	WifiMsg.RxFlag = 0;
	WifiMsg.RxNum = 0;//�����䲻֪���ò��üӣ��᲻����tim3�ж��е�����dma�г�ͻ
//	Uart2_RxLength = 0;
}

//volatile u32 gTimer;
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET)//�������ź�
	{		
		u1_printf("���յ�����\r\n");                                  //������ʾ����
		WifiMsg.U2_RxCompleted = 1;                                   //����2������ɱ�־λ��λ
		memcpy(&RXbuff[2],Usart2_RxBuff,Usart2_RxCounter);            //��������
		RXbuff[0] = WiFi_RxCounter/256;                               //��¼���յ�������		
		RXbuff[1] = WiFi_RxCounter%256;                               //��¼���յ�������
		RXbuff[WiFi_RxCounter+2] = '\0';                              //���������
		WiFi_RxCounter=0;                                             //�������ֵ
		TIM_Cmd(TIM3, DISABLE);                        				  //�ر�TIM4��ʱ��
		TIM_SetCounter(TIM3, 0);                        			  //���㶨ʱ��4������
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);     			  //���TIM4����жϱ�־ 
		
//		printf("Hello\r\n");
//		gTimer--;
	}
}
