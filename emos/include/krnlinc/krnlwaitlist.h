#ifndef __KRNLWAITLIST_H__
#define __KRNLWAITLIST_H__

#define __WAIT_LIST_INIT(name) {.wl_lock = __SPIN_LOCK_UNLOCKED() \
						   ,.wl_count = 0 \
						   ,.wl_list = LIST_HEAD_INIT(name.wl_list)}
/*
* 等待队列
*/
typedef struct waitlist{
	spinlock_t 	wl_lock;				/*等待队列spinlock*/
	uint_t	   	wl_count;				/*等待队列计数*/
	struct list_head	wl_list;		/*等待队列链表*/
}waitlist_t;

void krnl_waitlist_init(waitlist_t *wait_list);
void krnl_waitlist_wait(waitlist_t *wait_list, uint_t ms);
void krnl_waitlist_up(waitlist_t *wait_list);
void krnl_waitlist_upall(waitlist_t *wait_list);

#endif

