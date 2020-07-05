#include "timer.h"
#include "interrupt.h"
#include "uart.h"

volatile unsigned int time_val = 1000;

void time_function(void);

/*  功能：
 *  启动某个定时器，如启动定时器0，则调用start_time(0);
 */
void start_timer(unsigned long num)
{
	if(num < 2)
	{
		var_int(TCON) |= 1 << (num * 8);
	}
	else
	{
		var_int(TCON) |= 1 << ((num + 1 )* 4);
	}
}

/*  功能：
 *  关闭某个定时器，如关闭定时器3，则调用start_time(3);
 */
void stop_timer(unsigned long num)
{
	if(num < 2)
	{
		var_int(TCON) &= ~(1<< (num * 8));
	}
	else
	{
		var_int(TCON) &= ~(1<< ((num + 1 )* 4));
	}
	
}

/*  功能：
 *  复位并关闭定时器0
 */
void reset_time_val(void)
{
	time_val = 0;
	stop_timer(0);
}


/* 功能：
 * 关闭所有PWM定时器 
 */
void stop_all_timer(void)
{
	var_int(TCON) = 0;
}

/*************************************************************************
*  函数名称：timer_init
*  功能说明：初始化定时器
*  参数说明：timer_num   定时器编号(0 -- 4)
*            S    		 定时时间（单位秒）
*  函数返回：无
*  修改时间：2014-6-22   已测试
*  备    注：无
*************************************************************************/
void timer_init(TIMEx timer_num, float S)
{
	unsigned long temp0;

	temp0 = var_int(TCFG0); 		 																			//设置预分频系数为66
		
	if((timer_num < 2) && (timer_num >= 0))     																// 一级预分频参数的设置（timer0,1）
	{
		temp0 = (temp0 & (~0xff)) | (64 << 0);
		var_int(TCFG0) = temp0;
	}
	else if((timer_num < 5) && (timer_num >= 2))  																// 一级预分频参数的设置(timer2,3,4)
	{
		temp0 = (temp0 & (~(0xff << 8))) | (64 << 8);
		var_int(TCFG0) = temp0;
	}

	var_int(TCFG1) = (var_int(TCFG1) & (~(0xf << 4 * timer_num)) & (~(1<<20))) | (4 << 4 * timer_num) ; 		// 二级预分频，这里是16分频

	switch(timer_num)  		// 1s = 62500hz，即每秒计数器计数62500次
	{
		case 0:
				var_int(TCNTB0) = (unsigned int)(62500 * S);
				var_int(TCMPB0) = 0;
				break;

		case 1:
				var_int(TCNTB1) = (unsigned int)(62500 * S);
				var_int(TCMPB1) = 0;
				break;

		case 2:
				var_int(TCNTB2) = (unsigned int)(62500 * S);
				var_int(TCMPB2) = 0;
				break;

		case 3:
				var_int(TCNTB3) = (unsigned int)(62500 * S);
				var_int(TCMPB3) = 0;
				break;

		case 4:
				var_int(TCNTB4) = (unsigned int)(62500 * S);
				break;
		default:
				break;
	}
	
	if(timer_num < 2)																					//第一次必须手动更新
	{
		var_int(TCON) |= (1 << (timer_num * 8 + 1));
		var_int(TCON) &= ~(1 << (timer_num * 8 + 1));													//紧接着要清除手动更新位
		var_int(TCON) |= ( 1 << ( 3 + (timer_num*8)) );  												// 自动加载(1 << ( 0 + (timer_num*8) )) | 
	}
	else
	{
		var_int(TCON) |= 1 << ((timer_num + 1 )* 4 + 1);
		var_int(TCON) &= ~(1 << ((timer_num + 1 )* 4 + 1));
		var_int(TCON) |=  (1 << ((timer_num + 1 )* 4 + 3));												//(1 << ((timer_num + 1 )* 4)) |
	}	      

	var_int(VIC0VECTADDR21 + timer_num * 4) = (unsigned int)time_function;								//设置中断函数地址

	var_int(TINT_CSTAT) = (var_int(TINT_CSTAT) & (~(1 << timer_num))) | (1 << timer_num);  				// 使能timer中断
	
	var_int(VIC0INTSELECT) &= ~(0x01 << (21 + timer_num));   											//将定时器中断配置成IRQ
	var_int(VIC0INTENABLE) |= (0x01 << (21 + timer_num));       										//打开定时器中断

	enable_interrupts();
}

/*中断服务函数*/
void time_function(void)
{
	var_int(TINT_CSTAT) = var_int(TINT_CSTAT);			//清楚中断状态

	var_int(VIC0ADDRESS) = 0;							//清中断向量

	if(time_val)
		time_val--;
}

void delay_ms(unsigned int ms)
{
	time_val = ms;

	start_timer(0);

	while(time_val);

	stop_timer(0);
}