//#include "sys.h"
#include "uart3.h"
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

  * @brief   uart3.c������RS485 - uart3��DMA���յĳ�ʼ��

  ******************************************************************************

  * @attention
  *
  * ��ʼ��Uart3��������115200��DMA���շ�ʽ���ж����ȼ�6����������RS485ͨ��
  *
  ******************************************************************************
  */

#if EN_USART3_RX   //���ʹ���˽���

uint8_t Uart3_RxBuff[DMA_UART3_RX_SIZE];
uint8_t Uart3_TxBuff[DMA_UART3_TX_SIZE];
uint16_t Uart3_RxLength;	//Uart3 DMA һ��ʵ�ʽ��յ������ݳ���

extern QueueHandle_t Uart3_Message_Queue;		//��Ϣ���о��

/*
	Uart3-PN532����
*/
void uart3_init(u32 bound){
	/*���崮�ڳ�ʼ���ṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/*ʹ���ڲ�����ʱ��*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//ʹ��USART3ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��GPIOBʱ��
	
	/*GPIO�˿�����*/
	/*USART3_TX   GPIOB.10*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 				//PB.10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//����������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//��ʼ��GPIOB.10
   
	/*USART3_RX	  GPIOB.11*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				//PB.11
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//��������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//��ʼ��GPIOB.11
  
	/*USART3 ��������*/
	USART_InitStructure.USART_BaudRate = bound;					//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//ֹͣλ 1λ
	USART_InitStructure.USART_Parity = USART_Parity_No;			//��żУ��λ ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	
	USART_Init(USART3, &USART_InitStructure); //��ʼ������3
	
	/*Usart3 NVIC ����*/
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;			//����3�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;	//��ռ���ȼ�6
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	/*����UART3 �жϴ���ģʽ*/
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//RX NO Empty��RX�ǿգ�RX�������жϡ��������ڽ����жϣ�ʹ��DMA����ʱҪʧ�������ʹ�ܿ����ж�
	//USART_ITConfig(USART3, USART_IT_TC, ENABLE);// Transmit Complete����������жϡ��������ڷ�������жϣ�����һ���ֽھͻ�����жϣ�ֻ��Ҫ����жϱ�־λ������Ҫ�ر��ж�
	//USART_IT_TXE TX Empty��TXΪ�գ����ͼĴ���DR���㡣���ͼĴ��������жϣ�������һ���ֽں󣬱���ر�����жϣ�����ֻҪ�Ĵ���Ϊ��ֵ���ͻᷴ�������ж�
	//USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//�������ڿ����жϣ������ͻ᲻�ᴥ�������ж�
	
	/*����UART3 */
	USART_Cmd(USART3, ENABLE);                    		//ʹ�ܴ���3
}

void uart3_dma_rx_configuration(void)
{
	/*����DMA��ʼ���ṹ��*/
	DMA_InitTypeDef  DMA_InitStructure;
	
	/*ʹ���ڲ�����ʱ��*/ 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMAʱ��
	
	/*DMA ͨ��3 ��������*/
	DMA_DeInit(DMA1_Channel3);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR;		//DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Uart3_RxBuff;			//DMA�ڴ����ַ,�ѽ��յ������ݷŵ��Ķ�
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//���ݴ��䷽�򣬴��ڴ�������ȡ��������Ϊ������Դ
	DMA_InitStructure.DMA_BufferSize = DMA_UART3_RX_SIZE;				//DMAͨ���Ļ���Ĵ�С��һ�ν��յ�����ֽ���
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//�����ַ�Ĵ���������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //�������ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 	//�ڴ����ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  						//����������ģʽ�������˲��ٽ��գ�������ѭ������
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 				//DMAͨ�� �����ȼ�
//	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; 			//DMAͨ�� ���ȼ��ܸ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  						//DMAͨ��Ϊ�ڴ����ڴ�ͨ�ţ������ڴ浽�ڴ�
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);						//����DMA
	
	/*����UART3 �жϴ���ģʽ*/
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);//RX NO Empty��RX�ǿգ�RX�������жϡ��������ڽ����жϣ�ʹ��DMA����ʱҪʧ�������ʹ�ܿ����ж�
	//USART_ITConfig(USART3, USART_IT_TC, ENABLE);// Transmit Complete����������жϡ��������ڷ�������жϣ�����һ���ֽھͻ�����жϣ�ֻ��Ҫ����жϱ�־λ������Ҫ�ر��ж�
	//USART_IT_TXE TX Empty��TXΪ�գ����ͼĴ���DR���㡣���ͼĴ��������жϣ�������һ���ֽں󣬱���ر�����жϣ�����ֻҪ�Ĵ���Ϊ��ֵ���ͻᷴ�������ж�
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//�������ڿ����жϣ������ͻ᲻�ᴥ�������ж�
	
	/*����DMA����*/
	DMA_Cmd(DMA1_Channel3, ENABLE);     								//����DMA ͨ��3���䣬�����մ���
	USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);						//ʹ��UART3��DMA����
}

//��֤uart3��DMA���ڲ�������ն��е�����£����������ݻḲ��ԭ�������ݣ�δ���ǵ����ݻᱣ�������Խ���һ�����ݲ�ʹ����ɺ���Ҫ������һ�����ݣ����뽫���ջ���������
//��֤���uart3 IDLE�жϣ���ȷ�ķ������ȶ�SR���ٶ�DR
void USART3_IRQHandler(void)                	//����3�жϷ������
{
	uint8_t temp;
	BaseType_t xHigherPriorityTaskWoken;
	
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		DMA_Cmd(DMA1_Channel3,DISABLE);//�ر�DMAͨ��
		
		//û��,�������Ǽ��ϰ�
        DMA_ClearFlag(DMA1_FLAG_TC3);//DMA ͨ��3 ���жϱ�־�������һֱ�ж�
		USART_ClearITPendingBit(USART3, USART_IT_IDLE);//��������жϱ�־
		
		//IDLE��־λ����Ĺ�����:�ȶ�SR,�ٶ�DR�Ĵ���
		//ע�⣺������Ҫ�������ܹ�����жϱ�־λ��
		temp = USART3->SR;//�ȶ�SR��Ȼ���DR�������
		temp = USART3->DR;//���DR
		temp = temp;
		
		/*�����ﴦ�����ݳ��� �����ݴ����¼�*/
		Uart3_RxLength = DMA_UART3_RX_SIZE - DMA_GetCurrDataCounter(DMA1_Channel3);
		//DMA���������ݺ󣬲��ὫUart3_Rx_Buff�κ��������㣬��һ�����ݻ����¸��ǣ�û�и��ǵ����ݻᱣ�������Ի�ȡ���ݳ��Ⱥ�������ջ������Ǳ�Ҫ��
		
		//���ŰѴ���Ч֡����Uart3_Message_Queue���У�ȥ�������������������������������֡������һ֡��������ǰ���㣬Ҳ���Ƕ�����
		if((Uart3_Message_Queue!=NULL) && (Uart3_RxLength <= DMA_UART3_RX_SIZE))
		{
//			Rec_msg.Uart3_rxflag = 1;								//����Ч������Ϣ��¼���ṹ��
//			Rec_msg.Uart3_Effective_num = Uart3_RxLength;
//			
//			//��ÿ֡�����λ��Ϊ���ȱ�־λ���������������0xFF����Ч�ֽ�
//			//���ŰѴ���Ч֡����Uart3_Message_Queue���У�ȥ�������������������������������֡������һ֡��������ǰ���㣬Ҳ���Ƕ�����
//			//ͨ����Uart3 dma�У������ĩβǶ�׵�����֡������Ϣ��������Ч�жϴ˴ν��յ�������֡�Ƿ����
//			Uart3_Rx_Buff[DMA_UART3_RX_SIZE - 2] = 0xFF;			//�����ڶ����ֽ����ֽ��жϱ�־λ
//			Uart3_Rx_Buff[DMA_UART3_RX_SIZE - 1] = Uart3_RxLength;	//������һ���ֽڴ洢��Ч�ֽ���
//		
//			xQueueSendFromISR(Uart3_Message_Queue,Uart3_Rx_Buff,&xHigherPriorityTaskWoken);//������з�������
//				
//			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//�����Ҫ�Ļ�����һ�������л�
		}
		else
		{
//			Rec_msg.Uart3_rxflag = 0;	//���ձ�־λΪ0
//			//Rec_msg.Uart3_instruction = 0;	//���մ�����ָ��Ϊ0
		}
		
		/*���㱾��DMA���ջ�����*/
		//������ջ����������Բ�ȫ����գ��������Ч�ʣ�ȫ����գ�ȷ����ʱ���ȶ���
		//memset(Uart3_Rx_Buff,0,Uart3_RxLength);
//		memset(Uart3_Rx_Buff,0,DMA_UART3_RX_SIZE);//��uart���ջ�������Ŀ�������������ݽ��ջ�����USART_RX_BUF,������һ�����ݽ���
		
		/*���¿�����һ��DMA����*/
        DMA_SetCurrDataCounter(DMA1_Channel3,DMA_UART3_RX_SIZE);//DMAͨ����DMA����Ĵ�С�����ô�����Ŀ�����ٴδﵽ�����Ŀ�ͻ���ж�
        DMA_Cmd(DMA1_Channel3,ENABLE);//����DMAͨ��	
	}		
}

/*****************����3 ��RS485�����ַ���**********************/
//�͵�ƽʱ�����ͽ�ֹ��������Ч��������
//�ߵ�ƽʱ��������Ч�����ս�ֹ��������
void uart3_RS485_RD_GPIO(void)
{
	/*���崮�ڳ�ʼ���ṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/*ʹ���ڲ�����ʱ��*/ 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��GPIOBʱ��
	
	/*GPIO�˿�����*/
	/*RS485-RD   GPIOB.2*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 				//PB.2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//����������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//��ʼ��GPIOB.2
	
	/*�͵�ƽ ����״̬*/
	GPIO_ResetBits(GPIOB,GPIO_Pin_2);
}

/*****************����3�����ַ���**********************/
void USART3_sendonechar(u8 data)
{
	USART_SendData(USART3, data);        		 //����3��������
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);//�ȴ����ͽ���	
}

void USART3_RS485_sendstring(u8 *data,u16 length)
{
	unsigned char i;
	
	//RD�ߵ�ƽ��������
	GPIO_SetBits(GPIOB,GPIO_Pin_2); 
	
	for(i=0;i<length;i++)
	{
		USART3_sendonechar(*data);
		data++;
	}
	
	//RD�͵�ƽ��������
	GPIO_ResetBits(GPIOB,GPIO_Pin_2); 
}

#endif
