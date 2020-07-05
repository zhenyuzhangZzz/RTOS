#include "interrupt.h"

/* enable IRQ interrupts */
void enable_interrupts(void)
{
	unsigned long temp;
	__asm__ __volatile__(
		"mrs %0, cpsr\n" 
		"bic %0, %0, #0x80\n" 
		"msr cpsr_c, %0"
		:"=r"(temp)
		::"memory");
}

/* enable IRQ interrupts */
void disenable_interrupts(void)
{
	unsigned long temp;
	__asm__ __volatile__(
		"mrs %0, cpsr\n" 
		"orr %0, %0, #0x80\n" 
		"msr cpsr_c, %0"
		:"=r"(temp)
		::"memory");
}


void exit10_init(irq_handle_p irq_handle)
{
	enable_interrupts();
	
	var_int(GPH1CON) |= 0x0F << 8;							//GPH1_2设置为外部中断模式

	var_int(VIC0INTENCLEAR) = 0xffffffff;       			//禁止所有中断
	
	var_int(VIC0INTSELECT) &= ~(0x01 << 10);				//将外部中断10设为IRQ模式
	
	var_int(VIC0ADDRESS) = 0;                   			//清除需要处理的中断的中断处理函数的地址 
		
	var_int(EXT_INT_1_CON) &= ~(0x0F << 8);					//把外部中断设为高电平触发
	var_int(EXT_INT_1_CON) |= 0x01 << 8;
		
	var_int(EXT_INT_1_MASK) &= ~(0x01 << 2);				//清除外部中断的屏蔽，使能中断
	
	var_int(VIC0VECTPRIORITY10) = 0x00;						//网卡最高优先级
	var_int(VIC0VECTADDR10) = (unsigned int)irq_handle;		//设置中断服务程序地址

	var_int(VIC0INTENABLE)  |= 0x01 << 10; 					//使能中断控制器
}

void irq_handler(void)
{
	void (*isr_p)(void)  = (void (*)(void))var_int(VIC0ADDRESS);	//取出中断服务程序地址
	(*isr_p)();											//跳转真正的中断服务程序函数去
}