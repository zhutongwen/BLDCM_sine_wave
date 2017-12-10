#include "stm32f4_system.h"
#include "stm32f4xx_tim.h"
#include "tim.h"


//电机A
void Timer1Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);/*使能GPIOE的时钟*/

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;/*复用*/
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;/*推挽输出*/
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;/**/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;/**/
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_8 | 
									GPIO_Pin_9 | 
									GPIO_Pin_10 | 
									GPIO_Pin_11 | 
									GPIO_Pin_12 | 
									GPIO_Pin_13;
	
	GPIO_Init(GPIOE,&GPIO_InitStructure);/*初始化IO*/

	GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_TIM1);/*复用*/
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_TIM1);
	
	{
		uint16_t arr,ccr1,ccr2,ccr3;
		TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStrecture;
		TIM_OCInitTypeDef TIM_OCInitStructure;
		TIM_BDTRInitTypeDef TIM_BDTRInitStructure; 
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);/*使能定时器1的时钟*/
		
		arr = 4200; //SystemCoreClock/10000;
		/*10K*/
		ccr1 = 0;/*占空比40%*/
		ccr2 = 0;/*占空比25%*/
		ccr3 = 0;/*占空比33%*/

		TIM_TimeBaseInitStrecture.TIM_Period = arr;/*设置自动重装*/
		TIM_TimeBaseInitStrecture.TIM_Prescaler = 0;//时钟预分频数，对168M进行1(0+1)分频 
		TIM_TimeBaseInitStrecture.TIM_ClockDivision = TIM_CKD_DIV1;//采样分频
		TIM_TimeBaseInitStrecture.TIM_CounterMode = TIM_CounterMode_CenterAligned3;/*上下计数模式，*/
		TIM_TimeBaseInitStrecture.TIM_RepetitionCounter = 0;/**/
		TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStrecture);/*初始化*/

		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;/*设置PWM模式*/
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;/*起始输出*/
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;/*通道使能*/
		TIM_OCInitStructure.TIM_OutputNState  = TIM_OutputNState_Enable;/*互补通道使能*/
		TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;/**/
		TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;/*输出空闲状态1*/
		TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;/**/

		TIM_OCInitStructure.TIM_Pulse = ccr1;/*比较寄存器设置*/
		TIM_OC1Init(TIM1,&TIM_OCInitStructure);/*初始化通道1*/

		TIM_OCInitStructure.TIM_Pulse = ccr2;/*设置比较寄存器*/
		TIM_OC2Init(TIM1,&TIM_OCInitStructure);/*初始化通道2*/

		TIM_OCInitStructure.TIM_Pulse = ccr3;/*设置比较寄存器*/
		TIM_OC3Init(TIM1,&TIM_OCInitStructure);/*初始化通道3*/

//		TIM_OCInitStructure.TIM_Pulse = ccr4;/*设置比较寄存器*/
//		TIM_OC4Init(TIM1,&TIM_OCInitStructure);/*初始化通道4*/

		/**@step第五步死区和刹车功能配置,高级定时器才有的,通用定时器不用配置*/  
		/* Automatic Output enable, Break, dead time and lock configuration*/  
		TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;             //运行模式下输出  
		TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;             //空闲模式下输出选择   
		TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_1;                  //锁定设置，锁定级别1  
		TIM_BDTRInitStructure.TIM_DeadTime = 100;//死区时间1us  
		TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                    //刹车功能使能  
		TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;        //刹车输入极性，即刹车控制引脚接GND时，PWM停止  
		TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable; //自动输出使能  
		TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);  
		/*  刹车控制引脚为TIM1_BKIN pin(PB.12)，将PB12接GND，channel和其互补通道，都变为刹车后的电平，具体为0还是1，要看如下两个设置： 
			.TIM_OCIdleState = TIM_OCIdleState_Reset;   //刹车之后，PWM通道变为0 
			.TIM_OCNIdleState = TIM_OCNIdleState_Reset; //刹车之后，PWM互补通道变为0 
			 
			注意：如果没必要，还是不要开启刹车功能，因为会对PWM产生影响，特别是当PB12悬空时，波形将会有很大的波动。 
				  这里不打开刹车功能，即.TIM_Break = TIM_Break_Disable; 
		*/ 

		TIM_ClearFlag(TIM1,TIM_FLAG_Update);/*清更新标志位*/
		TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);/*使能中断*/

		TIM_Cmd(TIM1,ENABLE);/*计数使能*/
		TIM_CtrlPWMOutputs(TIM1,ENABLE);/*输出使能*/
	}
	//使能H桥驱动芯片
	{
		GPIO_InitTypeDef GPIO_InitStruct;

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOE, &GPIO_InitStruct);	
		
		PEO(7) = 1;
	}
}

//霍尔A
void Timer5Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//使能 GPIOA 时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);//是能 TIM5 时钟

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;/*复用*/
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;/**/
	GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;/**/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | 
								  GPIO_Pin_1 | 
								  GPIO_Pin_2;
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);/*初始化IO*/

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM5);/*复用*/
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_TIM5);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_TIM5);	
	{
		TIM5->PSC	= 83u;	   //预分频器
		TIM5->ARR	= 100000;  // 0.1s
		//	TIM3->RCR = 0;	//周期倍频
			
		TIM5->SMCR 	= 0x0044;
		TIM5->DIER	= 0x0003;  //Update interrupt enabled
		//	TIM3->SR	=
		TIM5->EGR	= 0x0000;	//event generation register
		TIM5->CCMR1	= 0x0303;
		TIM5->CCMR2	= 0x0003;
		TIM5->CCER	= 0x0111;
		
		TIM5->CR2	= 0x00d0;//CH1 | CH2 | CH3
		TIM5->CR1	= 0x0001;
	}
}

//霍尔B
void Timer2Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//使能 GPIOA 时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);//使能 GPIOB 时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//是能 TIM2 时钟

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;/*复用*/
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;/**/
	GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;/**/
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_15;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);/*初始化IO*/
	
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_3 | 
									GPIO_Pin_10;
	GPIO_Init(GPIOB,&GPIO_InitStructure);/*初始化IO*/

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource15,GPIO_AF_TIM2);/*复用*/
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource3,GPIO_AF_TIM2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_TIM2);	
	{
		TIM2->PSC	= 83u;	   //预分频器
		TIM2->ARR	= 100000;  // 100ms
		//	TIM2->RCR = 0;	//周期倍频
			
		TIM2->SMCR 	= 0x0044;
		TIM2->DIER	= 0x0003;  //Update interrupt enabled
		//	TIM3->SR	=
		TIM2->EGR	= 0x0000;	//event generation register
		TIM2->CCMR1	= 0x0303;
		TIM2->CCMR2	= 0x0003;
		TIM2->CCER	= 0x0111;
		
		TIM2->CR2	= 0x00d0;//CH1 | CH2 | CH3
		TIM2->CR1	= 0x0001;
	}
}

//电机B
void Timer8Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);/*使能GPIOA的时钟*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_TIM8);/*复用*/
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource0,GPIO_AF_TIM8);/*复用*/
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource1,GPIO_AF_TIM8);/*复用*/
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_TIM8);/*复用*/
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_TIM8);/*复用*/
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource8,GPIO_AF_TIM8);/*复用*/
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;/*复用*/
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;/*推挽输出*/
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;/**/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;/**/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 ;
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);/*初始化IO*/	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 ;	
	GPIO_Init(GPIOB,&GPIO_InitStructure);/*初始化IO*/
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 ;	
	GPIO_Init(GPIOC,&GPIO_InitStructure);/*初始化IO*/	
	
	{
		uint16_t arr,ccr1,ccr2,ccr3;
		TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStrecture;
		TIM_OCInitTypeDef TIM_OCInitStructure;
		TIM_BDTRInitTypeDef TIM_BDTRInitStructure; 
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);/*使能定时器1的时钟*/
		
		arr = 4200; //SystemCoreClock/10000;
		/*10K*/
		ccr1 = arr/2;/*占空比40%*/
		ccr2 = arr/2;/*占空比25%*/
		ccr3 = arr/2;/*占空比33%*/

		TIM_TimeBaseInitStrecture.TIM_Period = arr;/*设置自动重装*/
		TIM_TimeBaseInitStrecture.TIM_Prescaler = 0;//时钟预分频数，对168M进行1(0+1)分频 
		TIM_TimeBaseInitStrecture.TIM_ClockDivision = TIM_CKD_DIV1;//采样分频
		TIM_TimeBaseInitStrecture.TIM_CounterMode = TIM_CounterMode_CenterAligned3;/*上下计数模式，*/
		TIM_TimeBaseInitStrecture.TIM_RepetitionCounter = 0;/**/
		TIM_TimeBaseInit(TIM8,&TIM_TimeBaseInitStrecture);/*初始化*/

		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;/*设置PWM模式*/
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;/*起始输出*/
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;/*通道使能*/
		TIM_OCInitStructure.TIM_OutputNState  = TIM_OutputNState_Enable;/*互补通道使能*/
		TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;/**/
		TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;/*输出空闲状态1*/
		TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;/**/

		TIM_OCInitStructure.TIM_Pulse = ccr1;/*比较寄存器设置*/
		TIM_OC1Init(TIM8,&TIM_OCInitStructure);/*初始化通道1*/

		TIM_OCInitStructure.TIM_Pulse = ccr2;/*设置比较寄存器*/
		TIM_OC2Init(TIM8,&TIM_OCInitStructure);/*初始化通道2*/

		TIM_OCInitStructure.TIM_Pulse = ccr3;/*设置比较寄存器*/
		TIM_OC3Init(TIM8,&TIM_OCInitStructure);/*初始化通道3*/

//		TIM_OCInitStructure.TIM_Pulse = ccr4;/*设置比较寄存器*/
//		TIM_OC4Init(TIM8,&TIM_OCInitStructure);/*初始化通道4*/


		/**@step第五步死区和刹车功能配置,高级定时器才有的,通用定时器不用配置*/  
		/* Automatic Output enable, Break, dead time and lock configuration*/  
		TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;             //运行模式下输出  
		TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;             //空闲模式下输出选择   
		TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_1;                  //锁定设置，锁定级别1  
		TIM_BDTRInitStructure.TIM_DeadTime = 100u;//死区时间1us  
		TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                    //刹车功能使能  
		TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;        //刹车输入极性，即刹车控制引脚接GND时，PWM停止  
		TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable; //自动输出使能  
		TIM_BDTRConfig(TIM8, &TIM_BDTRInitStructure);  
		/*  刹车控制引脚为TIM1_BKIN pin(PB.12)，将PB12接GND，channel和其互补通道，都变为刹车后的电平，具体为0还是1，要看如下两个设置： 
			.TIM_OCIdleState = TIM_OCIdleState_Reset;   //刹车之后，PWM通道变为0 
			.TIM_OCNIdleState = TIM_OCNIdleState_Reset; //刹车之后，PWM互补通道变为0 
			 
			注意：如果没必要，还是不要开启刹车功能，因为会对PWM产生影响，特别是当PB12悬空时，波形将会有很大的波动。 
				  这里不打开刹车功能，即.TIM_Break = TIM_Break_Disable; 
		*/ 

		TIM_ClearFlag(TIM8,TIM_FLAG_Update);/*清更新标志位*/
		TIM_ITConfig(TIM8,TIM_IT_Update,ENABLE);/*使能中断*/

		TIM_Cmd(TIM8,ENABLE);/*计数使能*/
		TIM_CtrlPWMOutputs(TIM8,ENABLE);/*输出使能*/
	}
	//使能H桥驱动芯片
	{
		GPIO_InitTypeDef GPIO_InitStruct;

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOC, &GPIO_InitStruct);	
		
		PCO(9) = 1;
	}
}

void Time(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;  
	TIM_OCInitTypeDef  TIM_OCInitStructure;  
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;  
	uint16_t TimerPeriod = 0;  
	uint16_t Channel1Pulse = 0, Channel2Pulse = 0, Channel3Pulse = 0;  
	

    /**@step第一步配置时钟*/  
    /**@step第二步配置goio口*/  
    /**@step第三步定时器基本配置*/  
    /* Time Base configuration */  
    TIM_TimeBaseStructure.TIM_Prescaler = 0;//时钟预分频数，对168M进行1(0+1)分频  
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数   
    TIM_TimeBaseStructure.TIM_Period = TimerPeriod;//自动重装载寄存器的值，ARR=9561  
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;    //采样分频   
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;//重复寄存器，用于自动更新pwm占空比  
  
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);  
  
    /**@step第四步 PWM输出配置*/  
    /* Channel 1, 2 and 3 Configuration in PWM mode */  
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;             //PWM1为正常占空比模式，PWM2为反极性模式  
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       //High为占空比高极性，此时占空比为20%；Low则为反极性，占空比为80%  
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   //使能该通道输出  
    TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;  //设置占空比时间，CCR1_Val=4780，占空比为4780/(9561+1)=0.5  
      
    //-------下面几个参数是高级定时器才会用到通用定时器不用配置  
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; //使能互补端输出  
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;     //设置互补端输出极性  
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;    //刹车之后输出状态Specifies the TIM Output Compare pin state during Idle state  
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;  //刹车之后互补端输出状态  
    //-------  
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);//对channel1进行配置  
      
    TIM_OCInitStructure.TIM_Pulse = Channel2Pulse;//CCR2_Val=2390，比较值  
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);//对channel2进行配置  
    TIM_OCInitStructure.TIM_Pulse = Channel3Pulse;//CCR3_Val=1195，比较值  
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);//对channel3进行配置  
  
    /**@step第五步死区和刹车功能配置,高级定时器才有的,通用定时器不用配置*/  
    /* Automatic Output enable, Break, dead time and lock configuration*/  
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;             //运行模式下输出  
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;             //空闲模式下输出选择   
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_1;                  //锁定设置，锁定级别1  
    TIM_BDTRInitStructure.TIM_DeadTime = 0x94;//死区时间1us  
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                    //刹车功能使能  
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;        //刹车输入极性，即刹车控制引脚接GND时，PWM停止  
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable; //自动输出使能  
    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);  
    /*  刹车控制引脚为TIM1_BKIN pin(PB.12)，将PB12接GND，channel和其互补通道，都变为刹车后的电平，具体为0还是1，要看如下两个设置： 
        .TIM_OCIdleState = TIM_OCIdleState_Reset;   //刹车之后，PWM通道变为0 
        .TIM_OCNIdleState = TIM_OCNIdleState_Reset; //刹车之后，PWM互补通道变为0 
         
        注意：如果没必要，还是不要开启刹车功能，因为会对PWM产生影响，特别是当PB12悬空时，波形将会有很大的波动。 
              这里不打开刹车功能，即.TIM_Break = TIM_Break_Disable; 
    */  
      
		
			
    /**@step第六步使能端的打开*/  
    /* TIM1 counter enable */  
    TIM_Cmd(TIM1, ENABLE);//打开TIM1  
  
    /* Main Output Enable */  
    TIM_CtrlPWMOutputs(TIM1, ENABLE);//PWM输出使能,一定要记得打  
}
