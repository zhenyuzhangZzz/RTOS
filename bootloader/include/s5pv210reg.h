#ifndef __S5PV210_REG_H__
#define __S5PV210_REG_H__

/*210看门狗基地址*/
#define WATCHDOG_BASE_ADDR 0xE2700000

/*系统时钟相关*/
#define APLL_CON0		0xE0100100
#define MPLL_CON		0xE0100108
#define EPLL_CON0		0xE0100110
#define VPLL_CON		0xE0100120
#define CLK_SRC0 		0xE0100200
#define CLK_DIV0		0xE0100300

/*led相关*/
#define GPC0CON			0xE0200060
#define GPC0DAT			0xE0200064

/*按键1*/
#define GPH1CON			0xE0200C20

/*串口0端口配置寄存器*/
#define GPA0CON			0xE0200000
#define ULCON0			0xE2900000
#define UCON0			0xE2900004
#define UFCON0			0xE2900008
#define UTRSTAT0		0xE2900010
#define UBRDIV0			0xE2900028
#define UDIVSLOT0		0xE290002C
#define UTXH0			0xE2900020
#define URXH0			0xE2900024
#define	UINTM0			0xE2900038

/*nandflash相关寄存器*/
/*NAND Pins*/
#define	NFCONF  		0xB0E00000
#define	NFCONT  		0xB0E00004	
#define	NFCMMD  		0xB0E00008
#define	NFADDR 			0xB0E0000C
#define	NFDATA  		0xB0E00010
#define	NFSTAT  		0xB0E00028

//ecc
#define	NFECCCONF		0xB0E20000
#define NFECCCONT 		0xB0E20020
#define NFECCSTAT		0xB0E20030
#define NFECCPRGECC0	0xB0E20090
#define NFECCPRGECC1	0xB0E20094
#define NFECCPRGECC2	0xB0E20098
#define NFECCPRGECC3	0xB0E2009C

/* NAND GPIO Pins */
#define MP0_1CON  		0xE02002E0
#define	MP0_3CON  		0xE0200320
#define	MP0_6CON  		0xE0200380


/*DMC0寄存器地址*/
#define DMC0_BASE			0xF0000000
#define DMC0_CONCONTROL		(DMC0_BASE + 0x00)
#define DMC0_MEMCONTROL		(DMC0_BASE + 0x04)
#define DMC0_MEMCONFIG0		(DMC0_BASE + 0x08)
#define DMC0_MEMCONFIG1		(DMC0_BASE + 0x0C)
#define DMC0_DIRECTCMD		(DMC0_BASE + 0x10)
#define DMC0_PRECHCONFIG	(DMC0_BASE + 0x14)
#define DMC0_PHYCONTROL0 	(DMC0_BASE + 0x18)
#define DMC0_PHYCONTROL1 	(DMC0_BASE + 0x1C)
#define DMC0_PWRDNCONFIG 	(DMC0_BASE + 0x28)
#define DMC0_TIMINGAREF 	(DMC0_BASE + 0x30)
#define DMC0_TIMINGROW 		(DMC0_BASE + 0x34)
#define DMC0_TIMINGDATA 	(DMC0_BASE + 0x38)
#define DMC0_TIMINGPOWER 	(DMC0_BASE + 0x3C)
#define DMC0_PHYSTATUS 		(DMC0_BASE + 0x40)
#define DMC0_CHIP0STATUS 	(DMC0_BASE + 0x48)
#define DMC0_CHIP1STATUS 	(DMC0_BASE + 0x4C)
#define DMC0_AREFSTATUS 	(DMC0_BASE + 0x50)
#define DMC0_MRSTATUS 		(DMC0_BASE + 0x54)

/*DMC1寄存器地址*/
#define DMC1_BASE			0xF1400000
#define DMC1_CONCONTROL		(DMC1_BASE + 0x00)
#define DMC1_MEMCONTROL		(DMC1_BASE + 0x04)
#define DMC1_MEMCONFIG0		(DMC1_BASE + 0x08)
#define DMC1_MEMCONFIG1		(DMC1_BASE + 0x0C)
#define DMC1_DIRECTCMD		(DMC1_BASE + 0x10)
#define DMC1_PRECHCONFIG	(DMC1_BASE + 0x14)
#define DMC1_PHYCONTROL0 	(DMC1_BASE + 0x18)
#define DMC1_PHYCONTROL1 	(DMC1_BASE + 0x1C)
#define DMC1_PWRDNCONFIG 	(DMC1_BASE + 0x28)
#define DMC1_TIMINGAREF 	(DMC1_BASE + 0x30)
#define DMC1_TIMINGROW 		(DMC1_BASE + 0x34)
#define DMC1_TIMINGDATA 	(DMC1_BASE + 0x38)
#define DMC1_TIMINGPOWER 	(DMC1_BASE + 0x3C)
#define DMC1_PHYSTATUS 		(DMC1_BASE + 0x40)
#define DMC1_CHIP0STATUS 	(DMC1_BASE + 0x48)
#define DMC1_CHIP1STATUS 	(DMC1_BASE + 0x4C)
#define DMC1_AREFSTATUS 	(DMC1_BASE + 0x50)
#define DMC1_MRSTATUS 		(DMC1_BASE + 0x54)

/*外部中断*/
#define 	EXT_INT_1_CON			0xE0200E04
#define 	EXT_INT_1_MASK			0xE0200F04
#define 	VIC0INTSELECT			0xF200000C
#define 	VIC0INTENABLE			0xF2000010
#define 	VIC0INTENCLEAR			0xF2000014
#define 	VIC0VECTADDR10			0xF2000128
#define		VIC0VECTPRIORITY10		0xF2000228			//优先级
#define 	VIC0ADDRESS  			0xF2000F00
#define 	EXT_INT_1_PEND			0xE0200F44

/*dm9000 SROM bank1*/
#define SROM_BW				0xE8000000
#define SROM_BC1			0xE8000008

/*  定时器0、1、2、3、4公共的配置寄存器 */
#define		PWMTIMER_BASE			0xE2500000
#define		TCFG0    				(PWMTIMER_BASE+0x00)
#define		TCFG1    				(PWMTIMER_BASE+0x04)
#define		TCON      				(PWMTIMER_BASE+0x08)

/* 定时器0寄存器 */
#define		TCNTB0    				(PWMTIMER_BASE+0x0C)
#define		TCMPB0    				(PWMTIMER_BASE+0x10)
#define		TCNTO0    				(PWMTIMER_BASE+0x14)

/* 定时器1寄存器 */
#define		TCNTB1    				(PWMTIMER_BASE+0x18)
#define		TCMPB1    				(PWMTIMER_BASE+0x1C)
#define		TCNTO1    				(PWMTIMER_BASE+0x20)

/* 定时器2寄存器 */
#define		TCNTB2    				(PWMTIMER_BASE+0x24)
#define		TCMPB2    				(PWMTIMER_BASE+0x28)
#define		TCNTO2    				(PWMTIMER_BASE+0x2C)

/* 定时器3寄存器 */
#define		TCNTB3    				(PWMTIMER_BASE+0x30)
#define		TCMPB3    				(PWMTIMER_BASE+0x34)
#define		TCNTO3    				(PWMTIMER_BASE+0x38)

/* 定时器4寄存器 */
#define		TCNTB4    				(PWMTIMER_BASE+0x3C)
#define		TCNTO4    				(PWMTIMER_BASE+0x40)

#define 	VIC0VECTADDR21			0xF2000154
/* 中断控制和状态寄存器 */
#define		TINT_CSTAT 				(PWMTIMER_BASE+0x44)


#define var_int(addr) (*(volatile unsigned int *)(addr))
#define var_word(addr) (*(volatile unsigned short *)(addr))
#define var_byte(addr) (*(volatile unsigned char *)(addr))

#endif
