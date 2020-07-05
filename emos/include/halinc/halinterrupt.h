#ifndef __HALINTERRUPT_H__
#define __HALINTERRUPT_H__

#define S5P_IRQ(x)		((x))

#define S5P_VIC0_BASE		S5P_IRQ(0)
#define S5P_VIC1_BASE		S5P_IRQ(32)
#define S5P_VIC2_BASE		S5P_IRQ(64)
#define S5P_VIC3_BASE		S5P_IRQ(96)

#define VIC_BASE(x)		(S5P_VIC0_BASE + ((x)*32))
		
/* VIC based IRQs */

#define S5P_IRQ_VIC0(x)		(S5P_VIC0_BASE + (x))
#define S5P_IRQ_VIC1(x)		(S5P_VIC1_BASE + (x))
#define S5P_IRQ_VIC2(x)		(S5P_VIC2_BASE + (x))
#define S5P_IRQ_VIC3(x)		(S5P_VIC3_BASE + (x))

#define IRQ_BATF			S5P_IRQ_VIC0(17)
#define IRQ_MDMA			S5P_IRQ_VIC0(18)
#define IRQ_PDMA0			S5P_IRQ_VIC0(19)
#define IRQ_PDMA1			S5P_IRQ_VIC0(20)
#define IRQ_TIMER0_VIC		S5P_IRQ_VIC0(21)
#define IRQ_TIMER1_VIC		S5P_IRQ_VIC0(22)
#define IRQ_TIMER2_VIC		S5P_IRQ_VIC0(23)
#define IRQ_TIMER3_VIC		S5P_IRQ_VIC0(24)
#define IRQ_TIMER4_VIC		S5P_IRQ_VIC0(25)
#define IRQ_SYSTIMER		S5P_IRQ_VIC0(26)
#define IRQ_WDT				S5P_IRQ_VIC0(27)
#define IRQ_RTC_ALARM		S5P_IRQ_VIC0(28)
#define IRQ_RTC_TIC			S5P_IRQ_VIC0(29)
#define IRQ_GPIOINT			S5P_IRQ_VIC0(30)
#define IRQ_FIMC3			S5P_IRQ_VIC0(31)

/* VIC1: ARM, Power, Memory, Connectivity, Storage */

#define IRQ_PMU				S5P_IRQ_VIC1(0)
#define IRQ_CORTEX1			S5P_IRQ_VIC1(1)
#define IRQ_CORTEX2			S5P_IRQ_VIC1(2)
#define IRQ_CORTEX3			S5P_IRQ_VIC1(3)
#define IRQ_CORTEX4			S5P_IRQ_VIC1(4)
#define IRQ_IEMAPC			S5P_IRQ_VIC1(5)
#define IRQ_IEMIEC			S5P_IRQ_VIC1(6)
#define IRQ_ONENAND			S5P_IRQ_VIC1(7)
#define IRQ_NFC				S5P_IRQ_VIC1(8)
#define IRQ_CFCON			S5P_IRQ_VIC1(9)
#define IRQ_UART0			S5P_IRQ_VIC1(10)
#define IRQ_UART1			S5P_IRQ_VIC1(11)
#define IRQ_UART2			S5P_IRQ_VIC1(12)
#define IRQ_UART3			S5P_IRQ_VIC1(13)
#define IRQ_IIC				S5P_IRQ_VIC1(14)
#define IRQ_SPI0			S5P_IRQ_VIC1(15)
#define IRQ_SPI1			S5P_IRQ_VIC1(16)
#define IRQ_SPI2			S5P_IRQ_VIC1(17)
#define IRQ_IRDA			S5P_IRQ_VIC1(18)
#define IRQ_IIC2			S5P_IRQ_VIC1(19)
#define IRQ_IIC_HDMIPHY		S5P_IRQ_VIC1(20)
#define IRQ_HSIRX			S5P_IRQ_VIC1(21)
#define IRQ_HSITX			S5P_IRQ_VIC1(22)
#define IRQ_UHOST			S5P_IRQ_VIC1(23)
#define IRQ_OTG				S5P_IRQ_VIC1(24)
#define IRQ_MSM				S5P_IRQ_VIC1(25)
#define IRQ_HSMMC0			S5P_IRQ_VIC1(26)
#define IRQ_HSMMC1			S5P_IRQ_VIC1(27)
#define IRQ_HSMMC2			S5P_IRQ_VIC1(28)
#define IRQ_MIPI_CSIS		S5P_IRQ_VIC1(29)
#define IRQ_MIPIDSI			S5P_IRQ_VIC1(30)
#define IRQ_ONENAND_AUDI	S5P_IRQ_VIC1(31)

/* VIC2: Multimedia, Audio, Security */

#define IRQ_LCD0		S5P_IRQ_VIC2(0)
#define IRQ_LCD1		S5P_IRQ_VIC2(1)
#define IRQ_LCD2		S5P_IRQ_VIC2(2)
#define IRQ_LCD3		S5P_IRQ_VIC2(3)
#define IRQ_ROTATOR		S5P_IRQ_VIC2(4)
#define IRQ_FIMC0		S5P_IRQ_VIC2(5)
#define IRQ_FIMC1		S5P_IRQ_VIC2(6)
#define IRQ_FIMC2		S5P_IRQ_VIC2(7)
#define IRQ_JPEG		S5P_IRQ_VIC2(8)
#define IRQ_2D			S5P_IRQ_VIC2(9)
#define IRQ_3D			S5P_IRQ_VIC2(10)
#define IRQ_MIXER		S5P_IRQ_VIC2(11)
#define IRQ_HDMI		S5P_IRQ_VIC2(12)
#define IRQ_IIC1		S5P_IRQ_VIC2(13)
#define IRQ_MFC			S5P_IRQ_VIC2(14)
#define IRQ_SDO			S5P_IRQ_VIC2(15)
#define IRQ_I2S0		S5P_IRQ_VIC2(16)
#define IRQ_I2S1		S5P_IRQ_VIC2(17)
#define IRQ_I2S2		S5P_IRQ_VIC2(18)
#define IRQ_AC97		S5P_IRQ_VIC2(19)
#define IRQ_PCM0		S5P_IRQ_VIC2(20)
#define IRQ_PCM1		S5P_IRQ_VIC2(21)
#define IRQ_SPDIF		S5P_IRQ_VIC2(22)
#define IRQ_ADC			S5P_IRQ_VIC2(23)
#define IRQ_PENDN		S5P_IRQ_VIC2(24)
#define IRQ_TC			IRQ_PENDN
#define IRQ_KEYPAD		S5P_IRQ_VIC2(25)
#define IRQ_CG			S5P_IRQ_VIC2(26)
#define IRQ_SSS_INT		S5P_IRQ_VIC2(27)
#define IRQ_SSS_HASH	S5P_IRQ_VIC2(28)
#define IRQ_PCM2		S5P_IRQ_VIC2(29)
#define IRQ_SDMIRQ		S5P_IRQ_VIC2(30)
#define IRQ_SDMFIQ		S5P_IRQ_VIC2(31)

/* VIC3: Etc */

#define IRQ_IPC			S5P_IRQ_VIC3(0)
#define IRQ_HOSTIF		S5P_IRQ_VIC3(1)
#define IRQ_HSMMC3		S5P_IRQ_VIC3(2)
#define IRQ_CEC			S5P_IRQ_VIC3(3)
#define IRQ_TSI			S5P_IRQ_VIC3(4)
#define IRQ_MDNIE0		S5P_IRQ_VIC3(5)
#define IRQ_MDNIE1		S5P_IRQ_VIC3(6)
#define IRQ_MDNIE2		S5P_IRQ_VIC3(7)
#define IRQ_MDNIE3		S5P_IRQ_VIC3(8)
#define IRQ_ADC1		S5P_IRQ_VIC3(9)
#define IRQ_PENDN1		S5P_IRQ_VIC3(10)
#define IRQ_TC1			IRQ_PENDN1
#define IRQ_VIC_END		S5P_IRQ_VIC3(31)

#define S5P_EINT_BASE1		(S5P_IRQ_VIC0(0))
#define S5P_EINT_BASE2		(IRQ_VIC_END + 1)

/*外部中断*/
#define IRQ_EINT(x)			((x) < 16 ? ((x) + S5P_EINT_BASE1) \
								: ((x) - 16 + S5P_EINT_BASE2))

#define EXIT_GROUP_NUMBER		4
#define EINT_16_31				(1 << 16)

/* Set the default NR_IRQS */
#define NR_IRQS			(IRQ_EINT(31) + 1)

#define VIC0_START	 	S5P_IRQ_VIC0(0)
#define VIC0_END	 	S5P_IRQ_VIC0(31)
#define VIC1_START	 	S5P_IRQ_VIC1(0)
#define VIC1_END	 	S5P_IRQ_VIC1(31)
#define VIC2_START	 	S5P_IRQ_VIC2(0)
#define VIC2_END	 	S5P_IRQ_VIC2(31)
#define VIC3_START	 	S5P_IRQ_VIC3(0)
#define VIC3_END	 	S5P_IRQ_VIC3(31)

#define MAIN_INT_FLAG			0
#define EINT16_31_FLAG			1

#define IRQF_TRIGGER_LOW		0x00000000
#define IRQF_TRIGGER_HIGH		0x00000001
#define IRQF_TRIGGER_FALLING	0x00000002
#define IRQF_TRIGGER_RISING		0x00000003
#define IRQF_TRIGGER_BOTH		0x00000004
#define IRQF_TRIGGER_NONE		0x00000005

#define IRQF_TRIGGER_MASK		0x0000000F

#define IRQF_SHARED				0x00000080
#define IRQF_STATUS_MASK		0x000000F0

/*中断描述符*/
typedef struct interrupt_dscp
{
	spinlock_t int_lock;
	struct list_head int_list;					/*挂载中单服务函数*/
	uint_t int_serverfunccnt;					/*中断服务函数个数*/
	u32_t int_flag;								/*标记*/
	u32_t int_status;							/*状态*/
	u32_t int_pndbitnr;							/*中断在中断挂起寄存器的位序*/
	uint_t int_nr;								/*中断号*/
	u64_t int_occurcnt;							/*中断发生次数*/
}interrupt_dscp_t;

typedef struct interrupt_action
{
	spinlock_t int_lock;
	struct list_head int_list;					/*挂载中断的链表*/
	u32_t int_flag;								/*中断标志（中断触发类型，中断是否可共享）*/
	interrupt_dscp_t *int_dscp;					/*中断描述符*/
	void *device;								/*设备指针*/
	void *private_data;							/*私有数据指针*/
	const char *name;
	uint_t runcnt;
	irq_handler_t handle;						/*中断回调函数*/
}interrupt_action_t;

typedef struct exitreg_map
{
	uint_t nr_start;			/*中断号起始*/
	uint_t nr_end;				/*中断结束*/
	uint_t oneusedbit;			/*一个中断使用多少位*/
	adr_t con;					/*gpio 配置寄存器*/
	adr_t mode_con;				/*触发方式配置寄存器*/
}exitreg_map_t;

void hal_interrupt_init(void);
bool_t hal_clear_irqpnd(uint_t int_nr);
bool_t hal_enalbe_irq(uint_t int_nr);
bool_t hal_disalbe_irq(uint_t int_nr);
interrupt_dscp_t *hal_rtn_intdscp(uint_t int_nr);
exitreg_map_t *hal_rtn_exitregmap(uint_t int_nr);
void hal_exit_mode_init(uint_t int_nr, uint_t flag);
u32_t hal_getvicirqstatus(u32_t vicnr);

#endif