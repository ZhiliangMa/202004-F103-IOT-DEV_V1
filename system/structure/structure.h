#ifndef __STRUCTURE_H
#define __STRUCTURE_H

#include "stm32f10x.h"

/**
  ******************************************************************************

  * @file    structure.h

  * @author  zhiLiangMa

  * @version V0.0.1

  * @date    2020/05/16

  * @brief   structure.h包含了状态标志位结构体的定义

  ******************************************************************************

  * @attention
  *
  *
  * 结构体怎么构建
  ******************************************************************************
  */

struct xRecMsg
{
	/* Uart2 wifi模块接收状态标志位 */
	uint8_t RxFlag;
	uint16_t RxNum;
	
	uint8_t U2_RxCompleted;	// 定义一个变量 0：表示接收未完成 1：表示接收完成
	uint8_t ConnectFlag;	// 连接TCP/UDP标志位，0-未连接，AT指令状态。1-已连接
	
	uint8_t Mode;			// 8266状态标志 0：需要复位或重新复位  1：正常状态（模块工作模式，正常，配网，透传）
	uint8_t Status;			// 连接服务器状态位。连接上服务器需要将ConnectFlag置1
};

extern struct xRecMsg WifiMsg;

// 开关状态
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

// 日期
struct xDate
{
	uint16_t year;
	enum xMonth month;
	uint8_t day;
	enum xWeek week;
};

// 时间
struct xTime
{
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
};

// 心知天气的 天气现象代码说明，前缀为n的表示夜晚天气
// 晴间多云，为 晴天夹杂着云（跟晴转多云和多云转晴不通过）
// 规定数据类型为 uint8_t
//enum xWeatherCode : uint8_t
enum xWeatherCode
{
	Sunny 			= 0,	// 晴（国内城市白天晴）
	nClear 			= 1,	// 晴（国内城市夜晚晴）
	Fair 			= 2,	// 晴（国外城市白天晴）
	nFair 			= 3,	// 晴（国外城市夜晚晴）
	
	Cloudy 			= 4,	// 多云
	
	PartlyCloudy 	= 5,	// 晴间多云
	nPartlyCloudy 	= 6,	// 晴间多云
	
	MostlyCloudy 	= 7,	// 大部多云
	nMostlyCloudy 	= 8,	// 大部多云
	
	Overcast 		= 9,	// 阴
	
	Shower 			= 10,	// 阵雨
	Thundershower 	= 11,	// 雷阵雨
	ThundershowerwithHail = 12,	// 雷阵雨伴有冰雹
	LightRain 		= 13,	// 小雨
	ModerateRain 	= 14,	// 中雨
	HeavyRain 		= 15,	// 大雨
	Storm 			= 16,	// 暴雨
	HeavyStorm 		= 17,	// 大暴雨
	SevereStorm 	= 18,	// 特大暴雨
	
	IceRain 		= 19,	// 冻雨
	Sleet 			= 20,	// 雨夹雪
	SnowFlurry 		= 21,	// 阵雪
	LightSnow 		= 22,	// 小雪
	ModerateSnow 	= 23,	// 中雪
	HeavySnow 		= 24,	// 大雪
	Snowstorm 		= 25,	// 暴雪
	
	Dust 			= 26,	// 浮尘
	Sand 			= 27,	// 扬沙
	Duststorm 		= 28,	// 沙尘暴
	Sandstorm 		= 29,	// 强沙尘暴
	
	Foggy 			= 30,	// 雾
	Haze 			= 31,	// 霾
	
	Windy 			= 32,	// 风
	Blustery 		= 33,	// 大风
	Hurricane 		= 34,	// 飓风
	TropicalStorm 	= 35,	// 热带风暴
	Tornado 		= 36,	// 龙卷风
	
	Cold 			= 37,	// 冷
	Hot 			= 38,	// 热
	
	Unknown 		= 99	// 未知
};


/* 跟下面他俩其实是可以合并一起的 */
struct xPeriodWeather
{
	enum xWeatherCode weaCode;
};

struct xDateWeather
{
	//年月日
	struct xDate Date;
	
	//白天天气代码
	struct xPeriodWeather day;
	//夜晚天气代码
	struct xPeriodWeather night;
	
	//分段天气
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
	
	struct xDate Data;	// 当前日期，由心知天气获得
	struct xTime Time;	// 当前时间，由心知天气获得
};

extern struct xDevice Device;

#endif
