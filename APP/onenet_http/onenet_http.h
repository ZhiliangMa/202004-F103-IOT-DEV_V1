/*-------------------------------------------------*/
/*            ��γ����STM32ϵ�п�����              */
/*-------------------------------------------------*/
/*                                                 */
/*            onenet_http���ܵ�ͷ�ļ�              */
/*                                                 */
/*-------------------------------------------------*/

#ifndef _ONENET_HTTP_H
#define _ONENET_HTTP_H

#define SWITCH_DID         "597934026"	//4·���أ���ƷID
#define TEMPHUMI_DID       "530382191"
#define API_KEY            "D3P7UAU2MJWTnt76NHgKrckVKpE="//��Ʒ��Master-APIkey����ʱ��ѯ���ǲ����

extern char *ServerIP;         //��ŷ�����IP��������
extern int  ServerPort;        //��ŷ������Ķ˿ں���
extern char RXbuff[2048];      //�������ݻ�����
extern char TXbuff[2048];      //�������ݻ�����

void OneNet_GET(char *device_id);
void OneNet_POST(char *device_id);

void OneNET_GetSwitch(void);
	
#endif
