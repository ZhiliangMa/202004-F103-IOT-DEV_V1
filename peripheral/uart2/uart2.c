//#include "sys.h"
#include "uart2.h"
#include <string.h>
#include "stdarg.h"		//������Ҫ��ͷ�ļ� 
#include "structure.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��OS,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOSʹ��
#include "queue.h"
#endif

/**
  ******************************************************************************

  * @file    uart.c

  * @author  zhiLiangMa

  * @version V0.0.1

  * @date    2020/05/16

  * @brief   uart2.c������uart2,DMA���յĳ�ʼ��

  ******************************************************************************

  * @attention
  *
  * ��ʼ��Uart2��������115200��DMA���շ�ʽ���ж����ȼ�6��������WIFI����
  *
  ******************************************************************************
  */

 
#if EN_USART2_RX   //���ʹ���˽���

//uint8_t Uart2_RxBuff[DMA_UART2_RX_SIZE];
//uint8_t Uart2_TxBuff[DMA_UART2_TX_SIZE];
char Usart2_RxBuff[USART2_RXBUFF_SIZE];
char Usart2_TxBuff[USART2_TXBUFF_SIZE];
uint16_t Usart2_RxCounter;	//Usart2 ���յ������ݳ��ȼ�����

extern QueueHandle_t Wifi_Message_Queue;		//��Ϣ���о��
  
void uart2_init(u32 bound){
	/*���崮�ڳ�ʼ���ṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/*ʹ���ڲ�����ʱ�ӣ�uart2��ʱ��ֻ�ֿܷ�д�������ʼ��������*/ 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	//ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);	//ʹ��USART2ʱ��
	
	/*GPIO�˿�����*/
	/*USART2_TX   GPIOA.2*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 				//PA.2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//����������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//��ʼ��GPIOA.2
   
	/*USART2_RX	  GPIOA.3*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				//PA.3
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//��������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//��ʼ��GPIOA.3
  
	/*USART2 ��������*/
	USART_InitStructure.USART_BaudRate = bound;					//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//ֹͣλ 1λ
	USART_InitStructure.USART_Parity = USART_Parity_No;			//��żУ��λ ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	
	USART_Init(USART2, &USART_InitStructure); //��ʼ������2
	
	/*Usart2 NVIC ����*/
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;			//����2�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;	//��ռ���ȼ�6
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	/*����UART2 �жϴ���ģʽ*/
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//RX NO Empty��RX�ǿգ�RX�������жϡ��������ڽ����жϣ�ʹ��DMA����ʱҪʧ�������ʹ�ܿ����ж�
	//USART_ITConfig(USART2, USART_IT_TC, ENABLE);// Transmit Complete����������жϡ��������ڷ�������жϣ�����һ���ֽھͻ�����жϣ�ֻ��Ҫ����жϱ�־λ������Ҫ�ر��ж�
	//USART_IT_TXE TX Empty��TXΪ�գ����ͼĴ���DR���㡣���ͼĴ��������жϣ�������һ���ֽں󣬱���ر�����жϣ�����ֻҪ�Ĵ���Ϊ��ֵ���ͻᷴ�������ж�
	//USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//�������ڿ����жϣ������ͻ᲻�ᴥ�������ж�
	
	/*����UART2 */
	USART_Cmd(USART2, ENABLE);                    		//ʹ�ܴ���2
}

void uart2_dma_rx_configuration(void)
{
	/*����DMA��ʼ���ṹ��*/
	DMA_InitTypeDef  DMA_InitStructure;
	
	/*ʹ���ڲ�����ʱ��*/ 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMAʱ��
	
	/*DMA ͨ��6 ��������*/
	DMA_DeInit(DMA1_Channel6);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;		//DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Usart2_RxBuff;			//DMA�ڴ����ַ,�ѽ��յ������ݷŵ��Ķ�
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//���ݴ��䷽�򣬴��ڴ�������ȡ��������Ϊ������Դ
	DMA_InitStructure.DMA_BufferSize = USART2_RXBUFF_SIZE;				//DMAͨ���Ļ���Ĵ�С��һ�ν��յ�����ֽ���
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//�����ַ�Ĵ���������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //�������ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 	//�ڴ����ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  						//����������ģʽ�������˲��ٽ��գ�������ѭ������
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 				//DMAͨ�� �����ȼ�
//	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; 			//DMAͨ�� ���ȼ��ܸ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  						//DMAͨ��Ϊ�ڴ����ڴ�ͨ�ţ������ڴ浽�ڴ�
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);						//����DMA
	
	/*����UART2 �жϴ���ģʽ*/
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);//RX NO Empty��RX�ǿգ�RX�������жϡ��������ڽ����жϣ�ʹ��DMA����ʱҪʧ�������ʹ�ܿ����ж�
	//USART_ITConfig(USART2, USART_IT_TC, ENABLE);// Transmit Complete����������жϡ��������ڷ�������жϣ�����һ���ֽھͻ�����жϣ�ֻ��Ҫ����жϱ�־λ������Ҫ�ر��ж�
	//USART_IT_TXE TX Empty��TXΪ�գ����ͼĴ���DR���㡣���ͼĴ��������жϣ�������һ���ֽں󣬱���ر�����жϣ�����ֻҪ�Ĵ���Ϊ��ֵ���ͻᷴ�������ж�
//	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//�������ڿ����жϣ������ͻ᲻�ᴥ�������ж�
	
	/*����DMA����*/
	DMA_Cmd(DMA1_Channel6, ENABLE);     								//����DMA ͨ��6���䣬�����մ���
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);						//ʹ��UART2��DMA����
}

void uart2_dma_rxtx_configuration(void)
{
	/*����DMA��ʼ���ṹ��*/
	DMA_InitTypeDef  DMA_InitStructure;
	
	/*ʹ���ڲ�����ʱ��*/ 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMAʱ��
	
	/*DMA ͨ��6 ��������*/
	DMA_DeInit(DMA1_Channel6);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;		//DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Usart2_RxBuff;			//DMA�ڴ����ַ,�ѽ��յ������ݷŵ��Ķ�
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//���ݴ��䷽�򣬴��ڴ��ȡ���͵����裬������Ϊ������Դ
	DMA_InitStructure.DMA_BufferSize = USART2_RXBUFF_SIZE;				//DMAͨ���Ļ���Ĵ�С��һ�ν��յ�����ֽ���
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//�����ַ�Ĵ���������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //�������ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 	//�ڴ����ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  						//����������ģʽ�������˲��ٽ��գ�������ѭ������
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 				//DMAͨ�� �����ȼ�
//	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; 			//DMAͨ�� ���ȼ��ܸ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  						//DMAͨ��Ϊ�ڴ�������ͨ�ţ������ڴ浽�ڴ�
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);						//����DMA ͨ��6
	
	/*DMA ͨ��7 ��������*/
	DMA_DeInit(DMA1_Channel7);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;		//DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Usart2_TxBuff;			//DMA���͵��ڴ��ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;					//���ݴ��䷽�򣬴��ڴ浽���跢�ͣ�������Ϊ���ݴ���Ŀ�ĵ�
	//��Ϊ�տ�ʼ��ʼ��ʱ����Ҫ�������ݣ����Է��ͳ���Ϊ0
	DMA_InitStructure.DMA_BufferSize = 0;				//���ͳ���Ϊ0
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);						//����DMA ͨ��7
	
	/*����UART2 �жϴ���ģʽ*/
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);//RX NO Empty��RX�ǿգ�RX�������жϡ��������ڽ����жϣ�ʹ��DMA����ʱҪʧ�������ʹ�ܿ����ж�
	//USART_ITConfig(USART2, USART_IT_TC, ENABLE);// Transmit Complete����������жϡ��������ڷ�������жϣ�����һ���ֽھͻ�����жϣ�ֻ��Ҫ����жϱ�־λ������Ҫ�ر��ж�
	//USART_IT_TXE TX Empty��TXΪ�գ����ͼĴ���DR���㡣���ͼĴ��������жϣ�������һ���ֽں󣬱���ر�����жϣ�����ֻҪ�Ĵ���Ϊ��ֵ���ͻᷴ�������ж�
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//�������ڿ����жϣ������ͻ᲻�ᴥ�������ж�
	
	/*����DMA����	��Ϊ�տ�ʼ��ʼ��ʱ����Ҫ�������ݣ����� DMA����ͨ��7 ������*/
	DMA_Cmd(DMA1_Channel7, ENABLE);     								//����DMA ͨ��7���䣬�����մ���
	USART_DMACmd(USART2,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);		//ʹ��UART2��DMA���ͺͽ���
}

uint16_t Uart2_DMA_Send_Data(void * buffer, u16 size)
{
	if(!size) return 0;// �жϳ����Ƿ���Ч
	while (DMA_GetCurrDataCounter(DMA1_Channel7));// ���DMA����ͨ�����Ƿ�������
	if(buffer) memcpy(Usart2_TxBuff, buffer,(size > USART2_TXBUFF_SIZE?USART2_TXBUFF_SIZE:size));//�жϷ��ͳ����Ƿ����DMA�ɴ��䳤��
	//DMA��������-Ҫ�ȹ�DMA�������÷��ͳ��ȣ������DMA
	DMA_Cmd(DMA1_Channel7, DISABLE);
	DMA1_Channel7->CNDTR = size;// ���÷��ͳ���
	DMA_Cmd(DMA1_Channel7, ENABLE);// ����DMA����
	return size;
}

/*-------------------------------------------------*/
/*������������2�����жϺ���                        */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void USART2_IRQHandler(void)   
{                      
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{  //���USART_IT_RXNE��־��λ����ʾ�����ݵ��ˣ�����if��֧
		if(WifiMsg.ConnectFlag==0)
		{                                					//���Connect_flag����0����ǰ��û�����ӷ�����������ָ������״̬
			if(USART2->DR)
			{                                 				//����ָ������״̬ʱ������ֵ�ű��浽������	
				Usart2_RxBuff[Usart2_RxCounter]=USART2->DR; //���浽������	
				Usart2_RxCounter ++;                        //ÿ����1���ֽڵ����ݣ�Usart2_RxCounter��1����ʾ���յ���������+1 
			}		
		}else
		{	                                        		//��֮Connect_flag����1�������Ϸ�������	
			Usart2_RxBuff[Usart2_RxCounter] = USART2->DR;   //�ѽ��յ������ݱ��浽Usart2_RxBuff��				
			if(Usart2_RxCounter == 0)
			{    											//���Usart2_RxCounter����0����ʾ�ǽ��յĵ�1�����ݣ�����if��֧
				TIM_SetCounter(TIM3,0);				
				TIM_Cmd(TIM3,ENABLE); 
			}else
			{                        						//else��֧����ʾ��Usart2_RxCounter������0�����ǽ��յĵ�һ������
				TIM_SetCounter(TIM3,0);  
			}	
			Usart2_RxCounter ++;         				    //ÿ����1���ֽڵ����ݣ�Usart2_RxCounter��1����ʾ���յ���������+1 
		}		
	}
}

/*****************����2�����ַ���**********************/
void USART2_sendonechar(u8 data)
{
	USART_SendData(USART2, data);        		 //����2��������
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���	
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

//Ϊ�˼��ݳ�ά��WIFI������
/*-------------------------------------------------*/
/*������������2 printf����                         */
/*��  ����char* fmt,...  ��ʽ������ַ����Ͳ���    */
/*����ֵ����                                       */
/*-------------------------------------------------*/
//#define USART2_TXBUFF_SIZE   1024   //���崮��2 ���ͻ�������С 1024�ֽ�
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

