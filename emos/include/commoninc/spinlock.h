#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

typedef struct spinklock
{
	volatile u32_t rlock;
}spinlock_t;

/*
*对于SMP系统, spinlock需完成如下操作
*1、禁止内核抢占
*2、保存cpsr, 并关闭中断
*3、请求自旋锁 
*对非SMP系统，关闭系统中断即可保证原子性
*/
#ifdef CONFIG_SMP
#define spin_lock_save(lock, flags) 	\
	do{						\
		preempt_disable();	\
		local_fiqirq_disable(flags); \
		raw_spinlock_lock(lock); \
	}while(0)

#define spin_unlock_restore(lock, flags) \
	do{						\
		preempt_enable();	\
		local_fiqirq_enable(flags); \
		raw_spinlock_unlock(lock); \
	}while(0)
#else
#define spin_lock_save(lock, flags) 	\
	do{						\
		local_fiqirq_disable(flags); \
		(void)lock;	\
	}while(0)

#define spin_unlock_restore(lock, flags) \
	do{						\
		local_fiqirq_restore(flags); \
		(void)lock;	\
	}while(0)
#endif

#define DEFINE_SPINLOCK(lock)  \
	spinlock_t lock = {.rlock = 0}

#define __SPIN_LOCK_UNLOCKED() {.rlock = 0}	
	
#endif