#ifndef _TIMER_H_
#define _TIMER_H_

/*定时器类型*/
typedef enum TIMEx
{
	TIME0,
	TIME1,
	TIME2,
	TIME3,
	TIME4,
}TIMEx;

/* 函数声明 */
void start_timer(unsigned long );
void stop_timer(unsigned long );
void stop_all_timer(void);
void reset_time_val(void);
void timer_init(TIMEx timer_num, float S);
void delay_ms(unsigned int ms);

#endif //_TIME_H_