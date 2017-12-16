/**
  ******************************************************************************
  * @file    motor_control.c 
  * @author  zhutongwen
  * @version V1.0
  * @date    2017.6.13
  * @brief   无刷电机控制程序
  ******************************************************************************
  */ 
#include "motor_control.h"
#include "bsp.h"
#include "math.h"
#include "adc.h"
#include "arm_math.h"


s_motor_info  motor_a;
s_motor_info  motor_b;
extern  s16   OutData[5];

extern float sinf_wave[3600];
extern u8  time_flag_5ms;//在定时器中断中被累加，在IMU滤波是置零。



//svpwm
//角度范围 0--3600(-3600 +7200)   单位0.1度
//幅度范围 0--4100
static void svpwm(char a_or_b, s16 angle, u16 range)
{
	__IO uint32_t *pCCR1 = 0;
	__IO uint32_t *pCCR2 = 0;
	__IO uint32_t *pCCR3 = 0;
	u16 delay_ofset = 70;
	
	if(range > 4100)	return;
	
	if(a_or_b == 'a')
	{
		pCCR1 = &(TIM1->CCR3);
		pCCR2 = &(TIM1->CCR2);
		pCCR3 = &(TIM1->CCR1);	
	}
	
	else if(a_or_b == 'b')
	{
		pCCR1 = &(TIM8->CCR3);
		pCCR2 = &(TIM8->CCR2);
		pCCR3 = &(TIM8->CCR1);	
	}
	
	angle = (angle >= 3600)? 	(angle - 3600) : angle;
	angle = (angle <  0)? 		(angle + 3600) : angle;	
	
	//第一扇区
	if(angle>=0 && angle<600)
	{
		*pCCR1	= delay_ofset + range*(sinf_wave[angle] + sinf_wave[600-angle]);
		*pCCR2	= delay_ofset + range*sinf_wave[angle];
		*pCCR3	= delay_ofset;	
	}
	//第二扇区
	if(angle>=600 && angle<1200)
	{
		angle -= 600;
		*pCCR1	= delay_ofset + range*sinf_wave[600-angle];
		*pCCR2	= delay_ofset + range*(sinf_wave[angle] + sinf_wave[600-angle]);
		*pCCR3	= delay_ofset;
	}
	//第三扇区
	if(angle>=1200 && angle<1800)
	{
		angle -= 1200;
		*pCCR1	= delay_ofset;
		*pCCR2	= delay_ofset + range*(sinf_wave[angle] + sinf_wave[600-angle]);
		*pCCR3	= delay_ofset + range*sinf_wave[angle];
	}
	//第四扇区
	if(angle>=1800 && angle<2400)
	{
		angle -= 1800;
		*pCCR1	= delay_ofset;	
		*pCCR2	= delay_ofset + range*sinf_wave[600-angle];
		*pCCR3	= delay_ofset + range*(sinf_wave[angle] + sinf_wave[600-angle]);		
	}
	//第五扇区
	if(angle>=2400 && angle<3000)
	{
		angle -= 2400;
		*pCCR1	= delay_ofset + range*sinf_wave[angle];	
		*pCCR2	= delay_ofset;
		*pCCR3	= delay_ofset + range*(sinf_wave[angle] + sinf_wave[600-angle]);
	}
	//第六扇区
	if(angle>=3000 && angle<3600)
	{
		angle -= 3000;
		*pCCR1	= delay_ofset + range*(sinf_wave[angle] + sinf_wave[600-angle]);
		*pCCR2	= delay_ofset;
		*pCCR3	= delay_ofset + range*sinf_wave[600-angle];
	}
}

//方波驱动
// a_or_b  {'a' 'b'}
// section {0 1 2 3 4 5}
// set_speed { -4100  +4100}
static void square_wave_drive(char a_or_b, u8 section, s16 set_speed)
{
	s16 off_set = 0;
	if(a_or_b == 'a')
	{
		off_set = -300;
	}
	else if(a_or_b == 'b')
	{
		off_set = 2100;
	}		
	if((set_speed > 4100) || (set_speed < -4100))
	{
		return;
	}
	//正转
	if(set_speed >= 0)
	{
		switch(section)
		{
			case 0:
				svpwm(a_or_b, 1800+off_set, set_speed);
			break;
			case 1:
				svpwm(a_or_b, 2400+off_set, set_speed);			
			break;
			case 2:
				svpwm(a_or_b, 3000+off_set, set_speed);
			break;
			case 3:
				svpwm(a_or_b, 0+off_set, set_speed);
			break;
			case 4:
				svpwm(a_or_b, 600+off_set, set_speed);
			break;
			case 5:
				svpwm(a_or_b, 1200+off_set, set_speed);
			break;	
		}		
	}
	else // (set_speed < 0)  反转
	{
		switch(section)
		{
			case 0:
				svpwm(a_or_b, 0+off_set, -set_speed);
			break;
			case 1:
				svpwm(a_or_b, 600+off_set, -set_speed);			
			break;
			case 2:
				svpwm(a_or_b, 1200+off_set, -set_speed);
			break;
			case 3:
				svpwm(a_or_b, 1800+off_set, -set_speed);
			break;
			case 4:
				svpwm(a_or_b, 2400+off_set, -set_speed);
			break;
			case 5:
				svpwm(a_or_b, 3000+off_set, -set_speed);
			break;	
		}		
	}
}



//正玄波驱动
// a_or_b  {'a' 'b'}
// angle 0--3600   单位0.1度
// set_speed { -4100  +4100}
static void sine_wave_drive(char a_or_b, s16 angle, s16 set_speed)
{
	s16 off_set = 0;
	if(a_or_b == 'a')
	{
		off_set = 1200;
	}
	else if(a_or_b == 'b')
	{
		off_set = 0;
	}		
	if((set_speed > 4100) || (set_speed < -4100))
	{
		return;
	}
	//正转
	if(set_speed >= 0)
	{
		svpwm(a_or_b, angle+off_set, set_speed);	
	}
	else // (set_speed < 0)  反转
	{
		svpwm(a_or_b, angle + 1800 +off_set, -set_speed);	
	}
}
//更新转子所在的象限
static u8 update_section(char a_or_b)
{
	u8 section;
	u8	hall_status;
	//霍尔传感器反馈位置
	if(a_or_b == 'a')
	{
		hall_status =	GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) << 2 |
						GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) << 1 |
						GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) << 0;	
	}
	else if(a_or_b == 'b')
	{
		hall_status =	GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) << 0 |
						GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) << 1 |
						GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) << 2;	
	}

	switch(hall_status)
	{
		case 0x04:
		{
			section = 0;
			break;
		}
		case 0x05:
		{
			section = 1;
			break;
		}
		case 0x01:
		{
			section = 2;
			break;
		}
		case 0x03:
		{
			section = 3;
			break;
		}
		case 0x02:
		{
			section = 4;
			break;
		}
		case 0x06:
		{
			section = 5;
			break;
		}
	}
	return section;
}


//电机A
void TIM1_UP_TIM10_IRQHandler(void)
{
	if(TIM1->CNT > 4000) //计数器波峰中断
	{		
//		square_wave_forawrd('a', motor_a.current_section, motor_a.set_speed);
//		sine_wave_drive('a', motor_a.electrical_angle, motor_a.set_speed);
		if(1)//速度PI
		{
			static float pid =0;
			static float p =0;
			static float i =0;
			
			float ki = 1.0;
			
			p = 1*(motor_a.set_speed - motor_a.current_speed);
			i += 0.0002f*(motor_a.set_speed - motor_a.current_speed);
			
			i = i > 1000? 		1000:i;
			i = i < (-1000)?  	(-1000):i;
			
			pid = 0.74f*motor_a.set_speed + p + ki*i;
			
			pid = pid < -4000? 		-4000:pid;
			pid = pid >  4000?		 4000:pid;
			
//			square_wave_drive('a', motor_a.current_section, pid);
			sine_wave_drive('a', motor_a.electrical_angle, pid);
		}
		else
		{
//			square_wave_drive('a', motor_a.current_section, motor_a.set_speed);	
			sine_wave_drive('a', motor_a.electrical_angle, motor_a.set_speed);
		}
		
		if(1)//转子电角度计算
		{
			static float section_time = 0;
			s16 add_angle = 0;
			
			motor_a.electrical_angle = 600*motor_a.current_section;
			
			if((++section_time) > 2000)	section_time = 0;//最大计时时间0.1s；
			if(motor_a.is_section_chenged)
			{
				motor_a.is_section_chenged = 0;
				section_time  = 0;
			}
			
			add_angle = (s16)((section_time/166.67f)*((float)motor_a.current_speed));// time*speed; 单位由转每分到 0.1度每秒
			add_angle = add_angle > 600?	 600:add_angle;
			add_angle = add_angle < -600?	-600:add_angle;
			add_angle = motor_a.current_speed < 0?		 (600+add_angle):add_angle;
			
			motor_a.electrical_angle += add_angle;
		}
	}
	
	if(TIM1->CNT < 400) //计数器波底中断
	{
		//ad转换
		{
			static u16 u16counter_adc_average;
			static u32 u32adc_sum = 0;
			if(u16counter_adc_average < 1000)
			{
				u16counter_adc_average++;
				u32adc_sum += ADC2ConvertedValue[2];
				u32adc_sum += ADC2ConvertedValue[1];
				if(u16counter_adc_average == 999)
				{
					ADC2ConvertedValue[0] = (u32adc_sum + 1000)/2000; //四舍五入 第一次全零
				}
			}
			ADC2->CR2 |= (uint32_t)ADC_CR2_SWSTART;//启动一次ADC采样		
		}	
		// 5ms 定时
		{
			static u16 u16counter = 0;
			if(++u16counter >= 100) //5ms
			{
				u16counter = 0;
				time_flag_5ms++;
			}
		}	
	}
	TIM_ClearFlag(TIM1,TIM_FLAG_Update);/*清中断标志*/
}


//霍尔A
void TIM5_IRQHandler(void)
{
	static u32 timer_12[13] = {0};//连续n次霍尔信号时间和
	static u16 counter_timer_12 = 0;//连续n次霍尔信号 同向
	static s8 counter_direction = 0;
	u16 state = TIM5->SR;
	
	LED_2 = !LED_2;
	motor_a.current_section = update_section('a');
	
	//超过 0.1s 还没有霍尔变化
	if((state&0x0003) == 0x0001)
	{
		TIM5->SR &= ~0x01;	//清除中断-标志
		counter_direction = 0;
		motor_a.current_speed = 0;
		motor_a.period = 0;
		for(counter_timer_12 = 0; counter_timer_12 < 12; counter_timer_12++)
		{
			timer_12[counter_timer_12] = 0;
		}
		motor_a.last_direction = 0;
	}
	
	//0.1s中内有霍尔信号变化，则进入此中断	
	else if((state&0x0003) == 0x0003) //counter 更新中断
	{
		TIM5->SR &= (~0x03);	//清除中断-标志位
		motor_a.is_section_chenged = 1;//==1 表示转子象限改变了，用于转子电角度计算
							
		//旋转方向判断 和 速度计算
		{
			//本次正转
			if(motor_a.current_section - motor_a.last_section == 1 || motor_a.current_section - motor_a.last_section == -5)
			{
				//上次正转
				if(motor_a.last_direction > 0)
				{				
					if(++counter_timer_12 >= 12)	counter_timer_12=0;		
					motor_a.period -= timer_12[counter_timer_12];
					timer_12[counter_timer_12] = TIM5->CCR1;
					motor_a.period += timer_12[counter_timer_12];	
					
					//连续12次正转
					if((++counter_direction) > 12)	counter_direction = 12;
					motor_a.current_speed = (5000000*counter_direction)/motor_a.period;
				}	
				//上次反转 or stop
				else
				{
					counter_direction = 0;
					motor_a.current_speed = 0;
					motor_a.period = 0;
					for(counter_timer_12 = 0; counter_timer_12 < 12; counter_timer_12++)
					{
						timer_12[counter_timer_12] = 0;
					}
				}
				motor_a.last_direction = 1;
			}
			//本次反转
			if(motor_a.current_section - motor_a.last_section == -1 || motor_a.current_section - motor_a.last_section == 5)
			{
				//last direction backward
				if(motor_a.last_direction < 0)
				{
					if(++counter_timer_12 >= 12)	counter_timer_12=0;		
					motor_a.period -= timer_12[counter_timer_12];
					timer_12[counter_timer_12] = TIM5->CCR1;
					motor_a.period += timer_12[counter_timer_12];						
				
					//连续12次反转
					if((--counter_direction) < -12)	counter_direction = -12;		
					motor_a.current_speed = 0 - (0 - counter_direction*5000000)/motor_a.period;	
				}
				//上次正转 or stop
				else
				{
					counter_direction = 0;
					motor_a.current_speed = 0;
					motor_a.period = 0;
					for(counter_timer_12 = 0; counter_timer_12 < 12; counter_timer_12++)
					{
						timer_12[counter_timer_12] = 0;
					}
				}
				motor_a.last_direction = -1;
			}	
		}
	}
//	OutData[3] = motor_a.current_speed;
	motor_a.last_section = motor_a.current_section;	
}

//电机B
void TIM8_UP_TIM13_IRQHandler(void)
{
	if(TIM8->CNT > 4000) //计数器波峰中断
	{		
		if(1)//速度PI
		{
			static float pid =0;
			static float p =0;
			static float i =0;
			
			float ki = 1.0;
			
			p = 1*(motor_b.set_speed - motor_b.current_speed);
			i += 0.0002f*(motor_b.set_speed - motor_b.current_speed);
			
			i = i > 1000? 		1000:i;
			i = i < (-1000)?  	(-1000):i;
			
			pid = 0.74f*motor_b.set_speed + p + ki*i;
			
			pid = pid < -4000? 		-4000:pid;
			pid = pid >  4000?		 4000:pid;
			
//			square_wave_drive('b', motor_b.current_section, pid);
			sine_wave_drive('b', motor_b.electrical_angle, pid);
		}
		else
		{
//			square_wave_drive('b', motor_b.current_section, motor_b.set_speed);	
			sine_wave_drive('b', motor_b.electrical_angle, motor_b.set_speed);
		}
		if(1)//转子电角度计算
		{
			static float section_time = 0;
			s16 add_angle = 0;
			
			motor_b.electrical_angle = 600*motor_b.current_section;
			
			if((++section_time) > 2000)	section_time = 0;//最大计时时间0.1s；
			if(motor_b.is_section_chenged)
			{
				motor_b.is_section_chenged = 0;
				section_time  = 0;
			}
			
			add_angle = (s16)((section_time/166.67f)*((float)motor_b.current_speed));// time*speed; 单位由转每分到 0.1度每秒
			add_angle = add_angle > 600?	 600:add_angle;
			add_angle = add_angle < -600?	-600:add_angle;
			add_angle = motor_b.current_speed < 0?		 (600+add_angle):add_angle;
			
			motor_b.electrical_angle += add_angle;
		}		
	}
	
	if(TIM8->CNT < 400) //计数器波底中断
	{
		//ad转换
		{
			static u16 u16counter_adc_average;
			static u32 u32adc_sum = 0;
			if(u16counter_adc_average < 1000)
			{
				u16counter_adc_average++;
//				u32adc_sum += ADC3ConvertedValue[3];
				u32adc_sum += ADC3ConvertedValue[2];
				u32adc_sum += ADC3ConvertedValue[1];
				if(u16counter_adc_average == 999)
				{
					ADC3ConvertedValue[0] = (u32adc_sum + 1000)/2000; //四舍五入 第一次全零
				}
			}
			ADC3->CR2 |= (uint32_t)ADC_CR2_SWSTART;//启动一次ADC采样		
		}			
	}
	TIM_ClearFlag(TIM8,TIM_FLAG_Update);/*清中断标志*/
}
//霍尔B
void TIM2_IRQHandler(void)
{				
	static u32 timer_12[12] = {0};//连续n次霍尔信号时间和
	static u16 counter_timer_12 = 0;//连续n次霍尔信号 同向
	static s16  counter_direction = 0;
	u16 state = TIM2->SR;
	
	LED_1 = !LED_1;
	motor_b.current_section = update_section('b');
	
	//超过 0.1s 还没有霍尔变化
	if((state&0x0003) == 0x0001)
	{
		TIM2->SR &= ~0x01;	//清除中断-标志
		counter_direction = 0;
		motor_b.current_speed = 0;
		motor_b.period = 0;
		for(counter_timer_12 = 0; counter_timer_12 < 12; counter_timer_12++)
		{
			timer_12[counter_timer_12] = 0;
		}
		motor_b.last_direction = 0;
	}
	
	//0.1s中内有霍尔信号变化，则进入此中断	
	else if((state&0x0003) == 0x0003) //counter 更新中断
	{
		TIM2->SR &= (~0x03);	//清除中断-标志位
		motor_b.is_section_chenged = 1;//==1 表示转子象限改变了，用于转子电角度计算
							
		//旋转方向判断 和 速度计算
		{				
			//转子旋转方向判断
			{
				//本次正转
				if(motor_b.current_section - motor_b.last_section == 1 || motor_b.current_section - motor_b.last_section == -5)
				{
					//上次正转
					if(motor_b.last_direction > 0)
					{				
						if(++counter_timer_12 >= 12)	counter_timer_12=0;		
						motor_b.period -= timer_12[counter_timer_12];
						timer_12[counter_timer_12] = TIM2->CCR1;
						motor_b.period += timer_12[counter_timer_12];	
						
						//连续12次正转
						if((++counter_direction) > 12)	counter_direction = 12;	
						motor_b.current_speed = (5000000*counter_direction)/motor_b.period; 						
					}	
					//上次反转 or stop
					else
					{
						counter_direction = 0;
						motor_b.current_speed = 0;
						motor_b.period = 0;
						for(counter_timer_12 = 0; counter_timer_12 < 12; counter_timer_12++)
						{
							timer_12[counter_timer_12] = 0;
						}
					}
					motor_b.last_direction = 1;
				}
				//本次反转
				if(motor_b.current_section - motor_b.last_section == -1 || motor_b.current_section - motor_b.last_section == 5)
				{
					//上次反转
					if(motor_b.last_direction < 0)
					{
						if(++counter_timer_12 >= 12)	counter_timer_12=0;		
						motor_b.period -= timer_12[counter_timer_12];
						timer_12[counter_timer_12] = TIM2->CCR1;
						motor_b.period += timer_12[counter_timer_12];	
						
						//连续12次反转
						if((--counter_direction) < -12)	counter_direction = -12;
						motor_b.current_speed = 0 - (0 - counter_direction*5000000)/motor_b.period;
					}
					//上次正转 or stop
					else
					{
						counter_direction = 0;
						motor_b.current_speed = 0;
						motor_b.period = 0;
						for(counter_timer_12 = 0; counter_timer_12 < 12; counter_timer_12++)
						{
							timer_12[counter_timer_12] = 0;
						}
					}
					motor_b.last_direction = -1;
				}
				motor_b.last_section = motor_b.current_section;			
			}
		}
	}
//	OutData[1] = motor_b.current_speed;
}
/*=====================================================================================================*/
/*=====================================================================================================*/
//电机参数初始化
void motor_init(char a_or_b)
{
	if(a_or_b == 'b')
	{
		motor_b.current_section = update_section('b');
		motor_b.last_section = motor_b.current_section;
	}
}
