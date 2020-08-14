#ifndef __UART3_H
#define __UART3_H
#include <stdio.h>
#include "sys.h" 
/************************************************

************************************************/
#define EN_USART3_RX 		1		//使能（1）/禁止（0）串口3接收


//DMA部分 部分参考https://blog.csdn.net/gdjason/article/details/51019219
#define	DMA_UART3_RX_SIZE	30
#define	DMA_UART3_TX_SIZE	30

/*宏定义为大写+下划线，变量为小驼峰+下划线*/
extern uint8_t Uart3_RxBuff[DMA_UART3_RX_SIZE];
extern uint8_t Uart3_TxBuff[DMA_UART3_TX_SIZE];
extern uint16_t Uart3_RxLength;	//Uart3 DMA 一次接收到的数据长度



void uart3_init(u32 bound);
void uart3_dma_rx_configuration(void);
void uart3_RS485_RD_GPIO(void);
void USART3_sendonechar(u8 data);
void USART3_RS485_sendstring(u8 *data,u16 length);

#endif


