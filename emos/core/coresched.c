#include "lmosem.h"

private void thd_list_init(thd_list_t *ptr)
{
	init_list_head(&ptr->thdl_list);
	ptr->thdl_cnt = 0;
}

private void thdsched_init(thdsched_t *ptr)
{
	spin_lock_init(&ptr->schd_lock);
	ptr->schd_cpuid = hal_rtn_cpuid();
	ptr->schd_flag = CPU_NO_NEED_SCHED;
	ptr->schd_preempt = KRNL_PREEMPT_DISENABLE;
	ptr->schd_threadnr = 0;
	ptr->schd_dyncprority = 0;
	bitmap_init(&ptr->schd_priobitmap);
	ptr->schd_cpuidle = NULL;
	ptr->schd_curthread = NULL;
	for(uint_t i = 0; i < TD_PRIO_MAX; i++)
	{
		thd_list_init(&ptr->schd_thdlist[i]);
	}
}

private void thdsched_tlb_init(thdsched_tlb_t *ptr)
{
	spin_lock_init(&ptr->thschtlb_lock);
	ptr->thschtlb_cpunr = CPU_NUM;
	ptr->thschtlb_thdnr = 0;
	for(uint_t i = 0; i < CPU_NUM; i++)
	{
		thdsched_init(&ptr->thschtlb_tlb[i]);
	}
}

/*
* krnl_find_thread() - 寻找一个处于就绪态的进程并发返回
*/
private thread_t *krnl_rtn_cpuidle(void)
{
	cpuflg_t flag;
	thread_t *rnt_thd;

	uint_t cpuid = hal_rtn_cpuid();

	thdsched_t *thdsch =  &thdsched_tlb.thschtlb_tlb[cpuid];

	spin_lock_save(&thdsch->schd_lock, flag);

	rnt_thd = thdsch->schd_cpuidle;

	spin_unlock_restore(&thdsch->schd_lock, flag);

	if(rnt_thd == NULL)
		system_error("cpu idle thread is empty!\n");

	return rnt_thd;
}

/*
* krnl_find_thread() - 返回现在正在运行的进程
*/
private thread_t *krnl_rtn_currentthd(void)
{
	cpuflg_t flag;
	thread_t *rnt_thd;

	uint_t cpuid = hal_rtn_cpuid();

	thdsched_t *thdsch = &thdsched_tlb.thschtlb_tlb[cpuid];

	spin_lock_save(&thdsch->schd_lock, flag);

	rnt_thd = thdsch->schd_curthread;

	if(rnt_thd == NULL)
		system_error("cpu idle thread is empty!\n");

	spin_unlock_restore(&thdsch->schd_lock, flag);

	return rnt_thd;
}


/*
* krnl_find_thread() - 寻找一个处于就绪态的进程并返回
*/
private thread_t *krnl_find_thread(void)
{
	cpuflg_t flag;
	uint_t prio;
	thread_t *rtn_thd;

	uint_t cpuid = hal_rtn_cpuid();

	thdsched_t *thdsch =  &thdsched_tlb.thschtlb_tlb[cpuid];

	spin_lock_save(&thdsch->schd_lock, flag);

	/*获取最高优先级的待运行的进程*/
	prio = bitmap_get_first_set(&thdsch->schd_priobitmap);
	
	//printk("prio: %d\n", prio);
	
	/*没有新建的别的线程*/
	if(prio == bitmap_bit_count())
	{
		rtn_thd = krnl_rtn_cpuidle();

		goto out;
	}

	/*检查参数合法性，如果位图设置了但无数据系统死机*/
	if(thdsch->schd_thdlist[prio].thdl_cnt == 0)
		system_error("kernel error in line %d of %s\n", __LINE__, __FILE__);

	if(list_empty_careful(&thdsch->schd_thdlist[prio].thdl_list) == TRUE)
		system_error("kernel error in line %d of %s\n", __LINE__, __FILE__);

	/*取出队首结构*/
	rtn_thd = list_entry(thdsch->schd_thdlist[prio].thdl_list.next, thread_t, thd_list);

	/*从队头删除添加到队尾*/
	list_del(&rtn_thd->thd_list);
	list_add_tail(&rtn_thd->thd_list, &thdsch->schd_thdlist[prio].thdl_list);

out:
	spin_unlock_restore(&thdsch->schd_lock, flag);
	return rtn_thd;
}

void krnl_deal_new_thread(thread_t *thd)
{
	uint_t cpuid = hal_rtn_cpuid();

	thdsched_t *thdsch =  &thdsched_tlb.thschtlb_tlb[cpuid];

	/*设置调度结构中当前运行的进程为thd*/
	thdsch->schd_curthread = thd;

	/*如果是新建的进程*/
	if(thd->thd_stus == TDSTUS_NEW)
	{
		krnl_run_thread(thd);
	}
}
/*
* krnl_save_contex_to_sched() - 保存进程上下文并切换到下一个进程
*/

void krnl_save_contex_to_sched(thread_t *prv, thread_t *next)
{
	cpuflg_t flag;
	u32_t tmp;

	/*关闭中断并保持标记*/
	local_fiqirq_disable(flag);

	/*保存当前模式的寄存器，下面要使用这些寄存器*/
	__asm__ __volatile__("stmfd sp!, {r0-r12, lr}\n":::"memory");

	//printk("sp: %x, cpsr: %x, spsr: %x\n", get_sp(), get_cpsr(), get_spsr());
	__asm__ __volatile__(
		"mrs %0, spsr\n"					/*保存当前进程的spsr*/
		"str %0, [%2]\n"						
		"mrs %0, cpsr\n"					/*保存当前进程的cpsr*/
		"str %0, [%3]\n"
		"str sp, [%1]\n"					/*保存当前进程的sp*/
		"msr spsr, %5\n"					/*恢复下一个进程的spsr*/				
		"msr cpsr, %6\n"					/*恢复下一个进程的cpsr*/	
		"ldr sp, [%4]\n"					/*恢复下一个进程的sp*/
		/*
		* 到这里进程已经切换了，只要函数返回，就切换到了下一个进程，但有一种特殊情况，当进程被新建时，
		* 若它还没有发生切换，它的堆栈里面是没有内核函数的调用路径的，即它不可能运行到这里，如果对于
		* 新建的进程也这样返回的画，就会出错了，因为它的堆栈里没有这个函数返回后需要做的事。
		* 那么对于新建的进程如要切换过去，需要特殊处理一下。
		*/
		"mov r0, %7\n"
		"bl krnl_deal_new_thread\n"					/*如果线程是新的，那么切换过去后该函数就不会发返回了*/							
		: "=&r" (tmp)
		:"r" (&prv->thd_contexs.ctx_svcsp), "r" (&prv->thd_contexs.ctx_svcspsr), "r" (&prv->thd_contexs.ctx_cpsr),
		 "r" (&next->thd_contexs.ctx_svcsp), "r" (next->thd_contexs.ctx_svcspsr), "r" (next->thd_contexs.ctx_cpsr),
		 "r" (next)
		:"cc", "memory"
	);
	//printk("sp: %x, cpsr: %x, spsr: %x\n", get_sp(), get_cpsr(), get_spsr());
	__asm__ __volatile__("ldmfd sp!, {r0-r12, lr}\n":::"memory");
	/*恢复标记*/
	local_fiqirq_restore(flag);
}
/*
* krnl_thread_sched() - 内核进程调度函数
*/
void krnl_thread_sched(void)
{
	/*返回当前进程*/
	thread_t *prv = krnl_rtn_currentthd();

	/*寻找下一个可运行的进程*/
	thread_t *next = krnl_find_thread();
	// printk("prv: %x, next: %x\n", prv, next);
	if(prv != next)
		krnl_save_contex_to_sched(prv, next);
}

void krnl_try_sched(void)
{
	cpuflg_t flag;

	uint_t sched = 0, cpuid = hal_rtn_cpuid();

	thdsched_t *thdsch =  &thdsched_tlb.thschtlb_tlb[cpuid];

	spin_lock_save(&thdsch->schd_lock, flag);

	if(thdsch->schd_flag == CPU_NEED_SCHED && thdsch->schd_preempt == KRNL_PREEMPT_DISENABLE)
	{
		thdsch->schd_flag = CPU_NO_NEED_SCHED;
		sched = 1;
	}

	spin_unlock_restore(&thdsch->schd_lock, flag);

	/*如果标志被设置，表示需要调度了*/
	if(sched == 1)
	{
		krnl_thread_sched();
	}
}

void krnl_sched_init(void)
{
	thdsched_tlb_init(&thdsched_tlb);
}

void krnl_thdsched_tlb_add_thread(thread_t *thd)
{
	cpuflg_t flag;

	uint_t cpuid = hal_rtn_cpuid();

	thdsched_t *thdsch = &thdsched_tlb.thschtlb_tlb[cpuid];

	/*上锁*/
	spin_lock_save(&thdsch->schd_lock, flag);

	/*根据优先级添加到对应的链表上*/
	list_add(&thd->thd_list, &thdsch->schd_thdlist[thd->thd_priority].thdl_list);
	/*进程计数++*/
	thdsch->schd_thdlist[thd->thd_priority].thdl_cnt++;
	/*cpu上总进程数++*/
	thdsch->schd_threadnr++;

	/*设置优先级位图*/
	bitmap_set_bit(&thdsch->schd_priobitmap, thd->thd_priority);

	spin_unlock_restore(&thdsch->schd_lock, flag);

	spin_lock_save(&thdsched_tlb.thschtlb_lock, flag);
	/*内核总进程数++*/
	thdsched_tlb.thschtlb_thdnr++;
	spin_unlock_restore(&thdsched_tlb.thschtlb_lock, flag);
}

void krnl_set_schedflag(void)
{
	cpuflg_t flag;
	uint_t cpuid = hal_rtn_cpuid();

	thdsched_t *thdsch =  &thdsched_tlb.thschtlb_tlb[cpuid];

	spin_lock_save(&thdsch->schd_lock, flag);

	thdsch->schd_flag = CPU_NEED_SCHED;

	spin_unlock_restore(&thdsch->schd_lock, flag);
}

/*
* krnl_sched_wait() - 将进程挂入等待队列
*/
void krnl_sched_wait(waitlist_t *wlst, uint_t ms)
{
	cpuflg_t thdsch_flag, thd_flag, wlst_flag;
	uint_t prio;
	thread_t *thd = krnl_rtn_curtthread();
	uint_t cpuid = hal_rtn_cpuid();

	thdsched_t *thdsch = &thdsched_tlb.thschtlb_tlb[cpuid];

	if(wlst == NULL)
	{
		printk("wait list con't empty!");
		return;
	}

	if(thd->sleep_ticks != 0)
		return;

	spin_lock_save(&thdsch->schd_lock, thdsch_flag);

	spin_lock_save(&thd->thd_lock, thd_flag);

	prio = thd->thd_priority;

	/*将进程从就绪队列删除*/
	list_del(&thd->thd_list);

	thd->sleep_ticks = ms;
	thd->thd_stus = TDSTUS_SUSPEND;

	spin_unlock_restore(&thd->thd_lock, thd_flag);

	thdsch->schd_thdlist[prio].thdl_cnt--;

	// printk("thdl_cnt: %d\n", thdsch->schd_thdlist[prio].thdl_cnt);

	if(thdsch->schd_thdlist[prio].thdl_cnt == 0)
		bitmap_clear_bit(&thdsch->schd_priobitmap, prio);

	spin_lock_save(&wlst->wl_lock, wlst_flag);

	/*挂载到等待队列尾部*/
	list_add_tail(&thd->thd_list, &wlst->wl_list);
	wlst->wl_count++;

	spin_unlock_restore(&wlst->wl_lock, wlst_flag);

	spin_unlock_restore(&thdsch->schd_lock, thdsch_flag);

	/*执行一次调度*/
	//krnl_set_schedflag();
	krnl_thread_sched();
}

/*
* krnl_sched_wait() - 唤醒优先级最高的进程
*/
void krnl_sched_up(waitlist_t *wlst)
{
	cpuflg_t thdsch_flag, thd_flag, wlst_flag;
	uint_t prio;
	thread_t *thd, *tmp_thd;
	struct list_head *list;
	uint_t cpuid = hal_rtn_cpuid();

	thdsched_t *thdsch = &thdsched_tlb.thschtlb_tlb[cpuid];

	if(wlst == NULL)
	{
		printk("wait list con't empty!");
		return;
	}

	spin_lock_save(&thdsch->schd_lock, thdsch_flag);

	spin_lock_save(&wlst->wl_lock, wlst_flag);
	
	if(wlst->wl_count == 0)
		return;

	/*取出首个等待的进程*/
	thd	= list_entry(wlst->wl_list.next, thread_t, thd_list);
	prio = thd->thd_priority;

	list_for_each(list, &(wlst->wl_list))
	{
		tmp_thd	= list_entry(list, thread_t, thd_list);
		if(thd->thd_priority <= prio)
			thd = tmp_thd;
	}

	/*将进程从等待队列删除*/
	list_del(&thd->thd_list);
	wlst->wl_count--;

	spin_unlock_restore(&wlst->wl_lock, wlst_flag);

	spin_lock_save(&thd->thd_lock, thd_flag);

	prio = thd->thd_priority;

	/*将进程添加到就绪队列头部*/
	list_add(&thd->thd_list, &thdsch->schd_thdlist[prio].thdl_list);
	thd->thd_stus = TDSTUS_READY;
	spin_unlock_restore(&thd->thd_lock, thd_flag);

	bitmap_set_bit(&thdsch->schd_priobitmap, prio);

	thdsch->schd_thdlist[prio].thdl_cnt++;

	spin_unlock_restore(&thdsch->schd_lock, thdsch_flag);
}
