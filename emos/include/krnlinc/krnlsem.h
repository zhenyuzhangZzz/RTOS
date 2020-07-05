#ifndef __KRNLSEM_H__
#define __KRNLSEM_H__

#define DEFINE_SEM(sem, count) \
	static sem_t sem = {.sem_lock = __SPIN_LOCK_UNLOCKED() \
					   ,.sem_count = count \
					   ,.sem_list = __WAIT_LIST_INIT(sem.sem_list) }

/*
*信号量
*/
typedef struct sem{
	spinlock_t sem_lock;				/*自旋锁*/
	sint_t	sem_count;					/*信号量计数*/
	waitlist_t sem_list;				/*信号量等待队列*/
}sem_t;

void krnl_sem_init(sem_t *sem, sint_t count);
void krnl_print_sem(sem_t *sem);
void krnl_sem_down(sem_t *sem);
void krnl_sem_up(sem_t *sem);

#endif