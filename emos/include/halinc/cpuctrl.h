#ifndef __CPUIO_H__
#define __CPUIO_H__

#define IRQMASK		(1 << 7)
#define FIQMASK		(1 << 6)

#define barrier()	__asm__ __volatile__("":::"memory")

#define preempt_disable() 

#ifndef ARCH_HAS_PREFETCHW
#define prefetchw(x) __builtin_prefetch(x,1)
#endif

#define local_irq_disable(flags) \
	do{	flags = raw_local_irq_disable(); } while(0)

#define local_irq_restore(flags) \
	do{ raw_local_cpuflag_restore(flags); } while(0)

#define local_fiq_disable(flags) \
	do{	flags = raw_local_fiq_disable(); }while(0)

#define  local_fiq_restore(flags) \
	do{ raw_local_cpuflag_restore(flags); }while(0)

#define local_fiqirq_disable(flags) \
	do{	flags = raw_local_fiqirq_disable(); }while(0)

#define  local_fiqirq_restore(flags) \
	do{ raw_local_cpuflag_restore(flags); }while(0)

	
KLINE u8_t readb(uint_t addr)
{
	u8_t ret;
	__asm__ __volatile__(
		"ldrb %0, [%1]"
		:"=&r" (ret)
		:"r" (addr)
		:"memory"
	);
	return ret;
}

KLINE u16_t readw(uint_t addr)
{
	u16_t ret;
	__asm__ __volatile__(
		"ldrh %0, [%1]"
		:"=&r" (ret)
		:"r" (addr)
		:"memory"
	);
	return ret;
}

KLINE u32_t readl(uint_t addr)
{
	u32_t ret;
	__asm__ __volatile__(
		"ldr %0, [%1]"
		:"=&r" (ret)
		:"r" (addr)
		:"memory"
	);
	return ret;
}

KLINE void writeb(uint_t addr, u8_t dat)
{
	__asm__ __volatile__(
		"strb %1, [%0]"
		:
		:"r" (addr), "r" (dat)
		:"memory"
	);
	return;
}

KLINE void writew(uint_t addr, u16_t dat)
{
	__asm__ __volatile__(
		"strh %1, [%0]"
		:
		:"r" (addr), "r" (dat)
		:"memory"
	);
	return;
}

KLINE void writel(uint_t addr, u32_t dat)
{
	__asm__ __volatile__(
		"str %1, [%0]"
		:
		:"r" (addr), "r" (dat)
		:"memory"
	);
	return;
}


KLINE void raw_local_cpuflag_restore(cpuflg_t flags)
{
	__asm__ __volatile__(
		"msr cpsr, %0\n"
		:
		:"r" (flags)
		:"cc"
	);
}

/*
* 初始化自旋锁为0
*/
KLINE void spin_lock_init(spinlock_t* lock)
{
	__asm__ __volatile__(
		"str %1, [%0]\n"
		:
		:"r" (&lock->rlock), "r" (0)
		:"memory"
	);
}
/*
* 获取自旋锁，原子操作
*/
KLINE void raw_spinlock_lock(spinlock_t *lock)
{
	reg_t tmp;

	/*数据预取*/
	prefetchw((void *)&lock->rlock);

	__asm__ __volatile__(
		"1:\n"
		"ldrex  %0, [%1]\n"						//取lock->lock放在tmp里，并且设置&lock->lock这个内存地址为独占访问(Exclusive独占标记)
		"teq %0, #0\n"							//测试lock_lock是否为0，影响标志位z
		"strexeq %0, %2, [%1]\n"				//若相等则+1上锁，并清除Exclusive独占标记
		"teqeq %0, #0\n"						//如果lock_lock是0，并且strexeq返回了0，表示加锁成功，返回
		"bne 1b\n"								//如果上面的条件(1：lock->lock里不为0，2：strexeq失败)有一个符合，就在原地打转
		:"=&r" (tmp)
		:"r" (&lock->rlock), "r" (1)
		:"cc", "memory"
	);
}

KLINE void raw_spinlock_unlock(spinlock_t* lock)
{
	barrier();

	__asm__ __volatile__(
		"str %1, [%0]\n"
		:
		:"r" (&lock->rlock), "r" (0)
		:"memory"
	);
}

KLINE cpuflg_t get_cpsr(void)
{
	cpuflg_t ret;
	__asm__ __volatile__("mrs %0, cpsr\n":"=&r" (ret) ::"cc");
	return ret;
}

KLINE cpuflg_t get_spsr(void)
{
	cpuflg_t ret;
	__asm__ __volatile__("mrs %0, spsr\n":"=&r" (ret) ::"cc");
	return ret;
}

KLINE cpuflg_t get_sp(void)
{
	cpuflg_t ret;
	__asm__ __volatile__("mov %0, sp\n":"=&r" (ret) ::"memory");
	return ret;
}

KLINE uint_t hal_rtn_cpuid(void)
{
	return 0;
}

cpuflg_t raw_local_irq_disable(void);
cpuflg_t raw_local_fiq_disable(void);
cpuflg_t raw_local_fiqirq_disable(void);

#endif