#ifndef __UART3_H
#define __UART3_H
#include <stdio.h>
#include "sys.h" 
/************************************************

************************************************/
#define EN_USART3_RX 		1		//ʹ�ܣ�1��/��ֹ��0������3����


//DMA���� ���ֲο�https://blog.csdn.net/gdjason/article/details/51019219
#define	DMA_UART3_RX_SIZE	30
#define	DMA_UART3_TX_SIZE	30

/*�궨��Ϊ��д+�»��ߣ�����ΪС�շ�+�»���*/
extern uint8_t Uart3_RxBuff[DMA_UART3_RX_SIZE];
extern uint8_t Uart3_TxBuff[DMA_UART3_TX_SIZE];
extern uint16_t Uart3_RxLength;	//Uart3 DMA һ�ν��յ������ݳ���



void uart3_init(u32 bound);
void uart3_dma_rx_configuration(void);
void uart3_RS485_RD_GPIO(void);
void USART3_sendonechar(u8 data);
void USART3_RS485_sendstring(u8 *data,u16 length);

#endif


