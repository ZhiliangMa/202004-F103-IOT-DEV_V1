#include "oled.h"
#include "oledfont.h"


//向SSD1306写入一个字节。
//dat:要写入的数据/命令
//cmd:数据/命令标志 0,表示命令;1,表示数据;
void OLED_WR_Byte(unsigned char  dat,unsigned char  cmd)
{
	//unsigned char  i;
	if(cmd)
	  OLED_RS_Set();
	else
	  OLED_RS_Clr();	//命令模式或者数据模式
	  OLED_CS_Clr();//片选
	
	//添加硬件读写SPI函数
	SPI1_ReadWriteByte(dat);
	
	OLED_CS_Set();
	OLED_RS_Set();
}
	  	  
//开启OLED显示    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}

//关闭OLED显示     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}



////向SSD1306写入一个字节。
////dat:要写入的数据/命令
////cmd:数据/命令标志 0,表示命令;1,表示数据;
//void OLED_WR_Byte(unsigned char  dat,unsigned char  cmd)
//{	
//	//unsigned char  i;			  
//	if(cmd)
//	  OLED_RS_Set();
//	else 
//	  OLED_RS_Clr();	//命令模式或者数据模式	  
//	  OLED_CS_Clr();//片选
//	
//	//添加硬件读写SPI函数
//	
//	SPI1_ReadWriteByte(dat);
//	 		  
//	OLED_CS_Set();
//	OLED_RS_Set();   	  
//}

//void OLED_ClrLine(u8 line)
//{
//	u8 x;	
//	OLED_WrCmd(0xb0+line);
//	OLED_WrCmd(0x01);
//	OLED_WrCmd(0x10); 
//	for(x=0;x<X_WIDTH;x++)
//    OLED_WrDat(0);
//}


//OLED的显存
//存放格式如下.
//[0]0 1 2 3 ... 127
//[1]0 1 2 3 ... 127
//[2]0 1 2 3 ... 127
//[3]0 1 2 3 ... 127
//[4]0 1 2 3 ... 127
//[5]0 1 2 3 ... 127
//[6]0 1 2 3 ... 127
//[7]0 1 2 3 ... 127

uint8_t OLED_GRAM[8][128];	//开机图标加载，过程结束后作为时钟界面
uint8_t switchIFS[8][128];	//开关界面
uint8_t weatherIFS[8][128];	//天气界面
uint8_t blendIFS[8][128];	//混合界面，不作为单独的功能GUI界面使用，相当于界面滑动的缓存

/*
 * 功能：清除整个显存区域
 *
 * uint8_t (*ram)[128]：显示界面缓存的二维数组指针
 *
 */
void ma_OLED_Gram_Clear(uint8_t (*ram)[128])
{  
	unsigned char  i,n;  
	for(i=0;i<8;i++)
	{
		for(n=0;n<128;n++)
		{
			*(*(ram+i)+n)=0X00;
		}
	}
}

/*
 * 功能：将图片加载到指定显存位置
 *
 * uint8_t x：要加载位置的x坐标，0~127
 * uint8_t y：要加载位置的y坐标，0~7
 * uint8_t (*ram)[128]：显示界面缓存的二维数组指针
 * const unsigned char *p：要加载到显存中的图片指针
 * uint8_t imgWidth：图片的宽度像素数目
 * uint8_t imgHeight：图片的高度像素数目
 *
 */
void load_Img(uint8_t x, uint8_t y, uint8_t (*ram)[128], const unsigned char *p, uint8_t imgWidth, uint8_t imgHeight)
{
	uint16_t i,j;
	
	for(j=0; j<(imgHeight/8); j++)
	{
		for(i=0; i<imgWidth; i++)
		{
			*(*(ram+7-y-j)+x+i) = *(p+i+j*imgWidth);
		}
	}
}

/*
 * 功能：将指定显存更新到当前屏幕上显示
 *
 * uint8_t (*ram)[128]：显示界面缓存的二维数组指针
 *
 */
void ma_OLED_Refresh_Gram(uint8_t (*ram)[128])
{
	unsigned char  i,n;
	
	for(i=0;i<8;i++)  
	{
		/* 设置坐标点 */
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7），对应着显存的第几行，每行对应128Byte。
		//OLED_WR_Byte (0x02,OLED_CMD);      //设置显示位置—列低地址。！！！以前的驱动这里有坑，02会往后移2像素
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址
		
		/* 向GRAM中写入显示的数据 */
		OLED_RS_Set();//DC为1,表示数据;
		OLED_CS_Clr();//片选
		/* 以后使用DMA替代，OLED的显存由二维数组，替换为8个一维数组 */
		for(n=0;n<128;n++)
		{
			SPI1_ReadWriteByte(*(*(ram+i)+n));
		}
		OLED_CS_Set();
		OLED_RS_Set();
	}   
}

/*
 * 功能：将两张显存在水平方向混合
 *
 * uint8_t x：要加载位置的x坐标，0~128
 * uint8_t (*ram1)[128]：显示界面缓存的二维数组指针
 * uint8_t (*ram2)[128]：显示界面缓存的二维数组指针
 * uint8_t (*blend)[128]：混合后输出的显示界面缓存的二维数组指针
 *
 */
void blend_Gram_H(uint8_t x, uint8_t (*ram1)[128], uint8_t (*ram2)[128], uint8_t (*blend)[128])
{
	uint8_t i=0;
	for(i=0; i<128-x; i++)
	{
		blend[0][i] = *(*(ram1+0)+i+x);
		blend[1][i] = *(*(ram1+1)+i+x);
		blend[2][i] = *(*(ram1+2)+i+x);
		blend[3][i] = *(*(ram1+3)+i+x);
		blend[4][i] = *(*(ram1+4)+i+x);
		blend[5][i] = *(*(ram1+5)+i+x);
		blend[6][i] = *(*(ram1+6)+i+x);
		blend[7][i] = *(*(ram1+7)+i+x);
	}
	for(i=128-x; i<128; i++)
	{
		blend[0][i] = *(*(ram2+0)+i-(128-x));
		blend[1][i] = *(*(ram2+1)+i-(128-x));
		blend[2][i] = *(*(ram2+2)+i-(128-x));
		blend[3][i] = *(*(ram2+3)+i-(128-x));
		blend[4][i] = *(*(ram2+4)+i-(128-x));
		blend[5][i] = *(*(ram2+5)+i-(128-x));
		blend[6][i] = *(*(ram2+6)+i-(128-x));
		blend[7][i] = *(*(ram2+7)+i-(128-x));
	}
}

/*
 * 功能：将两张图片在垂直方向混合
 *
 * uint8_t y：要加载位置的y坐标，0~64，但应该在 0~h 的范围内
 * uint8_t d：垂直移动方向，0-下移，1-上移
 * uint8_t w：图片宽度，0~128
 * uint8_t h：图片高度，8~64
 *
 * uint8_t *img1：显示图片的数组指针
 * uint8_t *img2：显示图片的数组指针
 * uint8_t *blend：混合后的输出图片数组指针
 *
 * 直接将每幅图片拆分成一个个纵列，每个纵列合成 uint64，最大容纳64个像素点。
 * 接着对合成后的 uint64进行整体移位，实现图像上下移位。最后将两个uint64按位或混合成一帧，再拆分为字节输出到内存。
 * 好处是不用开辟两块跟图像一样大的内存，有2个uint64即可。
 */
void blend_Gram_V(uint8_t y, uint8_t d, uint8_t w, uint8_t h, uint8_t *img1, uint8_t *img2, uint8_t *blend)
{
	int16_t i,j=0;
	uint64_t tmp1,tmp2=0;
		
	// 之后将img1、img2按输入方向和坐标，移动位置
	// 将每纵列计算为一个 uint64，移位后拼接，再重新分配回原空间
	for(i=0; i<w; i++)
	{
		// 将每纵列计算为一个 uint64，最大容纳64个像素点
		tmp1 = 0;
		tmp2 = 0;
		for(j=0; j<(h/8); j++)
		{
			tmp1 = tmp1 << 8;
			tmp1 |= img1[i+j*w];
			//tmp = tmp << 8;
			tmp2 = tmp2 << 8;
			tmp2 |= img2[i+j*w];
		}
		
		// 选择滑动方向
		if(d)
		{
			// 上移，整体移位
			tmp1 = tmp1 << y;
			tmp2 = tmp2 >> (h-y);
		}else{
			// 下移，整体移位
			tmp1 = tmp1 >> y;
			tmp2 = tmp2 << (h-y);
		}
		
		//两幅图像拼接
		tmp1 = tmp1 | tmp2;
		
		// 拼接后移位，重新分配给输出内存
		for(j=(h/8-1); j>-1; j--)
		{
			blend[i+j*w] = tmp1 & 0x00000000000000FF;
			tmp1 = tmp1 >> 8;
		}
	}
}

/*
 * 功能：将指定显存更新到当前屏幕上显示,平时别用，仅测试用，测试刷点方向
 *
 * uint8_t x：要加载位置的x坐标，0~127
 * uint8_t y：要加载位置的y坐标，0~7
 * uint8_t xram：写入的内容
 *
 * ma_OLED_Gram_Test(0, 0, 0xF1);
 */
void ma_OLED_Gram_Test(uint8_t x, uint8_t y, uint8_t xram)
{
	/* 设置坐标点 */
	OLED_WR_Byte (0xb0+y,OLED_CMD);    //设置页地址（0~7），对应着显存的第几行，每行对应128Byte。
	//OLED_WR_Byte (0x02,OLED_CMD);      //设置显示位置—列低地址。！！！以前的驱动这里有坑，02会往后移2像素
	OLED_WR_Byte (0x00+x,OLED_CMD);      //设置显示位置—列低地址
	OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址
	
	/* 向GRAM中写入显示的数据 */
	OLED_RS_Set();//DC为1,表示数据;
	OLED_CS_Clr();//片选
	
	SPI1_ReadWriteByte(xram);
	
	OLED_CS_Set();
	OLED_RS_Set();
}

/*
 * 功能：显示数字
 *
 * uint8_t x：要加载位置的x坐标，0~127
 * uint8_t y：要加载位置的y坐标，0~7
 * uint8_t (*ram)[128]：显示界面缓存的二维数组指针
 *
 * uint16_t num：显示的数字
 * uint8_t len： 数字长度，1~9
 *
 */
void OLED_show1608Num(uint8_t x, uint8_t y, uint8_t (*ram)[128], uint16_t num, uint8_t len)
{
	uint8_t i,j;
	uint8_t k,l;
	//k为数字对应的字符编码，数字对应ASC2是+48，而精简过功能键对应编码是(ASC2-32)
	//l为打印第几位的位数
	//j、i进行单个字符描点
	
	char chr[9] = "         ";
	
	sprintf(chr, "%09d", num);
	for(l=0; l<len; l++)
	{
		k = chr[9-len+l] -32;	// 数位转换，数字转换为字符
		//k = + 48;	// 数位转换，数字转换为字符
		
		for(j=0; j<(16/8); j++)
		{
			for(i=0; i<8; i++)
			{
				*(*(ram+7-y-j)+x+i+l*8) = oled_asc2_1608[k][j*8+i];
			}
		}
	}
}

/*
 * 功能：显示字符串
 *
 * uint8_t x：要加载位置的x坐标，0~127
 * uint8_t y：要加载位置的y坐标，0~7
 * uint8_t (*ram)[128]：显示界面缓存的二维数组指针
 *
 * char* p：	 字符串，最大长度为16字节
 * uint8_t len： 字符串长度，1~16
 *
 */
void OLED_show1608String(uint8_t x, uint8_t y, uint8_t (*ram)[128], char* p, uint8_t len)
{
	uint8_t i,j;
	uint8_t k,l;
	//k为数字对应的字符编码，数字对应ASC2是+48，而精简过功能键对应编码是(ASC2-32)
	//l为打印第几位的位数
	//j、i进行单个字符描点
	
	//char chr[len];
	char chr[16];
	
	sprintf(chr, "%s", p);
	for(l=0; l<len; l++)
	{
		k = chr[l] -32;	// 数位转换，数字转换为字符
		//k = + 48;	// 数位转换，数字转换为字符
		
		for(j=0; j<(16/8); j++)
		{
			for(i=0; i<8; i++)
			{
				*(*(ram+7-y-j)+x+i+l*8) = oled_asc2_1608[k][j*8+i];
			}
		}
	}
}

/*
 * OLED_RST -> PA6
 * DC 		-> PB0
 * CS 		-> PB1
 */
void OLED_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);//使能PD端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 ;	 			//OLED_RST
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//速度50MHz
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 ;	//OLED_RST
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//速度50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_6);	//OLED_RST高电平，不复位
	GPIO_SetBits(GPIOB, GPIO_Pin_0);	//DC高电平，数据
	GPIO_SetBits(GPIOB, GPIO_Pin_1);	//CS高电平，失能
}

//初始化SSD1306					    
void OLED_Init(void)
{
	SPI1_Init();
	SPI1_SetSpeed(SPI_BaudRatePrescaler_4);	//SPI1-4分频，18MHz
	
	OLED_GPIO_Init();
	
	OLED_RST_Clr();	//RST输出0，复位OLED
	delay_xms(50);
	OLED_RST_Set();
	delay_xms(50);
	
	OLED_WR_Byte(0xAE,OLED_CMD); //关闭显示
	OLED_WR_Byte(0xD5,OLED_CMD); //设置时钟分频因子,震荡频率
	OLED_WR_Byte(80,OLED_CMD);   //[3:0],分频因子;[7:4],震荡频率
	OLED_WR_Byte(0xA8,OLED_CMD); //设置驱动路数
	OLED_WR_Byte(0X3F,OLED_CMD); //默认0X3F(1/64) 
	OLED_WR_Byte(0xD3,OLED_CMD); //设置显示偏移
	OLED_WR_Byte(0X00,OLED_CMD); //默认为0

	OLED_WR_Byte(0x40,OLED_CMD); //设置显示开始行 [5:0],行数.
													    
	OLED_WR_Byte(0x8D,OLED_CMD); //电荷泵设置
	OLED_WR_Byte(0x14,OLED_CMD); //bit2，开启/关闭
	OLED_WR_Byte(0x20,OLED_CMD); //设置内存地址模式
	OLED_WR_Byte(0x02,OLED_CMD); //[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
	OLED_WR_Byte(0xA1,OLED_CMD); //段重定义设置,bit0:0,0->0;1,0->127;
	OLED_WR_Byte(0xC0,OLED_CMD); //设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
	OLED_WR_Byte(0xDA,OLED_CMD); //设置COM硬件引脚配置
	OLED_WR_Byte(0x12,OLED_CMD); //[5:4]配置
		 
	OLED_WR_Byte(0x81,OLED_CMD); //对比度设置
	OLED_WR_Byte(0xEF,OLED_CMD); //1~255;默认0X7F (亮度设置,越大越亮)
	OLED_WR_Byte(0xD9,OLED_CMD); //设置预充电周期
	OLED_WR_Byte(0xf1,OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_WR_Byte(0xDB,OLED_CMD); //设置VCOMH 电压倍率
	OLED_WR_Byte(0x30,OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0xA4,OLED_CMD); //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
	OLED_WR_Byte(0xA6,OLED_CMD); //设置显示方式;bit0:1,反相显示;0,正常显示	    						   
	OLED_WR_Byte(0xAF,OLED_CMD); //开启显示	 
	
	/* 清除显存并更新到屏幕上 */
	ma_OLED_Gram_Clear(OLED_GRAM);
	ma_OLED_Refresh_Gram(OLED_GRAM);
}  

