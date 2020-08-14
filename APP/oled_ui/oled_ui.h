#ifndef __OLED_UI_H
#define __OLED_UI_H

#include "stm32f10x.h"
#include "oled.h"


void ui_ShowBootImg(void);
void ui_ClockReload(uint8_t x, uint8_t y, uint8_t (*ram)[128]);
void ui_WeatherReload(uint8_t (*ram)[128]);

void ui_Show1608Time(uint8_t x, uint8_t y, uint8_t (*ram)[128], struct xTime *Time);
void ui_ShowTime(uint8_t x, uint8_t y, uint8_t (*ram)[128], struct xTime* Time);

void ui_ShowLed(uint8_t xnum, uint8_t status);

void ui_ShowAllWeather(struct xWeather* weather);

#endif
