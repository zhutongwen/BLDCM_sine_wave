/*=====================================================================================================*/
/*=====================================================================================================*/

#include "usart.h"

extern s16 OutData[5];

/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : RS232_Config
**功能 : 
**輸入 : 
**輸出 : 
**使用 : 
**=====================================================================================================*/
/*=====================================================================================================*/
void Usart1Init( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,  ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	/* USART1 Tx PA9  */	/* USART1 Rx PA10 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

//	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
//	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
//	GPIO_Init(GPIOA, &GPIO_InitStruct);
	

	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStruct);
	USART_Cmd(USART1, ENABLE);

	USART_ClearFlag(USART1, USART_FLAG_TC);
}


//函数功能：数据校验。。详情参考虚拟示波器软件(VisualScope_Cracked.exe)的help
uint16_t CRC_CHECK(short int *Buf, unsigned char CRC_CNT)
{
	uint16_t CRC_Temp;
	unsigned char i,j;
	CRC_Temp = 0xffff; 
	for (i=0;i<CRC_CNT; i++)
	{      
		CRC_Temp ^= Buf[i];
		for (j=0;j<16;j++) 
		{
			if (CRC_Temp & 0x01)
				CRC_Temp = (CRC_Temp >>1 ) ^ 0xa001;
			else
				CRC_Temp = CRC_Temp >> 1;
		}
	}
	return(CRC_Temp);
} 

//函数功能：向上位机发送数据。详情参考虚拟示波器软件(VisualScope_Cracked.exe)的help
void OutPutData(void)		  //低位在前 ， 高位在后
{
	unsigned char i;
	OutData[4] = CRC_CHECK(OutData,4);	
	for(i=0;i<5;i++)	  
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	//判断是否发送完毕
		USART_SendData(USART1,OutData[i]);	
		
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	//判断是否发送完毕
		USART_SendData(USART1,OutData[i]>>8);
	}	
} 
