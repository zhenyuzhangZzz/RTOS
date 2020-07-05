#include "lmosem.h"

int uart2_open(device_t *dev)
{
	printk("open sistick device!\n");
	return 0;
}

struct drv_operations uart2_ops = {
	.open = uart2_open,
};

/*
* uart2_init() 初始化system tick定时器,定时10ms
*/
// private void uart2_init(void)
// {
// 	/*复位uart2 tick*/
// 	writel(TCFG, readl(TCFG) | (1 << 16));

// 	/*
// 	* [14]: 0 integer divider
// 	* [13:12]:01 XrtcXTI作为输入
// 	* [10:8]: 100 1/16
// 	* [7:0]: 23 
// 	* TCLK = 32768 / (0 + 1) / 16 = 2048HZ
// 	*/
// 	writel(TCFG, (0x01 << 12) | (0x04 << 8) | (0 << 0));

// 	/*TICK = TCLK / 2 = 1024hz*/
// 	writel(TICNTB, 1);
// 	while(!((readl(INT_CSTAT) >> 2) & 0x01));
// 	writel(INT_CSTAT, readl(INT_CSTAT));

// 	/*设置ICNTB，定时10ms*/
// 	writel(ICNTB, 10);

// 	while(!((readl(INT_CSTAT) >> 4) & 0x01));
// 	writel(INT_CSTAT, readl(INT_CSTAT));

// 	/*Interrupt counter expired (INTCNT=0) Interrupt Enable*/
// 	writel(INT_CSTAT, readl(TINT_CSTAT) | (1 << 6) | (1 << 0));

// 	/*更新ICNTB,并设置为自动重装*/
// 	writel(TCON, readl(TCON) | (1 << 5 | (1 << 4) | (1 << 0)));
// 	while(!((readl(INT_CSTAT) >> 5) & 0x01));
// 	writel(INT_CSTAT, readl(INT_CSTAT));

// 	/*Interrupt Start */
// 	writel(TCON, readl(TCON) | (0x01 << 3));
// 	while(!((readl(INT_CSTAT) >> 5) & 0x01));
// 	writel(INT_CSTAT, readl(INT_CSTAT));

// }

/*
*中断回调函数
*/
void uart2_handle(uint_t irq, void *data)
{
	//printk("system tick handled!\n");
	/*增加线程tick*/
	krnl_inc_thread_tick(krnl_rtn_curtthread());

	/*处理休眠队列*/
	krnl_deal_sleep();
}

drvstus_t uart2_drv_init(void)
{
	device_t *dev;
	driver_t *drv;
	dev_t dev_id;

	/*1、分配driver结构*/
	drv = driver_alloc("uart2", &uart2_ops);

	if(drv == NULL)
		return DFCERRSTUS;

	/*2、注册字符设备并返回设备号和device*/
	dev_id = register_chrdev(0, &dev, "uart2");

	if(dev_id == 0)
		return DFCERRSTUS;

	/*3、将设备添加到驱动上去*/
	driver_add_device(drv, dev);
	
	/*4、将驱动添加到内核上 */
	driver_add_system(drv);

	/*5、硬件相关的操作*/
	//uart2_init();

	//request_irq(IRQ_UART2, uart2_handle, dev, IRQF_TRIGGER_NONE, "uart2", NULL);

	return DFCOKSTUS;
}