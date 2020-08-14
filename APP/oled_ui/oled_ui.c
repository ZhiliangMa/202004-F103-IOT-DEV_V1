#include "oled_ui.h"
#include "oled_ui_font.h"


/*
 * 功能：开机图片加载，各功能界面加载
 *
 * 加载 开机显示图片，默认为 OneNet的图标
 * 加载  LED状态图标，默认为 全灭
 * 加载 天气状态图标，默认为 N/A
 *
 */
void ui_ShowBootImg(void)
{
	// 加载开机图片
	load_Img(12, 0, OLED_GRAM, gImage_BootOnenet, 103, 64);
	ma_OLED_Refresh_Gram(OLED_GRAM);
	
	// 延时2s
	delay_ms(2000);
	// 清空界面
	ma_OLED_Gram_Clear(OLED_GRAM);
	// 开机后时钟界面预加载0。避免上电数位是0的话，没有0显示
	ui_ClockReload(0, 2, OLED_GRAM);
	
	// 预加载开关界面图标
	ui_ShowLed(3,0);
	ui_ShowLed(2,0);
	ui_ShowLed(1,0);
	ui_ShowLed(0,0);
	
	// 开机后天气界面预加载N/A。避免上电是空白界面。
	ui_WeatherReload(weatherIFS);
}


/*
 * 功能：开机后，时钟界面预加载0。避免上电是空白界面。
 *
 * uint8_t x：要加载位置的x坐标，0~127
 * uint8_t y：要加载位置的y坐标，0~7
 * uint8_t (*ram)[128]：显示界面缓存的二维数组指针
 *
 */
void ui_ClockReload(uint8_t x, uint8_t y, uint8_t (*ram)[128])
{
	load_Img(x+112, y, ram, oled_asc2_3216[0], 16, 32);
	load_Img(x+96, y, ram, oled_asc2_3216[0], 16, 32);
	load_Img(x+64, y, ram, oled_asc2_3216[0], 16, 32);
	load_Img(x+48, y, ram, oled_asc2_3216[0], 16, 32);
	load_Img(x+16, y, ram, oled_asc2_3216[0], 16, 32);
	load_Img(x, y, ram, oled_asc2_3216[0], 16, 32);
}


/*
 * 功能：开机后天气界面预加载N/A。避免上电是空白界面。
 *
 * uint8_t (*ram)[128]：显示界面缓存的二维数组指针
 *
 */
void ui_WeatherReload(uint8_t (*ram)[128])
{
	load_Img(8, 0, ram, gImage_99_NA, 32, 32);
	load_Img(8, 4, ram, gImage_99_NA, 32, 32);
	
	load_Img(8+32+8, 0, ram, gImage_99_NA, 32, 32);
	load_Img(8+32+8, 4, ram, gImage_99_NA, 32, 32);
	
	load_Img(8+32+8+32+8, 0, ram, gImage_99_NA, 32, 32);
	load_Img(8+32+8+32+8, 4, ram, gImage_99_NA, 32, 32);
}


/*
 * 测试用，删掉过后
 */
void showV(void)
{
	uint8_t gImage_blend[128];
	static uint8_t i=0;
	//blend_Gram_V(i, 1, 32, 32, gImage_0_Sunny, gImage_9_Overcast, gImage_blend);
	//load_Img(0, 0, blendIFS, gImage_blend, 32, 32);
	blend_Gram_V(i, 1, 16, 32, oled_asc2_3216[0], oled_asc2_3216[1], gImage_blend);
	load_Img(0, 0, blendIFS, gImage_blend, 16, 32);
	
	i++;
	if(i>32+1)
		i=0;
}


/*
 * 功能：显示时间，以1608字符串的格式
 *
 * uint8_t x：要加载位置的x坐标，0~127
 * uint8_t y：要加载位置的y坐标，0~7
 * uint8_t (*ram)[128]：显示界面缓存的二维数组指针
 *
 * struct xTime *Time：显示的时间
 *
 */
void ui_Show1608Time(uint8_t x, uint8_t y, uint8_t (*ram)[128], struct xTime *Time)
{
	OLED_show1608Num(x, y, switchIFS, Time->hour, 2);
	OLED_show1608Num(x+24, y+0, switchIFS, Time->min, 2);
	OLED_show1608Num(x+48, y+0, switchIFS, Time->sec, 2);
	OLED_show1608String(x+16, y+0, switchIFS, ":", 1);
	OLED_show1608String(x+40, y+0, switchIFS, ":", 1);
}


/*
 * 功能：滚动刷新时钟，需要单独运行于一个任务
 *
 * uint8_t x：要加载位置的x坐标，0~127
 * uint8_t y：要加载位置的y坐标，0~7
 * uint8_t (*ram)[128]：显示界面缓存的二维数组指针
 *
 * struct xTime* Time：显示的时间
 *
 * 因为使用的是检查数位变化，才刷新显示，所以如果上电为0的话，会没有0显示。需要在开机后预加载0。
 * G_blend[128]作为时钟滚动刷新的缓存，没有临时变量是为了加快代码运行速度
 *
 */
uint8_t G_blend[128];
void ui_ShowTime(uint8_t x, uint8_t y, uint8_t (*ram)[128], struct xTime* Time)
{
	static uint8_t Thour,Dhour,Tmin,Dmin,Tsec,Dsec=0;
	static uint8_t Thour_flag,Dhour_flag,Tmin_flag,Dmin_flag,Tsec_flag,Dsec_flag=0;
	
//	Thour = Time->hour / 10;
//	Dhour = Time->hour % 10;
//	Tmin = Time->min / 10;
//	Dmin = Time->min % 10;
//	Tsec = Time->sec / 10;
//	Dsec = Time->sec % 10;
	
	// 秒钟的个位数
	if(((Time->sec % 10) != Dsec) || (Dsec_flag))
	{
		blend_Gram_V(Dsec_flag, 1, 16, 32, oled_asc2_3216[Dsec], oled_asc2_3216[Time->sec % 10], G_blend);
		load_Img(x+112, y, ram, G_blend, 16, 32);
		
		Dsec_flag++;
		if(Dsec_flag >=32)
		{
			Dsec = Time->sec % 10;
			Dsec_flag=0;
		}
	}
	
	// 秒钟的十位数
	if(((Time->sec / 10) != Tsec) || (Tsec_flag))
	{
		blend_Gram_V(Tsec_flag, 1, 16, 32, oled_asc2_3216[Tsec], oled_asc2_3216[Time->sec / 10], G_blend);
		load_Img(x+96, y, ram, G_blend, 16, 32);
		
		Tsec_flag++;
		if(Tsec_flag >=32)
		{
			Tsec = Time->sec / 10;
			Tsec_flag=0;
		}
	}
	
	// 分钟的个位数
	if(((Time->min % 10) != Dmin) || (Dmin_flag))
	{
		blend_Gram_V(Dmin_flag, 1, 16, 32, oled_asc2_3216[Dmin], oled_asc2_3216[Time->min % 10], G_blend);
		load_Img(x+64, y, ram, G_blend, 16, 32);
		
		Dmin_flag++;
		if(Dmin_flag >=32)
		{
			Dmin = Time->min % 10;
			Dmin_flag=0;
		}
	}
	
	// 分钟的十位数
	if(((Time->min / 10) != Tmin) || (Tmin_flag))
	{
		blend_Gram_V(Tmin_flag, 1, 16, 32, oled_asc2_3216[Tmin], oled_asc2_3216[Time->min / 10], G_blend);
		load_Img(x+48, y, ram, G_blend, 16, 32);
		
		Tmin_flag++;
		if(Tmin_flag >=32)
		{
			Tmin = Time->min / 10;
			Tmin_flag=0;
		}
	}
	
	// 刻钟的个位数
	if(((Time->hour % 10) != Dhour) || (Dhour_flag))
	{
		blend_Gram_V(Dhour_flag, 1, 16, 32, oled_asc2_3216[Dhour], oled_asc2_3216[Time->hour % 10], G_blend);
		load_Img(x+16, y, ram, G_blend, 16, 32);
		
		Dhour_flag++;
		if(Dhour_flag >=32)
		{
			Dhour = Time->hour % 10;
			Dhour_flag=0;
		}
	}
	
	// 刻钟的十位数
	if(((Time->hour / 10) != Thour) || (Thour_flag))
	{
		blend_Gram_V(Thour_flag, 1, 16, 32, oled_asc2_3216[Thour], oled_asc2_3216[Time->hour / 10], G_blend);
		load_Img(x, y, ram, G_blend, 16, 32);
		
		Thour_flag++;
		if(Thour_flag >=32)
		{
			Thour = Time->hour / 10;
			Thour_flag=0;
		}
	}
	
	// 两个：
	load_Img(x+32, y, ram, oled_asc2_3216[10], 16, 32);
	load_Img(x+80, y, ram, oled_asc2_3216[10], 16, 32);
	
//	// 两个-
//	load_Img(x+32, y, blendIFS, oled_asc2_3216[11], 16, 32);
//	load_Img(x+80, y, blendIFS, oled_asc2_3216[11], 16, 32);
}


/*
 * 功能：刷开关图片测试，实际代码中没用
 *
 */
void load_ImgTest(void)
{
	load_Img(0, 0, OLED_GRAM, gImage_LedOn, 32, 32);
	load_Img(0+32, 0, OLED_GRAM, gImage_LedOn, 32, 32);
	load_Img(0+32+32, 0, OLED_GRAM, gImage_LedOn, 32, 32);
	load_Img(0+32+32+32, 0, OLED_GRAM, gImage_LedOn, 32, 32);
	
	load_Img(0, 4, OLED_GRAM, gImage_LedOff, 32, 32);
	load_Img(0+32, 4, OLED_GRAM, gImage_LedOff, 32, 32);
	load_Img(0+32+32, 4, OLED_GRAM, gImage_LedOff, 32, 32);
	load_Img(0+32+32+32, 4, OLED_GRAM, gImage_LedOff, 32, 32);
}


/*
 * 功能：按输入值变化开关的图形状态。
 *
 * uint8_t xnum：第几个灯/开关，0/1/2/3
 * uint8_t status：当前灯/开关状态，0/1
 *
 */
void ui_ShowLed(uint8_t xnum, uint8_t status)
{
	if(status)
	{
		load_Img(xnum*32, 2, switchIFS, gImage_LedOn, 32, 32);
	}else
	{
		load_Img(xnum*32, 2, switchIFS, gImage_LedOff, 32, 32);
	}
}


/*
 * 功能：由心知天气的天气代码，转换为对应的图像图标指针
 *
 * const unsigned char*：图像图标指针
 * uint8_t weaCode：心知天气的天气代码
 *
 * 本图标主题，缺失图标：2、3、4、8、12、17、18、21、26、27、29、31、33、34、35、36、37、38，已经由其他替换，有误的已经标出
 * 26、27、31、34、35、36、37、38 未补齐
 *
 */
const unsigned char* WeatherToImg(uint8_t weaCode)
{
	switch(weaCode)
	{
		case 0 :
			return gImage_0_Sunny;  break;
		case 1 :
			return gImage_1_nClear;  break;
		case 2 :
			return gImage_0_Sunny;  break;	// 与0共用，无误
		case 3 :
			return gImage_1_nClear;  break;	// 与1共用，无误
		case 4 :
			return gImage_7_MostlyCloudy;  break;	// 与7/9共用，有误
		case 5 :
			return gImage_5_PartlyCloudy;  break;
		case 6 :
			return gImage_6_nPartlyCloudy;  break;
		case 7 :
			return gImage_7_MostlyCloudy;  break;
		case 8 :
			return gImage_6_nPartlyCloudy;  break;	// 与6共用，无误
		case 9 :
			return gImage_9_Overcast;  break;
		case 10 :
			return gImage_10_Shower;  break;
		case 11 :
			return gImage_11_Thundershower;  break;
		case 12 :
			return gImage_19_IceRain;  break;		// 与19共用，有误
		case 13 :
			return gImage_13_LightRain;  break;
		case 14 :
			return gImage_14_ModerateRain;  break;
		case 15 :
			return gImage_15_HeavyRain;  break;
		case 16 :
			return gImage_16_Storm;  break;
		case 17 :
			return gImage_16_Storm;  break;			// 与16共用，有误
		case 18 :
			return gImage_16_Storm;  break;			// 与16共用，有误
		case 19 :
			return gImage_19_IceRain;  break;
		case 20 :
			return gImage_20_Sleet;  break;
		case 21 :
			return gImage_22_LightSnow;  break;		// 与22共用，有误
		case 22 :
			return gImage_22_LightSnow;  break;
		case 23 :
			return gImage_23_ModerateSnow;  break;
		case 24 :
			return gImage_24_HeavySnow;  break;
		case 25 :
			return gImage_25_Snowstorm;  break;
		case 28 :
			return gImage_28_Duststorm;  break;
		case 29 :
			return gImage_28_Duststorm;  break;		// 与28共用，有误
		case 30 :
			return gImage_30_Foggy;  break;
		case 32 :
			return gImage_32_Windy;  break;
		case 33 :
			return gImage_32_Windy;  break;			// 与32共用，无误
		case 99 :
			return gImage_99_NA;  break;
		default :
			return gImage_99_NA;
	}
}

/*
 * 功能：根据天气结构体信息，显示近三日天气
 *
 * struct xWeather* weather：天气结构体，包含近三日天气
 *
 */
void ui_ShowAllWeather(struct xWeather* weather)
{
	load_Img(8, 0, weatherIFS, WeatherToImg(weather->today.day.weaCode), 32, 32);
	load_Img(8, 4, weatherIFS, WeatherToImg(weather->today.night.weaCode), 32, 32);
	
	load_Img(8+32+8, 0, weatherIFS, WeatherToImg(weather->tomorrow.day.weaCode), 32, 32);
	load_Img(8+32+8, 4, weatherIFS, WeatherToImg(weather->tomorrow.night.weaCode), 32, 32);
	
	load_Img(8+32+8+32+8, 0, weatherIFS, WeatherToImg(weather->dat.day.weaCode), 32, 32);
	load_Img(8+32+8+32+8, 4, weatherIFS, WeatherToImg(weather->dat.night.weaCode), 32, 32);
}
