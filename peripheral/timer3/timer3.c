#include "timer3.h"
#include "uart2.h"
#include "structure.h"

#include "uart1.h"
#include "uart2.h"
#include "string.h"
#include "onenet_http.h"  //包含需要的头文件
#include "wifi.h"	      //包含需要的头文件

void Timer3_Configuration(uint16_t xms)
{
	/* 定义Tim初始化结构体 */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* 使能内部外设时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//使能TIM3时钟
	
	/* Tim3 参数设置 */
	TIM_DeInit(TIM3);//使用缺省值初始化TIM外设寄存器
	TIM_TimeBaseStructure.TIM_Prescaler=(7200-1);//时钟预分频数为7200
	TIM_TimeBaseStructure.TIM_Period=(xms * 10 - 1);//20ms//自动重装载寄存器,10 - 1ms,10000 - 1000ms - 1s
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//采样分频倍数1
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//上升模式
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);//清除更新标志位
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//使能中断
	//TIM_Cmd(TIM3,ENABLE);//使能TIM3定时器
	TIM_Cmd(TIM3,DISABLE);//关闭TIM3定时器
	
	/* TIM3 NVIC 配置*/
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; 			//Tim3中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;	//抢占优先级6
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器
}

void Tim3_Counter_Clear(void)
{
	TIM3->CNT = 0;
	TIM_Cmd(TIM3,ENABLE);//使能TIM3定时器
}

void Wifi_RxFlag_Clear(void)
{
	WifiMsg.RxFlag = 0;
	WifiMsg.RxNum = 0;//这两句不知道该不该加，会不会与tim3中断中的重启dma有冲突
//	Uart2_RxLength = 0;
}

//volatile u32 gTimer;
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET)//检查溢出信号
	{		
		u1_printf("接收到数据\r\n");                                  //串口提示数据
		WifiMsg.U2_RxCompleted = 1;                                   //串口2接收完成标志位置位
		memcpy(&RXbuff[2],Usart2_RxBuff,Usart2_RxCounter);            //拷贝数据
		RXbuff[0] = WiFi_RxCounter/256;                               //记录接收的数据量		
		RXbuff[1] = WiFi_RxCounter%256;                               //记录接收的数据量
		RXbuff[WiFi_RxCounter+2] = '\0';                              //加入结束符
		WiFi_RxCounter=0;                                             //清零计数值
		TIM_Cmd(TIM3, DISABLE);                        				  //关闭TIM4定时器
		TIM_SetCounter(TIM3, 0);                        			  //清零定时器4计数器
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);     			  //清除TIM4溢出中断标志 
		
//		printf("Hello\r\n");
//		gTimer--;
	}
}
