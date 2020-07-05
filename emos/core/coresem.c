#include "lmosem.h"

void krnl_sem_init(sem_t *sem, sint_t count)
{
	spin_lock_init(&sem->sem_lock);				
	sem->sem_count = count;
	krnl_waitlist_init(&sem->sem_list);				
}

void krnl_print_sem(sem_t *sem)
{
	printk("lock: %d, count: %d ", sem->sem_lock.rlock, sem->sem_count);
	printk("wl_lock: %d, wl_count: %d, wl_list.prev: %x, wl_list.next: %x, wl_list: %x\n",\
		sem->sem_list.wl_lock.rlock, sem->sem_list.wl_count, sem->sem_list.wl_list.prev, sem->sem_list.wl_list.next, &sem->sem_list.wl_list);
}

void krnl_sem_down(sem_t *sem)
{
	cpuflg_t flag;

check_again:
	spin_lock_save(&sem->sem_lock, flag);

	if(sem->sem_count < 1)
	{
		krnl_waitlist_wait(&sem->sem_list, 0);
		// printk("wait thread nr: %d\n", sem->sem_list.wl_count);
		spin_unlock_restore(&sem->sem_lock, flag);

		/*执行调度*/
		krnl_thread_sched();
		goto check_again;
	}
	sem->sem_count--;
	spin_unlock_restore(&sem->sem_lock, flag);
}

void krnl_sem_up(sem_t *sem)
{
	cpuflg_t flag;

	spin_lock_save(&sem->sem_lock, flag);

	sem->sem_count++;

	/*总是高优先级的任务首先获取资源，而不是等待的时间长短来获取资源*/
	krnl_waitlist_up(&sem->sem_list);
	krnl_set_schedflag();

	spin_unlock_restore(&sem->sem_lock, flag);	
}