#include "lmosem.h"

private void krnl_thread_init(thread_t *thd)
{
	spin_lock_init(&thd->thd_lock);
	init_list_head(&thd->thd_list);
	thd->thd_flag = 0;
	thd->thd_stus = TDSTUS_NEW;
	thd->cpu_id = hal_rtn_cpuid();
	thd->thd_id = (uint_t)thd;
	thd->thd_ticks = 0;
	thd->sleep_ticks = 0;
	thd->thd_priority = TD_PRIO_MAX;
	thd->thd_runmode = 0;
	thd->thd_krnlstackstart = NULL;
	thd->thd_krnlstackend = NULL;
	thd->thd_usrstackstart = NULL;
	thd->thd_usrlstackend = NULL;
	thd->thd_contexs.ctx_usrsp = 0;
	thd->thd_contexs.ctx_svcsp = 0;
	thd->thd_contexs.ctx_svcspsr = 0;
	thd->thd_contexs.ctx_cpsr = 0;
	thd->thd_contexs.ctx_lr = 0;
	for(uint_t i = 0; i < TD_RES_MAX; i++)
		thd->thd_resources[i] = NULL;
}

thread_t *krnl_new_thread_dscp()
{
	thread_t *rtn_thd;

	rtn_thd = (thread_t *)krnl_alloc(sizeof(thread_t));

	if(rtn_thd == NULL)
		return NULL;

	krnl_thread_init(rtn_thd);

	return rtn_thd;
}

void krnl_new_thread_stack_init(thread_t *thd, void *func, reg_t cpsr, reg_t spsr)
{
	/*栈对齐*/
	thd->thd_krnlstackstart &= ~(0x0f);
	thd->thd_usrstackstart &= ~(0x0f);

	armreg_t *reg = (armreg_t *)(thd->thd_krnlstackstart - sizeof(armreg_t));

	/*新建任务时上下文为空*/
	reg->r0 = 0;
	reg->r1 = 0;
	reg->r2 = 0;
	reg->r3 = 0;
	reg->r4 = 0;
	reg->r5 = 0;
	reg->r6 = 0;
	reg->r7 = 0;
	reg->r8 = 0;
	reg->r9 = 0;
	reg->r10 = 0;
	reg->r11 = 0;
	reg->r12 = 0;
	reg->sp = (reg_t)(thd->thd_usrstackstart);
	reg->lr = (reg_t)func;

	thd->thd_contexs.ctx_usrsp = (reg_t)(thd->thd_usrstackstart);
	thd->thd_contexs.ctx_svcsp = (reg_t)reg;
	thd->thd_contexs.ctx_svcspsr = spsr;
	thd->thd_contexs.ctx_cpsr = cpsr;
	thd->thd_contexs.ctx_lr = (reg_t)func;
}

/*
* krnl_run_thread() - 根据进程中thd->thd_contexs.ctx_svcspsr, thd->thd_contexs.ctx_svcsp恢复上下文
*/
void krnl_run_thread(thread_t *thd)
{
	thd->thd_stus = TDSTUS_RUN;

	__asm__ __volatile__(
		"msr spsr, %1 \n"
		"mov sp, %0 \n"
		"mov lr, %2 \n"
		"ldmia sp, {r0-lr}^ \n"					/*^表示弹出到用户模式的寄存器中对svc下sp无影响*/
		"add sp, sp, #60 \n"					/*恢复内核堆栈*/
		"movs pc, lr \n"
		:
		:"r" (thd->thd_contexs.ctx_svcsp), 
		 "r" (thd->thd_contexs.ctx_svcspsr), "r" (thd->thd_contexs.ctx_lr)
		:"cc", "memory"
	);
}

thread_t *krnl_new_thread(void *func, uint_t prio, uint_t flag, size_t usr_stack_size, size_t krnl_stack_size)
{
	adr_t usr_stack_addr = 0, krnl_stack_addr = 0;
	bool_t bl;
	thread_t *rtn_thd;

	/*检查阐述合法性*/
	if(func == NULL)
		return NULL;

	if(prio >= TD_PRIO_MAX)
	{
		printk("priority must between %d and %d\n", TD_PRIO_MIN, TD_PRIO_MAX);
		return NULL;
	}

	if(usr_stack_size > TD_FAULT_USRSTACK && krnl_stack_size > TD_FAULT_KRNLSTACK)
	{
		usr_stack_size = TD_FAULT_USRSTACK;
		krnl_stack_size = TD_FAULT_KRNLSTACK;
	}

	/*内核态进程不需要用户堆栈*/
	if(flag == TD_FLAG_USR)
	{
		usr_stack_addr = (adr_t)krnl_alloc(usr_stack_size);

		if(usr_stack_addr == NULL)
			return NULL;
	}
	else
		usr_stack_addr = 0;

	krnl_stack_addr = (adr_t)krnl_alloc(krnl_stack_size);
	
	if(krnl_stack_addr == NULL)
		return NULL;

	rtn_thd = krnl_new_thread_dscp();

	if(rtn_thd == NULL)
	{
		if(flag == TD_FLAG_USR)
		{
			bl = krnl_free(usr_stack_addr, usr_stack_size);
			if(bl == FALSE)
				system_error("memory free failed in function create_cpuidle_thread of krnlcpuidle.c");
		}

		bl = krnl_free(krnl_stack_addr, krnl_stack_size);
		
		if(bl == FALSE)
			system_error("memory free failed in function create_cpuidle_thread of krnlcpuidle.c");
		return NULL;
	}

	/*设置优先级*/
	rtn_thd->thd_priority = prio;

	/*设置内核标记*/
	rtn_thd->thd_flag = flag;

	/*设置用户态，内核态堆栈*/
	rtn_thd->thd_krnlstackstart = krnl_stack_addr + krnl_stack_size - 1;
	rtn_thd->thd_krnlstackend = krnl_stack_addr;

	if(flag == TD_FLAG_USR)
	{
		rtn_thd->thd_usrstackstart = usr_stack_addr + usr_stack_size - 1;
		rtn_thd->thd_usrlstackend = usr_stack_addr;
	}


	/*初始化堆栈，由DEFAULT_USR_SPSR切换到用户模式，并使用用户模式的堆栈*/
	if(flag == TD_FLAG_USR)
		krnl_new_thread_stack_init(rtn_thd, func, DEFAULT_CPSR, DEFAULT_USR_SPSR);
	else
		krnl_new_thread_stack_init(rtn_thd, func, DEFAULT_CPSR, DEFAULT_CPUIDLE_SPSR);
	
	/*将进程按优先级添加到调度表*/
	krnl_thdsched_tlb_add_thread(rtn_thd);

	return rtn_thd;
}


void krnl_inc_thread_tick(thread_t *thd)
{
	cpuflg_t flag;

	spin_lock_save(&thd->thd_lock, flag);

	thd->thd_ticks++;

	/*时间片用完，执行调度*/
	if(thd->thd_ticks > TDRUN_TICK)
	{
		thd->thd_ticks = 0;
		krnl_set_schedflag();
	}
	spin_unlock_restore(&thd->thd_lock, flag);
}

thread_t *krnl_rtn_curtthread(void)
{
	uint_t cpuid = hal_rtn_cpuid();

	thdsched_t *thdsch = &thdsched_tlb.thschtlb_tlb[cpuid];

	if(thdsch->schd_curthread == NULL)
	{
		system_error("no activite thread to run!\n");
	}

	return thdsch->schd_curthread;
}