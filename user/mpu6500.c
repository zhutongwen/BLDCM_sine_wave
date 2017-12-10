#include "stm32f4xx_spi.h"
#include "mpu6500.h"

s_mpu6500 mpu6500_info;


//	函数：delay_ms(uint32_t ms);
//  功能：软件延时
//	输入：ms——延时约多少毫秒
static void delay_ms(uint32_t ms)
{
	uint32_t loop;
	for(;ms>0;ms--)
	{
		for(loop=2700*3;loop>0;loop--);
	}
}

void SPI2Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure; 
	
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //开启时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);  
	
	//片选管脚，由软件控制
	SPI_NSS_HIGH;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	//spi管脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
    
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2); //打开引脚的复用功能
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2);
	
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //全双工模式
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;   //作为主机使用
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;   //数据长度8
    SPI_InitStructure.SPI_CPOL  = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;   //软件设置NSS功能
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2,&SPI_InitStructure);
    SPI_Cmd(SPI2,ENABLE);
}

s16 SpiReadWrite16Bits(s16 TxData)
{
	while( (SPI2->SR&0x02) == 0);//等待直到发送寄存器空
	SPI2->DR = TxData;
	while((SPI2->SR&0x01) == 0);//等待收到数据
	return SPI2->DR;
}

//姿态传感器MPU6500初始化函数
void mpu6500_init(void)
{
	delay_ms(500);
	SPI2Init();
	
	SPI_NSS_LOW;
	SpiReadWrite16Bits(106<<8|0x10);
	SPI_NSS_HIGH;
	delay_ms(1);
	
	SPI_NSS_LOW;
	SpiReadWrite16Bits(0x6b<<8|0x00);		//向地址x6b写入0x00  //使能温度传感器，使能PLL
	SPI_NSS_HIGH;
	delay_ms(1);
	
	SPI_NSS_LOW;
	SpiReadWrite16Bits(0x19<<8|0x00);		//数据采样速率设为	1KHZ
	SPI_NSS_HIGH;
	delay_ms(1);
	
	SPI_NSS_LOW;
	SpiReadWrite16Bits(0x1a<<8|0x03);	
	SPI_NSS_HIGH;
	delay_ms(1);
	
	SPI_NSS_LOW;
	SpiReadWrite16Bits(0x1b<<8|0x18);		//角速度最大测量值  +-2000/s
	SPI_NSS_HIGH;
	delay_ms(1);
	
	SPI_NSS_LOW;
	SpiReadWrite16Bits(0x1c<<8|0x10);		//加速度最大测量值 	+-8g
	SPI_NSS_HIGH;
	delay_ms(1);
}
