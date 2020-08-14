#ifndef __SENIVERSE_HTTP_H
#define __SENIVERSE_HTTP_H

#include "sys.h"
#include "wifi.h"
#include "uart2.h"
#include "string.h"
#include "delay.h"
#include "usart.h"
#include "onenet_http.h"
#include "structure.h"
#include "uart1.h"
#include "stdlib.h"
#include "oled_ui.h"
#include "net_time.h"
#include "FreeRTOS.h"
#include "timers.h"


#define SeniverseServerIP	"116.62.81.138"
#define SeniverseServerPort	80
#define SeniverseAPI_KEY	"S6eMmAGubL0Twlnxo"		// 用户私钥
#define HefeiCity			"hefei"					// 所在城市


void Seniverse_GetWeather(void);

#endif
