#ifndef __UART1_H
#define __UART1_H
#include <stdio.h>	
#include "sys.h" 
/************************************************

************************************************/
#define EN_USART1_RX 		1		//使能（1）/禁止（0）串口1接收

#define u1_printf printf
//DMA部分 部分参考https://blog.csdn.net/gdjason/article/details/51019219
#define	DMA_UART1_RX_SIZE	20
#define	DMA_UART1_TX_SIZE	20

/*宏定义为大写+下划线，变量为小驼峰+下划线*/
extern uint8_t Uart1_RxBuff[DMA_UART1_RX_SIZE];
extern uint8_t Uart1_TxBuff[DMA_UART1_TX_SIZE];
extern uint16_t Uart1_RxLength;	//Uart1 DMA 一次接收到的数据长度


void uart1_init(u32 bound);
void uart1_dma_rx_configuration(void);
void uart1_dma_rxtx_configuration(void);
uint16_t Uart1_DMA_Send_Data(void * buffer, u16 size);

void USART1_sendonechar(u8 data);
void USART1_sendstring(char *data,u16 length);

#endif


