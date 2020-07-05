#include "lmosem.h"

LKHEAD_T void hal_init(void)
{
	/*1、初始化平台*/
	platform_init();

	/*2、串口初始化*/
	hal_uart_init();

	printk("emos kernel hardware init start!\n");

	/*3、单板信息描述符初始化*/
	hal_board_dscp_init();

	printk("data structure of memory management init finished!\n");
	/*4、内存管理初始化*/
	hal_mm_init();
	
	/*5、中断管理初始化*/
	hal_interrupt_init();

	printk("data structure of interrupt management modle init finished!\n");
}