/*-------------------------------------------------*/
/*            ��γ����STM32ϵ�п�����              */
/*-------------------------------------------------*/
/*                                                 */
/*            onenet_http���ܵ�Դ�ļ�              */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"    //������Ҫ��ͷ�ļ�
#include "onenet_http.h"  //������Ҫ��ͷ�ļ�
#include "stdio.h"        //������Ҫ��ͷ�ļ�
#include "stdarg.h"		  //������Ҫ��ͷ�ļ� 
#include "string.h"       //������Ҫ��ͷ�ļ�
//#include "usart1.h"       //������Ҫ��ͷ�ļ�
//#include "dht12.h"  	  //������Ҫ��ͷ�ļ�

#include "wifi.h"
#include "uart2.h"
#include "delay.h"
#include "usart.h"
#include "structure.h"
#include "uart1.h"
#include "led.h"
#include "oled_ui.h"
#include "net_time.h"

char *ServerIP = "183.230.40.33";          //OneNet������ IP��ַ
int   ServerPort = 80;                     //OneNet������ �˿ں�
char RXbuff[2048];                         //�������ݻ�����
char TXbuff[2048];                         //�������ݻ�����

/*-------------------------------------------------*/
/*��������OneNet������ HTTP GET����                */
/*��  ����device_id�� �豸ID                       */
/*����ֵ��0����ȷ  ����������                      */
/*-------------------------------------------------*/
void OneNet_GET(char *device_id)
{
	char temp[128];
	
	memset(TXbuff,0,2048);   //��ջ�����
    memset(temp,0,128);      //��ջ�����                                             
	sprintf(TXbuff,"GET /devices/%s/datapoints HTTP/1.1\r\n",device_id);//��������
	sprintf(temp,"api-key:%s\r\n",API_KEY);                             //��������
	strcat(TXbuff,temp);                                                //׷�ӱ���
	strcat(TXbuff,"Host:api.heclouds.com\r\n\r\n");                     //׷�ӱ���
}
/*-------------------------------------------------*/
/*��������OneNet������ HTTP POST����               */
/*��  ����device_id�� �豸ID                       */
/*����ֵ��0����ȷ  ����������                      */
/*-------------------------------------------------*/
void OneNet_POST(char *device_id)
{
	char temp[128]; 
	char databuff[128]; 
	unsigned char data[5];
	
	memset(TXbuff,0,2048);   //��ջ�����
    memset(temp,0,128);      //��ջ�����
    memset(databuff,0,128);  //��ջ�����
	sprintf(TXbuff,"POST /devices/%s/datapoints?type=3 HTTP/1.1\r\n",device_id);//��������
	sprintf(temp,"api-key:%s\r\n",API_KEY);          //��������
	strcat(TXbuff,temp);                             //׷�ӱ���
	strcat(TXbuff,"Host:api.heclouds.com\r\n");      //׷�ӱ���	

	while(1){                                                //ѭ��������
		//DHT12_ReadData(data);                                //��ȡ��ʪ������
		if((data[0]+data[1]+data[2]+data[3])==data[4]){      //�ж�У�飬if������ʾ������ȷ	
			sprintf(databuff,"{\"temp_data\":%d.%d,\"humi_data\":%d.%d}",data[2],data[3],data[0],data[1]);   //�����ϱ�����
		    break;                                           //����while
		}//else u1_printf("��ʪ������У��������¶�ȡ\r\n"); //���������Ϣ
	}
	sprintf(temp,"Content-Length:%d\r\n\r\n",strlen(databuff));  //��������
	strcat(TXbuff,temp);                                         //׷�ӱ���
	sprintf(temp,"%s\r\n\r\n",databuff);                         //��������
	strcat(TXbuff,temp);                                         //׷�ӱ���
	//u1_printf("%s\r\n",databuff);
}



/*
 * ���ܣ��� OneNET��������ȡ4·����״̬
 *
 */
void OneNET_GetSwitch(void)
{
	uint16_t xcnt=0;
	char *swcStatus;
	
	/* ESP8266 TCP���� OneNet��ƽ̨��ʹ��HTTP��ȡԶ�̿���״̬ */
	if(!WifiMsg.ConnectFlag)
	{
		u1_printf("׼������ OneNET\r\n");	//������ʾ����
		/* TCP���� OneNET��������������͸��ģʽ */
		WifiMsg.Status = WiFi_Connect_Server(50);	//���ӷ�������100ms��ʱ��λ���ܼ�5s��ʱʱ��
		printf("���ӷ���ֵ��%1d\r\n", WifiMsg.Status);
		/* �ж��Ƿ��Ѿ����ӷ����� */
		if(WifiMsg.Status == 0)				//���ӳɹ�����0	
		{
			/* ���������ӳɹ� */
			u1_printf("���� OneNET�ɹ�\r\n"); //������ʾ����
			WiFi_RxCounter=0;                           //WiFi������������������                        
			memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //���WiFi���ջ����� 
			Connect_flag = 1;				//���ӳɹ���־��λ
			WifiMsg.ConnectFlag = 1;
			/* �� OneNET���� GET���󣬻�ȡ4·����״̬ */
			WifiMsg.U2_RxCompleted = 0;		//����2������ɱ�־λ����
			OneNet_GET(SWITCH_DID);			//������ѯ����״̬�ı���
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
			
				/* �Խ��յ�����Ϣ��������ȡ4·����״̬ */
				if(strstr(&RXbuff[2],"200 OK")){                                //����200 OK ��ʾ������ȷ������if	
					if(strstr(&RXbuff[2],"\"errno\":0")){                       //����"errno":0 ��ʾ������������ȷ������if
						if(strstr(&RXbuff[2],"datastreams")){                   //���� datastreams ��ʾ��ȡ������������GET���� ��ѯ����״̬
							
//									/* ��OLED Gram */
//									ma_OLED_Gram_Clear(switchIFS);
							
							swcStatus = strstr(&RXbuff[2],"\"id\":\"switch_1\""); //���� "id":"switch_1" ��ѯ����1״̬
							if(swcStatus!=NULL){                                  //����������ˣ�����if
								if(*(swcStatus-4) == '0') { printf("\r\nswitch1 OFF\r\n"); Device.Led.Val1=0;LED1_OFF;ui_ShowLed(3,0); }	//�����0���ر�LED1
								else { printf("\r\nswitch1 ON\r\n"); Device.Led.Val1=1;LED1_ON;ui_ShowLed(3,1); }                        	//��֮��1����LED1
								u1_printf("��ѯ����1״̬��ȷ\r\n");										//������ʾ����										
							}
							else u1_printf("��ѯ����1״̬ʧ�ܣ���������Ϣ������\r\n");					//������ʾ����
							
							swcStatus = strstr(&RXbuff[2],"\"id\":\"switch_2\""); //���� "id":"switch_2" ��ѯ����2״̬
							if(swcStatus!=NULL){                                  //����������ˣ�����if
								if(*(swcStatus-4) == '0') { printf("\r\nswitch2 OFF\r\n"); Device.Led.Val2=0;LED2_OFF;ui_ShowLed(2,0); }	//�����0���ر�LED2
								else { printf("\r\nswitch2 ON\r\n"); Device.Led.Val2=1;LED2_ON;ui_ShowLed(2,1); }                        	//��֮��1����LED2
								u1_printf("��ѯ����2״̬��ȷ\r\n");										//������ʾ����										
							}
							else u1_printf("��ѯ����2״̬ʧ�ܣ���������Ϣ������\r\n");					//������ʾ����
							
							swcStatus = strstr(&RXbuff[2],"\"id\":\"switch_3\""); //���� "id":"switch_3" ��ѯ����3״̬
							if(swcStatus!=NULL){                                  //����������ˣ�����if
								if(*(swcStatus-4) == '0') { printf("\r\nswitch3 OFF\r\n"); Device.Led.Val3=0;LED3_OFF;ui_ShowLed(1,0); }	//�����0���ر�LED3
								else { printf("\r\nswitch3 ON\r\n"); Device.Led.Val3=1;LED3_ON;ui_ShowLed(1,1); }                        	//��֮��1����LED3
								u1_printf("��ѯ����3״̬��ȷ\r\n");										//������ʾ����										
							}
							else u1_printf("��ѯ����3״̬ʧ�ܣ���������Ϣ������\r\n");					//������ʾ����
							
							swcStatus = strstr(&RXbuff[2],"\"id\":\"switch_4\""); //���� "id":"switch_4" ��ѯ����4״̬
							if(swcStatus!=NULL){                                  //����������ˣ�����if
								if(*(swcStatus-4) == '0') { printf("\r\nswitch OFF\r\n"); Device.Led.Val4=0;LED4_OFF;ui_ShowLed(0,0); }	//�����0���ر�LED4
								else { printf("\r\nswitch4 ON\r\n"); Device.Led.Val4=1;LED4_ON;ui_ShowLed(0,1); }                        	//��֮��1����LED4
								u1_printf("��ѯ����4״̬��ȷ\r\n");										//������ʾ����										
							}
							else u1_printf("��ѯ����4״̬ʧ�ܣ���������Ϣ������\r\n");					//������ʾ����
							
//									ma_OLED_Refresh_Gram(switchIFS);
						}
					}
				}else{
					u1_printf("���Ĵ���\r\n");          //������ʾ����
					u1_printf("%s\r\n",&RXbuff[2]);     //������ʾ����
				}
			}
		}else if(WifiMsg.Status == 1)		//������δ��������1
		{
			/* ����������ʧ�� */
			Connect_flag = 0;                //���ӳɹ���־���
			WifiMsg.ConnectFlag = 0;
			u1_printf("���� OneNETʧ��\r\n"); //������ʾ����
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
