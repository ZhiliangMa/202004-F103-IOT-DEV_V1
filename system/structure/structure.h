#ifndef __STRUCTURE_H
#define __STRUCTURE_H

#include "stm32f10x.h"

/**
  ******************************************************************************

  * @file    structure.h

  * @author  zhiLiangMa

  * @version V0.0.1

  * @date    2020/05/16

  * @brief   structure.h������״̬��־λ�ṹ��Ķ���

  ******************************************************************************

  * @attention
  *
  *
  * �ṹ����ô����
  ******************************************************************************
  */

struct xRecMsg
{
	/* Uart2 wifiģ�����״̬��־λ */
	uint8_t RxFlag;
	uint16_t RxNum;
	
	uint8_t U2_RxCompleted;	// ����һ������ 0����ʾ����δ��� 1����ʾ�������
	uint8_t ConnectFlag;	// ����TCP/UDP��־λ��0-δ���ӣ�ATָ��״̬��1-������
	
	uint8_t Mode;			// 8266״̬��־ 0����Ҫ��λ�����¸�λ  1������״̬��ģ�鹤��ģʽ��������������͸����
	uint8_t Status;			// ���ӷ�����״̬λ�������Ϸ�������Ҫ��ConnectFlag��1
};

extern struct xRecMsg WifiMsg;

// ����״̬
struct xLed
{
	uint8_t Val1;
	uint8_t Val2;
	uint8_t Val3;
	uint8_t Val4;
};

enum xMonth
{
	Jan = 1,
	Feb,
	Mar,
	Apr,
	May,
	Jun,
	Jul,
	Aug,
	Sep,
	Oct,
	Nov,
	Dec
};

enum xWeek
{
	Mon = 1,
	Tue,
	Wed,
	Thu,
	Fri,
	Sat,
	Sun
};

// ����
struct xDate
{
	uint16_t year;
	enum xMonth month;
	uint8_t day;
	enum xWeek week;
};

// ʱ��
struct xTime
{
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
};

// ��֪������ �����������˵����ǰ׺Ϊn�ı�ʾҹ������
// �����ƣ�Ϊ ����������ƣ�����ת���ƺͶ���ת�粻ͨ����
// �涨��������Ϊ uint8_t
//enum xWeatherCode : uint8_t
enum xWeatherCode
{
	Sunny 			= 0,	// �磨���ڳ��а����磩
	nClear 			= 1,	// �磨���ڳ���ҹ���磩
	Fair 			= 2,	// �磨������а����磩
	nFair 			= 3,	// �磨�������ҹ���磩
	
	Cloudy 			= 4,	// ����
	
	PartlyCloudy 	= 5,	// ������
	nPartlyCloudy 	= 6,	// ������
	
	MostlyCloudy 	= 7,	// �󲿶���
	nMostlyCloudy 	= 8,	// �󲿶���
	
	Overcast 		= 9,	// ��
	
	Shower 			= 10,	// ����
	Thundershower 	= 11,	// ������
	ThundershowerwithHail = 12,	// ��������б���
	LightRain 		= 13,	// С��
	ModerateRain 	= 14,	// ����
	HeavyRain 		= 15,	// ����
	Storm 			= 16,	// ����
	HeavyStorm 		= 17,	// ����
	SevereStorm 	= 18,	// �ش���
	
	IceRain 		= 19,	// ����
	Sleet 			= 20,	// ���ѩ
	SnowFlurry 		= 21,	// ��ѩ
	LightSnow 		= 22,	// Сѩ
	ModerateSnow 	= 23,	// ��ѩ
	HeavySnow 		= 24,	// ��ѩ
	Snowstorm 		= 25,	// ��ѩ
	
	Dust 			= 26,	// ����
	Sand 			= 27,	// ��ɳ
	Duststorm 		= 28,	// ɳ����
	Sandstorm 		= 29,	// ǿɳ����
	
	Foggy 			= 30,	// ��
	Haze 			= 31,	// ��
	
	Windy 			= 32,	// ��
	Blustery 		= 33,	// ���
	Hurricane 		= 34,	// 쫷�
	TropicalStorm 	= 35,	// �ȴ��籩
	Tornado 		= 36,	// �����
	
	Cold 			= 37,	// ��
	Hot 			= 38,	// ��
	
	Unknown 		= 99	// δ֪
};


/* ������������ʵ�ǿ��Ժϲ�һ��� */
struct xPeriodWeather
{
	enum xWeatherCode weaCode;
};

struct xDateWeather
{
	//������
	struct xDate Date;
	
	//������������
	struct xPeriodWeather day;
	//ҹ����������
	struct xPeriodWeather night;
	
	//�ֶ�����
	//struct xDate Date;
	//enum xWeatherCode weaCode;
};

struct xWeather
{
	struct xDateWeather today;
	//struct xDataWeather tonight;
	
	struct xDateWeather tomorrow;
	//struct xDataWeather tomnight;
	
	struct xDateWeather dat;	//day after tomorrow
	//struct xDataWeather datnight;
};

struct xDevice
{
	struct xLed Led;
	
	//uint8_t weather;
	//enum xWeatherCode weather;
	struct xWeather Weather;
	
	struct xDate Data;	// ��ǰ���ڣ�����֪�������
	struct xTime Time;	// ��ǰʱ�䣬����֪�������
};

extern struct xDevice Device;

#endif
