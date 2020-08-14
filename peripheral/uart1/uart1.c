//#include "sys.h"
#include "uart1.h"
#include <string.h>
//#include "structure.h"
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

  * @brief   uart1.c包含了uart1,DMA接收的初始化

  ******************************************************************************

  * @attention
  *
  * 初始化Uart1，波特率115200，DMA接收方式，中断优先级6。用来对上位机发送调试信息
  *
  ******************************************************************************
  */

 
#if EN_USART1_RX   //如果使能了接收

uint8_t Uart1_RxBuff[DMA_UART1_RX_SIZE];
uint8_t Uart1_TxBuff[DMA_UART1_TX_SIZE];
uint16_t Uart1_RxLength;	//Uart1 DMA 一次实际接收到的数据长度

extern QueueHandle_t Uart1_Message_Queue;		//信息队列句柄
  
void uart1_init(u32 bound){
	/*定义串口初始化结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/*使能内部外设时钟，uart1的时钟只能合起来写，否则初始化不正常*/ 
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	//使能GPIOA时钟
//	RCC_APB1PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);	//使能USART1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
	
	/*GPIO端口设置*/
	/*USART1_TX   GPIOA.9*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 				//PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//输出最大速率
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化GPIOA.9
   
	/*USART1_RX	  GPIOA.10*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				//PA.10
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化GPIOA.10  
  
	/*USART1 参数设置*/
	USART_InitStructure.USART_BaudRate = bound;					//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//停止位 1位
	USART_InitStructure.USART_Parity = USART_Parity_No;			//奇偶校验位 无
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	
	USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
	/*Usart1 NVIC 配置*/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;			//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;	//抢占优先级6
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	/*设置UART1 中断触发模式*/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//RX NO Empty，RX非空，RX有数据中断。开启串口接受中断，使用DMA接收时要失能这个，使能空闲中断
	//USART_ITConfig(USART1, USART_IT_TC, ENABLE);// Transmit Complete，发送完成中断。开启串口发送完成中断，发送一个字节就会进入中断，只需要清除中断标志位，不需要关闭中断
	//USART_IT_TXE TX Empty，TX为空，发送寄存器DR清零。发送寄存器空闲中断，发送完一个字节后，必须关闭这个中断，否则只要寄存器为空值，就会反复进入中断
	//USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//开启串口空闲中断？？发送会不会触发空闲中断
	
	/*开启UART1 */
	USART_Cmd(USART1, ENABLE);                    		//使能串口1
}

void uart1_dma_rx_configuration(void)
{
	/*定义DMA初始化结构体*/
	DMA_InitTypeDef  DMA_InitStructure;
	
	/*使能内部外设时钟*/ 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA时钟
	
	/*DMA 通道5 接收配置*/
	DMA_DeInit(DMA1_Channel5);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;		//DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Uart1_RxBuff;			//DMA内存基地址,把接收到的数据放到哪儿
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//数据传输方向，从内存从外设读取，外设作为数据来源
	DMA_InitStructure.DMA_BufferSize = DMA_UART1_RX_SIZE;				//DMA通道的缓存的大小，一次接收的最大字节数
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//外设地址寄存器不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //外设数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 	//内存数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  						//工作在正常模式，即满了不再接收，而不是循环储存
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 				//DMA通道 中优先级
//	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; 			//DMA通道 优先级很高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  						//DMA通道为内存与内存通信，而非内存到内存
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);						//启动DMA
	
	/*设置UART1 中断触发模式*/
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//RX NO Empty，RX非空，RX有数据中断。开启串口接受中断，使用DMA接收时要失能这个，使能空闲中断
	//USART_ITConfig(USART1, USART_IT_TC, ENABLE);// Transmit Complete，发送完成中断。开启串口发送完成中断，发送一个字节就会进入中断，只需要清除中断标志位，不需要关闭中断
	//USART_IT_TXE TX Empty，TX为空，发送寄存器DR清零。发送寄存器空闲中断，发送完一个字节后，必须关闭这个中断，否则只要寄存器为空值，就会反复进入中断
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//开启串口空闲中断？？发送会不会触发空闲中断
	
	/*开启DMA接收*/
	DMA_Cmd(DMA1_Channel5, ENABLE);     								//开启DMA 通道5传输，即接收传输
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);						//使能UART1的DMA接收
}

void uart1_dma_rxtx_configuration(void)
{
	/*定义DMA初始化结构体*/
	DMA_InitTypeDef  DMA_InitStructure;
	
	/*使能内部外设时钟*/ 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA时钟
	
	/*DMA 通道5 接收配置*/
	DMA_DeInit(DMA1_Channel5);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;		//DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Uart1_RxBuff;			//DMA内存基地址,把接收到的数据放到哪儿
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//数据传输方向，从内存读取发送到外设，外设作为数据来源
	DMA_InitStructure.DMA_BufferSize = DMA_UART1_RX_SIZE;				//DMA通道的缓存的大小，一次接收的最大字节数
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//外设地址寄存器不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //外设数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 	//内存数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  						//工作在正常模式，即满了不再接收，而不是循环储存
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 				//DMA通道 中优先级
//	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; 			//DMA通道 优先级很高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  						//DMA通道为内存与外设通信，而非内存到内存
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);						//配置DMA 通道5
	
	/*DMA 通道4 发送配置*/
	DMA_DeInit(DMA1_Channel4);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;		//DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Uart1_TxBuff;			//DMA发送的内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;					//数据传输方向，从内存到外设发送，外设作为数据传输目的地
	//因为刚开始初始化时候不需要发送数据，所以发送长度为0
	DMA_InitStructure.DMA_BufferSize = 0;				//发送长度为0
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);						//配置DMA 通道4
	
	/*设置UART1 中断触发模式*/
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//RX NO Empty，RX非空，RX有数据中断。开启串口接受中断，使用DMA接收时要失能这个，使能空闲中断
	//USART_ITConfig(USART1, USART_IT_TC, ENABLE);// Transmit Complete，发送完成中断。开启串口发送完成中断，发送一个字节就会进入中断，只需要清除中断标志位，不需要关闭中断
	//USART_IT_TXE TX Empty，TX为空，发送寄存器DR清零。发送寄存器空闲中断，发送完一个字节后，必须关闭这个中断，否则只要寄存器为空值，就会反复进入中断
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//开启串口空闲中断？？发送会不会触发空闲中断
	
	/*开启DMA接收	因为刚开始初始化时候不需要发送数据，所以 DMA发送通道4 不开启*/
	DMA_Cmd(DMA1_Channel5, ENABLE);     								//开启DMA 通道5传输，即接收传输
	USART_DMACmd(USART1,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);		//使能UART1的DMA发送和接收
}

uint16_t Uart1_DMA_Send_Data(void * buffer, u16 size)
{
	if(!size) return 0;// 判断长度是否有效
	while (DMA_GetCurrDataCounter(DMA1_Channel4));// 检查DMA发送通道内是否还有数据
	if(buffer) memcpy(Uart1_TxBuff, buffer,(size > DMA_UART1_TX_SIZE?DMA_UART1_TX_SIZE:size));//判断发送长度是否大于DMA可传输长度
	//DMA发送数据-要先关DMA，再设置发送长度，最后开启DMA
	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA1_Channel4->CNDTR = size;// 设置发送长度
	DMA_Cmd(DMA1_Channel4, ENABLE);// 启动DMA发送
	return size;
}

//验证uart1的DMA，在不清除接收队列的情况下，后来的数据会覆盖原来的数据，未覆盖的数据会保留。所以接收一次数据并使用完成后，想要接收下一次数据，必须将接收缓存区清零
//验证清除uart1 IDLE中断，正确的方法是先读SR，再度DR
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	uint8_t temp;
	BaseType_t xHigherPriorityTaskWoken;
	
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		DMA_Cmd(DMA1_Channel5,DISABLE);//关闭DMA通道
		
		//没用,不过还是加上吧
        DMA_ClearFlag(DMA1_FLAG_TC5);//DMA 通道5 清中断标志，否则会一直中断
		USART_ClearITPendingBit(USART1, USART_IT_IDLE);//清除空闲中断标志
		
		//IDLE标志位清零的过程是:先读SR,再读DR寄存器
		//注意：这句必须要，否则不能够清除中断标志位。
		temp = USART1->SR;//先读SR，然后读DR才能清除
		temp = USART1->DR;//清除DR
		temp = temp;
		
		/*在这里处理数据长度 及数据处理事件*/
		Uart1_RxLength = DMA_UART1_RX_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);
		//DMA接收完数据后，不会将Uart1_Rx_Buff任何数据清零，下一次数据会重新覆盖，没有覆盖的数据会保留，所以获取数据长度和清零接收缓冲区是必要的
		
		//数据帧长度未溢出，且第一个字节为0x04，断定此帧为上位机RS485发送过来的消息
		//将每帧最后两位作为长度标志位，倒数最后两个是0xFF，有效字节
		//接着把此有效帧放入Uart1_Message_Queue队列，去触发任务处理解析。不符合条件的数据帧会在下一帧接收启动前清零，也就是丢弃掉
		if((Uart1_Message_Queue!=NULL) && (Uart1_RxLength <= DMA_UART1_RX_SIZE) && (Uart1_RxBuff[0] == 0x04))
		{
//			Rec_msg.Uart1_rxflag = 1;								//将有效数据信息记录到结构体
//			Rec_msg.Uart1_Effective_num = Uart1_RxLength;
//			
//			Uart1_Rx_Buff[DMA_UART1_RX_SIZE - 2] = 0xFF;			//倒数第二个字节做字节判断标志位
//			Uart1_Rx_Buff[DMA_UART1_RX_SIZE - 1] = Uart1_RxLength;	//倒数第一个字节存储有效字节数
//		
//			xQueueSendFromISR(Uart1_Message_Queue,Uart1_Rx_Buff,&xHigherPriorityTaskWoken);//向队列中发送数据
//				
//			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//如果需要的话进行一次任务切换
		}
		else
		{
//			Rec_msg.Uart1_rxflag = 0;	//接收标志位为0
//			Rec_msg.Uart1_instruction = 0;	//接收处理到的指令为0
		}
		
		/*清零本次DMA接收缓冲区*/
		//清零接收缓存区，可以不全部清空，提高运行效率；全部清空，确保长时间稳定性
		//memset(Uart1_Rx_Buff,0,Uart1_RxLength);
//		memset(Uart1_Rx_Buff,0,DMA_UART1_RX_SIZE);//待uart接收缓存区项目处理完后，清除数据接收缓冲区USART_RX_BUF,用于下一次数据接收
		
		/*重新开启下一次DMA接收*/
        DMA_SetCurrDataCounter(DMA1_Channel5,DMA_UART1_RX_SIZE);//DMA通道的DMA缓存的大小。重置传输数目，当再次达到这个数目就会进中断
        DMA_Cmd(DMA1_Channel5,ENABLE);//开启DMA通道	
	}		
}

/*****************串口1发送字符串**********************/
void USART1_sendonechar(u8 data)
{
	USART_SendData(USART1, data);        		 //串口1发送数据
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束	
}
//void USART1_sendstring(u8 *data,u16 length)
//{
//	unsigned char i;
//	
//	for(i=0;i<length;i++)
//	{
//		USART1_sendonechar(*data);
//		data++;
//	}
//}
void USART1_sendstring(char *data,u16 length)
{
	u16 i;
	
	for(i=0;i<length;i++)
	{
		USART1_sendonechar(*data);
		data++;
	}
}

#endif

