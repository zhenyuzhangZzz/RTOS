#include "lmosem.h"

void krnl_init(void)
{
	printk("run in kernel level!\n");

	/*页级和字级内存管理初始化*/
	krnl_mm_init();

	interrupt_test();
	
	printk("kernel start init driver model!\n");
	/*内核驱动核心初始化*/
	driver_system_init();

	printk("kernel start to load device driver.\n");
	/*加载驱动程序*/
	driver_load();
	
	printk("all device:\n");
	/*列举出内核所有的驱动*/
	driver_list_all();

	printk("kernel scheduler init finished!\n");
	/*内核调度初始化*/
	krnl_sched_init();

	/*内核休眠队列初始化*/
	krnl_waitlist_init(&sleep_list);
	
	krnl_task_init();

	krnl_cpuidle_init();
	
	system_error("run in krnl level!\n");
}