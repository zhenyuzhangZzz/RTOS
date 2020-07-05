#include "lmosem.h"

private void interrupt_action_init(interrupt_action_t *ptr, interrupt_dscp_t *dscp, u32_t flag, 
							void *device, void *pdata, const char *name, irq_handler_t handler)
{
	spin_lock_init(&ptr->int_lock);

	init_list_head(&ptr->int_list);

	ptr->int_flag = flag;
	ptr->int_dscp = dscp;
	ptr->device = device;
	ptr->private_data = pdata;
	ptr->name = name;
	ptr->runcnt = 0;
	ptr->handle = handler;
}

private bool_t irq_add_action(interrupt_action_t *ptr, interrupt_dscp_t *dscp)
{
	struct list_head *list;
	interrupt_action_t *action_ptr;

	if(ptr == NULL || dscp == NULL)
		return FALSE;

	/*循环中断里面的每一个action,如果申请的标记不一样则结束*/
	list_for_each(list, &dscp->int_list)
	{
		action_ptr = list_entry(list, interrupt_action_t, int_list);

		if(action_ptr->int_flag != ptr->int_flag)
		{
			printk("error: this irq requested in different flag\n");
			return  FALSE;	
		}
	}

	/*将action加入dscp链表*/
	list_add(&ptr->int_list, &dscp->int_list);

	dscp->int_serverfunccnt++;

	return TRUE;
}

bool_t request_irq(unsigned int irq, irq_handler_t handler, void *device, unsigned long flags, const char *name, void *dev_id)
{
	cpuflg_t flag;

	if(device == NULL || handler == NULL)
		return FALSE;

	interrupt_dscp_t *dscp = hal_rtn_intdscp(irq);

	if(dscp == NULL)
		return FALSE;

	spin_lock_save(&dscp->int_lock, flag);

	if(dscp->int_serverfunccnt != 0 && dscp->int_status != IRQF_SHARED)
	{
		printk("error: this irq requested and no share!\n");
		goto rtn_tail;
	}

	/*该中断已经注册为共享中断, 但新注册的中断请求时未加共享标记*/
	if(dscp->int_status == IRQF_SHARED && (flags & IRQF_STATUS_MASK) != IRQF_SHARED)
	{
		printk("error: request irq must have share flag!\n");
		goto rtn_tail;
	}

	/*动态分配一个action*/
	interrupt_action_t *action = (interrupt_action_t *)krnl_alloc(sizeof(interrupt_action_t));

	if(action == NULL)
	{
		system_error("alloc memory failed in request_irq function of krnlinterrupt.c\n");
		return DFCERRSTUS;
	}

	/*初始化*/
	interrupt_action_init(action, dscp, flags & IRQF_TRIGGER_MASK, device, dev_id, name, handler);

	/*加入失败,唯一原因就是flag不一样*/
	if(irq_add_action(action, dscp) == FALSE)
	{
		if(krnl_free((adr_t)action, sizeof(interrupt_action_t)) == FALSE)
		{
			system_error("free memory failed in request_irq function of krnlinterrupt.c\n");
			return FALSE;
		}
		goto rtn_tail;
	}

	/*若为外部中断，则初始化外部中断对应的gpio*/
	hal_exit_mode_init(irq, flags & IRQF_TRIGGER_MASK);

	/*使能对应中断*/
	hal_enalbe_irq(irq);

	dscp->int_status = flags & IRQF_STATUS_MASK;

rtn_tail:
	spin_unlock_restore(&dscp->int_lock, flag);

	return TRUE;
}

bool_t free_irq(unsigned int irq, void *dev_id)
{
	interrupt_dscp_t *dscp = hal_rtn_intdscp(irq);
	struct list_head *list;
	interrupt_action_t *action_ptr;
	cpuflg_t flag;

	if(dscp == NULL)
		return FALSE;

	spin_lock_save(&dscp->int_lock, flag);

	/*没有安装中单服务函数*/
	if(dscp->int_serverfunccnt == 0)
		goto rtn_tail;

	list_for_each(list, &dscp->int_list)
	{
		action_ptr = list_entry(list, interrupt_action_t, int_list);

		/*根据传入的私有数据释放action*/
		if(action_ptr->private_data == dev_id)
		{
			/*从双向链表删除*/
			list_del(&action_ptr->int_list);

			if(krnl_free((adr_t)action_ptr, sizeof(interrupt_action_t)) == FALSE)
			{
				system_error("free memory failed in request_irq function of krnlinterrupt.c\n");
				return FALSE;
			}
			break;
		}
	}

	if(dscp->int_serverfunccnt > 0)
		dscp->int_serverfunccnt--;

	hal_disalbe_irq(irq);
rtn_tail:
	spin_unlock_restore(&dscp->int_lock, flag);

	return TRUE;
}

u32_t dataup = 1;
u32_t datadown = 2;
uint_t device = 1;

void irq_handler(uint_t irq, void *data)
{

	if(irq == IRQ_EINT(0))
		printk("key up down, irq number of key: %d\n", irq);
	if(irq == IRQ_EINT(2))
	{
		printk("key left down, irq number of key: %d and  ", irq);

		if(free_irq(IRQ_EINT(0), &dataup) == TRUE)
			printk("free irq: %d success!\n", IRQ_EINT(0));
		else
			printk("free irq failed!\n");
	}

	if(irq == IRQ_EINT(3))
	{
		printk("key right down, irq number of key: %d and try to request key up\n", irq);

		request_irq(IRQ_EINT(0), irq_handler, (void *)&device, IRQF_TRIGGER_FALLING, "keyup", (void *)&dataup);
	}
}

void interrupt_test(void)
{
	request_irq(IRQ_EINT(0), irq_handler, (void *)&device, IRQF_TRIGGER_FALLING, "keyup", (void *)&dataup);
	request_irq(IRQ_EINT(1), irq_handler, (void *)&device, IRQF_TRIGGER_FALLING, "keydown", (void *)&datadown);
	request_irq(IRQ_EINT(2), irq_handler, (void *)&device, IRQF_TRIGGER_FALLING, "keyleft", (void *)&dataup);
	request_irq(IRQ_EINT(3), irq_handler, (void *)&device, IRQF_TRIGGER_FALLING, "keyright", (void *)&datadown);
	request_irq(IRQ_EINT(4), irq_handler, (void *)&device, IRQF_TRIGGER_FALLING, "keyenter", (void *)&datadown);
	request_irq(IRQ_EINT(5), irq_handler, (void *)&device, IRQF_TRIGGER_FALLING, "keyexit", (void *)&dataup);
	request_irq(IRQ_EINT(22), irq_handler, (void *)&device, IRQF_TRIGGER_FALLING, "keyhome", (void *)&datadown);
	request_irq(IRQ_EINT(23), irq_handler, (void *)&device, IRQF_TRIGGER_FALLING, "keypower", (void *)&datadown);
}
