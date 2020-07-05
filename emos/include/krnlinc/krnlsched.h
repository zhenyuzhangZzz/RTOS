#ifndef __KRNLSCHED_H__
#define __KRNLSCHED_H__

#define KRNL_PREEMPT_ENABLE			0
#define KRNL_PREEMPT_DISENABLE		1

#define CPU_NO_NEED_SCHED			0
#define CPU_NEED_SCHED				1

/*进程队列*/
typedef struct thd_list
{
	struct list_head thdl_list;					/*挂载进程的链表*/
	uint_t thdl_cnt;							/*链表下进程计数*/
}thd_list_t;

/*调度相关的数据结构*/
typedef struct thdsched
{
	spinlock_t schd_lock;
	uint_t schd_cpuid;							/*当前cpuid*/
	uint_t schd_flag;							/*调度标记*/
	uint_t schd_preempt;						/*是否可抢占*/
	uint_t schd_threadnr;						/*当前cpu下的线程总数*/
	uint_t schd_dyncprority;					/*当前的动态优先级*/
	bitmap_t schd_priobitmap;					/*优先级位图*/
	thread_t *schd_cpuidle;						/*当前cpu的空转进程*/
	thread_t *schd_curthread;					/*当前cpu正在运行的进程*/
	thd_list_t schd_thdlist[TD_PRIO_MAX];		/*进程挂载表*/
}thdsched_t;

typedef struct thdsched_tlb
{
	spinlock_t thschtlb_lock;
	uint_t 	   thschtlb_cpunr;					/*cpu总数*/
	uint_t 	   thschtlb_thdnr;					/*线程总数*/
	thdsched_t thschtlb_tlb[CPU_NUM];			/*调度表*/
}thdsched_tlb_t;


void krnl_sched_init(void);
void krnl_thdsched_tlb_add_thread(thread_t *thd);
void krnl_thread_sched(void);
void krnl_set_schedflag(void);
void krnl_try_sched(void);

void krnl_sched_wait(waitlist_t *wlst, uint_t ms);
void krnl_sched_up(waitlist_t *wlst);

#endif