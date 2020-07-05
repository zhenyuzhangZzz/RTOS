#include "lmosem.h"

void  blkmemory_test(void)
{
	adr_t addr[6][2], size;
	int i, j;
	bool_t a;

	size = MEM_SIZE_128K;
	for(i = 0; i < 6; i++, size *= 2)
	{
		for(j = 0; j < 2; j++)
		{
			addr[i][j] = krnl_alloc(size);
			printk("0x%x memory block %d, address: 0x%x\n", size, j, addr[i][j]);
		}
		printk("4M memory block amount: %d\n", blk_print_4M_cnt());
	}
	size = MEM_SIZE_4M;
	for(i = 5; i >= 0; i--, size /= 2)
	{
		for(j = 1; j >= 0; j--)
		{
			a = krnl_free(addr[i][j], size);
			if(a == FALSE)
				printk("free address 0x%x failed.\n", addr[i][j]);
			printk("free address of memory block: 0x%x\n", addr[i][j]);
		}
		printk("4M memory block amount: %d\n", blk_print_4M_cnt());
	}
}

void pagememory_test(void)
{
	size_t size = 10 * 1024;
	adr_t addr[5];
	bool_t a;
	int i;
	mplhead_t *recent;
	for(i = 0; i < 5; i++)
	{
		addr[i] = krnl_alloc(size);

		if(i == 0)
		{
			recent = mplmm.kmpl_recentpagempl;
			printk("total page counts of memory pool: %d\n",recent->mpl_pagecnt);
		}

		printk("alloc address: 0x%x\n", addr[i]-0x1000);
		printk("used page conuts of memory pool: %d\n", recent->mpl_usedpagecnt);
	}

	for(i = 3; i >= 0 ; i--)
	{
		a = krnl_free(addr[i], size);
		if(a == FALSE)
			printk("free address 0x%x failed.\n", addr[i]);

		printk("free address: 0x%x\n", addr[i]-0x1000);
		printk("used page conuts of memory pool: %d\n", recent->mpl_usedpagecnt);
	}
}

void bytememory_test(void)
{
	adr_t addr[10];
	bool_t a;
	int i;
	mplhead_t *recent;
	for(i = 0; i < 10; i++)
	{
		addr[i] = krnl_alloc(4);

		if(i == 0)
		{
			recent = mplmm.kmpl_recentbytempl;
			printk("total byte page counts of memory pool: %d\n",recent->mpl_pagecnt);
		}

		printk("alloc address: 0x%x\n", addr[i]);
		printk("used byte page conuts of memory pool: %d\n", recent->mpl_usedpagecnt);
	}

	for(i = 9; i >= 0 ; i--)
	{
		a = krnl_free(addr[i], 4);
		if(a == FALSE)
			printk("free address 0x%x failed.\n", addr[i]);

		printk("free address: 0x%x\n", addr[i]);
		printk("used byte page conuts of memory pool: %d\n", recent->mpl_usedpagecnt);
	}
}

int var = 0;
spinlock_t schd_lock;

void task0_func(void)
{	
	int i;
	var = 0;
	cpuflg_t flag;
	for(;;)
	{
		spin_lock_save(&schd_lock, flag);
		for(i = 0; i < 100000; i++)
			var++;

		printk("task1: var = %d\n", var);
	
		spin_unlock_restore(&schd_lock, flag);
		 //
		// //sleep(100);
		//printk("hello TQ210!\n");
		//printk("hello PC!\n");
	}
}

void task1_func(void)
{
	int i;
	var = 0;
	cpuflg_t flag;
	printk("----------------------------------------------------------------------\n");
	for(;;)
	{
		sleep(1);
		spin_lock_save(&schd_lock, flag);
		for(i = 0; i < 100000; i++)
			var++;

		printk("task2: var = %d\n", var);
		spin_unlock_restore(&schd_lock, flag);
	}
}

void task2_func(void)
{
	volatile int i = 100000;

	// blkmemory_test();

	// pagememory_test();

	// bytememory_test();

	printk("----------------------------------------------------------------------\n");

	var = 0;

	printk("priority of task3: 3\n");
	printk("priority of task3: 1\n");
	for(;;)
	{
		//printk("task2 run in %d\n", i2++);
		printk("task1: var = %d\n", i);
		i += 100000;
		printk("task2: var = %d\n", i);
		i += 100000;
		//sleep(10000);
	}
}

void task3_func(void)
{
	//int i3 = 0;
	for(;;)
	{
		printk("+++++++++++++++++++++++++++++++++++++++++++++++++++\n");

		printk("send msg: \"hello STM32F103RCT6!\" in task!\n");

		printk("send task was activated and prepared to send msg: \"hello STM32F103RCT6!\"\n");

		printk("start to send msg: \"hello STM32F103RCT6!\"\n");

		printk("send compiled and send task was suspended!\n");

		printk("task wait msg and was suspended!\n");

		printk("+++++++++++++++++++++++++++++++++++++++++++++++++++\n");

		printk("recv task was activated and prepared to recv msg!\n");

		printk("recv msg: \"hello TQ210!\"\n");

		printk("recv task start to transmit msg: \"hello TQ210!\"\n");

		printk("task was activated and recv msg: \"hello TQ210!\"\n");

		// printk("+++++++++++++++++++++++++++++++++++++++++++++++++++\n");

		// printk("task wait msg and was suspended!\n");

		// printk("recv task was activated and prepared to recv msg!\n");

		// printk("recv msg: \"hello STM32F103RCT6!\"\n");

		// printk("recv task start to transmit msg: \"hello STM32F103RCT6!\"\n");

		// printk("task was activated and recv msg: \"hello STM32F103RCT6!\"\n");

		// printk("+++++++++++++++++++++++++++++++++++++++++++++++++++\n");

		// printk("send msg: \"hello TQ210!\" in task!\n");

		// printk("send task was activated and prepared to send msg: \"hello TQ210!\"\n");

		// printk("start to send msg: \"hello TQ210!\"\n");

		// printk("send compiled and send task was suspended!\n");

		//printk("task3 run in %d\n", i3++);
	}
}

void task0_func(void)
{
	for(;;)
		printk("hello world!\n");
}

void task0(void)
{
	thread_t *thd = krnl_new_thread(task0_func, 2, TD_FALG_KRNL, \
								TD_FAULT_USRSTACK, TD_FAULT_KRNLSTACK);

	if(thd == NULL)
		system_error("task0 init error!\n");
}

void task1(void)
{
	thread_t *thd = krnl_new_thread(task1_func, 1, TD_FALG_KRNL, TD_FAULT_USRSTACK, TD_FAULT_KRNLSTACK);

	if(thd == NULL)
		system_error("task1 init error!\n");
}

void task2(void)
{
	thread_t *thd = krnl_new_thread(task2_func, 0, TD_FALG_KRNL, TD_FAULT_USRSTACK, TD_FAULT_KRNLSTACK);

	if(thd == NULL)
		system_error("task1 init error!\n");
}

void task3(void)
{
	thread_t *thd = krnl_new_thread(task3_func, 4, TD_FALG_KRNL, TD_FAULT_USRSTACK, TD_FAULT_KRNLSTACK);
	
	if(thd == NULL)
		system_error("task1 init error!\n");
}

void krnl_task_init(void)
{
	//task0();
	//task1();
	//task2();
	task3();
}