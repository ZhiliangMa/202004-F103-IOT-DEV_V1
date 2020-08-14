#include "net_time.h"

/*
 * ���ܣ���HTTP��Date�ν�����GMTʱ�䣬��ת��Ϊ ����ʱ��
 * ʾ����Date: Thu, 11 Jul 2015 15:33:24 GMT
 *
 * char *strP��Ҫ������HTTP��Date��
 * struct xTime *Time����������ʱ�䣬��ŵĽṹ��
 *
 */
void GMT_ToBeijingTime(char *strP, struct xTime *Time)
{
	uint8_t xhour,xmin,xsec;
	
	strP = strstr(strP, "Date:");
	strP += 6;
	strP = strstr(strP, ":");
	
	xhour = atoi(strP-2);
	xmin  = atoi(strP+1);
	xsec  = atoi(strP+4);
	
	// ת��ʱ��������ʱ�� = GMT+8h
	xhour += 8;
	if(xhour >24)
		xhour -= 24;
	
	Time->hour = xhour;
	Time->min = xmin;
	Time->sec = xsec;
}

/*
 * ���ܣ�ʱ�� +1s
 *
 * struct xTime *Time�����ʱ��Ľṹ��
 *
 */
void TimePlusOneSecond(struct xTime *Time)
{
	Time->sec ++;
	if(Time->sec > 59)
	{
		Time->sec = 0;
		Time->min ++;
	}
	
	if(Time->min > 59)
	{
		Time->min = 0;
		Time->hour ++;
	}
	
	if(Time->hour > 24)
	{
		Time->hour = 1;
	}
}
