/*-------------------------------------------------*/
/*            超纬电子STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*            onenet_http功能的源文件              */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"    //包含需要的头文件
#include "onenet_http.h"  //包含需要的头文件
#include "stdio.h"        //包含需要的头文件
#include "stdarg.h"		  //包含需要的头文件 
#include "string.h"       //包含需要的头文件
//#include "usart1.h"       //包含需要的头文件
//#include "dht12.h"  	  //包含需要的头文件

#include "wifi.h"
#include "uart2.h"
#include "delay.h"
#include "usart.h"
#include "structure.h"
#include "uart1.h"
#include "led.h"
#include "oled_ui.h"
#include "net_time.h"

char *ServerIP = "183.230.40.33";          //OneNet服务器 IP地址
int   ServerPort = 80;                     //OneNet服务器 端口号
char RXbuff[2048];                         //接收数据缓冲区
char TXbuff[2048];                         //发送数据缓冲区

/*-------------------------------------------------*/
/*函数名：OneNet服务器 HTTP GET报文                */
/*参  数：device_id： 设备ID                       */
/*返回值：0：正确  其他：错误                      */
/*-------------------------------------------------*/
void OneNet_GET(char *device_id)
{
	char temp[128];
	
	memset(TXbuff,0,2048);   //清空缓冲区
    memset(temp,0,128);      //清空缓冲区                                             
	sprintf(TXbuff,"GET /devices/%s/datapoints HTTP/1.1\r\n",device_id);//构建报文
	sprintf(temp,"api-key:%s\r\n",API_KEY);                             //构建报文
	strcat(TXbuff,temp);                                                //追加报文
	strcat(TXbuff,"Host:api.heclouds.com\r\n\r\n");                     //追加报文
}
/*-------------------------------------------------*/
/*函数名：OneNet服务器 HTTP POST报文               */
/*参  数：device_id： 设备ID                       */
/*返回值：0：正确  其他：错误                      */
/*-------------------------------------------------*/
void OneNet_POST(char *device_id)
{
	char temp[128]; 
	char databuff[128]; 
	unsigned char data[5];
	
	memset(TXbuff,0,2048);   //清空缓冲区
    memset(temp,0,128);      //清空缓冲区
    memset(databuff,0,128);  //清空缓冲区
	sprintf(TXbuff,"POST /devices/%s/datapoints?type=3 HTTP/1.1\r\n",device_id);//构建报文
	sprintf(temp,"api-key:%s\r\n",API_KEY);          //构建报文
	strcat(TXbuff,temp);                             //追加报文
	strcat(TXbuff,"Host:api.heclouds.com\r\n");      //追加报文	

	while(1){                                                //循环读数据
		//DHT12_ReadData(data);                                //读取温湿度数据
		if((data[0]+data[1]+data[2]+data[3])==data[4]){      //判断校验，if成立表示数据正确	
			sprintf(databuff,"{\"temp_data\":%d.%d,\"humi_data\":%d.%d}",data[2],data[3],data[0],data[1]);   //构建上报数据
		    break;                                           //跳出while
		}//else u1_printf("温湿度数据校验错误，重新读取\r\n"); //串口输出信息
	}
	sprintf(temp,"Content-Length:%d\r\n\r\n",strlen(databuff));  //构建报文
	strcat(TXbuff,temp);                                         //追加报文
	sprintf(temp,"%s\r\n\r\n",databuff);                         //构建报文
	strcat(TXbuff,temp);                                         //追加报文
	//u1_printf("%s\r\n",databuff);
}



/*
 * 功能：从 OneNET服务器获取4路开关状态
 *
 */
void OneNET_GetSwitch(void)
{
	uint16_t xcnt=0;
	char *swcStatus;
	
	/* ESP8266 TCP连接 OneNet云平台，使用HTTP获取远程开关状态 */
	if(!WifiMsg.ConnectFlag)
	{
		u1_printf("准备连接 OneNET\r\n");	//串口提示数据
		/* TCP连接 OneNET服务器，并进入透传模式 */
		WifiMsg.Status = WiFi_Connect_Server(50);	//连接服务器，100ms超时单位，总计5s超时时间
		printf("连接返回值：%1d\r\n", WifiMsg.Status);
		/* 判断是否已经连接服务器 */
		if(WifiMsg.Status == 0)				//连接成功返回0	
		{
			/* 服务器连接成功 */
			u1_printf("连接 OneNET成功\r\n"); //串口提示数据
			WiFi_RxCounter=0;                           //WiFi接收数据量变量清零                        
			memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //清空WiFi接收缓冲区 
			Connect_flag = 1;				//连接成功标志置位
			WifiMsg.ConnectFlag = 1;
			/* 向 OneNET发送 GET请求，获取4路开关状态 */
			WifiMsg.U2_RxCompleted = 0;		//串口2接收完成标志位清零
			OneNet_GET(SWITCH_DID);			//构建查询开关状态的报文
			WiFi_printf(TXbuff);			//把构建好的报文发给服务器
			//开定时器3 3s的定时 如果3s内服务器 没有数据来，要重新复位8266
			/* 计算报文返回超时时间，最长3s */
			xcnt = 300;
			while((!WifiMsg.U2_RxCompleted) && xcnt)
			{
				--xcnt;
				vTaskDelay(10);
			}
			printf("接收超时 = %d ms\r\n", (300-xcnt)*10);
			
			/* 如果接收到，打印消息；没有接收到，重新连接GET */
			if(WifiMsg.U2_RxCompleted)
			{
				WifiMsg.U2_RxCompleted = 0;		//串口2接收完成标志位清零
				printf(RXbuff+2);			//打印接收到ESP8266的HTTP消息
			
				/* 对接收到的消息解析，获取4路开关状态 */
				if(strstr(&RXbuff[2],"200 OK")){                                //搜索200 OK 表示报文正确，进入if	
					if(strstr(&RXbuff[2],"\"errno\":0")){                       //搜索"errno":0 表示数据流操作正确，进入if
						if(strstr(&RXbuff[2],"datastreams")){                   //搜索 datastreams 表示获取的数据流，是GET报文 查询开关状态
							
//									/* 清OLED Gram */
//									ma_OLED_Gram_Clear(switchIFS);
							
							swcStatus = strstr(&RXbuff[2],"\"id\":\"switch_1\""); //搜索 "id":"switch_1" 查询开关1状态
							if(swcStatus!=NULL){                                  //如果搜索到了，进入if
								if(*(swcStatus-4) == '0') { printf("\r\nswitch1 OFF\r\n"); Device.Led.Val1=0;LED1_OFF;ui_ShowLed(3,0); }	//如果是0，关闭LED1
								else { printf("\r\nswitch1 ON\r\n"); Device.Led.Val1=1;LED1_ON;ui_ShowLed(3,1); }                        	//反之是1，打开LED1
								u1_printf("查询开关1状态正确\r\n");										//串口提示数据										
							}
							else u1_printf("查询开关1状态失败，数据流消息不正常\r\n");					//串口提示数据
							
							swcStatus = strstr(&RXbuff[2],"\"id\":\"switch_2\""); //搜索 "id":"switch_2" 查询开关2状态
							if(swcStatus!=NULL){                                  //如果搜索到了，进入if
								if(*(swcStatus-4) == '0') { printf("\r\nswitch2 OFF\r\n"); Device.Led.Val2=0;LED2_OFF;ui_ShowLed(2,0); }	//如果是0，关闭LED2
								else { printf("\r\nswitch2 ON\r\n"); Device.Led.Val2=1;LED2_ON;ui_ShowLed(2,1); }                        	//反之是1，打开LED2
								u1_printf("查询开关2状态正确\r\n");										//串口提示数据										
							}
							else u1_printf("查询开关2状态失败，数据流消息不正常\r\n");					//串口提示数据
							
							swcStatus = strstr(&RXbuff[2],"\"id\":\"switch_3\""); //搜索 "id":"switch_3" 查询开关3状态
							if(swcStatus!=NULL){                                  //如果搜索到了，进入if
								if(*(swcStatus-4) == '0') { printf("\r\nswitch3 OFF\r\n"); Device.Led.Val3=0;LED3_OFF;ui_ShowLed(1,0); }	//如果是0，关闭LED3
								else { printf("\r\nswitch3 ON\r\n"); Device.Led.Val3=1;LED3_ON;ui_ShowLed(1,1); }                        	//反之是1，打开LED3
								u1_printf("查询开关3状态正确\r\n");										//串口提示数据										
							}
							else u1_printf("查询开关3状态失败，数据流消息不正常\r\n");					//串口提示数据
							
							swcStatus = strstr(&RXbuff[2],"\"id\":\"switch_4\""); //搜索 "id":"switch_4" 查询开关4状态
							if(swcStatus!=NULL){                                  //如果搜索到了，进入if
								if(*(swcStatus-4) == '0') { printf("\r\nswitch OFF\r\n"); Device.Led.Val4=0;LED4_OFF;ui_ShowLed(0,0); }	//如果是0，关闭LED4
								else { printf("\r\nswitch4 ON\r\n"); Device.Led.Val4=1;LED4_ON;ui_ShowLed(0,1); }                        	//反之是1，打开LED4
								u1_printf("查询开关4状态正确\r\n");										//串口提示数据										
							}
							else u1_printf("查询开关4状态失败，数据流消息不正常\r\n");					//串口提示数据
							
//									ma_OLED_Refresh_Gram(switchIFS);
						}
					}
				}else{
					u1_printf("报文错误\r\n");          //串口提示数据
					u1_printf("%s\r\n",&RXbuff[2]);     //串口提示数据
				}
			}
		}else if(WifiMsg.Status == 1)		//服务器未开启返回1
		{
			/* 服务器连接失败 */
			Connect_flag = 0;                //连接成功标志清除
			WifiMsg.ConnectFlag = 0;
			u1_printf("连接 OneNET失败\r\n"); //串口提示数据
		}else	//返回2，已经建立连接返回2
		{
			/* 服务器已经建立连接 */
		}
		
		/* 无论接收到返回消息与否，都关闭连接 关闭连接的检验未加 */
		WiFi_Close(50);
		Connect_flag = 0;                //连接成功标志置位
		WifiMsg.ConnectFlag = 0;
		u1_printf("关闭连接成功\r\n");
	}else
	{
		
	}
}
