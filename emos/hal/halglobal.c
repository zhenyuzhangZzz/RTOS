#define HAL_GLOBAL_VAR
#include "lmosem.h"

HAL_GLOBAL_VARIABLE(board_dscp_t, bd);												/*板级描述符*/
HAL_GLOBAL_VARIABLE(mmap_tlb_t, mmap_tlb);											/*用于快速查询内存块*/

HAL_GLOBAL_VARIABLE(interrupt_dscp_t, irq_desc)[NR_IRQS];							/*中断描述符*/

HAL_GLOBAL_VARIABLE(exitreg_map_t, exit_gpiomap)[EXIT_GROUP_NUMBER] = {				/*外部中断寄存器描述*/
	{IRQ_EINT(0), IRQ_EINT(7), 4, GPH0CON, EXT_INT_0_CON},
	{IRQ_EINT(8), IRQ_EINT(15), 4, GPH1CON, EXT_INT_1_CON},
	{IRQ_EINT(16), IRQ_EINT(23), 4, GPH2CON, EXT_INT_2_CON},
	{IRQ_EINT(24), IRQ_EINT(31), 4, GPH3CON, EXT_INT_3_CON},
};						


HAL_GLOBAL_VARIABLE(adrspce_dscp_t, adr_space)[PLFM_ADRSPCE_NR] = {					/*芯片地址空间描述符*/
	{ADRSPCE_SDRAM, 0, 0x20000000, 0x60000000},
};

