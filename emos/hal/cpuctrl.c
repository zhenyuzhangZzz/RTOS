#include "lmosem.h"

cpuflg_t raw_local_fiqirq_disable(void)
{
	cpuflg_t ret;
	reg_t tmp = 0;
	__asm__ __volatile__(
		"mrs %1, cpsr\n"
		"mov %0, %1\n"
		"orr %1, %1, %2\n"
		"msr cpsr, %1\n"
		:"=&r" (ret), "=&r" (tmp)
		:"I" (FIQMASK | IRQMASK)
		:"cc", "memory"
	);
	return ret;
}

cpuflg_t raw_local_fiq_disable(void)
{
	cpuflg_t ret;
	reg_t tmp = 0;
	__asm__ __volatile__(
		"mrs %1, cpsr\n"
		"mov %0, %1\n"
		"orr %1, %1, %2\n"
		"msr cpsr, %1\n"
		:"=&r" (ret), "=&r" (tmp)
		:"I" (FIQMASK)
		:"cc", "memory"
	);
	return ret;
}

cpuflg_t raw_local_irq_disable(void)
{
	cpuflg_t ret;
	reg_t tmp = 0;
	__asm__ __volatile__(
		"mrs %1, cpsr\n"
		"mov %0, %1\n"
		"orr %1, %1, %2\n"
		"msr cpsr, %1\n"
		:"=&r" (ret), "=&r" (tmp)
		:"I" (IRQMASK)
		:"cc", "memory"
	);
	return ret;
}