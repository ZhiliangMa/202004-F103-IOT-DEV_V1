#include "spi1.h"

//SPI1-DMA:http://www.51hei.com/bbs/dpj-38799-1.html
//blog.csdn.net/qq_28877125/article/details/80532502
/*
 * SPI1_CLK -> PA5
 * //SPI1_MISO -> PA6 PA6����û�б���ΪSPI��MISO���ţ�����������TFT_RST
 * SPI1_MOSI -> PA7
 */
void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );//PORTAʱ��ʹ�� 
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1,  ENABLE );//SPI1ʱ��ʹ��   

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;//GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //PA 5/6/7����������� 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);  //PA567 ����
	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_7);  //PA57 ����

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;        //����SPI����ģʽ:����Ϊ��SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;    //����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;           //����ͬ��ʱ�ӵĿ���״̬Ϊ�͵�ƽ
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;         //����ͬ��ʱ�ӵĵ�1�������أ��������½������ݱ�����
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;            //NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;     //���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ8��72/8 = 9MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;   //ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
    SPI_InitStructure.SPI_CRCPolynomial = 7;             //CRCֵ����Ķ���ʽ
    SPI_Init(SPI1, &SPI_InitStructure);                  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

    SPI_Cmd(SPI1, ENABLE); //ʹ��SPI����

    SPI1_ReadWriteByte(0xff);//��������      
}   
//SPI �ٶ����ú���
//SpeedSet:
//SPI_BaudRatePrescaler_2   2��Ƶ   
//SPI_BaudRatePrescaler_8   8��Ƶ   
//SPI_BaudRatePrescaler_16  16��Ƶ  
//SPI_BaudRatePrescaler_256 256��Ƶ   
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
    SPI1->CR1&=0XFFC7;
    SPI1->CR1|=SPI_BaudRatePrescaler;   //����SPI1�ٶ� 
    SPI_Cmd(SPI1,ENABLE); 
} 

//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
//���жϷ��ͻ������գ�Ȼ�������ݣ��ٵȴ������������
u8 SPI1_ReadWriteByte(u8 TxData) //TxData ������8λ��16λ�ģ�������SPI֮ǰ��ȷ��������֡��ʽ
{       
    u8 retry = 0;                         
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //0�����ͻ���ǿ�  �ȴ����ͻ��������
    {
        retry++;
        if(retry>200)return 0;
    }             
    SPI_I2S_SendData(SPI1, TxData);
    retry=0;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//�ȴ������������
    {
        retry++;
        if(retry>200)return 0;
    }                               
    return SPI_I2S_ReceiveData(SPI1); //����������յ����ݣ�SPI_DR�Ĵ��������                      
}
