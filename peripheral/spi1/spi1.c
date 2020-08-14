#include "spi1.h"

//SPI1-DMA:http://www.51hei.com/bbs/dpj-38799-1.html
//blog.csdn.net/qq_28877125/article/details/80532502
/*
 * SPI1_CLK -> PA5
 * //SPI1_MISO -> PA6 PA6这里没有被作为SPI的MISO引脚，而是用作了TFT_RST
 * SPI1_MOSI -> PA7
 */
void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );//PORTA时钟使能 
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1,  ENABLE );//SPI1时钟使能   

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;//GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //PA 5/6/7复用推挽输出 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);  //PA567 上拉
	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_7);  //PA57 上拉

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;        //设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;    //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;           //串行同步时钟的空闲状态为低电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;         //串行同步时钟的第1个跳变沿（上升或下降）数据被采样
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;            //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;     //定义波特率预分频的值:波特率预分频值为8，72/8 = 9MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;   //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;             //CRC值计算的多项式
    SPI_Init(SPI1, &SPI_InitStructure);                  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

    SPI_Cmd(SPI1, ENABLE); //使能SPI外设

    SPI1_ReadWriteByte(0xff);//启动传输      
}   
//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   
//SPI_BaudRatePrescaler_8   8分频   
//SPI_BaudRatePrescaler_16  16分频  
//SPI_BaudRatePrescaler_256 256分频   
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
    SPI1->CR1&=0XFFC7;
    SPI1->CR1|=SPI_BaudRatePrescaler;   //设置SPI1速度 
    SPI_Cmd(SPI1,ENABLE); 
} 

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
//先判断发送缓冲器空，然后发送数据，再等待接收数据完成
u8 SPI1_ReadWriteByte(u8 TxData) //TxData 可以是8位或16位的，在启用SPI之前就确定好数据帧格式
{       
    u8 retry = 0;                         
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //0：发送缓冲非空  等待发送缓冲器变空
    {
        retry++;
        if(retry>200)return 0;
    }             
    SPI_I2S_SendData(SPI1, TxData);
    retry=0;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//等待接收数据完成
    {
        retry++;
        if(retry>200)return 0;
    }                               
    return SPI_I2S_ReceiveData(SPI1); //返回最近接收的数据，SPI_DR寄存器里面的                      
}
