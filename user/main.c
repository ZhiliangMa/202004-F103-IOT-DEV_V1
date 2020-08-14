#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "uart1.h"
#include "uart2.h"
#include "uart3.h"
#include "timer3.h"
#include "structure.h"

#include "spi1.h"
#include "oled.h"
#include "oled_ui.h"

#include "wifi.h"
#include "onenet_http.h"
#include "seniverse_http.h"
#include "net_time.h"
#include <stdio.h>
#include <string.h>

/**
  ******************************************************************************

  * @file    main.c

  * @author  zhiLiangMa

  * @version V0.0.1

  * @date    2020/05/16

  * @brief   main.c������ϵͳ������Ӳ���������̣��Լ�ÿһ�������ִ������

  ******************************************************************************

  * @attention
  *
  * FreeRTOS + ESP8266-WIFI + OLED
  *
  ******************************************************************************
  */


/* Uart1 - UsbDebug ����Ϣ���ն��� */
#define Uart1_MESSAGE_Q_NUM   4   		//�������ݵ���Ϣ���е�����
QueueHandle_t Uart1_Message_Queue;		//��Ϣ���о��

/* Uart2 - Wifi ����Ϣ���ն��� */
#define Wifi_MESSAGE_Q_NUM   4   		//�������ݵ���Ϣ���е�����
QueueHandle_t Wifi_Message_Queue;		//��Ϣ���о��

/* Uart3 - RS485 ����Ϣ���ն��� */
#define Uart3_MESSAGE_Q_NUM   4   		//�������ݵ���Ϣ���е�����
QueueHandle_t Uart3_Message_Queue;		//��Ϣ���о��


/* FreeRTOS�����б� */
/* ��ʼ���������� */
#define START_TASK_PRIO		1			//�������ȼ�
#define START_STK_SIZE		128			//�����ջ��С
TaskHandle_t StartTask_Handler;			//������
void start_task(void *pvParameters);	//������
/* WIFI��������HTTP-GET OneNET����״̬����֪�������� */
#define WIFI_TASK_PRIO		2			//�������ȼ�
#define WIFI_STK_SIZE		128			//�����ջ��С
TaskHandle_t WIFITask_Handler;			//������
void wifi_task(void *pvParameters);		//������
/* OLED����ˢ�����񣬽�������������ƻ��� */
#define UI_TASK_PRIO		3			//�������ȼ�
#define UI_STK_SIZE			128			//�����ջ��С
TaskHandle_t UITask_Handler;			//������
void uiRefresh_task(void *pvParameters);//������
/* ʱ�ӽ�����ʾˢ�� */
#define CLOCKIFS_TASK_PRIO	3			//�������ȼ�
#define CLOCKIFS_STK_SIZE	128			//�����ջ��С
TaskHandle_t CLOCKIFSTask_Handler;		//������
void clockIFS_task(void *pvParameters);	//������

/* FreeRTOS�����ʱ����1s�жϣ���������ʱ�Ӽ�����Ӳ��������stm32��RTC�������񣬲����ѵ���������� */
TimerHandle_t 	netClockCountTimer_Handle;			//���ڶ�ʱ�����
extern TimerHandle_t 	netClockCountTimer_Handle;	//���ڶ�ʱ�����
void netClockCountCallback(TimerHandle_t xTimer); 	//���ڶ�ʱ���ص�����


int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init();	    				//��ʱ������ʼ��
	
	OLED_Init();						//OLED��ʾ����ʼ��
	ui_ShowBootImg();					//����ͼƬ���أ������ܽ������
	
	LED_Init();							//LED�������ų�ʼ��
	uart1_init(115200);					//��ʼ��UART1,������USB_Debug��ӡ������Ϣ
	uart1_dma_rx_configuration();		//UART1 - RX - DMA
	uart2_init(115200);					//��ʼ��UART2,����WIFIͨ��
	uart3_init(115200);					//��ʼ��UART3,����RS485ͨ��
	uart3_dma_rx_configuration();		//UART3 - RX - DMA
	
	Timer3_Configuration(5);			//Tim3��ʱ��������wifi-uart2�Ľ������
	
	WiFi_ResetIO_Init();				//wifi - RST���ų�ʼ��
	 
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

/* ��ʼ���������� */
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
	
	//���� Uart1  - UsbDebug ������Ϣ����
    Uart1_Message_Queue = xQueueCreate(Uart1_MESSAGE_Q_NUM,DMA_UART1_RX_SIZE); //��������Ŀ��Uart1_MESSAGE_Q_NUM����������Ǵ���DMA���ջ���������
	
	//���� Uart2 - Wifi ������Ϣ����
    Wifi_Message_Queue = xQueueCreate(Wifi_MESSAGE_Q_NUM,1); //��������Ŀ��Wifi_MESSAGE_Q_NUM����������Ǵ���DMA���ջ���������
	
	//���� Uart3 - RS485 ������Ϣ����
    Uart3_Message_Queue = xQueueCreate(Uart3_MESSAGE_Q_NUM,DMA_UART3_RX_SIZE); //��������Ŀ��Uart3_MESSAGE_Q_NUM����������Ǵ���DMA���ջ���������
	
	//����1s������ڶ�ʱ������������ʱ�Ӽ���
    netClockCountTimer_Handle=xTimerCreate((const char*		)"netClockCountTimer",
									    (TickType_t			)1000,
							            (UBaseType_t		)pdTRUE,
							            (void*				)1,
							            (TimerCallbackFunction_t)netClockCountCallback); //���ڶ�ʱ����ID=1������1s(1000��ʱ�ӽ���)
	
	//����1s������ڶ�ʱ������������ʱ�Ӽ���
	if(netClockCountTimer_Handle!=NULL)
	{
		xTimerStart(netClockCountTimer_Handle,0);	//�������ڶ�ʱ��
	}
	
    //����WIFI��������
    xTaskCreate((TaskFunction_t )wifi_task,     	
                (const char*    )"wifi_task",   	
                (uint16_t       )WIFI_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )WIFI_TASK_PRIO,	
                (TaskHandle_t*  )&WIFITask_Handler);   
    //����OLED����ˢ������
    xTaskCreate((TaskFunction_t )uiRefresh_task,     
                (const char*    )"ui_task",   
                (uint16_t       )UI_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )UI_TASK_PRIO,
                (TaskHandle_t*  )&UITask_Handler);
	//ʱ�ӽ�����ʾˢ��
    xTaskCreate((TaskFunction_t )clockIFS_task,     
                (const char*    )"clockIFS_task",   
                (uint16_t       )CLOCKIFS_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )CLOCKIFS_TASK_PRIO,
                (TaskHandle_t*  )&CLOCKIFSTask_Handler);
				
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

/* WIFI��������HTTP-GET OneNET����״̬����֪�������� */
void wifi_task(void *pvParameters)
{
	uint16_t wnum = 0;
    while(1)
    {
		/* ESP8266 ����wifi�ȵ� */
		if (WifiMsg.Mode != 1)
		{	// ��λ���ɹ�����Ҫ���¸�λ
			if(!WiFi_Init())
			{
				u1_printf("ESP8266״̬��ʼ������\r\n");		//���������Ϣ
				WifiMsg.Mode = 1;							//r_flag��־��λ����ʾ8266״̬���������Լ���������TCP����
			}
		}else	// 8266״̬���������Լ���������TCP����
		{
			/* ESP8266 TCP���� OneNet��ƽ̨��ʹ��HTTP��ȡ4·Զ�̿���״̬ */
			OneNET_GetSwitch();
			//��ʱ������GET�����ʣ������ڴ�Լ 3��һ��
			vTaskDelay(2000);
			
			/* ESP8266 TCP���� ��֪������ʹ��HTTP��ȡ�����������͵�ǰʱ�� */
			// Լ600s - 10���Ӹ���һ������
			if((wnum == 0) || (wnum > 200))
			{
				wnum = 0;
				Seniverse_GetWeather();
				vTaskDelay(100);
			}
			wnum ++;
		}
		//vTaskDelay(2000);
    }
}

/* OLED����ˢ�����񣬽�������������ƻ��� */
void uiRefresh_task(void *pvParameters)
{
	// ��������ʱ���ɵ��ڻ���ƽ�ƵĿ�����Ҳ��ϵ���ٴ�С�20̫����10��9��8��7�п����У�6��5��˳������ˬ��4��3��С����
	// 2˿����1��û2˿��Ҳû0��ôˬ��0��ˬ���ǿ�����ͷ����
	uint8_t slideDelay = 2;
	
	while(1)
	{
		uint8_t i=0;
		
		/* ����OneNET/ʱ�ӽ��� ˢ�����ܼ�ռ��ʱ��Լ 3000ms */
		for(i=0; i<100; i++)
		{
			ma_OLED_Refresh_Gram(OLED_GRAM);
			vTaskDelay(25);
		}
		//vTaskDelay(3000);
		
		/* ����OneNET/ʱ�ӽ��� �뿪�ؽ��� ��Ϲ��� */
		for(i=0; i<128; i++)
		{
			blend_Gram_H(i, OLED_GRAM, switchIFS, blendIFS);
			ma_OLED_Refresh_Gram(blendIFS);
			vTaskDelay(slideDelay);
		}
		
		/* ���ؽ��� ˢ�����ܼ�ռ��ʱ��Լ 3000ms */
		for(i=0; i<100; i++)
		{
			ma_OLED_Refresh_Gram(switchIFS);
			vTaskDelay(25);
		}
		//vTaskDelay(3000);
		
		/* ���ؽ��� ���������� ��Ϲ��� */
		for(i=0; i<128; i++)
		{
			blend_Gram_H(i, switchIFS, weatherIFS, blendIFS);
			ma_OLED_Refresh_Gram(blendIFS);
			vTaskDelay(slideDelay);
		}
		
		/* �������� ˢ�����ܼ�ռ��ʱ��Լ 3000ms */
		for(i=0; i<100; i++)
		{
			ma_OLED_Refresh_Gram(weatherIFS);
			vTaskDelay(25);
		}
		//vTaskDelay(3000);
		
		/* �������� �뿪��OneNET/ʱ�ӽ��� ��Ϲ��� */
		for(i=0; i<128; i++)
		{
			blend_Gram_H(i, weatherIFS, OLED_GRAM,blendIFS);
			ma_OLED_Refresh_Gram(blendIFS);
			vTaskDelay(slideDelay);
		}
    }
}

/* ʱ�ӽ�����ʾˢ�� */
void clockIFS_task(void *pvParameters)
{
	// ʱ�����ֻ�������ʱ���ɵ��ڻ����Ŀ�����Ҳ��ϵ����Ӧ
	// ����28���������ӵ�����5�Ļ����Ѿ���ЩӰ���ʱ���ˣ�0����е������һ����ˬ����Ӧ�û�ռ�ô�������
	// ����������ʾʱ�ӣ�3�Ǻ�˳���ġ���һ��������Ľ���ƽ�ƾͲ�һ���ˣ����Щ����
	// ���ź���������ã�ÿsʱ����£���һ���ź�����������ִ��32��
	uint8_t moveDelay = 10;
	while(1)
	{
		//showV();
		ui_ShowTime(0, 2, OLED_GRAM, &Device.Time);
		//ma_OLED_Refresh_Gram(OLED_GRAM);
		vTaskDelay(moveDelay);
	}
}

uint16_t xtep=0;
// �����ʱ���Ļص�����
// �����ʱ����ID=1������1s(1000��ʱ�ӽ���)
/* FreeRTOS�����ʱ����1s�жϣ���������ʱ�Ӽ�����Ӳ��������stm32��RTC�������񣬲����ѵ���������� */
void netClockCountCallback(TimerHandle_t xTimer)
{
	xtep++;
	TimePlusOneSecond(&Device.Time);
}
