#include "spi.h"

//OLED调用的基类
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1, ENABLE );	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);

	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;  //设置SPI单向数据模式
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//选择了串行时钟的稳态:时钟悬空低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//数据捕获于第一个时钟沿
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//定义波特率预分频的值
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 
	SPI_Cmd(SPI1, ENABLE); //使能SPI外设
	SPI1_ReadWriteByte(0xff);//启动传输		 
}  

//无线模块调用的基类
void SPI2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI2_InitStructure;
	
	RCC_APB2PeriphClockCmd(	RCC_SPI2, ENABLE);	 
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2, ENABLE);	
	
	GPIO_InitStructure.GPIO_Pin = SCK_PIN | MISO_PIN | MOSI_PIN; //硬件SPI
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI2_PORT, &GPIO_InitStructure);
 	GPIO_SetBits(SPI2_PORT,SCK_PIN | MISO_PIN | MOSI_PIN); //初始状态时，将各引脚拉高

	SPI2_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI2_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI2_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI2_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//选择了串行时钟的稳态:时钟悬空低电平
	
//	#ifdef LT_MODULE
//		SPI2_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//数据捕获于第2个时钟沿(LT)
//	#else
		SPI2_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//数据捕获于第1个时钟沿(NRF)
//	#endif
	
	SPI2_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI2_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
	SPI2_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI2_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI2, &SPI2_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 

	SPI_Cmd(SPI2, ENABLE); //使能SPI外设
	//SPI2_ReadWriteByte(0xff);//启动传输		 
}  

//SPI 速度设置函数
void SPI1_Speed(u8 speed)
{
	SPI1->CR1&=0XFFC7; 
	SPI1->CR1|=speed;	//设置SPI1速度  
	SPI1->CR1|=1<<6; 		//SPI设备使能 
}

void SPI2_Speed(u8 speed)
{
	SPI2->CR1&=0XFFC7; 
	SPI2->CR1|=speed;	//设置SPI2速度  
	SPI2->CR1|=1<<6; 		//SPI设备使能 
} 

//SPI1写字节操作（OLED只需要写即可）
void SPI1_ReadWriteByte(u8 dat)
{		
	u8 i=0;	//超时错误，退出。			 	
	
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
	{
		if(i++>200)return ;
	}
	i=0;
  SPI_I2S_SendData(SPI1, dat); //发送缓冲区准备好，准备发送数据				    
}

//SPI2读写字节操作
u8 SPI2_ReadWriteByte(u8 dat)
{		
	u8 i=0;	//超时错误，退出。			 	
	
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
	{
		if(i++>200)return 0;
	}
	i=0;
	SPI_I2S_SendData(SPI2, dat); //发送缓冲区准备好，准备发送数据	
	
	//全双工模式
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位
	{
		i++;
		if(i>200)return 0;
	}	  						    
	return SPI_I2S_ReceiveData(SPI2); //接收缓冲区准备好，准备接收数据				    
}

