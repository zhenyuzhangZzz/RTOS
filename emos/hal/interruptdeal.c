#include "lmosem.h"

void hal_undefine_handle(void)
{
	system_error("undefine instruction abort occur!");
}

void hal_prefeabort_handle(void)
{
	system_error("prefetch instruction abort occur!");
}

void hal_dataabort_handle(void)
{
	system_error("data access abort occur!");
}

void  hal_swi_handle(uint_t intnr)
{

}

/*根据中断号执行中断处理程序*/
private void hal_do_irqhandle(uint_t intnr)
{

	if(intnr >= NR_IRQS)
	{
		printk("interrupt number error!\n");
		return;
	}

	interrupt_dscp_t *dscp = hal_rtn_intdscp(intnr);
	interrupt_action_t *action_ptr;

	struct list_head *list;

	if(dscp == NULL)
	{
		printk("interrupt number error!\n");
		return;
	}

	/*没有安装中单服务函数*/
	if(dscp->int_serverfunccnt == 0)
		return;

	dscp->int_occurcnt++;

	list_for_each(list, &dscp->int_list)
	{
		action_ptr = list_entry(list, interrupt_action_t, int_list);

		action_ptr->handle(intnr, action_ptr->private_data);

		action_ptr->runcnt++;
	}
}

void hal_irq_handle(void)
{
	/*
		读取中断状态寄存器，判断是哪个控制器产生了中断,有四个中断控制器，VIC0 - VIC3
	*/
	u32_t intstatus = hal_getvicirqstatus(0);
	s32_t tmp;
	uint_t int_nr = NR_IRQS;

	if(intstatus != 0)
	{
		/*EINT16 - EINT31*/
		if(intstatus == EINT_16_31)
		{
			/*读取状态寄存器并保存*/
			tmp = ((readb(EXT_INT_3_PEND) << 8) | readb(EXT_INT_2_PEND)) & 0xffff;

			for(uint_t i = 0; i < 16; i++)
			{
				if(((tmp >> i) & 0x01) == 0x01)
				{
					int_nr = IRQ_EINT(16) + i;	//获取中断编号
					goto status_tail;
				}
			}	
		}
		else 
		{
			for(uint_t i = 0; i < 32; i++)
			{
				if(((intstatus >> i) & 0x01) == 0x01)
				{
					int_nr = VIC0_START + i;
					goto status_tail;
				}
			}

		}
	}

	intstatus = hal_getvicirqstatus(1);

	if(intstatus != 0)
	{
		for(uint_t i = 0; i < 32; i++)
		{
			if(((intstatus >> i) & 0x01) == 0x01)
			{
				int_nr = VIC1_START + i;
				goto status_tail;
			}
		}
	}

	intstatus = hal_getvicirqstatus(2);

	if(intstatus != 0)
	{
		for(uint_t i = 0; i < 32; i++)
		{
			if(((intstatus >> i) & 0x01) == 0x01)
			{
				int_nr = VIC2_START + i;
				goto status_tail;
			}
		}
	}

	intstatus = hal_getvicirqstatus(3);

	if(intstatus != 0)
	{
		for(uint_t i = 0; i < 32; i++)
		{
			if(((intstatus >> i) & 0x01) == 0x01)
			{
				int_nr = VIC3_START + i;
				goto status_tail;
			}
		}
	}
status_tail:
	hal_do_irqhandle(int_nr);
	/*清除挂起标志*/
	hal_clear_irqpnd(int_nr);
	
	/*检查进程调度标志并试图调度进程*/
	krnl_try_sched();
}

void hal_fiq_handle(void)
{
	system_error("fiq interrrupt occur!");
}