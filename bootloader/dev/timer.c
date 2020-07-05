#include "timer.h"
#include "interrupt.h"
#include "uart.h"

volatile unsigned int time_val = 1000;

void time_function(void);

/*  ���ܣ�
 *  ����ĳ����ʱ������������ʱ��0�������start_time(0);
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

/*  ���ܣ�
 *  �ر�ĳ����ʱ������رն�ʱ��3�������start_time(3);
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

/*  ���ܣ�
 *  ��λ���رն�ʱ��0
 */
void reset_time_val(void)
{
	time_val = 0;
	stop_timer(0);
}


/* ���ܣ�
 * �ر�����PWM��ʱ�� 
 */
void stop_all_timer(void)
{
	var_int(TCON) = 0;
}

/*************************************************************************
*  �������ƣ�timer_init
*  ����˵������ʼ����ʱ��
*  ����˵����timer_num   ��ʱ�����(0 -- 4)
*            S    		 ��ʱʱ�䣨��λ�룩
*  �������أ���
*  �޸�ʱ�䣺2014-6-22   �Ѳ���
*  ��    ע����
*************************************************************************/
void timer_init(TIMEx timer_num, float S)
{
	unsigned long temp0;

	temp0 = var_int(TCFG0); 		 																			//����Ԥ��Ƶϵ��Ϊ66
		
	if((timer_num < 2) && (timer_num >= 0))     																// һ��Ԥ��Ƶ���������ã�timer0,1��
	{
		temp0 = (temp0 & (~0xff)) | (64 << 0);
		var_int(TCFG0) = temp0;
	}
	else if((timer_num < 5) && (timer_num >= 2))  																// һ��Ԥ��Ƶ����������(timer2,3,4)
	{
		temp0 = (temp0 & (~(0xff << 8))) | (64 << 8);
		var_int(TCFG0) = temp0;
	}

	var_int(TCFG1) = (var_int(TCFG1) & (~(0xf << 4 * timer_num)) & (~(1<<20))) | (4 << 4 * timer_num) ; 		// ����Ԥ��Ƶ��������16��Ƶ

	switch(timer_num)  		// 1s = 62500hz����ÿ�����������62500��
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
	
	if(timer_num < 2)																					//��һ�α����ֶ�����
	{
		var_int(TCON) |= (1 << (timer_num * 8 + 1));
		var_int(TCON) &= ~(1 << (timer_num * 8 + 1));													//������Ҫ����ֶ�����λ
		var_int(TCON) |= ( 1 << ( 3 + (timer_num*8)) );  												// �Զ�����(1 << ( 0 + (timer_num*8) )) | 
	}
	else
	{
		var_int(TCON) |= 1 << ((timer_num + 1 )* 4 + 1);
		var_int(TCON) &= ~(1 << ((timer_num + 1 )* 4 + 1));
		var_int(TCON) |=  (1 << ((timer_num + 1 )* 4 + 3));												//(1 << ((timer_num + 1 )* 4)) |
	}	      

	var_int(VIC0VECTADDR21 + timer_num * 4) = (unsigned int)time_function;								//�����жϺ�����ַ

	var_int(TINT_CSTAT) = (var_int(TINT_CSTAT) & (~(1 << timer_num))) | (1 << timer_num);  				// ʹ��timer�ж�
	
	var_int(VIC0INTSELECT) &= ~(0x01 << (21 + timer_num));   											//����ʱ���ж����ó�IRQ
	var_int(VIC0INTENABLE) |= (0x01 << (21 + timer_num));       										//�򿪶�ʱ���ж�

	enable_interrupts();
}

/*�жϷ�����*/
void time_function(void)
{
	var_int(TINT_CSTAT) = var_int(TINT_CSTAT);			//����ж�״̬

	var_int(VIC0ADDRESS) = 0;							//���ж�����

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