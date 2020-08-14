#ifndef __UART2_H
#define __UART2_H
#include <stdio.h>
#include "sys.h" 
/************************************************

************************************************/
#define EN_USART2_RX 		1		//使能（1）/禁止（0）串口2接收


//DMA部分 部分参考https://blog.csdn.net/gdjason/article/details/51019219
#define	USART2_RXBUFF_SIZE	2000
#define	USART2_TXBUFF_SIZE	200

/*宏定义为大写+下划线，变量为小驼峰+下划线*/
//extern uint8_t Uart2_RxBuff[DMA_UART2_RX_SIZE];
//extern uint8_t Uart2_TxBuff[DMA_UART2_TX_SIZE];
extern char Usart2_RxBuff[USART2_RXBUFF_SIZE];
extern char Usart2_TxBuff[USART2_TXBUFF_SIZE];
extern uint16_t Usart2_RxCounter;	//Usart2 接收到的数据长度计数器


void uart2_init(u32 bound);
void uart2_dma_rx_configuration(void);
void uart2_dma_rxtx_configuration(void);
uint16_t Uart2_DMA_Send_Data(void * buffer, u16 size);

void USART2_sendonechar(u8 data);
void USART2_sendstring(u8 *data,u16 length);

//为了兼容超维的WIFI函数库
void u2_printf(char*,...) ;

#endif


