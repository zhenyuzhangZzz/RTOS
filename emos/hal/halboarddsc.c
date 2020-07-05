#include "lmosem.h"

void hal_board_dscp_init(void)
{
	spin_lock_init(&bd.bd_lock);

	init_list_head(&bd.bd_list);

	bd.bd_krnlstartaddr = (adr_t)&__begin_kernel;
	bd.bd_krnlendaddr = (adr_t)&__end_kernel;

	bd.mmap_baseaddr = (mmap_dscp_t *)ALIGN((adr_t)&__end_kernel, 4096);						//位图描述符在内核结束后4K对齐后
	bd.mmap_cnt = 0;
	bd.adrspce_baseaddr = adr_space;															//芯片地址空间分布
	bd.adrspce_cnt = PLFM_ADRSPCE_NR;															
	bd.irq_dscp_baseaddr = irq_desc;
	bd.irqsource_cnt = NR_IRQS;

	printk("SDRAM size: 1GB， IO space: 0x20000000 - 0x60000000.\n");
	printk("irq number: %d\n", NR_IRQS);
}

void hal_board_dscp_print(void)
{
	uint_t i;
	struct list_head *head;

	printk("bd.bd_lock: %d\n", bd.bd_lock);
	printk("bd.bd_list: 0x%x, prev: 0x%x, next: 0x%x\n", bd.bd_list, bd.bd_list.prev, bd.bd_list.next);
	printk("bd.bd_krnlstartaddr: 0x%x\n", bd.bd_krnlstartaddr);
	printk("bd.bd_krnlendaddr: 0x%x\n", bd.bd_krnlendaddr);

	printk("bd.mmap_baseaddr: 0x%x\n", bd.mmap_baseaddr);
	printk("bd.mmap_cnt: %d\n", bd.mmap_cnt);
	printk("bd.adrspce_baseaddr: 0x%x\n", bd.adrspce_baseaddr);
	printk("bd.adrspce_cnt: %d\n", bd.adrspce_cnt);

	printk("description size: %d\n", sizeof(mmap_dscp_t));

	head = mmap_tlb.tlb_allocmem_list[5].am_empty_list.prev;

	for(i = 255; i > 200; i--)
	{
		printk("start: 0x%x end: 0x%x, list addr: 0x%x alloc_bit: 0x%x flags: 0x%x 		", bd.mmap_baseaddr[i].map_phy_startaddr, bd.mmap_baseaddr[i].map_phy_endaddr,  \
				&bd.mmap_baseaddr[i].map_list, bd.mmap_baseaddr[i].map_alloc_bit, bd.mmap_baseaddr[i].map_flag);
		
		printk("tlb addr: 0x%x \n", head);

		head = head->prev;
	}
}