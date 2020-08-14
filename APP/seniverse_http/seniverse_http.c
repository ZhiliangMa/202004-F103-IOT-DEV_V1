#include "seniverse_http.h"


/*-------------------------------------------------*/
/*函数名：连接TCP服务器，并进入透传模式            */
/*参  数：timeout： 超时时间（100ms的倍数）        */
/*返回值：0：正确  其他：错误                      */
/*-------------------------------------------------*/
char WiFi_Connect_Seniverse_Server(int timeout)
{	
	WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区   
	WiFi_printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",SeniverseServerIP,SeniverseServerPort);//发送连接服务器指令
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
/*函数名：Seniverse服务器 HTTP GET报文             */
/*参  数：device_id： 设备ID                       */
/*返回值：0：正确  其他：错误                      */
/*-------------------------------------------------*/
void Seniverse_GET(char *location)
{
	memset(TXbuff,0,2048);   //清空缓冲区
	sprintf(TXbuff,"GET https://api.seniverse.com/v3/weather/daily.json?key=%s&location=%s&language=en&unit=c&start=0&days=5 HTTP/1.1\r\n",SeniverseAPI_KEY,location);//构建报文
	strcat(TXbuff,"Host: api.seniverse.com\r\n\r\n");	//追加报文
}

/*
 * 功能：从 "2020-08-10" 格式字符串中，提前日期
 *
 * char *xstr：要解析的字符串
 * struct xDate *xdate：解析出的日期存放的结构体
 *
 */
void Get_dateLine(char *xstr, struct xDate *xdate)
{
	xdate->year = atoi(xstr);
	xdate->month = atoi(xstr+5);
	xdate->day = atoi(xstr+8);
}

/*
 * 功能：从 心知天气返回的Json报文中，提取当日天气（早晚两个天气代码）
 *
 * char *xstr：要解析的字符串
 * struct xDateWeather *DateWeather：存放解析出天气代码的结构体
 * 返回值：解析天气后的字符串索引指针，在解析多日天气时级联使用
 *
 */
char* JsonGetDateWeather(char *xstr, struct xDateWeather *DateWeather)
{	
	xstr = strstr(xstr,"\"date\":"); //搜索 "date": 查询 第一天天气
	if(xstr!=NULL){                       //如果搜索到了，进入if
		// 解析今天的日期
		Get_dateLine(xstr+8, &DateWeather->Date );
		
		// 解析今天白天的天气代码
		xstr = strstr(xstr,"\"code_day\":"); //搜索 "code_day": 查询 第一天白天的天气代码
		if(xstr!=NULL){
			DateWeather->day.weaCode = atoi(xstr+12);
		}
		// 解析今天夜晚的天气代码
		xstr = strstr(xstr,"\"code_night\":"); //搜索 "code_night": 查询 第一天夜晚的天气代码
		if(xstr!=NULL){
			DateWeather->night.weaCode = atoi(xstr+14);
		}
		
		return xstr;
	}
	else
		return 0;
}

/*
 * 功能：从 心知天气返回的Json报文中，提取三天天气
 *
 * char *xstr：要解析的字符串
 * struct xWeather *Weather：存放解析出天气代码的结构体
 *
 */
void JsonGetAllDateWeather(char *strP, struct xWeather *Weather)
{
	strP = strstr(strP,"\"results\":");			//搜索"results": 表示数据流操作正确，进入if
	if(strP){
		strP = strstr(strP,"\"daily\":");		//搜索 "daily": 表示获取的数据流，是GET报文 查询的天气
		if(strP){
			
			// 解析今天的日期和天气
			strP = JsonGetDateWeather(strP, &Weather->today);
			// 解析明天的日期和天气
			strP = JsonGetDateWeather(strP, &Weather->tomorrow);
			// 解析后天的日期和天气
			strP = JsonGetDateWeather(strP, &Weather->dat);
		}
	}
}

/*
 * 功能：从 心知天气服务器获取近三天天气，和当前时间用于校准本地时钟
 * 任务流程
 *
 */
extern TimerHandle_t 	netClockCountTimer_Handle;	//周期定时器句柄
void Seniverse_GetWeather(void)
{
	uint16_t xcnt=0;
	
	/* ESP8266 TCP连接 心知天气，使用HTTP获取天气 */
	if(!WifiMsg.ConnectFlag)
	{
		u1_printf("准备连接 心知天气\r\n");	//串口提示数据
		/* TCP连接 心知天气服务器，并进入透传模式 */
		WifiMsg.Status = WiFi_Connect_Seniverse_Server(50);	//连接服务器，100ms超时单位，总计5s超时时间
		printf("连接返回值：%1d\r\n", WifiMsg.Status);
		/* 判断是否已经连接服务器 */
		if(WifiMsg.Status == 0)				//连接成功返回0	
		{
			/* 服务器连接成功 */
			u1_printf("连接 心知天气成功\r\n"); //串口提示数据
			WiFi_RxCounter=0;                           //WiFi接收数据量变量清零
			memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //清空WiFi接收缓冲区
			Connect_flag = 1;				//连接成功标志置位
			WifiMsg.ConnectFlag = 1;
			/* 向 心知天气发送 GET请求，获取天气 */
			WifiMsg.U2_RxCompleted = 0;		//串口2接收完成标志位清零
			Seniverse_GET(HefeiCity);		//构建查询所在城市天气的报文
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
			
				/* 天气解析 */
				if(strstr(&RXbuff[2],"200 OK")){		//搜索200 OK 表示报文正确，进入if
					
					/* 解析返回JSON格式的近几日天气 */
					JsonGetAllDateWeather(&RXbuff[2], &Device.Weather);
					
					/* OLED 显示天气 */
					ui_ShowAllWeather(&Device.Weather);
					
				}else{
					u1_printf("报文错误\r\n");          //串口提示数据
					u1_printf("%s\r\n",&RXbuff[2]);     //串口提示数据
				}
				
				/* 时间解析 */
				if(strstr(&RXbuff[2],"Date:")){
					//复位软定时器，防止刚更新网络时间后马上就+1
					xTimerReset(netClockCountTimer_Handle, 0);
					// 时间解析
					GMT_ToBeijingTime(RXbuff+2, &Device.Time);
					// OLED显示时间
					//OLED_show1608Time(32, 0, switchIFS, &Device.Time);
				}else{
					
				}
			}
		}else if(WifiMsg.Status == 1)		//服务器未开启返回1
		{
			/* 服务器连接失败 */
			Connect_flag = 0;                //连接成功标志清除
			WifiMsg.ConnectFlag = 0;
			u1_printf("连接 心知天气失败\r\n"); //串口提示数据
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

// 一组请求和返回的HTTP报文
//GET https://api.seniverse.com/v3/weather/daily.json?key=S6eMmAGubL0Twlnxo&location=hefei&language=en&unit=c&start=0&days=5 HTTP/1.1
//Host: api.seniverse.com
//
//

//HTTP/1.1 200 OK
//Date: Mon, 10 Aug 2020 01:15:23 GMT
//Content-Type: application/json; charset=utf-8
//Content-Length: 1006
//Connection: keep-alive
//X-Instance-Id: 15823285-716b-470e-9e82-f6f9d04e7729
//X-RateLimit-Limit-minute: 20
//X-RateLimit-Remaining-minute: 5
//X-Powered-By: Express
//ETag: W/"3ee-8onZwZ51HvCMX8XMGxfjSHkmG3M"
//X-Kong-Upstream-Latency: 1
//X-Kong-Proxy-Latency: 8
//Via: kong/0.14.1
//
//{"results":[{"location":{"id":"WTEMH46Z5N09","name":"Hefei","country":"CN","path":"Hefei,Hefei,Anhui,China","timezone":"Asia/Shanghai","timezone_offset":"+08:00"},"daily":[{"date":"2020-08-10","text_day":"Thundershower","code_day":"11","text_night":"Thundershower","code_night":"11","high":"32","low":"25","rainfall":"1.3","precip":"","wind_direction":"SE","wind_direction_degree":"135","wind_speed":"16.20","wind_scale":"3","humidity":"96"},{"date":"2020-08-11","text_day":"Light rain","code_day":"13","text_night":"Overcast","code_night":"9","high":"32","low":"26","rainfall":"9.9","precip":"","wind_direction":"SE","wind_direction_degree":"136","wind_speed":"25.20","wind_scale":"4","humidity":"93"},{"date":"2020-08-12","text_day":"Light rain","code_day":"13","text_night":"Cloudy","code_night":"4","high":"34","low":"26","rainfall":"2.5","precip":"","wind_direction":"S","wind_direction_degree":"180","wind_speed":"16.20","wind_scale":"3","humidity":"79"}],"last_update":"2020-08-10T07:35:51+08:00"}]}
