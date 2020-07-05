#include "lmosem.h"

LKHEAD_T void platform_init(void)
{
	extern char vector;
	/*1、初始化mmu*/
	s5pv210_mmu_init();
	/*2、初始化中断向量表*/
	s5pv210_set_vector_base((uint_t)&vector);					
}


#ifdef CFG_S5PV210_PLATFORM

LKHEAD_T void s5pv210_mmu_init(void)
{
	uint_t *entry = (uint_t *)TTB_PHY_ADDR;		
	uint_t page_table_dsc = 0;
	uint_t phy_addr = 0;

	/*1、设置页表项*/
	for(u32_t i = 0; i < PAGE_TABLE_SIZE; i++)
	{
		page_table_dsc = phy_addr | DSC_SELECT_AP(AP_ALL_WR) | DSC_SELECT_DOMAIN(0)| DSC_SELECT_4BIT | DSC_SELECT_NOCW | DSC_SELECT_IDE;
		entry[i] = page_table_dsc;
		phy_addr += 0x100000;
	}
	/*2、设置tbl基址*/
	s5pv210_set_ttb(TTB_PHY_ADDR);
	/*3、设置主区域*/
	s5pv210_set_domain(0xffffffff);
	/*4、使Icache, Dcache, TLB失效*/
	s5pv210_invalid_tlb_cache();

	/*5、打开icache dcache*/
	s5pv210_enable_cache();

	/*6、打开mmu*/
	s5pv210_enable_mmu();
}

LKHEAD_T void s5pv210_set_ttb(uint_t ttb_phy_addr)
{
	__asm__ __volatile__(
		"mcr p15, 0, %[ttbaddr], c2, c0, 0"
		:
		:[ttbaddr] "r" (ttb_phy_addr)
		:"cc", "memory"
	);
	return;
}

LKHEAD_T void s5pv210_set_domain(uint_t domain)
{
	__asm__ __volatile__(
		"mcr p15, 0, %[a], c3, c0, 0"
		:
		:[a] "r" (domain)
		:"cc", "memory"
	);
	return;
}

LKHEAD_T void s5pv210_invalid_tlb_cache(void)
{
	__asm__ __volatile__(
		"mov r0, #0 \n\t"
		"mcr p15, 0, r0, c7, c5, 0 \n\t"				//invalidate icache
		"mcr p15, 0, r0, c7, c6, 1 \n\t"				//invalidate dcache
		"mcr p15, 0, r0, c8, c7, 0 \n\t"				//Invalidate Inst-TLB and Data-TLB
		::
		:"cc", "memory", "r0"
	);
	return;
}

LKHEAD_T void s5pv210_enable_cache(void)
{
	__asm__ __volatile__(
		"mrc p15, 0, r0, c1, c0, 0 \n\t"
		"orr r0, r0, %[a]  		   \n\t"					//使能icache dcache align write buffer
		"mcr p15, 0, r0, c1, c0, 0 \n\t"
		:
		:[a] "r" (0x0000100e)
		:"cc", "memory", "r0"
	);
	return;
}

LKHEAD_T void s5pv210_enable_mmu(void)
{
	__asm__ __volatile__(
		"mrc p15, 0, r0, c1, c0, 0 \n\t"
		"orr r0, r0, #0x01 \n\t"
		"mcr p15, 0, r0, c1, c0, 0 \n\t"
		"nop \n\t"
		"nop \n\t"
		"nop \n\t"
		"nop \n\t"
		"nop \n\t"
		"nop \n\t"
		"nop \n\t"
	::
	:"cc", "memory", "r0"
	);
	return;
}

LKHEAD_T void s5pv210_disable_mmu(void)
{
	__asm__ __volatile__(
		"mrc p15, 0, r0, c1, c0, 0 \n\t"
		"bic r0, r0, #0x01 \n\t"
		"mcr p15, 0, r0, c1, c0, 0 \n\t"
		::
		:"cc", "memory", "r0"
	);
	return;
}

LKHEAD_T void s5pv210_vector_init(void)
{
	__asm__ __volatile__(
		"mrc p15, 0, r0, c1, c0, 0 \n\t"
		"bic r0, r0, #0x2000 \n\t"					
		"mcr p15, 0, r0, c1, c0, 0 \n\t"
		::
		:"cc", "memory", "r0"
	);
	return;
}

LKHEAD_T void s5pv210_set_vector_base(uint_t addr)
{
	__asm__ __volatile__(
		"mrc	p15, 0, r0, c1, c0, 0 \n\t"			//将c1[13]位设置为0，这样向量表的基地址由The Vector Base Address Registers决定
		"bic	r0, #(1 << 13)		  \n\t"
		"mcr	p15, 0, r0, c1, c0, 0 \n\t"	

		"mcr	p15, 0, %[a], c12, c0, 0 \n\t"		//set VBAR
		:
		:[a] "r" (addr)
		:"cc", "memory", "r0"										
	);
	return;
}
#endif
