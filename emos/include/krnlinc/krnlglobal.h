#ifndef __KRNLGLOBAL_H__
#define __KRNLGLOBAL_H__

#ifdef KRNL_GLOBAL_VAR
#undef	KEXTERN
#define KEXTERN 
#endif

#define KRNL_GLOBAL_VARIABLE(type, name) KEXTERN __attribute__((section(".data"))) type name

KRNL_GLOBAL_VARIABLE(krnl_mplmm_t, mplmm);
KRNL_GLOBAL_VARIABLE(device_t *, chrdevs)[CHRDEV_MAJOR_HASH_SIZE];					/*设备表*/	
KRNL_GLOBAL_VARIABLE(struct list_head, drvlist);									/*驱动表*/
KRNL_GLOBAL_VARIABLE(drvfunc_t, drventrytlb)[];										/*驱动程序入口*/
KRNL_GLOBAL_VARIABLE(thdsched_tlb_t, thdsched_tlb);									/*进程调度表*/
KRNL_GLOBAL_VARIABLE(waitlist_t, sleep_list);										/*延时队列*/

#endif

