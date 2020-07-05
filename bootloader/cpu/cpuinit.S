#include "s5pv210reg.h"

.globl init_memory

init_memory:
	@DMC0第一步
	ldr r0, = DMC0_PHYCONTROL0
	ldr r1, = 0x00101000
	str r1, [r0]
	
	ldr r1, = 0x00101002
	str r1, [r0]
	
	ldr r2, = DMC0_PHYCONTROL1
	ldr r1, = 0x00000086
	str r1, [r2]
	
	ldr r1, = 0x00101003
	str r1, [r0]
	@等待时钟锁定
wait_loop0:
	ldr r0, = DMC0_PHYSTATUS
	ldr r1, [r0]
	bic r1, r1, #0xFFFFFFF8
	cmp r1, #0x07
	bne wait_loop0
	
	ldr r0, = DMC0_CONCONTROL
	ldr r1, = 0x0FFF2350
	str r1, [r0]
	
	ldr r0, = DMC0_MEMCONTROL
	ldr r1, = 0x00202430
	str r1, [r0]
	
	ldr r0, = DMC0_MEMCONFIG0
	ldr r1, = 0x20E01323
	str r1, [r0]
	
	ldr r0, = DMC0_PRECHCONFIG
	ldr r1, = 0xFF000000
	str r1, [r0]
	
	ldr r0, = DMC0_PWRDNCONFIG
	ldr r1, = 0xFFFF00FF
	str r1, [r0]
	
	ldr r0, = DMC0_TIMINGAREF
	ldr r1, = 0x00000618
	str r1, [r0]
	
	ldr r0, = DMC0_TIMINGROW
	ldr r1, = 0x19233309
	str r1, [r0]
	
	ldr r0, = DMC0_TIMINGDATA
	ldr r1, = 0x23240204
	str r1, [r0]
	
	ldr r0, = DMC0_TIMINGPOWER
	ldr r1, = 0x09C80232
	str r1, [r0]
	
	ldr r0, = DMC0_DIRECTCMD
	ldr r1, = 0x07000000
	str r1, [r0]
	ldr r1, = 0x01000000
	str r1, [r0]
	ldr r1, = 0x00020000
	str r1, [r0]
	ldr r1, = 0x00030000
	str r1, [r0]
	ldr r1, = 0x00010400
	str r1, [r0]
	ldr r1, = 0x00000542
	str r1, [r0]
	ldr r1, = 0x01000000
	str r1, [r0]
	ldr r1, = 0x05000000
	str r1, [r0]
	ldr r1, = 0x05000000
	str r1, [r0]
	ldr r1, = 0x00000442
	str r1, [r0]
	ldr r1, = 0x00010780
	str r1, [r0]
	ldr r1, = 0x00010400
	str r1, [r0]
	
	ldr r0, = DMC0_CONCONTROL
	ldr r1, = 0x0FF02030
	str r1, [r0]
	
	ldr r0, = DMC0_PWRDNCONFIG
	ldr r1, = 0xFFFF00FF
	str r1, [r0]
	
	ldr r0, = DMC0_MEMCONTROL
	ldr r1, = 0x00202400
	str r1, [r0]
	
	
	@DMC1初始化
	ldr r0, = DMC1_PHYCONTROL0
	ldr r1, = 0x00101000
	str r1, [r0]
	
	ldr r1, = 0x00101002
	str r1, [r0]
	
	ldr r2, = DMC1_PHYCONTROL1
	ldr r1, = 0x00000086
	str r1, [r2]
	
	ldr r1, = 0x00101003
	str r1, [r0]
	@等待时钟锁定
wait_loop1:
	ldr r0, = DMC1_PHYSTATUS
	ldr r1, [r0]
	bic r1, r1, #0xFFFFFFF8
	cmp r1, #0x07
	bne wait_loop1
	
	ldr r0, = DMC1_CONCONTROL
	ldr r1, = 0x0FFF2350
	str r1, [r0]
	
	ldr r0, = DMC1_MEMCONTROL
	ldr r1, = 0x00202430
	str r1, [r0]
	
	ldr r0, = DMC1_MEMCONFIG0
	ldr r1, = 0x40E01323
	str r1, [r0]
	
	ldr r0, = DMC1_PRECHCONFIG
	ldr r1, = 0xFF000000
	str r1, [r0]
	
	ldr r0, = DMC1_PWRDNCONFIG
	ldr r1, = 0xFFFF00FF
	str r1, [r0]
	
	ldr r0, = DMC1_TIMINGAREF
	ldr r1, = 0x00000618
	str r1, [r0]
	
	ldr r0, = DMC1_TIMINGROW
	ldr r1, = 0x19233309
	str r1, [r0]
	
	ldr r0, = DMC1_TIMINGDATA
	ldr r1, = 0x23240204
	str r1, [r0]
	
	ldr r0, = DMC1_TIMINGPOWER
	ldr r1, = 0x09C80232
	str r1, [r0]
	
	ldr r0, = DMC1_DIRECTCMD
	ldr r1, = 0x07000000
	str r1, [r0]
	ldr r1, = 0x01000000
	str r1, [r0]
	ldr r1, = 0x00020000
	str r1, [r0]
	ldr r1, = 0x00030000
	str r1, [r0]
	ldr r1, = 0x00010400
	str r1, [r0]
	ldr r1, = 0x00000542
	str r1, [r0]
	ldr r1, = 0x01000000
	str r1, [r0]
	ldr r1, = 0x05000000
	str r1, [r0]
	ldr r1, = 0x05000000
	str r1, [r0]
	ldr r1, = 0x00000442
	str r1, [r0]
	ldr r1, = 0x00010780
	str r1, [r0]
	ldr r1, = 0x00010400
	str r1, [r0]
	
	ldr r0, = DMC1_CONCONTROL
	ldr r1, = 0x0FF02030
	str r1, [r0]
	
	ldr r0, = DMC1_PWRDNCONFIG
	ldr r1, = 0xFFFF00FF
	str r1, [r0]
	
	ldr r0, = DMC1_MEMCONTROL
	ldr r1, = 0x00202400
	str r1, [r0]
	
	mov pc, lr
	
#if 0	
/*
*nandflash初始化
*/
.globl init_nand

#define NFCONF_DATA ((0x1 << 23) | (0x3 << 12) |(0x2 << 8) | (0x1 << 4) | (0x0 << 3) | (0x0 << 2) |	(0x1 << 1))		

init_nand:
	ldr r0, = NFCONF
	ldr r1, = NFCONF_DATA
	str r1, [r0]
	
	ldr r0, = NFCONT
	ldr r1, = NFCONF_DATA | 0x03
	str r1, [r0]
	
	ldr r0, = MP0_1CON
	ldr r1, [r0]
	ldr r3, = 0xffff00
	bic r1, r1, r3
	ldr r3, = 0x333300
	orr r1, r1, r3
	str r1, [r0]
	
	ldr r0, = MP0_3CON
	ldr r1, = 0x22222222
	str r1, [r0]
	
	ldr r0, = MP0_6CON
	ldr r1, = 0x22222222
	str r1, [r0]
	
	mov pc, lr
#endif

/*
*初始化系统时钟
*/
.globl init_clock

init_clock:
	
	ldr r0, = APLL_CON0
	ldr r1, = (0x03  << 8) | (125 << 16) | (0x01 << 0) | (0x01 << 31) 	/* FOUT_APLL = 1000MHz */
	ldr r1, [r0]
	
	ldr r0, = MPLL_CON
	ldr r1, = (0x0C << 8) | (667 << 16) | (0x01 << 0) | (0x01 << 31) /* FOUT_MPLL = 667MHz */
	ldr r1, [r0]
	
	ldr r0, = EPLL_CON0
	ldr r1, = (0x03  << 8) | (48  << 16) | (0x02 << 0) | (0x01 << 31) 	/* FOUT_EPLL = 96MHz */
	ldr r1, [r0]
	
	ldr r0, = VPLL_CON
	ldr r1, = (0x06  << 8) | (108 << 16) | (0x03 << 0) | (0x01 << 31) 	/* FOUT_VPLL = 54MHz */
	ldr r1, [r0]
	
	/*设置系统时钟源，选择PLL为时钟输出 */
	ldr r0, = CLK_SRC0
	ldr r1, = (0x01 << 0) | (0x01 << 4) | (0x01 << 8) | (0x01 << 12)
	str r1, [r0]
	
	/*设置系统时钟源，选择PLL为时钟输出 */
	ldr r0, = CLK_DIV0
	ldr r1, = (0 << 0) | (4 << 4) | (4 << 8) | (1 << 12) | (3 << 16) | (1 << 20) | (4 << 24) | (1 << 28)
	str r1, [r0]
	
	mov pc ,lr

