#ifndef __HALGLOBAL_H__
#define __HALGLOBAL_H__

#ifdef HAL_GLOBAL_VAR
#undef	EXTERN
#define EXTERN 
#endif

#define HAL_GLOBAL_VARIABLE(type, name) EXTERN __attribute__((section(".data"))) type name

HAL_GLOBAL_VARIABLE(board_dscp_t, bd);
HAL_GLOBAL_VARIABLE(mmap_tlb_t, mmap_tlb);
HAL_GLOBAL_VARIABLE(interrupt_dscp_t, irq_desc)[NR_IRQS];							
HAL_GLOBAL_VARIABLE(adrspce_dscp_t, adr_space)[PLFM_ADRSPCE_NR];
HAL_GLOBAL_VARIABLE(exitreg_map_t, exit_gpiomap)[EXIT_GROUP_NUMBER];

#endif