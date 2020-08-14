//#include "sys.h"
#include "uart1.h"
#include <string.h>
//#include "structure.h"
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

  * @brief   uart1.c������uart1,DMA���յĳ�ʼ��

  ******************************************************************************

  * @attention
  *
  * ��ʼ��Uart1��������115200��DMA���շ�ʽ���ж����ȼ�6����������λ�����͵�����Ϣ
  *
  ******************************************************************************
  */

 
#if EN_USART1_RX   //���ʹ���˽���

uint8_t Uart1_RxBuff[DMA_UART1_RX_SIZE];
uint8_t Uart1_TxBuff[DMA_UART1_TX_SIZE];
uint16_t Uart1_RxLength;	//Uart1 DMA һ��ʵ�ʽ��յ������ݳ���

extern QueueHandle_t Uart1_Message_Queue;		//��Ϣ���о��
  
void uart1_init(u32 bound){
	/*���崮�ڳ�ʼ���ṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/*ʹ���ڲ�����ʱ�ӣ�uart1��ʱ��ֻ�ܺ�����д�������ʼ��������*/ 
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	//ʹ��GPIOAʱ��
//	RCC_APB1PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);	//ʹ��USART1ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
	
	/*GPIO�˿�����*/
	/*USART1_TX   GPIOA.9*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 				//PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//����������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//��ʼ��GPIOA.9
   
	/*USART1_RX	  GPIOA.10*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				//PA.10
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//��������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//��ʼ��GPIOA.10  
  
	/*USART1 ��������*/
	USART_InitStructure.USART_BaudRate = bound;					//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//ֹͣλ 1λ
	USART_InitStructure.USART_Parity = USART_Parity_No;			//��żУ��λ ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	
	USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	
	/*Usart1 NVIC ����*/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;			//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;	//��ռ���ȼ�6
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	/*����UART1 �жϴ���ģʽ*/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//RX NO Empty��RX�ǿգ�RX�������жϡ��������ڽ����жϣ�ʹ��DMA����ʱҪʧ�������ʹ�ܿ����ж�
	//USART_ITConfig(USART1, USART_IT_TC, ENABLE);// Transmit Complete����������жϡ��������ڷ�������жϣ�����һ���ֽھͻ�����жϣ�ֻ��Ҫ����жϱ�־λ������Ҫ�ر��ж�
	//USART_IT_TXE TX Empty��TXΪ�գ����ͼĴ���DR���㡣���ͼĴ��������жϣ�������һ���ֽں󣬱���ر�����жϣ�����ֻҪ�Ĵ���Ϊ��ֵ���ͻᷴ�������ж�
	//USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//�������ڿ����жϣ������ͻ᲻�ᴥ�������ж�
	
	/*����UART1 */
	USART_Cmd(USART1, ENABLE);                    		//ʹ�ܴ���1
}

void uart1_dma_rx_configuration(void)
{
	/*����DMA��ʼ���ṹ��*/
	DMA_InitTypeDef  DMA_InitStructure;
	
	/*ʹ���ڲ�����ʱ��*/ 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMAʱ��
	
	/*DMA ͨ��5 ��������*/
	DMA_DeInit(DMA1_Channel5);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;		//DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Uart1_RxBuff;			//DMA�ڴ����ַ,�ѽ��յ������ݷŵ��Ķ�
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//���ݴ��䷽�򣬴��ڴ�������ȡ��������Ϊ������Դ
	DMA_InitStructure.DMA_BufferSize = DMA_UART1_RX_SIZE;				//DMAͨ���Ļ���Ĵ�С��һ�ν��յ�����ֽ���
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//�����ַ�Ĵ���������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //�������ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 	//�ڴ����ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  						//����������ģʽ�������˲��ٽ��գ�������ѭ������
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 				//DMAͨ�� �����ȼ�
//	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; 			//DMAͨ�� ���ȼ��ܸ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  						//DMAͨ��Ϊ�ڴ����ڴ�ͨ�ţ������ڴ浽�ڴ�
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);						//����DMA
	
	/*����UART1 �жϴ���ģʽ*/
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//RX NO Empty��RX�ǿգ�RX�������жϡ��������ڽ����жϣ�ʹ��DMA����ʱҪʧ�������ʹ�ܿ����ж�
	//USART_ITConfig(USART1, USART_IT_TC, ENABLE);// Transmit Complete����������жϡ��������ڷ�������жϣ�����һ���ֽھͻ�����жϣ�ֻ��Ҫ����жϱ�־λ������Ҫ�ر��ж�
	//USART_IT_TXE TX Empty��TXΪ�գ����ͼĴ���DR���㡣���ͼĴ��������жϣ�������һ���ֽں󣬱���ر�����жϣ�����ֻҪ�Ĵ���Ϊ��ֵ���ͻᷴ�������ж�
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//�������ڿ����жϣ������ͻ᲻�ᴥ�������ж�
	
	/*����DMA����*/
	DMA_Cmd(DMA1_Channel5, ENABLE);     								//����DMA ͨ��5���䣬�����մ���
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);						//ʹ��UART1��DMA����
}

void uart1_dma_rxtx_configuration(void)
{
	/*����DMA��ʼ���ṹ��*/
	DMA_InitTypeDef  DMA_InitStructure;
	
	/*ʹ���ڲ�����ʱ��*/ 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMAʱ��
	
	/*DMA ͨ��5 ��������*/
	DMA_DeInit(DMA1_Channel5);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;		//DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Uart1_RxBuff;			//DMA�ڴ����ַ,�ѽ��յ������ݷŵ��Ķ�
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//���ݴ��䷽�򣬴��ڴ��ȡ���͵����裬������Ϊ������Դ
	DMA_InitStructure.DMA_BufferSize = DMA_UART1_RX_SIZE;				//DMAͨ���Ļ���Ĵ�С��һ�ν��յ�����ֽ���
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//�����ַ�Ĵ���������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //�������ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 	//�ڴ����ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  						//����������ģʽ�������˲��ٽ��գ�������ѭ������
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 				//DMAͨ�� �����ȼ�
//	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; 			//DMAͨ�� ���ȼ��ܸ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  						//DMAͨ��Ϊ�ڴ�������ͨ�ţ������ڴ浽�ڴ�
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);						//����DMA ͨ��5
	
	/*DMA ͨ��4 ��������*/
	DMA_DeInit(DMA1_Channel4);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;		//DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Uart1_TxBuff;			//DMA���͵��ڴ��ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;					//���ݴ��䷽�򣬴��ڴ浽���跢�ͣ�������Ϊ���ݴ���Ŀ�ĵ�
	//��Ϊ�տ�ʼ��ʼ��ʱ����Ҫ�������ݣ����Է��ͳ���Ϊ0
	DMA_InitStructure.DMA_BufferSize = 0;				//���ͳ���Ϊ0
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);						//����DMA ͨ��4
	
	/*����UART1 �жϴ���ģʽ*/
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//RX NO Empty��RX�ǿգ�RX�������жϡ��������ڽ����жϣ�ʹ��DMA����ʱҪʧ�������ʹ�ܿ����ж�
	//USART_ITConfig(USART1, USART_IT_TC, ENABLE);// Transmit Complete����������жϡ��������ڷ�������жϣ�����һ���ֽھͻ�����жϣ�ֻ��Ҫ����жϱ�־λ������Ҫ�ر��ж�
	//USART_IT_TXE TX Empty��TXΪ�գ����ͼĴ���DR���㡣���ͼĴ��������жϣ�������һ���ֽں󣬱���ر�����жϣ�����ֻҪ�Ĵ���Ϊ��ֵ���ͻᷴ�������ж�
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//�������ڿ����жϣ������ͻ᲻�ᴥ�������ж�
	
	/*����DMA����	��Ϊ�տ�ʼ��ʼ��ʱ����Ҫ�������ݣ����� DMA����ͨ��4 ������*/
	DMA_Cmd(DMA1_Channel5, ENABLE);     								//����DMA ͨ��5���䣬�����մ���
	USART_DMACmd(USART1,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);		//ʹ��UART1��DMA���ͺͽ���
}

uint16_t Uart1_DMA_Send_Data(void * buffer, u16 size)
{
	if(!size) return 0;// �жϳ����Ƿ���Ч
	while (DMA_GetCurrDataCounter(DMA1_Channel4));// ���DMA����ͨ�����Ƿ�������
	if(buffer) memcpy(Uart1_TxBuff, buffer,(size > DMA_UART1_TX_SIZE?DMA_UART1_TX_SIZE:size));//�жϷ��ͳ����Ƿ����DMA�ɴ��䳤��
	//DMA��������-Ҫ�ȹ�DMA�������÷��ͳ��ȣ������DMA
	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA1_Channel4->CNDTR = size;// ���÷��ͳ���
	DMA_Cmd(DMA1_Channel4, ENABLE);// ����DMA����
	return size;
}

//��֤uart1��DMA���ڲ�������ն��е�����£����������ݻḲ��ԭ�������ݣ�δ���ǵ����ݻᱣ�������Խ���һ�����ݲ�ʹ����ɺ���Ҫ������һ�����ݣ����뽫���ջ���������
//��֤���uart1 IDLE�жϣ���ȷ�ķ������ȶ�SR���ٶ�DR
void USART1_IRQHandler(void)                	//����1�жϷ������
{
	uint8_t temp;
	BaseType_t xHigherPriorityTaskWoken;
	
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		DMA_Cmd(DMA1_Channel5,DISABLE);//�ر�DMAͨ��
		
		//û��,�������Ǽ��ϰ�
        DMA_ClearFlag(DMA1_FLAG_TC5);//DMA ͨ��5 ���жϱ�־�������һֱ�ж�
		USART_ClearITPendingBit(USART1, USART_IT_IDLE);//��������жϱ�־
		
		//IDLE��־λ����Ĺ�����:�ȶ�SR,�ٶ�DR�Ĵ���
		//ע�⣺������Ҫ�������ܹ�����жϱ�־λ��
		temp = USART1->SR;//�ȶ�SR��Ȼ���DR�������
		temp = USART1->DR;//���DR
		temp = temp;
		
		/*�����ﴦ�����ݳ��� �����ݴ����¼�*/
		Uart1_RxLength = DMA_UART1_RX_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);
		//DMA���������ݺ󣬲��ὫUart1_Rx_Buff�κ��������㣬��һ�����ݻ����¸��ǣ�û�и��ǵ����ݻᱣ�������Ի�ȡ���ݳ��Ⱥ�������ջ������Ǳ�Ҫ��
		
		//����֡����δ������ҵ�һ���ֽ�Ϊ0x04���϶���֡Ϊ��λ��RS485���͹�������Ϣ
		//��ÿ֡�����λ��Ϊ���ȱ�־λ���������������0xFF����Ч�ֽ�
		//���ŰѴ���Ч֡����Uart1_Message_Queue���У�ȥ�������������������������������֡������һ֡��������ǰ���㣬Ҳ���Ƕ�����
		if((Uart1_Message_Queue!=NULL) && (Uart1_RxLength <= DMA_UART1_RX_SIZE) && (Uart1_RxBuff[0] == 0x04))
		{
//			Rec_msg.Uart1_rxflag = 1;								//����Ч������Ϣ��¼���ṹ��
//			Rec_msg.Uart1_Effective_num = Uart1_RxLength;
//			
//			Uart1_Rx_Buff[DMA_UART1_RX_SIZE - 2] = 0xFF;			//�����ڶ����ֽ����ֽ��жϱ�־λ
//			Uart1_Rx_Buff[DMA_UART1_RX_SIZE - 1] = Uart1_RxLength;	//������һ���ֽڴ洢��Ч�ֽ���
//		
//			xQueueSendFromISR(Uart1_Message_Queue,Uart1_Rx_Buff,&xHigherPriorityTaskWoken);//������з�������
//				
//			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//�����Ҫ�Ļ�����һ�������л�
		}
		else
		{
//			Rec_msg.Uart1_rxflag = 0;	//���ձ�־λΪ0
//			Rec_msg.Uart1_instruction = 0;	//���մ�����ָ��Ϊ0
		}
		
		/*���㱾��DMA���ջ�����*/
		//������ջ����������Բ�ȫ����գ��������Ч�ʣ�ȫ����գ�ȷ����ʱ���ȶ���
		//memset(Uart1_Rx_Buff,0,Uart1_RxLength);
//		memset(Uart1_Rx_Buff,0,DMA_UART1_RX_SIZE);//��uart���ջ�������Ŀ�������������ݽ��ջ�����USART_RX_BUF,������һ�����ݽ���
		
		/*���¿�����һ��DMA����*/
        DMA_SetCurrDataCounter(DMA1_Channel5,DMA_UART1_RX_SIZE);//DMAͨ����DMA����Ĵ�С�����ô�����Ŀ�����ٴδﵽ�����Ŀ�ͻ���ж�
        DMA_Cmd(DMA1_Channel5,ENABLE);//����DMAͨ��	
	}		
}

/*****************����1�����ַ���**********************/
void USART1_sendonechar(u8 data)
{
	USART_SendData(USART1, data);        		 //����1��������
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���	
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

