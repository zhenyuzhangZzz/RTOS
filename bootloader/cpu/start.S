#include "s5pv210reg.h"

.text
.globl _start

_start:
	b reset
	ldr pc, undefined_instruction
	ldr pc, software_interrupt
	ldr pc, prefetch_abrot
	ldr pc, data_abrot
	ldr pc, no_use
	ldr pc, irq_interrupt
	ldr pc, fiq_interrupt

undefined_instruction: .word _undefined_instruction
software_interrupt: .word _software_interrupt
prefetch_abrot: .word _prefetch_abrot
data_abrot: .word _data_abrot
irq_interrupt: .word _irq_interrupt
fiq_interrupt: .word _fiq_interrupt

/*未定义指令异常*/
_undefined_instruction:
	nop
/*软中断异常*/
_software_interrupt:
	nop

/*预取指令异常*/
_prefetch_abrot:
	nop

/*访问存储器异常*/
_data_abrot:
	nop

/*未定义位*/
no_use:
	nop

/* irq中断 */
#define IRQ_STACK 0xD0035400
_irq_interrupt:
	ldr sp, = IRQ_STACK			/*设置IRQ模式下的堆栈*/
	sub lr, lr, #4				/* 因为流水线，PC的值等于当前执行的地址 + 8，那么返回值就等于PC - 4*/
	stmfd sp!, {r0-r12, lr}  	/*保护现场*/
	bl irq_handler
	ldmfd sp!, {r0-r12, pc}^  	/*^表示把spsr恢复到cpsr*/

/*fiq中断*/
_fiq_interrupt:
	nop

reset:
	ldr sp, = IRQ_STACK

	bl set_svc    				/*设置CPU工作在SVC模式，关闭irq fiq中断*/

	bl set_vector				/*设置向量表*/

	bl close_watchdog   		/*S5PV210关闭看门狗*/

	bl close_mmu            	/*关闭mmu*/

	bl init_clock				/*初始化时钟*/

	bl init_memory				/*初始化内存*/

	bl nand_init				/*初始化nand flash*/

	/*代码段大小*/
	ldr r0, = __begin_bss
	ldr r1, = _start - 16		/*因为代码有16byte的头信息不需要*/
	sub r0, r0, r1

	bl copy_code_to_ram			/*nand启动, 将代码拷贝到内存*/

	bl clean_bss				/*清除bss段*/

	ldr sp, =0x60000000			/*内存的最高地址*/

	ldr pc, = main				/*跳到BL2执行*/

set_svc:
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd3
	msr cpsr, r0
	mov pc, lr

close_watchdog:
	ldr r0, = WATCHDOG_BASE_ADDR
	mov r1, #0x00
	str r1, [r0]
	mov pc, lr

/*
*1.使Dcache失效，刚开始运行时内存还没初始化好，dcache里的数据是无效的
*2.关闭Icache和Dcache, MMU
*/
close_mmu:
	mov	r0, #0			@ set up for MCR
	mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs
	mcr	p15, 0, r0, c7, c5, 0	@ invalidate icache
	mcr	p15, 0, r0, c7, c5, 6	@ invalidate BP array
	mcr     p15, 0, r0, c7, c10, 4	@ DSB
	mcr     p15, 0, r0, c7, c5, 4	@ ISB

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
	bic	r0, r0, #0x00000007	@ clear bits 2:0 (-CAM)
	orr	r0, r0, #0x00000002	@ set bit 1 (--A-) Align
	orr	r0, r0, #0x00000800	@ set bit 11 (Z---) BTB
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-cache
	mov pc, lr

clean_bss:
	ldr r0, = __begin_bss
	ldr r1, = __end_bss

	mov r2, #0x00000000
clean_loop:	cmp r0, r1

	strlo r2, [r0]		
	addlo r0, r0, #4		//lo == cc

	blo clean_loop		
	mov pc, lr

#define CR_V (1 << 13)

set_vector:	
	/* Set vector address in CP15 VBAR register */
	mrc	p15, 0, r0, c1, c0, 0	@ Read CP15 SCTRL Register
	bic	r0, #CR_V		@ V = 0
	mcr	p15, 0, r0, c1, c0, 0	@ Write CP15 SCTRL Register

	ldr	r0, =_start
	mcr	p15, 0, r0, c12, c0, 0	@Set VBAR
	mov pc, lr

#define GPC0CON 0xE0200060
#define GPC0DAT 0xE0200064
light_led:
	ldr r0, = GPC0CON
	ldr r1, [r0]
	orr r1, r1, #0xFF000
	bic r1, r1, #0xEE000
	str r1, [r0]
	
	ldr r0, = GPC0DAT
	ldr r1, = 0x18
	str r1, [r0]
	mov pc, lr
