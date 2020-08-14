/*-------------------------------------------------*/
/*            超纬电子STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*            操作602Wifi功能的源文件              */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"    //包含需要的头文件
#include "string.h"
//#include "main.h"         //包含需要的头文件
#include "wifi.h"	      //包含需要的头文件
#include "delay.h"	      //包含需要的头文件
#include "uart1.h"	      //包含需要的头文件
//#include "led.h"          //包含需要的头文件
//#include "key.h"          //包含需要的头文件
#include "onenet_http.h"  //包含需要的头文件

char wifi_mode = 0;         //联网模式 0：SSID和密码写在程序里   1：Smartconfig方式用APP发送
char Connect_flag = 0;      //同服务器连接状态  0：还没有连接服务器  1：连接上服务器了

/*-------------------------------------------------*/
/*函数名：初始化WiFi的复位IO                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void WiFi_ResetIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;                      //定义一个设置IO端口参数的结构体
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE);   //使能PA端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;                 //准备设置PA4
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;         //速率50Mhz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   		  //推免输出方式
	GPIO_Init(GPIOA, &GPIO_InitStructure);            		  //设置PA4
	RESET_IO(1);                                              //复位IO拉高电平
}
/*-------------------------------------------------*/
/*函数名：WiFi发送设置指令                         */
/*参  数：cmd：指令                                */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_SendCmd(char *cmd, int timeout)
{
	WiFi_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //清空WiFi接收缓冲区 
	WiFi_printf("%s\r\n",cmd);                  //发送指令
	while(timeout--){                           //等待超时时间到0
		Delay_Ms(100);                          //延时100ms
		if(strstr(WiFi_RX_BUF,"OK"))            //如果接收到OK表示指令成功
			break;       						//主动跳出while循环
		u1_printf("%d ",timeout);               //串口输出现在的超时时间
	}
	u1_printf("\r\n");                          //串口输出信息
	if(timeout<=0)return 1;                     //如果timeout<=0，说明超时时间到了，也没能收到OK，返回1
	else return 0;		         				//反之，表示正确，说明收到OK，通过break主动跳出while
}
/*-------------------------------------------------*/
/*函数名：WiFi复位                                 */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_Reset(int timeout)
{
	int xtimeout = timeout;
	
	WiFi_RxCounter=0;                       //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE); //清空WiFi接收缓冲区
	RESET_IO(0);                                    //复位IO拉低电平
	Delay_Ms(500);                                  //延时500ms
	RESET_IO(1);                                    //复位IO拉高电平	
	while(timeout--){                               //等待超时时间到0
		Delay_Ms(100);                              //延时100ms
		if(strstr(WiFi_RX_BUF,"ready"))             //如果接收到ready表示复位成功
			break;       						    //主动跳出while循环
		u1_printf("timeout = %d ms\r\n", (xtimeout-timeout)*100);                   //串口输出现在的超时时间
	}
	u1_printf(Usart2_RxBuff);//调试输出
	u1_printf("\r\n");                              //串口输出信息
	if(timeout<=0)return 1;                         //如果timeout<=0，说明超时时间到了，也没能收到ready，返回1
	else return 0;		         				    //反之，表示正确，说明收到ready，通过break主动跳出while
}
/*-------------------------------------------------*/
/*函数名：WiFi加入路由器指令                       */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_JoinAP(int timeout)
{		
	WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区 
	WiFi_printf("AT+CWJAP=\"%s\",\"%s\"\r\n",SSID,PASS); //发送指令	
	while(timeout--){                               //等待超时时间到0
		Delay_Ms(1000);                             //延时1s
		if(strstr(WiFi_RX_BUF,"WIFI GOT IP\r\n\r\nOK")) //如果接收到WIFI GOT IP表示成功
			break;       						    //主动跳出while循环
		u1_printf("%d ",timeout);                   //串口输出现在的超时时间
	}
	u1_printf("\r\n");                              //串口输出信息
	if(timeout<=0)return 1;                         //如果timeout<=0，说明超时时间到了，也没能收到WIFI GOT IP，返回1
	return 0;                                       //正确，返回0
}
/*-------------------------------------------------*/
/*函数名：WiFi_Smartconfig                         */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_Smartconfig(int timeout)
{	
	WiFi_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //清空WiFi接收缓冲区     
	while(timeout--){                           //等待超时时间到0
		Delay_Ms(1000);                         //延时1s
		if(strstr(WiFi_RX_BUF,"connected"))     //如果串口接受到connected表示成功
			break;                              //跳出while循环  
		u1_printf("%d ",timeout);               //串口输出现在的超时时间  
	}	
	u1_printf("\r\n");                          //串口输出信息
	if(timeout<=0)return 1;                     //超时错误，返回1
	return 0;                                   //正确返回0
}
/*-------------------------------------------------*/
/*函数名：等待加入路由器                           */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_WaitAP(int timeout)
{		
	while(timeout--){                               //等待超时时间到0
		Delay_Ms(1000);                             //延时1s
		if(strstr(WiFi_RX_BUF,"WIFI GOT IP"))       //如果接收到WIFI GOT IP表示成功
			break;       						    //主动跳出while循环
		u1_printf("%d ",timeout);                   //串口输出现在的超时时间
	}
	u1_printf("\r\n");                              //串口输出信息
	if(timeout<=0)return 1;                         //如果timeout<=0，说明超时时间到了，也没能收到WIFI GOT IP，返回1
	return 0;                                       //正确，返回0
}
/*-------------------------------------------------*/
/*函数名：等待连接wifi，获取IP地址                 */
/*参  数：ip：保存IP的数组                         */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_GetIP(int timeout)
{
	char *presult1,*presult2;
	char ip[50];
	
	WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区 
	WiFi_printf("AT+CIFSR\r\n");                    //发送指令	
	while(timeout--){                               //等待超时时间到0
		Delay_Ms(100);                              //延时100ms
		if(strstr(WiFi_RX_BUF,"OK"))                //如果接收到OK表示成功
			break;       						    //主动跳出while循环
		u1_printf("%d ",timeout);                   //串口输出现在的超时时间
	}
	u1_printf("\r\n");                              //串口输出信息
	if(timeout<=0)return 1;                         //如果timeout<=0，说明超时时间到了，也没能收到OK，返回1
	else{
		presult1 = strstr(WiFi_RX_BUF,"\"");
		if( presult1 != NULL ){
			presult2 = strstr(presult1+1,"\"");
			if( presult2 != NULL ){
				memcpy(ip,presult1+1,presult2-presult1-1);
				u1_printf("ESP8266的IP地址：%s\r\n",ip);     //串口显示IP地址
				return 0;    //正确返回0
			}else return 2;  //未收到预期数据
		}else return 3;      //未收到预期数据	
	}
}
/*-------------------------------------------------*/
/*函数名：获取连接状态                             */
/*参  数：无                                       */
/*返回值：连接状态                                 */
/*        0：无状态                                */
/*        1：有客户端接入                          */
/*        2：有客户端断开                          */
/*-------------------------------------------------*/
char WiFi_Get_LinkSta(void)
{
	char id_temp[10]={0};    //缓冲区，存放ID
	char sta_temp[10]={0};   //缓冲区，存放状态
	
	if(strstr(WiFi_RX_BUF,"CONNECT")){                 //如果接受到CONNECT表示有客户端连接	
		sscanf(WiFi_RX_BUF,"%[^,],%[^,]",id_temp,sta_temp);
		u1_printf("有客户端接入，ID=%s\r\n",id_temp);  //串口显示信息
		WiFi_RxCounter=0;                              //WiFi接收数据量变量清零                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);        //清空WiFi接收缓冲区     
		return 1;                                      //有客户端接入
	}else if(strstr(WiFi_RX_BUF,"CLOSED")){            //如果接受到CLOSED表示有链接断开	
		sscanf(WiFi_RX_BUF,"%[^,],%[^,]",id_temp,sta_temp);
		u1_printf("有客户端断开，ID=%s\r\n",id_temp);        //串口显示信息
		WiFi_RxCounter=0;                                    //WiFi接收数据量变量清零                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);              //清空WiFi接收缓冲区     
		return 2;                                            //有客户端断开
	}else return 0;                                          //无状态改变	
}
/*-------------------------------------------------*/
/*函数名：获取客户端数据                           */
/*        两组回车换行符\r\n\r\n作为数据的结束符   */
/*参  数：data：数据缓冲区                         */
/*参  数：len： 数据量                             */
/*参  数：id：  发来数据的客户端的连接ID           */
/*返回值：数据状态                                 */
/*        0：无数据                                */
/*        1：有数据                                */
/*-------------------------------------------------*/
char WiFi_Get_Data(char *data, char *len, char *id)
{
	char temp[10]={0};      //缓冲区
	char *presult;

	if(strstr(WiFi_RX_BUF,"\r\n\r\n")){                     //两个连着的回车换行作为数据的结束符
		sscanf(WiFi_RX_BUF,"%[^,],%[^,],%[^:]",temp,id,len);//截取各段数据，主要是id和数据长度	
		presult = strstr(WiFi_RX_BUF,":");                  //查找冒号。冒号后的是数据
		if( presult != NULL )                               //找到冒号
			sprintf((char *)data,"%s",(presult+1));         //冒号后的数据，复制到data
		WiFi_RxCounter=0;                                   //WiFi接收数据量变量清零                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);             //清空WiFi接收缓冲区    
		return 1;                                           //有数据到来
	} else return 0;                                        //无数据到来
}
/*-------------------------------------------------*/
/*函数名：服务器发送数据                           */
/*参  数：databuff：数据缓冲区<2048                */
/*参  数：data_len：数据长度                       */
/*参  数：id：      客户端的连接ID                 */
/*参  数：timeout： 超时时间（10ms的倍数）         */
/*返回值：错误值                                   */
/*        0：无错误                                */
/*        1：等待发送数据超时                      */
/*        2：连接断开了                            */
/*        3：发送数据超时                          */
/*-------------------------------------------------*/
char WiFi_SendData(char id, char *databuff, int data_len, int timeout)
{    
	int i;
	
	WiFi_RxCounter=0;                                 //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);           //清空WiFi接收缓冲区 
	WiFi_printf("AT+CIPSEND=%d,%d\r\n",id,data_len);  //发送指令	
    while(timeout--){                                 //等待超时与否	
		Delay_Ms(10);                                 //延时10ms
		if(strstr(WiFi_RX_BUF,">"))                   //如果接收到>表示成功
			break;       						      //主动跳出while循环
		u1_printf("%d ",timeout);                     //串口输出现在的超时时间
	}
	if(timeout<=0)return 1;                                   //超时错误，返回1
	else{                                                     //没超时，正确       	
		WiFi_RxCounter=0;                                     //WiFi接收数据量变量清零                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);               //清空WiFi接收缓冲区 	
		for(i=0;i<data_len;i++)WiFi_printf("%c",databuff[i]); //发送数据	
		while(timeout--){                                     //等待超时与否	
			Delay_Ms(10);                                     //延时10ms
			if(strstr(WiFi_RX_BUF,"SEND OK")){                //如果接受SEND OK，表示发送成功			 
			WiFi_RxCounter=0;                                 //WiFi接收数据量变量清零                        
			memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);           //清空WiFi接收缓冲区 			
				break;                                        //跳出while循环
			} 
			if(strstr(WiFi_RX_BUF,"link is not valid")){      //如果接受link is not valid，表示连接断开			
				WiFi_RxCounter=0;                             //WiFi接收数据量变量清零                        
				memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);       //清空WiFi接收缓冲区 			
				return 2;                                     //返回2
			}
	    }
		if(timeout<=0)return 3;      //超时错误，返回3
		else return 0;	            //正确，返回0
	}	
}
/*-------------------------------------------------*/
/*函数名：连接TCP服务器，并进入透传模式            */
/*参  数：timeout： 超时时间（100ms的倍数）        */
/*返回值：0：正确  其他：错误                      */
/*-------------------------------------------------*/
char WiFi_Connect_Server(int timeout)
{	
	WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区   
	WiFi_printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",ServerIP,ServerPort);//发送连接服务器指令
	while(timeout--){                               //等待超时与否
		Delay_Ms(100);                              //延时100ms	
		if(strstr(WiFi_RX_BUF ,"CONNECT"))          //如果接受到CONNECT表示连接成功
			break;                                  //跳出while循环
		if(strstr(WiFi_RX_BUF ,"CLOSED"))           //如果接受到CLOSED表示服务器未开启
			return 1;                               //服务器未开启返回1
		if(strstr(WiFi_RX_BUF ,"ALREADY CONNECTED"))//如果接受到ALREADY CONNECTED已经建立连接
			return 2;                               //已经建立连接返回2
		u1_printf("%d ",timeout);                   //串口输出现在的超时时间  
	}
	u1_printf("\r\n");                        //串口输出信息
	if(timeout<=0)return 3;                   //超时错误，返回3
	else                                      //连接成功，准备进入透传
	{
		u1_printf("准备进入透传\r\n");                  //串口显示信息
		WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区     
		WiFi_printf("AT+CIPSEND\r\n");                  //发送进入透传指令
		while(timeout--){                               //等待超时与否
			Delay_Ms(100);                              //延时100ms	
			if(strstr(WiFi_RX_BUF,"\r\nOK\r\n\r\n>"))   //如果成立表示进入透传成功
				break;                          //跳出while循环
			u1_printf("%d ",timeout);           //串口输出现在的超时时间  
		}
		if(timeout<=0)return 4;                 //透传超时错误，返回4	
	}
	return 0;	                                //成功返回0	
}
/*-------------------------------------------------*/
/*函数名：断开连接                                 */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_Close(int timeout)
{		
    //Delay_Ms(200);                                  //延时
	WiFi_printf("+++");                             //发送 退出透传
	//Delay_Ms(1500);                                 //延时
	Delay_Ms(50);									// 延时，经测试最小为20ms正常。稳妥起见，50ms
	Connect_flag = 0;                               //连接成功标志置位清除
	WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区 
	WiFi_printf("AT+CIPCLOSE\r\n");                 //发送指令	
	while(timeout--){                               //等待超时时间到0
		Delay_Ms(100);                              //延时100ms
		if(strstr(WiFi_RX_BUF,"OK"))                //如果接收到OK表示成功
			break;       						    //主动跳出while循环
		u1_printf("%d ",timeout);                   //串口输出现在的超时时间
	}
	u1_printf("\r\n");                              //串口输出信息
	if(timeout<=0)return 1;                         //如果timeout<=0，说明超时时间到了，返回1
	return 0;                                       //正确，返回0
}
/*-------------------------------------------------*/
/*函数名：初始化wifi模块                           */
/*参  数：无                                       */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
uint8_t KEY2_IN_STA=1;
char WiFi_Init(void)
{		
	u1_printf("准备复位模块\r\n");                     //串口提示数据
	if(WiFi_Reset(50)){                                //复位，100ms超时单位，总计5s超时时间
		u1_printf("复位失败，准备重启\r\n");           //返回非0值，进入if，串口提示数据
		return 1;                                      //返回1
	}else u1_printf("复位成功\r\n");                   //串口提示数据
	
	u1_printf("准备设置STA模式\r\n");                  //串口提示数据
	if(WiFi_SendCmd("AT+CWMODE=1",50)){                //设置STA模式，100ms超时单位，总计5s超时时间
		u1_printf("设置STA模式失败，准备重启\r\n");    //返回非0值，进入if，串口提示数据
		return 2;                                      //返回2
	}else u1_printf("设置STA模式成功\r\n");            //串口提示数据
	
	if(wifi_mode==0){                                      //如果联网模式=0：SSID和密码写在程序里 
		u1_printf("准备取消自动连接\r\n");                 //串口提示数据
		if(WiFi_SendCmd("AT+CWAUTOCONN=0",50)){            //取消自动连接，100ms超时单位，总计5s超时时间
			u1_printf("取消自动连接失败，准备重启\r\n");   //返回非0值，进入if，串口提示数据
			return 3;                                      //返回3
		}else u1_printf("取消自动连接成功\r\n");           //串口提示数据
				
		u1_printf("准备连接路由器\r\n");                   //串口提示数据	
		if(WiFi_JoinAP(30)){                               //连接路由器,1s超时单位，总计30s超时时间
			u1_printf("连接路由器失败，准备重启\r\n");     //返回非0值，进入if，串口提示数据
			return 4;                                      //返回4	
		}else u1_printf("连接路由器成功\r\n");             //串口提示数据			
	}else{                                                 //如果联网模式=1：Smartconfig方式,用APP发送
		if(KEY2_IN_STA==0){                                    //如果此时K2是按下的
			u1_printf("准备设置自动连接\r\n");                 //串口提示数据
			if(WiFi_SendCmd("AT+CWAUTOCONN=1",50)){            //设置自动连接，100ms超时单位，总计5s超时时间
				u1_printf("设置自动连接失败，准备重启\r\n");   //返回非0值，进入if，串口提示数据
				return 3;                                      //返回3
			}else u1_printf("设置自动连接成功\r\n");           //串口提示数据	
			
			u1_printf("准备开启Smartconfig\r\n");              //串口提示数据
			if(WiFi_SendCmd("AT+CWSTARTSMART",50)){            //开启Smartconfig，100ms超时单位，总计5s超时时间
				u1_printf("开启Smartconfig失败，准备重启\r\n");//返回非0值，进入if，串口提示数据
				return 4;                                      //返回4
			}else u1_printf("开启Smartconfig成功\r\n");        //串口提示数据

			u1_printf("请使用APP软件传输密码\r\n");            //串口提示数据
			if(WiFi_Smartconfig(60)){                          //APP软件传输密码，1s超时单位，总计60s超时时间
				u1_printf("传输密码失败，准备重启\r\n");       //返回非0值，进入if，串口提示数据
				return 5;                                      //返回5
			}else u1_printf("传输密码成功\r\n");               //串口提示数据

			u1_printf("准备关闭Smartconfig\r\n");              //串口提示数据
			if(WiFi_SendCmd("AT+CWSTOPSMART",50)){             //关闭Smartconfig，100ms超时单位，总计5s超时时间
				u1_printf("关闭Smartconfig失败，准备重启\r\n");//返回非0值，进入if，串口提示数据
				return 6;                                      //返回6
			}else u1_printf("关闭Smartconfig成功\r\n");        //串口提示数据
		}else{                                                 //反之，此时K2是没有按下
			u1_printf("等待连接路由器\r\n");                   //串口提示数据	
			if(WiFi_WaitAP(30)){                               //等待连接路由器,1s超时单位，总计30s超时时间
				u1_printf("连接路由器失败，准备重启\r\n");     //返回非0值，进入if，串口提示数据
				return 7;                                      //返回7	
			}else u1_printf("连接路由器成功\r\n");             //串口提示数据					
		}
	}
	
	u1_printf("准备获取IP地址\r\n");                   //串口提示数据
	if(WiFi_GetIP(50)){                                //准备获取IP地址，100ms超时单位，总计5s超时时间
		u1_printf("获取IP地址失败，准备重启\r\n");     //返回非0值，进入if，串口提示数据
		return 10;                                     //返回10
	}else u1_printf("获取IP地址成功\r\n");             //串口提示数据
	
	u1_printf("准备开启透传\r\n");                     //串口提示数据
	if(WiFi_SendCmd("AT+CIPMODE=1",50)){               //开启透传，100ms超时单位，总计5s超时时间
		u1_printf("开启透传失败，准备重启\r\n");       //返回非0值，进入if，串口提示数据
		return 11;                                     //返回11
	}else u1_printf("关闭透传成功\r\n");               //串口提示数据
	
	u1_printf("准备关闭多路连接\r\n");                 //串口提示数据
	if(WiFi_SendCmd("AT+CIPMUX=0",50)){                //关闭多路连接，100ms超时单位，总计5s超时时间
		u1_printf("关闭多路连接失败，准备重启\r\n");   //返回非0值，进入if，串口提示数据
		return 12;                                     //返回12
	}else u1_printf("关闭多路连接成功\r\n");           //串口提示数据

	return 0;                                          //正确返回0	
}
