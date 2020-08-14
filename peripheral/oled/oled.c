#include "oled.h"
#include "oledfont.h"


//��SSD1306д��һ���ֽڡ�
//dat:Ҫд�������/����
//cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
void OLED_WR_Byte(unsigned char  dat,unsigned char  cmd)
{
	//unsigned char  i;
	if(cmd)
	  OLED_RS_Set();
	else
	  OLED_RS_Clr();	//����ģʽ��������ģʽ
	  OLED_CS_Clr();//Ƭѡ
	
	//���Ӳ����дSPI����
	SPI1_ReadWriteByte(dat);
	
	OLED_CS_Set();
	OLED_RS_Set();
}
	  	  
//����OLED��ʾ    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC����
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}

//�ر�OLED��ʾ     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC����
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}



////��SSD1306д��һ���ֽڡ�
////dat:Ҫд�������/����
////cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
//void OLED_WR_Byte(unsigned char  dat,unsigned char  cmd)
//{	
//	//unsigned char  i;			  
//	if(cmd)
//	  OLED_RS_Set();
//	else 
//	  OLED_RS_Clr();	//����ģʽ��������ģʽ	  
//	  OLED_CS_Clr();//Ƭѡ
//	
//	//���Ӳ����дSPI����
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


//OLED���Դ�
//��Ÿ�ʽ����.
//[0]0 1 2 3 ... 127
//[1]0 1 2 3 ... 127
//[2]0 1 2 3 ... 127
//[3]0 1 2 3 ... 127
//[4]0 1 2 3 ... 127
//[5]0 1 2 3 ... 127
//[6]0 1 2 3 ... 127
//[7]0 1 2 3 ... 127

uint8_t OLED_GRAM[8][128];	//����ͼ����أ����̽�������Ϊʱ�ӽ���
uint8_t switchIFS[8][128];	//���ؽ���
uint8_t weatherIFS[8][128];	//��������
uint8_t blendIFS[8][128];	//��Ͻ��棬����Ϊ�����Ĺ���GUI����ʹ�ã��൱�ڽ��滬���Ļ���

/*
 * ���ܣ���������Դ�����
 *
 * uint8_t (*ram)[128]����ʾ���滺��Ķ�ά����ָ��
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
 * ���ܣ���ͼƬ���ص�ָ���Դ�λ��
 *
 * uint8_t x��Ҫ����λ�õ�x���꣬0~127
 * uint8_t y��Ҫ����λ�õ�y���꣬0~7
 * uint8_t (*ram)[128]����ʾ���滺��Ķ�ά����ָ��
 * const unsigned char *p��Ҫ���ص��Դ��е�ͼƬָ��
 * uint8_t imgWidth��ͼƬ�Ŀ��������Ŀ
 * uint8_t imgHeight��ͼƬ�ĸ߶�������Ŀ
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
 * ���ܣ���ָ���Դ���µ���ǰ��Ļ����ʾ
 *
 * uint8_t (*ram)[128]����ʾ���滺��Ķ�ά����ָ��
 *
 */
void ma_OLED_Refresh_Gram(uint8_t (*ram)[128])
{
	unsigned char  i,n;
	
	for(i=0;i<8;i++)  
	{
		/* ��������� */
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //����ҳ��ַ��0~7������Ӧ���Դ�ĵڼ��У�ÿ�ж�Ӧ128Byte��
		//OLED_WR_Byte (0x02,OLED_CMD);      //������ʾλ�á��е͵�ַ����������ǰ�����������пӣ�02��������2����
		OLED_WR_Byte (0x00,OLED_CMD);      //������ʾλ�á��е͵�ַ
		OLED_WR_Byte (0x10,OLED_CMD);      //������ʾλ�á��иߵ�ַ
		
		/* ��GRAM��д����ʾ������ */
		OLED_RS_Set();//DCΪ1,��ʾ����;
		OLED_CS_Clr();//Ƭѡ
		/* �Ժ�ʹ��DMA�����OLED���Դ��ɶ�ά���飬�滻Ϊ8��һά���� */
		for(n=0;n<128;n++)
		{
			SPI1_ReadWriteByte(*(*(ram+i)+n));
		}
		OLED_CS_Set();
		OLED_RS_Set();
	}   
}

/*
 * ���ܣ��������Դ���ˮƽ������
 *
 * uint8_t x��Ҫ����λ�õ�x���꣬0~128
 * uint8_t (*ram1)[128]����ʾ���滺��Ķ�ά����ָ��
 * uint8_t (*ram2)[128]����ʾ���滺��Ķ�ά����ָ��
 * uint8_t (*blend)[128]����Ϻ��������ʾ���滺��Ķ�ά����ָ��
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
 * ���ܣ�������ͼƬ�ڴ�ֱ������
 *
 * uint8_t y��Ҫ����λ�õ�y���꣬0~64����Ӧ���� 0~h �ķ�Χ��
 * uint8_t d����ֱ�ƶ�����0-���ƣ�1-����
 * uint8_t w��ͼƬ��ȣ�0~128
 * uint8_t h��ͼƬ�߶ȣ�8~64
 *
 * uint8_t *img1����ʾͼƬ������ָ��
 * uint8_t *img2����ʾͼƬ������ָ��
 * uint8_t *blend����Ϻ�����ͼƬ����ָ��
 *
 * ֱ�ӽ�ÿ��ͼƬ��ֳ�һ�������У�ÿ�����кϳ� uint64���������64�����ص㡣
 * ���ŶԺϳɺ�� uint64����������λ��ʵ��ͼ��������λ���������uint64��λ���ϳ�һ֡���ٲ��Ϊ�ֽ�������ڴ档
 * �ô��ǲ��ÿ��������ͼ��һ������ڴ棬��2��uint64���ɡ�
 */
void blend_Gram_V(uint8_t y, uint8_t d, uint8_t w, uint8_t h, uint8_t *img1, uint8_t *img2, uint8_t *blend)
{
	int16_t i,j=0;
	uint64_t tmp1,tmp2=0;
		
	// ֮��img1��img2�����뷽������꣬�ƶ�λ��
	// ��ÿ���м���Ϊһ�� uint64����λ��ƴ�ӣ������·����ԭ�ռ�
	for(i=0; i<w; i++)
	{
		// ��ÿ���м���Ϊһ�� uint64���������64�����ص�
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
		
		// ѡ�񻬶�����
		if(d)
		{
			// ���ƣ�������λ
			tmp1 = tmp1 << y;
			tmp2 = tmp2 >> (h-y);
		}else{
			// ���ƣ�������λ
			tmp1 = tmp1 >> y;
			tmp2 = tmp2 << (h-y);
		}
		
		//����ͼ��ƴ��
		tmp1 = tmp1 | tmp2;
		
		// ƴ�Ӻ���λ�����·��������ڴ�
		for(j=(h/8-1); j>-1; j--)
		{
			blend[i+j*w] = tmp1 & 0x00000000000000FF;
			tmp1 = tmp1 >> 8;
		}
	}
}

/*
 * ���ܣ���ָ���Դ���µ���ǰ��Ļ����ʾ,ƽʱ���ã��������ã�����ˢ�㷽��
 *
 * uint8_t x��Ҫ����λ�õ�x���꣬0~127
 * uint8_t y��Ҫ����λ�õ�y���꣬0~7
 * uint8_t xram��д�������
 *
 * ma_OLED_Gram_Test(0, 0, 0xF1);
 */
void ma_OLED_Gram_Test(uint8_t x, uint8_t y, uint8_t xram)
{
	/* ��������� */
	OLED_WR_Byte (0xb0+y,OLED_CMD);    //����ҳ��ַ��0~7������Ӧ���Դ�ĵڼ��У�ÿ�ж�Ӧ128Byte��
	//OLED_WR_Byte (0x02,OLED_CMD);      //������ʾλ�á��е͵�ַ����������ǰ�����������пӣ�02��������2����
	OLED_WR_Byte (0x00+x,OLED_CMD);      //������ʾλ�á��е͵�ַ
	OLED_WR_Byte (0x10,OLED_CMD);      //������ʾλ�á��иߵ�ַ
	
	/* ��GRAM��д����ʾ������ */
	OLED_RS_Set();//DCΪ1,��ʾ����;
	OLED_CS_Clr();//Ƭѡ
	
	SPI1_ReadWriteByte(xram);
	
	OLED_CS_Set();
	OLED_RS_Set();
}

/*
 * ���ܣ���ʾ����
 *
 * uint8_t x��Ҫ����λ�õ�x���꣬0~127
 * uint8_t y��Ҫ����λ�õ�y���꣬0~7
 * uint8_t (*ram)[128]����ʾ���滺��Ķ�ά����ָ��
 *
 * uint16_t num����ʾ������
 * uint8_t len�� ���ֳ��ȣ�1~9
 *
 */
void OLED_show1608Num(uint8_t x, uint8_t y, uint8_t (*ram)[128], uint16_t num, uint8_t len)
{
	uint8_t i,j;
	uint8_t k,l;
	//kΪ���ֶ�Ӧ���ַ����룬���ֶ�ӦASC2��+48������������ܼ���Ӧ������(ASC2-32)
	//lΪ��ӡ�ڼ�λ��λ��
	//j��i���е����ַ����
	
	char chr[9] = "         ";
	
	sprintf(chr, "%09d", num);
	for(l=0; l<len; l++)
	{
		k = chr[9-len+l] -32;	// ��λת��������ת��Ϊ�ַ�
		//k = + 48;	// ��λת��������ת��Ϊ�ַ�
		
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
 * ���ܣ���ʾ�ַ���
 *
 * uint8_t x��Ҫ����λ�õ�x���꣬0~127
 * uint8_t y��Ҫ����λ�õ�y���꣬0~7
 * uint8_t (*ram)[128]����ʾ���滺��Ķ�ά����ָ��
 *
 * char* p��	 �ַ�������󳤶�Ϊ16�ֽ�
 * uint8_t len�� �ַ������ȣ�1~16
 *
 */
void OLED_show1608String(uint8_t x, uint8_t y, uint8_t (*ram)[128], char* p, uint8_t len)
{
	uint8_t i,j;
	uint8_t k,l;
	//kΪ���ֶ�Ӧ���ַ����룬���ֶ�ӦASC2��+48������������ܼ���Ӧ������(ASC2-32)
	//lΪ��ӡ�ڼ�λ��λ��
	//j��i���е����ַ����
	
	//char chr[len];
	char chr[16];
	
	sprintf(chr, "%s", p);
	for(l=0; l<len; l++)
	{
		k = chr[l] -32;	// ��λת��������ת��Ϊ�ַ�
		//k = + 48;	// ��λת��������ת��Ϊ�ַ�
		
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
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);//ʹ��PD�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 ;	 			//OLED_RST
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//�ٶ�50MHz
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 ;	//OLED_RST
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//�ٶ�50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_6);	//OLED_RST�ߵ�ƽ������λ
	GPIO_SetBits(GPIOB, GPIO_Pin_0);	//DC�ߵ�ƽ������
	GPIO_SetBits(GPIOB, GPIO_Pin_1);	//CS�ߵ�ƽ��ʧ��
}

//��ʼ��SSD1306					    
void OLED_Init(void)
{
	SPI1_Init();
	SPI1_SetSpeed(SPI_BaudRatePrescaler_4);	//SPI1-4��Ƶ��18MHz
	
	OLED_GPIO_Init();
	
	OLED_RST_Clr();	//RST���0����λOLED
	delay_xms(50);
	OLED_RST_Set();
	delay_xms(50);
	
	OLED_WR_Byte(0xAE,OLED_CMD); //�ر���ʾ
	OLED_WR_Byte(0xD5,OLED_CMD); //����ʱ�ӷ�Ƶ����,��Ƶ��
	OLED_WR_Byte(80,OLED_CMD);   //[3:0],��Ƶ����;[7:4],��Ƶ��
	OLED_WR_Byte(0xA8,OLED_CMD); //��������·��
	OLED_WR_Byte(0X3F,OLED_CMD); //Ĭ��0X3F(1/64) 
	OLED_WR_Byte(0xD3,OLED_CMD); //������ʾƫ��
	OLED_WR_Byte(0X00,OLED_CMD); //Ĭ��Ϊ0

	OLED_WR_Byte(0x40,OLED_CMD); //������ʾ��ʼ�� [5:0],����.
													    
	OLED_WR_Byte(0x8D,OLED_CMD); //��ɱ�����
	OLED_WR_Byte(0x14,OLED_CMD); //bit2������/�ر�
	OLED_WR_Byte(0x20,OLED_CMD); //�����ڴ��ַģʽ
	OLED_WR_Byte(0x02,OLED_CMD); //[1:0],00���е�ַģʽ;01���е�ַģʽ;10,ҳ��ַģʽ;Ĭ��10;
	OLED_WR_Byte(0xA1,OLED_CMD); //���ض�������,bit0:0,0->0;1,0->127;
	OLED_WR_Byte(0xC0,OLED_CMD); //����COMɨ�跽��;bit3:0,��ͨģʽ;1,�ض���ģʽ COM[N-1]->COM0;N:����·��
	OLED_WR_Byte(0xDA,OLED_CMD); //����COMӲ����������
	OLED_WR_Byte(0x12,OLED_CMD); //[5:4]����
		 
	OLED_WR_Byte(0x81,OLED_CMD); //�Աȶ�����
	OLED_WR_Byte(0xEF,OLED_CMD); //1~255;Ĭ��0X7F (��������,Խ��Խ��)
	OLED_WR_Byte(0xD9,OLED_CMD); //����Ԥ�������
	OLED_WR_Byte(0xf1,OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_WR_Byte(0xDB,OLED_CMD); //����VCOMH ��ѹ����
	OLED_WR_Byte(0x30,OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0xA4,OLED_CMD); //ȫ����ʾ����;bit0:1,����;0,�ر�;(����/����)
	OLED_WR_Byte(0xA6,OLED_CMD); //������ʾ��ʽ;bit0:1,������ʾ;0,������ʾ	    						   
	OLED_WR_Byte(0xAF,OLED_CMD); //������ʾ	 
	
	/* ����Դ沢���µ���Ļ�� */
	ma_OLED_Gram_Clear(OLED_GRAM);
	ma_OLED_Refresh_Gram(OLED_GRAM);
}  

