#include "lmosem.h"

void krnl_waitlist_init(waitlist_t *wait_list)
{
	spin_lock_init(&wait_list->wl_lock);
	wait_list->wl_count = 0;
	init_list_head(&wait_list->wl_list);
}

void krnl_waitlist_wait(waitlist_t *wait_list, uint_t ms)
{
	krnl_sched_wait(wait_list, ms);
}

void krnl_waitlist_up(waitlist_t *wait_list)
{
	if(list_empty_careful(&wait_list->wl_list) == TRUE)
		return;
	krnl_sched_up(wait_list);
}

void krnl_waitlist_upall(waitlist_t *wait_list)
{
	while(list_empty_careful(&wait_list->wl_list) == FALSE)
		krnl_sched_up(wait_list);
}