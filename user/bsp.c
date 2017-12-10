/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f4_system.h"
#include "bsp.h"
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LED_Config
**功能 : LED 設定 & 配置
**輸入 : None
**輸出 : None
**使用 : LED_Config();
**=====================================================================================================*/
/*=====================================================================================================*/
void LedInit( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	
	/* LED_1 PB6 */   /* LED_2 PB5 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	LED_1 = 1;
	LED_2 = 1;
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : KEY_Config
**功能 : KEY 設定 & 配置
**輸入 : None
**輸出 : None
**使用 : KEY_Config();
**=====================================================================================================*/
/*=====================================================================================================*/
void KeyInit( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	/* KEY1 PD10 */ 
	/* KEY2 PD11 */ 
	/* KEY3 PD9 */ 
	/* KEY4 PD8 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_9 | GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void NvicInit(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	//电机A
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;/*定时器1的中断通道使能*/
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;/*定时器1的中断通道使能*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;/*抢占优先级*/
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;/*响应优先级*/
	NVIC_Init(&NVIC_InitStructure);/*配置中断分组，并使能中断*/
	
	//霍尔A
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;       
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//电机B
	NVIC_InitStructure.NVIC_IRQChannel = TIM8_UP_TIM13_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//霍尔B
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;       
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

