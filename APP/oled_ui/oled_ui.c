#include "oled_ui.h"
#include "oled_ui_font.h"


/*
 * ���ܣ�����ͼƬ���أ������ܽ������
 *
 * ���� ������ʾͼƬ��Ĭ��Ϊ OneNet��ͼ��
 * ����  LED״̬ͼ�꣬Ĭ��Ϊ ȫ��
 * ���� ����״̬ͼ�꣬Ĭ��Ϊ N/A
 *
 */
void ui_ShowBootImg(void)
{
	// ���ؿ���ͼƬ
	load_Img(12, 0, OLED_GRAM, gImage_BootOnenet, 103, 64);
	ma_OLED_Refresh_Gram(OLED_GRAM);
	
	// ��ʱ2s
	delay_ms(2000);
	// ��ս���
	ma_OLED_Gram_Clear(OLED_GRAM);
	// ������ʱ�ӽ���Ԥ����0�������ϵ���λ��0�Ļ���û��0��ʾ
	ui_ClockReload(0, 2, OLED_GRAM);
	
	// Ԥ���ؿ��ؽ���ͼ��
	ui_ShowLed(3,0);
	ui_ShowLed(2,0);
	ui_ShowLed(1,0);
	ui_ShowLed(0,0);
	
	// ��������������Ԥ����N/A�������ϵ��ǿհ׽��档
	ui_WeatherReload(weatherIFS);
}


/*
 * ���ܣ�������ʱ�ӽ���Ԥ����0�������ϵ��ǿհ׽��档
 *
 * uint8_t x��Ҫ����λ�õ�x���꣬0~127
 * uint8_t y��Ҫ����λ�õ�y���꣬0~7
 * uint8_t (*ram)[128]����ʾ���滺��Ķ�ά����ָ��
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
 * ���ܣ���������������Ԥ����N/A�������ϵ��ǿհ׽��档
 *
 * uint8_t (*ram)[128]����ʾ���滺��Ķ�ά����ָ��
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
 * �����ã�ɾ������
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
 * ���ܣ���ʾʱ�䣬��1608�ַ����ĸ�ʽ
 *
 * uint8_t x��Ҫ����λ�õ�x���꣬0~127
 * uint8_t y��Ҫ����λ�õ�y���꣬0~7
 * uint8_t (*ram)[128]����ʾ���滺��Ķ�ά����ָ��
 *
 * struct xTime *Time����ʾ��ʱ��
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
 * ���ܣ�����ˢ��ʱ�ӣ���Ҫ����������һ������
 *
 * uint8_t x��Ҫ����λ�õ�x���꣬0~127
 * uint8_t y��Ҫ����λ�õ�y���꣬0~7
 * uint8_t (*ram)[128]����ʾ���滺��Ķ�ά����ָ��
 *
 * struct xTime* Time����ʾ��ʱ��
 *
 * ��Ϊʹ�õ��Ǽ����λ�仯����ˢ����ʾ����������ϵ�Ϊ0�Ļ�����û��0��ʾ����Ҫ�ڿ�����Ԥ����0��
 * G_blend[128]��Ϊʱ�ӹ���ˢ�µĻ��棬û����ʱ������Ϊ�˼ӿ���������ٶ�
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
	
	// ���ӵĸ�λ��
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
	
	// ���ӵ�ʮλ��
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
	
	// ���ӵĸ�λ��
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
	
	// ���ӵ�ʮλ��
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
	
	// ���ӵĸ�λ��
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
	
	// ���ӵ�ʮλ��
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
	
	// ������
	load_Img(x+32, y, ram, oled_asc2_3216[10], 16, 32);
	load_Img(x+80, y, ram, oled_asc2_3216[10], 16, 32);
	
//	// ����-
//	load_Img(x+32, y, blendIFS, oled_asc2_3216[11], 16, 32);
//	load_Img(x+80, y, blendIFS, oled_asc2_3216[11], 16, 32);
}


/*
 * ���ܣ�ˢ����ͼƬ���ԣ�ʵ�ʴ�����û��
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
 * ���ܣ�������ֵ�仯���ص�ͼ��״̬��
 *
 * uint8_t xnum���ڼ�����/���أ�0/1/2/3
 * uint8_t status����ǰ��/����״̬��0/1
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
 * ���ܣ�����֪�������������룬ת��Ϊ��Ӧ��ͼ��ͼ��ָ��
 *
 * const unsigned char*��ͼ��ͼ��ָ��
 * uint8_t weaCode����֪��������������
 *
 * ��ͼ�����⣬ȱʧͼ�꣺2��3��4��8��12��17��18��21��26��27��29��31��33��34��35��36��37��38���Ѿ��������滻��������Ѿ����
 * 26��27��31��34��35��36��37��38 δ����
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
			return gImage_0_Sunny;  break;	// ��0���ã�����
		case 3 :
			return gImage_1_nClear;  break;	// ��1���ã�����
		case 4 :
			return gImage_7_MostlyCloudy;  break;	// ��7/9���ã�����
		case 5 :
			return gImage_5_PartlyCloudy;  break;
		case 6 :
			return gImage_6_nPartlyCloudy;  break;
		case 7 :
			return gImage_7_MostlyCloudy;  break;
		case 8 :
			return gImage_6_nPartlyCloudy;  break;	// ��6���ã�����
		case 9 :
			return gImage_9_Overcast;  break;
		case 10 :
			return gImage_10_Shower;  break;
		case 11 :
			return gImage_11_Thundershower;  break;
		case 12 :
			return gImage_19_IceRain;  break;		// ��19���ã�����
		case 13 :
			return gImage_13_LightRain;  break;
		case 14 :
			return gImage_14_ModerateRain;  break;
		case 15 :
			return gImage_15_HeavyRain;  break;
		case 16 :
			return gImage_16_Storm;  break;
		case 17 :
			return gImage_16_Storm;  break;			// ��16���ã�����
		case 18 :
			return gImage_16_Storm;  break;			// ��16���ã�����
		case 19 :
			return gImage_19_IceRain;  break;
		case 20 :
			return gImage_20_Sleet;  break;
		case 21 :
			return gImage_22_LightSnow;  break;		// ��22���ã�����
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
			return gImage_28_Duststorm;  break;		// ��28���ã�����
		case 30 :
			return gImage_30_Foggy;  break;
		case 32 :
			return gImage_32_Windy;  break;
		case 33 :
			return gImage_32_Windy;  break;			// ��32���ã�����
		case 99 :
			return gImage_99_NA;  break;
		default :
			return gImage_99_NA;
	}
}

/*
 * ���ܣ����������ṹ����Ϣ����ʾ����������
 *
 * struct xWeather* weather�������ṹ�壬��������������
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
