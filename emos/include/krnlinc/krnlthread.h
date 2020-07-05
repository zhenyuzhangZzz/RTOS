#ifndef __KRNLTHREAD_H__
#define __KRNLTHREAD_H__

/*进程状态*/
#define TDSTUS_NEW		0
#define TDSTUS_RUN		1
#define TDSTUS_SLEEP 	2
#define TDSTUS_SUSPEND 	3
#define TDSTUS_READY	4
#define TDSTUS_DEAD		5

/*进程ticks*/
#define TDRUN_TICK		20

/*进程资源最大数*/
#define TD_RES_MAX		8

/*优先级最大值*/
#define TD_PRIO_MAX		64
#define TD_PRIO_MIN		0

/*进程标记*/
#define TD_FALG_KRNL	0
#define TD_FLAG_USR		1

#define TD_FAULT_USRSTACK		0x4000
#define TD_FAULT_KRNLSTACK		0x4000

#define DEFAULT_CPSR			0xd3		/*管理模式关中断*/
#define DEFAULT_CPUIDLE_SPSR	0x13		/*管理模式开中断*/
#define DEFAULT_USR_SPSR		0x10		/*用户模式开中断*/


/*进程上下文*/
typedef struct thd_contex
{
	reg_t ctx_usrsp;
	reg_t ctx_svcsp;
	reg_t ctx_svcspsr;	
	reg_t ctx_cpsr;						/*当前模式的cpsr*/
	reg_t ctx_lr;						/*内核模式下的lr*/
}thd_contex_t;

/*进程描述符*/
typedef struct thread
{
	spinlock_t thd_lock;
	struct list_head thd_list;
	uint_t thd_flag;					/*标记*/		
	uint_t thd_stus;					/*状态*/
	uint_t cpu_id;						/*cpuid*/
	uint_t thd_id;						/*进程id*/
	uint_t thd_ticks;					/*运行时间片*/
	uint_t sleep_ticks;					/*休眠时间片*/
	uint_t thd_priority;				/*优先级*/
	uint_t thd_runmode;					/*运行模式*/
	adr_t  thd_krnlstackstart;			/*内核栈起始地址*/
	adr_t  thd_krnlstackend;			/*内核栈结束地址*/
	adr_t  thd_usrstackstart;			/*用户栈起始地址*/
	adr_t  thd_usrlstackend;			/*用户栈结束地址*/
	thd_contex_t thd_contexs;			/*进程上下文*/
	void *thd_resources[TD_RES_MAX];	/*进程管理的资源*/
}thread_t;

/*新建一个用户进程*/
thread_t *krnl_new_thread(void *func, uint_t prio, uint_t flag, size_t usr_stack_size, size_t krnl_stack_size);
void krnl_inc_thread_tick(thread_t *thd);
thread_t *krnl_rtn_curtthread(void);
void krnl_run_thread(thread_t *thd);

#endif