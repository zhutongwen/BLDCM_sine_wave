/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f4_system.h"
#include "bsp.h"
#include "usart.h"
#include "adc.h"
#include "tim.h"
#include "math.h"
#include "motor_control.h"
#include "mpu6500.h"
#include "arm_math.h"


u8  time_flag_5ms = 0;//在定时器中断中被累加，在IMU滤波是置零。
s16 OutData[5];
float sinf_wave[3600]; //0--60度 正弦波



//生成正弦波
void getSinWave(void)
{
	uint32_t i=0;
	for(i=0; i<3600; i++)
	{
		sinf_wave[i] =sinf(i/572.9578);
	}
}


/*=====================================================================================================*/
/*=====================================================================================================*/
int main(void )
{
	s16 adc_data = 0;
	
	getSinWave();
	
	LedInit();
	KeyInit();
	Usart1Init();
	
	Adc1Init();//可调电阻电压采样
	
	Adc2Init();//电机A电流采样
	Adc3Init();//电机B电流采样
	
	Timer1Init();//电机A
	Timer5Init();//霍尔A	

	Timer8Init();//电机B
	Timer2Init();//霍尔B
		
	NvicInit();

	while(1)
	{
		if(time_flag_5ms == 1)
		{
			time_flag_5ms = 0;
			adc_data = ADC1->DR;
			
			if(adc_data > 4000)
			{
				adc_data = 4000;
			}				
			motor_a.set_speed = 2*(2000 - adc_data);
			
			motor_a.set_speed = ((motor_a.set_speed<300) && (motor_a.set_speed>(-300)))?		0:motor_a.set_speed;

			motor_b.set_speed = -motor_a.set_speed;
			
			if(!KEY_4)
			{
				motor_a.set_speed = 0;
				motor_b.set_speed = 0;
				while(1);
			}			
		}
		else if (time_flag_5ms > 1)//5ms没计算完
		{
			motor_a.set_speed = 0;
			motor_b.set_speed = 0;
			while(1);
		}

		{
			float iua = (3300.0/4096)*(ADC2ConvertedValue[1] - ADC2ConvertedValue[0]);
			float iwa = (3300.0/4096)*(ADC2ConvertedValue[0] - ADC2ConvertedValue[2]);
			
//			OutData[0] = (s16)iua;
//			OutData[1] = (s16)iwa;
//			OutPutData();
			
//			OutData[0] = (s16)motor_a.current_section;
//			OutData[1] = (s16)motor_a.set_speed;
//			OutData[2] = (s16)motor_a.current_speed;
//			OutData[3] = (s16)motor_a.electrical_angle;
//			OutPutData();
			
//			OutData[0] = (s16)motor_b.current_section;
//			OutData[1] = (s16)motor_b.set_speed;
//			OutData[2] = (s16)motor_b.current_speed;
//			OutData[3] = (s16)motor_b.electrical_angle;
//			OutPutData();
			
			OutData[0] = (s16)motor_a.set_speed ;
			OutData[1] = (s16)motor_a.current_speed ;
			OutData[2] = (s16)motor_b.set_speed;
			OutData[3] = (s16)motor_b.current_speed;
			OutPutData();
		}		
	}
}
