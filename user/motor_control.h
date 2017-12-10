#ifndef		_MOTOR_CONTROL_H
#define		_MOTOR_CONTROL_H

#include "stm32f4xx.h"

typedef enum 
{
	INIT = 0,
	START,
	SQUARE,
	SINE,
	END
} e_drive_mode;

typedef struct
{
	s16  i_offset; 	//电流检测直流量。 
	s16  ia;		//相电流 单位 mA
	s16  ib;
	s16  ic;
	u32  i_sum_50ms;//电流和

	u16  electrical_angle;			//转子电角度  单位0.1度， 取值范： [0 , 3600)	
	
	s32  set_position;		//期望位置，霍尔计数值，
	s32	 current_position;  //当前位置，霍尔计数值
	
	s16  set_speed;			//期望转速 0--3000 转每分钟
	s16  current_speed;		//当前转速， 单位转每分钟
	u32  period; //电机连续转一圈的时间，单位us
	
	u16  counter_stop;
	 
	e_drive_mode  drive_mode;//电机驱动方式
	
	u8 	is_section_chenged; //==1 表示转子象限改变了，用于转子电角度计算
	u8  current_section; // {0 1 2 3 4 5}	//当前，转子所在的象限
	u8  last_section; // {0 1 2 3 4 5}	    //最近一次变化前，转子所在的象限
	s8  last_direction; 	//电机转向  0:stop   1:forward       -1:bzckward
	
	s16  s16_test; // for test
	
}s_motor_info;


struct s_remote_info
{	
	s16 set_speed;
	s16 set_speed_difference;
};


extern s_motor_info  motor_a;
extern s_motor_info  motor_b;


void motor_init(char a_or_b);//电机参数初始化

#endif
