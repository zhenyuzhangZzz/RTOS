#include "lmosem.h"

private void interrupt_dscp_init(interrupt_dscp_t *ptr, u32_t status, u32_t flag, u32_t pndbitnr, uint_t int_nr)
{
	spin_lock_init(&ptr->int_lock);

	init_list_head(&ptr->int_list);

	ptr->int_serverfunccnt = 0;
	ptr->int_flag = flag;
	ptr->int_status = status;
	ptr->int_pndbitnr = pndbitnr;
	ptr->int_nr = int_nr;
}

/*
*	对s5pv210来说，每次中断完都要清楚中断向量寄存器
*/
private void hal_clear_vectaddr(void)
{
	writel(VIC0ADDRESS, 0);
	writel(VIC1ADDRESS, 0);
	writel(VIC2ADDRESS, 0);
	writel(VIC3ADDRESS, 0);
}

void hal_interrupt_init(void)
{
	uint_t i;
	interrupt_dscp_t *ptr = bd.irq_dscp_baseaddr;

	/*初始化中断描述符*/
	for(i = VIC0_START; i <= VIC0_END; i++)
	{
		interrupt_dscp_init(&ptr[i], 0, MAIN_INT_FLAG, i - VIC0_START, i);
	}

	for(i = VIC1_START; i <= VIC1_END; i++)
	{
		interrupt_dscp_init(&ptr[i], 0, MAIN_INT_FLAG, i - VIC1_START, i);
	}

	for(i = VIC2_START; i <= VIC2_END; i++)
	{
		interrupt_dscp_init(&ptr[i], 0, MAIN_INT_FLAG, i - VIC2_START, i);
	}
	for(i = VIC3_START; i <= VIC3_END; i++)
	{
		interrupt_dscp_init(&ptr[i], 0, MAIN_INT_FLAG, i - VIC3_START, i);
	}

	/*外部中断16 - 31*/
	for(i = IRQ_EINT(16); i < IRQ_EINT(31); i++)
	{
		interrupt_dscp_init(&ptr[i], 0, EINT16_31_FLAG, i - IRQ_EINT(16), i);
	}

	/*清除所有中断*/
	writel(VIC0INTENCLEAR, 0xffffffff);
	writel(VIC1INTENCLEAR, 0xffffffff);
	writel(VIC2INTENCLEAR, 0xffffffff);
	writel(VIC3INTENCLEAR, 0xffffffff);

	/*全部配置为irq中断*/
	writel(VIC0INTSELECT, 0);
	writel(VIC1INTSELECT, 0);
	writel(VIC2INTSELECT, 0);
	writel(VIC3INTSELECT, 0);

	/*清楚中断向量寄存器*/
	hal_clear_vectaddr();
}

/*
*根据中断编号返回描述符
*/
interrupt_dscp_t *hal_rtn_intdscp(uint_t int_nr)
{
	if(int_nr >= bd.irqsource_cnt)
		return NULL;

	return &bd.irq_dscp_baseaddr[int_nr];
}

exitreg_map_t *hal_rtn_exitregmap(uint_t int_nr)
{
	uint_t i;
	if(int_nr >= bd.irqsource_cnt)
		return NULL;

	for(i = 0; i < EXIT_GROUP_NUMBER; i++)
	{
		if(exit_gpiomap[i].nr_start <= int_nr && int_nr <= exit_gpiomap[i].nr_end)
			break;
	}

	return &exit_gpiomap[i];
}

void hal_exit_mode_init(uint_t int_nr, uint_t flag)
{
	uint_t offset;

	/*非外部中断的中断号*/
	if(int_nr > IRQ_EINT(15) && int_nr < IRQ_EINT(16))
		return;

	exitreg_map_t *ptr;

	ptr = hal_rtn_exitregmap(int_nr);

	offset = int_nr - ptr->nr_start;

	/*配置gpio为外部中断*/
	writel(ptr->con, readl(ptr->con) | 0x0F << (ptr->oneusedbit * offset));
	/*设置触发方式*/
	writel(ptr->mode_con, readl(ptr->mode_con) & ~(0x0F << (ptr->oneusedbit * offset)));
	writel(ptr->mode_con, readl(ptr->mode_con) | flag << (ptr->oneusedbit * offset));	
}

/*
*根据中断编号挂起寄存器
*/
bool_t hal_clear_irqpnd(uint_t int_nr)
{
	interrupt_dscp_t *ptr = hal_rtn_intdscp(int_nr);

	if(ptr == NULL)
		return FALSE;

	u32_t regoffset = ptr->int_pndbitnr;

	u32_t flag = ptr->int_flag;

	if(flag == MAIN_INT_FLAG)
	{
		if(regoffset < IRQ_EINT(8))
			writel(EXT_INT_0_PEND, readb(EXT_INT_0_PEND));
		else if(regoffset <= IRQ_EINT(15))
			writel(EXT_INT_1_PEND, readb(EXT_INT_1_PEND));
		else if(regoffset >= IRQ_TIMER0_VIC && regoffset <=IRQ_SYSTIMER)
		{
			writel(TINT_CSTAT, readl(TINT_CSTAT));
			writel(INT_CSTAT, readl(INT_CSTAT));
		}
	}
	
	if(flag == EINT16_31_FLAG)
	{
		if(regoffset < 8)
			writel(EXT_INT_2_PEND, readb(EXT_INT_2_PEND));
		else if(regoffset < 16)
			writel(EXT_INT_3_PEND, readb(EXT_INT_3_PEND));
	}

	return TRUE;
}

bool_t hal_enalbe_irq(uint_t int_nr)
{
	interrupt_dscp_t *ptr = hal_rtn_intdscp(int_nr);

	if(ptr == NULL)
		return FALSE;

	u32_t regoffset = ptr->int_pndbitnr;

	u32_t flag = ptr->int_flag;

	/*主中断*/
	if(flag == MAIN_INT_FLAG)
	{
		/*外部中断*/
		if(int_nr <= VIC0_END)
		{
			if(regoffset <= IRQ_EINT(15))
			{
				/*EXIT0 - EXIT7*/
				if(regoffset < 8)
				{
					/*开启外部中断屏蔽寄存器*/
					writel(EXT_INT_0_MASK, readl(EXT_INT_0_MASK) & ~(1 << regoffset));
				}
				else 		/*EXIT8 - EXIT15*/
				{
					writel(EXT_INT_1_MASK, readl(EXT_INT_1_MASK) & ~(1 << (regoffset - 8)));
				}

			}
			writel(VIC0INTENABLE, 1 << regoffset);
		}
		else if(int_nr <= VIC1_END)
			writel(VIC1INTENABLE, 1 << regoffset);
		else if(int_nr <= VIC2_END)
			writel(VIC2INTENABLE, 1 << regoffset);
		else if(int_nr <= VIC3_END)
			writel(VIC3INTENABLE, 1 << regoffset);
	}
	if(flag == EINT16_31_FLAG)
	{
		/*EXIT16 - EXIT23*/
		if(regoffset < 8)
		{
			/*开启外部中断屏蔽寄存器*/
			writel(EXT_INT_2_MASK, readl(EXT_INT_2_MASK) & ~(1 << regoffset));
		}
		else 		/*EXIT24 - EXIT31*/
		{
			writel(EXT_INT_3_MASK, readl(EXT_INT_3_MASK) & ~(1 << (regoffset - 8)));
		}
		writel(VIC0INTENABLE,  1 << 16);				//必须要开启主中断源中的16位
	}
	return TRUE;
}

bool_t hal_disalbe_irq(uint_t int_nr)
{
	interrupt_dscp_t *ptr = hal_rtn_intdscp(int_nr);

	if(ptr == NULL)
		return FALSE;

	u32_t regoffset = ptr->int_pndbitnr;

	u32_t flag = ptr->int_flag;

	/*主中断*/
	if(flag == MAIN_INT_FLAG)
	{
		/*外部中断*/
		if(int_nr <= VIC0_END)
		{
			if(int_nr <= IRQ_EINT(15))
			{
				/*EXIT0 - EXIT7*/
				if(regoffset < 8)
				{
					writel(EXT_INT_0_MASK, readl(EXT_INT_0_MASK) | (1 << regoffset));
				}
				else 		/*EXIT8 - EXIT15*/
				{
					writel(EXT_INT_1_MASK, readl(EXT_INT_1_MASK) | (1 << (regoffset - 8)));
				}

			}
			writel(VIC0INTENCLEAR, 1 << regoffset);
		}
		else if(int_nr <= VIC1_END)
			writel(VIC1INTENCLEAR, 1 << regoffset);
		else if(int_nr <= VIC2_END)
			writel(VIC2INTENCLEAR, 1 << regoffset);
		else if(int_nr <= VIC3_END)
			writel(VIC3INTENCLEAR, 1 << regoffset);
	}

	if(flag == EINT16_31_FLAG)
	{
		/*EXIT16 - EXIT23*/
		if(regoffset < 8)
		{
			writel(EXT_INT_2_MASK, readl(EXT_INT_2_MASK) | (1 << regoffset));
		}
		else 		/*EXIT24 - EXIT31*/
		{
			writel(EXT_INT_3_MASK, readl(EXT_INT_3_MASK) | (1 << (regoffset - 8)));
		}
		writel(VIC0INTENCLEAR, 1 << 16);							//关闭VIC0第16位
	}
	return TRUE;
}

u32_t hal_getvicirqstatus(u32_t vicnr)
{
	if(vicnr == 0)
		return readl(VIC0IRQSTATUS);
	else if(vicnr == 1)
		return readl(VIC1IRQSTATUS);
	else if(vicnr == 2)
		return readl(VIC2IRQSTATUS);
	else if(vicnr == 3)
		return readl(VIC3IRQSTATUS);

	return 0;
}