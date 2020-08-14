//#include "sys.h"
#include "uart2.h"
#include <string.h>
#include "stdarg.h"		//包含需要的头文件 
#include "structure.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用OS,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOS使用
#include "queue.h"
#endif

/**
  ******************************************************************************

  * @file    uart.c

  * @author  zhiLiangMa

  * @version V0.0.1

  * @date    2020/05/16

  * @brief   uart2.c包含了uart2,DMA接收的初始化

  ******************************************************************************

  * @attention
  *
  * 初始化Uart2，波特率115200，DMA接收方式，中断优先级6。用来与WIFI交互
  *
  ******************************************************************************
  */

 
#if EN_USART2_RX   //如果使能了接收

//uint8_t Uart2_RxBuff[DMA_UART2_RX_SIZE];
//uint8_t Uart2_TxBuff[DMA_UART2_TX_SIZE];
char Usart2_RxBuff[USART2_RXBUFF_SIZE];
char Usart2_TxBuff[USART2_TXBUFF_SIZE];
uint16_t Usart2_RxCounter;	//Usart2 接收到的数据长度计数器

extern QueueHandle_t Wifi_Message_Queue;		//信息队列句柄
  
void uart2_init(u32 bound){
	/*定义串口初始化结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/*使能内部外设时钟，uart2的时钟只能分开写，否则初始化不正常*/ 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	//使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);	//使能USART2时钟
	
	/*GPIO端口设置*/
	/*USART2_TX   GPIOA.2*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 				//PA.2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//输出最大速率
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化GPIOA.2
   
	/*USART2_RX	  GPIOA.3*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				//PA.3
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化GPIOA.3
  
	/*USART2 参数设置*/
	USART_InitStructure.USART_BaudRate = bound;					//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//停止位 1位
	USART_InitStructure.USART_Parity = USART_Parity_No;			//奇偶校验位 无
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	
	USART_Init(USART2, &USART_InitStructure); //初始化串口2
	
	/*Usart2 NVIC 配置*/
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;			//串口2中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;	//抢占优先级6
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	/*设置UART2 中断触发模式*/
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//RX NO Empty，RX非空，RX有数据中断。开启串口接受中断，使用DMA接收时要失能这个，使能空闲中断
	//USART_ITConfig(USART2, USART_IT_TC, ENABLE);// Transmit Complete，发送完成中断。开启串口发送完成中断，发送一个字节就会进入中断，只需要清除中断标志位，不需要关闭中断
	//USART_IT_TXE TX Empty，TX为空，发送寄存器DR清零。发送寄存器空闲中断，发送完一个字节后，必须关闭这个中断，否则只要寄存器为空值，就会反复进入中断
	//USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//开启串口空闲中断？？发送会不会触发空闲中断
	
	/*开启UART2 */
	USART_Cmd(USART2, ENABLE);                    		//使能串口2
}

void uart2_dma_rx_configuration(void)
{
	/*定义DMA初始化结构体*/
	DMA_InitTypeDef  DMA_InitStructure;
	
	/*使能内部外设时钟*/ 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA时钟
	
	/*DMA 通道6 接收配置*/
	DMA_DeInit(DMA1_Channel6);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;		//DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Usart2_RxBuff;			//DMA内存基地址,把接收到的数据放到哪儿
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//数据传输方向，从内存从外设读取，外设作为数据来源
	DMA_InitStructure.DMA_BufferSize = USART2_RXBUFF_SIZE;				//DMA通道的缓存的大小，一次接收的最大字节数
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//外设地址寄存器不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //外设数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 	//内存数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  						//工作在正常模式，即满了不再接收，而不是循环储存
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 				//DMA通道 中优先级
//	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; 			//DMA通道 优先级很高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  						//DMA通道为内存与内存通信，而非内存到内存
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);						//启动DMA
	
	/*设置UART2 中断触发模式*/
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);//RX NO Empty，RX非空，RX有数据中断。开启串口接受中断，使用DMA接收时要失能这个，使能空闲中断
	//USART_ITConfig(USART2, USART_IT_TC, ENABLE);// Transmit Complete，发送完成中断。开启串口发送完成中断，发送一个字节就会进入中断，只需要清除中断标志位，不需要关闭中断
	//USART_IT_TXE TX Empty，TX为空，发送寄存器DR清零。发送寄存器空闲中断，发送完一个字节后，必须关闭这个中断，否则只要寄存器为空值，就会反复进入中断
//	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//开启串口空闲中断？？发送会不会触发空闲中断
	
	/*开启DMA接收*/
	DMA_Cmd(DMA1_Channel6, ENABLE);     								//开启DMA 通道6传输，即接收传输
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);						//使能UART2的DMA接收
}

void uart2_dma_rxtx_configuration(void)
{
	/*定义DMA初始化结构体*/
	DMA_InitTypeDef  DMA_InitStructure;
	
	/*使能内部外设时钟*/ 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA时钟
	
	/*DMA 通道6 接收配置*/
	DMA_DeInit(DMA1_Channel6);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;		//DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Usart2_RxBuff;			//DMA内存基地址,把接收到的数据放到哪儿
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//数据传输方向，从内存读取发送到外设，外设作为数据来源
	DMA_InitStructure.DMA_BufferSize = USART2_RXBUFF_SIZE;				//DMA通道的缓存的大小，一次接收的最大字节数
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//外设地址寄存器不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //外设数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 	//内存数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  						//工作在正常模式，即满了不再接收，而不是循环储存
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 				//DMA通道 中优先级
//	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; 			//DMA通道 优先级很高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  						//DMA通道为内存与外设通信，而非内存到内存
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);						//配置DMA 通道6
	
	/*DMA 通道7 发送配置*/
	DMA_DeInit(DMA1_Channel7);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;		//DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Usart2_TxBuff;			//DMA发送的内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;					//数据传输方向，从内存到外设发送，外设作为数据传输目的地
	//因为刚开始初始化时候不需要发送数据，所以发送长度为0
	DMA_InitStructure.DMA_BufferSize = 0;				//发送长度为0
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);						//配置DMA 通道7
	
	/*设置UART2 中断触发模式*/
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);//RX NO Empty，RX非空，RX有数据中断。开启串口接受中断，使用DMA接收时要失能这个，使能空闲中断
	//USART_ITConfig(USART2, USART_IT_TC, ENABLE);// Transmit Complete，发送完成中断。开启串口发送完成中断，发送一个字节就会进入中断，只需要清除中断标志位，不需要关闭中断
	//USART_IT_TXE TX Empty，TX为空，发送寄存器DR清零。发送寄存器空闲中断，发送完一个字节后，必须关闭这个中断，否则只要寄存器为空值，就会反复进入中断
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//开启串口空闲中断？？发送会不会触发空闲中断
	
	/*开启DMA接收	因为刚开始初始化时候不需要发送数据，所以 DMA发送通道7 不开启*/
	DMA_Cmd(DMA1_Channel7, ENABLE);     								//开启DMA 通道7传输，即接收传输
	USART_DMACmd(USART2,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);		//使能UART2的DMA发送和接收
}

uint16_t Uart2_DMA_Send_Data(void * buffer, u16 size)
{
	if(!size) return 0;// 判断长度是否有效
	while (DMA_GetCurrDataCounter(DMA1_Channel7));// 检查DMA发送通道内是否还有数据
	if(buffer) memcpy(Usart2_TxBuff, buffer,(size > USART2_TXBUFF_SIZE?USART2_TXBUFF_SIZE:size));//判断发送长度是否大于DMA可传输长度
	//DMA发送数据-要先关DMA，再设置发送长度，最后开启DMA
	DMA_Cmd(DMA1_Channel7, DISABLE);
	DMA1_Channel7->CNDTR = size;// 设置发送长度
	DMA_Cmd(DMA1_Channel7, ENABLE);// 启动DMA发送
	return size;
}

/*-------------------------------------------------*/
/*函数名：串口2接收中断函数                        */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void USART2_IRQHandler(void)   
{                      
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{  //如果USART_IT_RXNE标志置位，表示有数据到了，进入if分支
		if(WifiMsg.ConnectFlag==0)
		{                                					//如果Connect_flag等于0，当前还没有连接服务器，处于指令配置状态
			if(USART2->DR)
			{                                 				//处于指令配置状态时，非零值才保存到缓冲区	
				Usart2_RxBuff[Usart2_RxCounter]=USART2->DR; //保存到缓冲区	
				Usart2_RxCounter ++;                        //每接收1个字节的数据，Usart2_RxCounter加1，表示接收的数据总量+1 
			}		
		}else
		{	                                        		//反之Connect_flag等于1，连接上服务器了	
			Usart2_RxBuff[Usart2_RxCounter] = USART2->DR;   //把接收到的数据保存到Usart2_RxBuff中				
			if(Usart2_RxCounter == 0)
			{    											//如果Usart2_RxCounter等于0，表示是接收的第1个数据，进入if分支
				TIM_SetCounter(TIM3,0);				
				TIM_Cmd(TIM3,ENABLE); 
			}else
			{                        						//else分支，表示果Usart2_RxCounter不等于0，不是接收的第一个数据
				TIM_SetCounter(TIM3,0);  
			}	
			Usart2_RxCounter ++;         				    //每接收1个字节的数据，Usart2_RxCounter加1，表示接收的数据总量+1 
		}		
	}
}

/*****************串口2发送字符串**********************/
void USART2_sendonechar(u8 data)
{
	USART_SendData(USART2, data);        		 //串口2发送数据
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束	
}
void USART2_sendstring(u8 *data,u16 length)
{
	unsigned char i;
	
	for(i=0;i<length;i++)
	{
		USART2_sendonechar(*data);
		data++;
	}
}

//为了兼容超维的WIFI函数库
/*-------------------------------------------------*/
/*函数名：串口2 printf函数                         */
/*参  数：char* fmt,...  格式化输出字符串和参数    */
/*返回值：无                                       */
/*-------------------------------------------------*/
//#define USART2_TXBUFF_SIZE   1024   //定义串口2 发送缓冲区大小 1024字节
//__align(8) char USART2_TxBuff[USART2_TXBUFF_SIZE];  
unsigned int length;
void u2_printf(char* fmt,...) 
{  
	//unsigned int i,length;
	unsigned int i;
	
	va_list ap;
	va_start(ap,fmt);
	vsprintf(Usart2_TxBuff,fmt,ap);
	va_end(ap);	
	
	length=strlen((const char*)Usart2_TxBuff);
	while((USART2->SR&0X40)==0);
	for(i = 0;i < length;i ++)
	{			
		USART2->DR = Usart2_TxBuff[i];
		while((USART2->SR&0X40)==0);	
	}	
}

#endif

