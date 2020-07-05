#include "lmosem.h"


/*
	*让进程休眠多少个ms, 实际休眠的时间是一个时间片10ms的整数倍
	*如ms=2，则实际休眠时间为2*10=20
*/
void sleep(uint_t ms)
{
	krnl_sched_wait(&sleep_list, ms);
}

void krnl_deal_sleep(void)
{
	cpuflg_t thdsch_flag, thd_flag, wlst_flag;
	uint_t prio;
	thread_t *thd;
	struct list_head *list;

	waitlist_t *wlst = &sleep_list;

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

	list_for_each(list, &wlst->wl_list)
	{
		/*取出首个等待的进程*/
		thd	= list_entry(list, thread_t, thd_list);

		// printk("wait thd: 0x%x, sleep_ticks: %d\n", thd, thd->sleep_ticks);
		spin_lock_save(&thd->thd_lock, thd_flag);

		/*休眠时间到*/
		if(--thd->sleep_ticks == 0)
		{
			/*将进程从等待队列删除*/
			__list_del_entry(&thd->thd_list);
			wlst->wl_count--;

			prio = thd->thd_priority;

			/*将进程添加到就绪队列头部*/
			list_add(&thd->thd_list, &thdsch->schd_thdlist[prio].thdl_list);
			thd->thd_stus = TDSTUS_READY;

			bitmap_set_bit(&thdsch->schd_priobitmap, prio);

			thdsch->schd_thdlist[prio].thdl_cnt++;
			// printk("------------------\n");
			krnl_set_schedflag();
			/*重新初始化，由于list_add将节点指向了别处*/
			list = wlst->wl_list.next;
		}
		spin_unlock_restore(&thd->thd_lock, thd_flag);
	}

	spin_unlock_restore(&wlst->wl_lock, wlst_flag);

	spin_unlock_restore(&thdsch->schd_lock, thdsch_flag);
}