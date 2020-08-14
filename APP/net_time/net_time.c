#include "net_time.h"

/*
 * 功能：由HTTP的Date段解析出GMT时间，并转换为 北京时间
 * 示例：Date: Thu, 11 Jul 2015 15:33:24 GMT
 *
 * char *strP：要解析的HTTP的Date段
 * struct xTime *Time：解析出的时间，存放的结构体
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
	
	// 转换时区，北京时区 = GMT+8h
	xhour += 8;
	if(xhour >24)
		xhour -= 24;
	
	Time->hour = xhour;
	Time->min = xmin;
	Time->sec = xsec;
}

/*
 * 功能：时间 +1s
 *
 * struct xTime *Time：存放时间的结构体
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
