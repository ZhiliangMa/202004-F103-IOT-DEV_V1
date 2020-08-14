#include "seniverse_http.h"


/*-------------------------------------------------*/
/*������������TCP��������������͸��ģʽ            */
/*��  ����timeout�� ��ʱʱ�䣨100ms�ı�����        */
/*����ֵ��0����ȷ  ����������                      */
/*-------------------------------------------------*/
char WiFi_Connect_Seniverse_Server(int timeout)
{	
	WiFi_RxCounter=0;                               //WiFi������������������                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //���WiFi���ջ�����   
	WiFi_printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",SeniverseServerIP,SeniverseServerPort);//�������ӷ�����ָ��
	while(timeout--){                               //�ȴ���ʱ���
		Delay_Ms(100);                              //��ʱ100ms	
		if(strstr(WiFi_RX_BUF ,"CONNECT"))          //������ܵ�CONNECT��ʾ���ӳɹ�
			break;                                  //����whileѭ��
		if(strstr(WiFi_RX_BUF ,"CLOSED"))           //������ܵ�CLOSED��ʾ������δ����
			return 1;                               //������δ��������1
		if(strstr(WiFi_RX_BUF ,"ALREADY CONNECTED"))//������ܵ�ALREADY CONNECTED�Ѿ���������
			return 2;                               //�Ѿ��������ӷ���2
		u1_printf("%d ",timeout);                   //����������ڵĳ�ʱʱ��  
	}
	u1_printf("\r\n");                        //���������Ϣ
	if(timeout<=0)return 3;                   //��ʱ���󣬷���3
	else                                      //���ӳɹ���׼������͸��
	{
		u1_printf("׼������͸��\r\n");                  //������ʾ��Ϣ
		WiFi_RxCounter=0;                               //WiFi������������������                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //���WiFi���ջ�����     
		WiFi_printf("AT+CIPSEND\r\n");                  //���ͽ���͸��ָ��
		while(timeout--){                               //�ȴ���ʱ���
			Delay_Ms(100);                              //��ʱ100ms	
			if(strstr(WiFi_RX_BUF,"\r\nOK\r\n\r\n>"))   //���������ʾ����͸���ɹ�
				break;                          //����whileѭ��
			u1_printf("%d ",timeout);           //����������ڵĳ�ʱʱ��  
		}
		if(timeout<=0)return 4;                 //͸����ʱ���󣬷���4	
	}
	return 0;	                                //�ɹ�����0	
}


/*-------------------------------------------------*/
/*��������Seniverse������ HTTP GET����             */
/*��  ����device_id�� �豸ID                       */
/*����ֵ��0����ȷ  ����������                      */
/*-------------------------------------------------*/
void Seniverse_GET(char *location)
{
	memset(TXbuff,0,2048);   //��ջ�����
	sprintf(TXbuff,"GET https://api.seniverse.com/v3/weather/daily.json?key=%s&location=%s&language=en&unit=c&start=0&days=5 HTTP/1.1\r\n",SeniverseAPI_KEY,location);//��������
	strcat(TXbuff,"Host: api.seniverse.com\r\n\r\n");	//׷�ӱ���
}

/*
 * ���ܣ��� "2020-08-10" ��ʽ�ַ����У���ǰ����
 *
 * char *xstr��Ҫ�������ַ���
 * struct xDate *xdate�������������ڴ�ŵĽṹ��
 *
 */
void Get_dateLine(char *xstr, struct xDate *xdate)
{
	xdate->year = atoi(xstr);
	xdate->month = atoi(xstr+5);
	xdate->day = atoi(xstr+8);
}

/*
 * ���ܣ��� ��֪�������ص�Json�����У���ȡ�������������������������룩
 *
 * char *xstr��Ҫ�������ַ���
 * struct xDateWeather *DateWeather����Ž�������������Ľṹ��
 * ����ֵ��������������ַ�������ָ�룬�ڽ�����������ʱ����ʹ��
 *
 */
char* JsonGetDateWeather(char *xstr, struct xDateWeather *DateWeather)
{	
	xstr = strstr(xstr,"\"date\":"); //���� "date": ��ѯ ��һ������
	if(xstr!=NULL){                       //����������ˣ�����if
		// �������������
		Get_dateLine(xstr+8, &DateWeather->Date );
		
		// ��������������������
		xstr = strstr(xstr,"\"code_day\":"); //���� "code_day": ��ѯ ��һ��������������
		if(xstr!=NULL){
			DateWeather->day.weaCode = atoi(xstr+12);
		}
		// ��������ҹ�����������
		xstr = strstr(xstr,"\"code_night\":"); //���� "code_night": ��ѯ ��һ��ҹ�����������
		if(xstr!=NULL){
			DateWeather->night.weaCode = atoi(xstr+14);
		}
		
		return xstr;
	}
	else
		return 0;
}

/*
 * ���ܣ��� ��֪�������ص�Json�����У���ȡ��������
 *
 * char *xstr��Ҫ�������ַ���
 * struct xWeather *Weather����Ž�������������Ľṹ��
 *
 */
void JsonGetAllDateWeather(char *strP, struct xWeather *Weather)
{
	strP = strstr(strP,"\"results\":");			//����"results": ��ʾ������������ȷ������if
	if(strP){
		strP = strstr(strP,"\"daily\":");		//���� "daily": ��ʾ��ȡ������������GET���� ��ѯ������
		if(strP){
			
			// ������������ں�����
			strP = JsonGetDateWeather(strP, &Weather->today);
			// ������������ں�����
			strP = JsonGetDateWeather(strP, &Weather->tomorrow);
			// ������������ں�����
			strP = JsonGetDateWeather(strP, &Weather->dat);
		}
	}
}

/*
 * ���ܣ��� ��֪������������ȡ�������������͵�ǰʱ������У׼����ʱ��
 * ��������
 *
 */
extern TimerHandle_t 	netClockCountTimer_Handle;	//���ڶ�ʱ�����
void Seniverse_GetWeather(void)
{
	uint16_t xcnt=0;
	
	/* ESP8266 TCP���� ��֪������ʹ��HTTP��ȡ���� */
	if(!WifiMsg.ConnectFlag)
	{
		u1_printf("׼������ ��֪����\r\n");	//������ʾ����
		/* TCP���� ��֪������������������͸��ģʽ */
		WifiMsg.Status = WiFi_Connect_Seniverse_Server(50);	//���ӷ�������100ms��ʱ��λ���ܼ�5s��ʱʱ��
		printf("���ӷ���ֵ��%1d\r\n", WifiMsg.Status);
		/* �ж��Ƿ��Ѿ����ӷ����� */
		if(WifiMsg.Status == 0)				//���ӳɹ�����0	
		{
			/* ���������ӳɹ� */
			u1_printf("���� ��֪�����ɹ�\r\n"); //������ʾ����
			WiFi_RxCounter=0;                           //WiFi������������������
			memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //���WiFi���ջ�����
			Connect_flag = 1;				//���ӳɹ���־��λ
			WifiMsg.ConnectFlag = 1;
			/* �� ��֪�������� GET���󣬻�ȡ���� */
			WifiMsg.U2_RxCompleted = 0;		//����2������ɱ�־λ����
			Seniverse_GET(HefeiCity);		//������ѯ���ڳ��������ı���
			WiFi_printf(TXbuff);			//�ѹ����õı��ķ���������
			//����ʱ��3 3s�Ķ�ʱ ���3s�ڷ����� û����������Ҫ���¸�λ8266
			/* ���㱨�ķ��س�ʱʱ�䣬�3s */
			xcnt = 300;
			while((!WifiMsg.U2_RxCompleted) && xcnt)
			{
				--xcnt;
				vTaskDelay(10);
			}
			printf("���ճ�ʱ = %d ms\r\n", (300-xcnt)*10);
			
			/* ������յ�����ӡ��Ϣ��û�н��յ�����������GET */
			if(WifiMsg.U2_RxCompleted)
			{
				WifiMsg.U2_RxCompleted = 0;		//����2������ɱ�־λ����
				printf(RXbuff+2);			//��ӡ���յ�ESP8266��HTTP��Ϣ
			
				/* �������� */
				if(strstr(&RXbuff[2],"200 OK")){		//����200 OK ��ʾ������ȷ������if
					
					/* ��������JSON��ʽ�Ľ��������� */
					JsonGetAllDateWeather(&RXbuff[2], &Device.Weather);
					
					/* OLED ��ʾ���� */
					ui_ShowAllWeather(&Device.Weather);
					
				}else{
					u1_printf("���Ĵ���\r\n");          //������ʾ����
					u1_printf("%s\r\n",&RXbuff[2]);     //������ʾ����
				}
				
				/* ʱ����� */
				if(strstr(&RXbuff[2],"Date:")){
					//��λ��ʱ������ֹ�ո�������ʱ������Ͼ�+1
					xTimerReset(netClockCountTimer_Handle, 0);
					// ʱ�����
					GMT_ToBeijingTime(RXbuff+2, &Device.Time);
					// OLED��ʾʱ��
					//OLED_show1608Time(32, 0, switchIFS, &Device.Time);
				}else{
					
				}
			}
		}else if(WifiMsg.Status == 1)		//������δ��������1
		{
			/* ����������ʧ�� */
			Connect_flag = 0;                //���ӳɹ���־���
			WifiMsg.ConnectFlag = 0;
			u1_printf("���� ��֪����ʧ��\r\n"); //������ʾ����
		}else	//����2���Ѿ��������ӷ���2
		{
			/* �������Ѿ��������� */
		}
		
		/* ���۽��յ�������Ϣ��񣬶��ر����� �ر����ӵļ���δ�� */
		WiFi_Close(50);
		Connect_flag = 0;                //���ӳɹ���־��λ
		WifiMsg.ConnectFlag = 0;
		u1_printf("�ر����ӳɹ�\r\n");
	}else
	{
		
	}
}

// һ������ͷ��ص�HTTP����
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
