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

  * @brief   main.c包含了系统及外设硬件启动过程，以及每一项任务的执行流程

  ******************************************************************************

  * @attention
  *
  * FreeRTOS + ESP8266-WIFI + OLED
  *
  ******************************************************************************
  */


/* Uart1 - UsbDebug 的消息接收队列 */
#define Uart1_MESSAGE_Q_NUM   4   		//接收数据的消息队列的数量
QueueHandle_t Uart1_Message_Queue;		//信息队列句柄

/* Uart2 - Wifi 的消息接收队列 */
#define Wifi_MESSAGE_Q_NUM   4   		//接收数据的消息队列的数量
QueueHandle_t Wifi_Message_Queue;		//信息队列句柄

/* Uart3 - RS485 的消息接收队列 */
#define Uart3_MESSAGE_Q_NUM   4   		//接收数据的消息队列的数量
QueueHandle_t Uart3_Message_Queue;		//信息队列句柄


/* FreeRTOS任务列表 */
/* 开始任务任务函数 */
#define START_TASK_PRIO		1			//任务优先级
#define START_STK_SIZE		128			//任务堆栈大小
TaskHandle_t StartTask_Handler;			//任务句柄
void start_task(void *pvParameters);	//任务函数
/* WIFI网络任务，HTTP-GET OneNET开关状态，心知天气解析 */
#define WIFI_TASK_PRIO		2			//任务优先级
#define WIFI_STK_SIZE		128			//任务堆栈大小
TaskHandle_t WIFITask_Handler;			//任务句柄
void wifi_task(void *pvParameters);		//任务函数
/* OLED界面刷新任务，界面与界面间的左移滑动 */
#define UI_TASK_PRIO		3			//任务优先级
#define UI_STK_SIZE			128			//任务堆栈大小
TaskHandle_t UITask_Handler;			//任务句柄
void uiRefresh_task(void *pvParameters);//任务函数
/* 时钟界面显示刷新 */
#define CLOCKIFS_TASK_PRIO	3			//任务优先级
#define CLOCKIFS_STK_SIZE	128			//任务堆栈大小
TaskHandle_t CLOCKIFSTask_Handler;		//任务句柄
void clockIFS_task(void *pvParameters);	//任务函数

/* FreeRTOS软件定时器，1s中断，用于网络时钟计数（硬件板子上stm32的RTC晶振不起振，不得已的替代方法） */
TimerHandle_t 	netClockCountTimer_Handle;			//周期定时器句柄
extern TimerHandle_t 	netClockCountTimer_Handle;	//周期定时器句柄
void netClockCountCallback(TimerHandle_t xTimer); 	//周期定时器回调函数


int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	delay_init();	    				//延时函数初始化
	
	OLED_Init();						//OLED显示屏初始化
	ui_ShowBootImg();					//开机图片加载，各功能界面加载
	
	LED_Init();							//LED开关引脚初始化
	uart1_init(115200);					//初始化UART1,用于与USB_Debug打印调试信息
	uart1_dma_rx_configuration();		//UART1 - RX - DMA
	uart2_init(115200);					//初始化UART2,用于WIFI通信
	uart3_init(115200);					//初始化UART3,用于RS485通信
	uart3_dma_rx_configuration();		//UART3 - RX - DMA
	
	Timer3_Configuration(5);			//Tim3定时器，用于wifi-uart2的接收完成
	
	WiFi_ResetIO_Init();				//wifi - RST引脚初始化
	 
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

/* 开始任务任务函数 */
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
	
	//创建 Uart1  - UsbDebug 接收消息队列
    Uart1_Message_Queue = xQueueCreate(Uart1_MESSAGE_Q_NUM,DMA_UART1_RX_SIZE); //队列项数目是Uart1_MESSAGE_Q_NUM，队列项长度是串口DMA接收缓冲区长度
	
	//创建 Uart2 - Wifi 接收消息队列
    Wifi_Message_Queue = xQueueCreate(Wifi_MESSAGE_Q_NUM,1); //队列项数目是Wifi_MESSAGE_Q_NUM，队列项长度是串口DMA接收缓冲区长度
	
	//创建 Uart3 - RS485 接收消息队列
    Uart3_Message_Queue = xQueueCreate(Uart3_MESSAGE_Q_NUM,DMA_UART3_RX_SIZE); //队列项数目是Uart3_MESSAGE_Q_NUM，队列项长度是串口DMA接收缓冲区长度
	
	//创建1s软件周期定时器，用于网络时钟计数
    netClockCountTimer_Handle=xTimerCreate((const char*		)"netClockCountTimer",
									    (TickType_t			)1000,
							            (UBaseType_t		)pdTRUE,
							            (void*				)1,
							            (TimerCallbackFunction_t)netClockCountCallback); //周期定时器，ID=1，周期1s(1000个时钟节拍)
	
	//开启1s软件周期定时器，用于网络时钟计数
	if(netClockCountTimer_Handle!=NULL)
	{
		xTimerStart(netClockCountTimer_Handle,0);	//开启周期定时器
	}
	
    //创建WIFI网络任务
    xTaskCreate((TaskFunction_t )wifi_task,     	
                (const char*    )"wifi_task",   	
                (uint16_t       )WIFI_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )WIFI_TASK_PRIO,	
                (TaskHandle_t*  )&WIFITask_Handler);   
    //创建OLED界面刷新任务
    xTaskCreate((TaskFunction_t )uiRefresh_task,     
                (const char*    )"ui_task",   
                (uint16_t       )UI_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )UI_TASK_PRIO,
                (TaskHandle_t*  )&UITask_Handler);
	//时钟界面显示刷新
    xTaskCreate((TaskFunction_t )clockIFS_task,     
                (const char*    )"clockIFS_task",   
                (uint16_t       )CLOCKIFS_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )CLOCKIFS_TASK_PRIO,
                (TaskHandle_t*  )&CLOCKIFSTask_Handler);
				
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

/* WIFI网络任务，HTTP-GET OneNET开关状态，心知天气解析 */
void wifi_task(void *pvParameters)
{
	uint16_t wnum = 0;
    while(1)
    {
		/* ESP8266 连接wifi热点 */
		if (WifiMsg.Mode != 1)
		{	// 复位不成功，需要重新复位
			if(!WiFi_Init())
			{
				u1_printf("ESP8266状态初始化正常\r\n");		//串口输出信息
				WifiMsg.Mode = 1;							//r_flag标志置位，表示8266状态正常，可以继续，进行TCP连接
			}
		}else	// 8266状态正常，可以继续，进行TCP连接
		{
			/* ESP8266 TCP连接 OneNet云平台，使用HTTP获取4路远程开关状态 */
			OneNET_GetSwitch();
			//延时，控制GET的速率，控制在大约 3秒一次
			vTaskDelay(2000);
			
			/* ESP8266 TCP连接 心知天气，使用HTTP获取近三日天气和当前时间 */
			// 约600s - 10分钟更新一次天气
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

/* OLED界面刷新任务，界面与界面间的左移滑动 */
void uiRefresh_task(void *pvParameters)
{
	// 滑动的延时，可调节滑动平移的快慢，也关系到顿挫感。20太慢，10、9、8、7有颗粒感，6、5很顺滑但不爽，4、3有小卡顿
	// 2丝滑，1既没2丝滑也没0那么爽，0巨爽就是看多了头会晕
	uint8_t slideDelay = 2;
	
	while(1)
	{
		uint8_t i=0;
		
		/* 开机OneNET/时钟界面 刷屏，总计占用时间约 3000ms */
		for(i=0; i<100; i++)
		{
			ma_OLED_Refresh_Gram(OLED_GRAM);
			vTaskDelay(25);
		}
		//vTaskDelay(3000);
		
		/* 开机OneNET/时钟界面 与开关界面 混合滚动 */
		for(i=0; i<128; i++)
		{
			blend_Gram_H(i, OLED_GRAM, switchIFS, blendIFS);
			ma_OLED_Refresh_Gram(blendIFS);
			vTaskDelay(slideDelay);
		}
		
		/* 开关界面 刷屏，总计占用时间约 3000ms */
		for(i=0; i<100; i++)
		{
			ma_OLED_Refresh_Gram(switchIFS);
			vTaskDelay(25);
		}
		//vTaskDelay(3000);
		
		/* 开关界面 与天气界面 混合滚动 */
		for(i=0; i<128; i++)
		{
			blend_Gram_H(i, switchIFS, weatherIFS, blendIFS);
			ma_OLED_Refresh_Gram(blendIFS);
			vTaskDelay(slideDelay);
		}
		
		/* 天气界面 刷屏，总计占用时间约 3000ms */
		for(i=0; i<100; i++)
		{
			ma_OLED_Refresh_Gram(weatherIFS);
			vTaskDelay(25);
		}
		//vTaskDelay(3000);
		
		/* 天气界面 与开机OneNET/时钟界面 混合滚动 */
		for(i=0; i<128; i++)
		{
			blend_Gram_H(i, weatherIFS, OLED_GRAM,blendIFS);
			ma_OLED_Refresh_Gram(blendIFS);
			vTaskDelay(slideDelay);
		}
    }
}

/* 时钟界面显示刷新 */
void clockIFS_task(void *pvParameters)
{
	// 时钟数字滑动的延时，可调节滑动的快慢，也关系到响应
	// 大于28跟不上秒钟递增，5的话就已经有些影响读时间了，0跟机械计数表一样巨爽不过应该会占用大量进程
	// 本来单独显示时钟，3是很顺滑的。可一加上上面的界面平移就不一样了，会带些卡顿
	// 用信号量来做最好，每s时间更新，发一个信号量，高速率执行32次
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
// 软件定时器的回调函数
// 软件定时器，ID=1，周期1s(1000个时钟节拍)
/* FreeRTOS软件定时器，1s中断，用于网络时钟计数（硬件板子上stm32的RTC晶振不起振，不得已的替代方法） */
void netClockCountCallback(TimerHandle_t xTimer)
{
	xtep++;
	TimePlusOneSecond(&Device.Time);
}
