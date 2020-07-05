#include "lmosem.h"

void cpuidle_thread(void)
{
	//printk("----------------------------------------------------------------------\n");
	//printk("cpu idle thread are running!\n");
	for(;;)
	{
		//printk("cpu idle thread are running!\n");
	}
}

void krnl_cpuidle_init(void)
{
	cpuflg_t flag;
	thread_t *thd_ptr;

	/*创建内核进程idle*/
	thd_ptr = krnl_new_thread(cpuidle_thread, TD_PRIO_MAX - 1, TD_FALG_KRNL, TD_FAULT_USRSTACK, TD_FAULT_KRNLSTACK);

	if(thd_ptr == NULL)
		system_error("init cpuidle error\n");

	uint_t cpuid = hal_rtn_cpuid();

	thdsched_t *thdsch = &thdsched_tlb.thschtlb_tlb[cpuid];

	/*上锁*/
	spin_lock_save(&thdsch->schd_lock, flag);

	/*设置空闲任务首先运行*/
	thdsch->schd_cpuidle = thd_ptr;
	thdsch->schd_curthread = thd_ptr;

	/*由于krnl_new_thread(), 将空闲进程添加到了队列，这里删除，免得调度时加入到正常的调度中*/
	list_del(&thd_ptr->thd_list);

	/*同时删除位图, 即空闲任务只被schd_cpuidle记录*/
	if(thdsch->schd_thdlist[thd_ptr->thd_priority].thdl_cnt == 1)
	{
		bitmap_clear_bit(&thdsch->schd_priobitmap, thd_ptr->thd_priority);
	}

	thdsch->schd_thdlist[thd_ptr->thd_priority].thdl_cnt--;

	spin_unlock_restore(&thdsch->schd_lock, flag);

	/*运行空闲任务*/
	krnl_run_thread(thd_ptr);
}