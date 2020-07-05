#define KRNL_GLOBAL_VAR
#include "lmosem.h"

KRNL_GLOBAL_VARIABLE(krnl_mplmm_t, mplmm);											/*内存池管理*/
KRNL_GLOBAL_VARIABLE(device_t *, chrdevs)[CHRDEV_MAJOR_HASH_SIZE];					/*设备表*/	
KRNL_GLOBAL_VARIABLE(struct list_head, drvlist);									/*驱动表*/
KRNL_GLOBAL_VARIABLE(drvfunc_t, drventrytlb)[] = {									/*驱动入口函数*/
	systick_drv_init , NULL
};
//uart2_drv_init
KRNL_GLOBAL_VARIABLE(thdsched_tlb_t, thdsched_tlb);									/*进程调度表*/	


KRNL_GLOBAL_VARIABLE(waitlist_t, sleep_list);										/*延时队列*/
					