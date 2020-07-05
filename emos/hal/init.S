#include "config.h"
#include "platform.h"

.equ WTCON, 			0xE2700000
.equ VICINTENCLEAR0, 	0xF2000014
.equ VICINTENCLEAR1, 	0xF2100014
.equ VICINTENCLEAR2, 	0xF2200014
.equ VICINTENCLEAR3, 	0xF2300014

.section .head.text						@下面的代码放在.head.text的段中
.code 32								@产生32位的指令
.extern lmosemhal_start					@硬件初始化C语言函数跳转地址

.global _start
.align 4								@4字节对齐

_start:
	@1、关闭看门狗
	mov r0, #0
	ldr r1, = WTCON
	str r1, [r0]

	@2、设置svc模式，关闭fiq irq
	mrs r0, cpsr
	bic r0, r0, #0x1f					
	orr r0, r0, #0xd
	msr cpsr, r0

	@3、关闭处理器中断
	mov r0, #0xffffffff
	ldr r1, = VICINTENCLEAR0
	str r0, [r1]

	ldr r1, = VICINTENCLEAR1
	str r0, [r1]

	ldr r1, = VICINTENCLEAR2
	str r0, [r1]

	ldr r1, = VICINTENCLEAR3
	str r0, [r1]

	@4、初始化bss段
	@bl clean_bss

	@5、设置堆栈，跳转到C语言
	ldr sp, = INT_STACK_HIGH_ADDR
	bl lmosemhal_start

#if 0
clean_bss:
	ldr r0, = __begin_bss
	ldr r1, = __end_bss

	mov r2, #0x00000000
clean_loop:	cmp r0, r1

	strcc r2, [r0]		
	addcc r0, r0, #4		//lo == cc

	bcc clean_loop		
	mov pc, lr
#endif 

clean_bss:
	ldr r0, = __begin_bss
	ldr r1, = __end_bss

	mov r2, #0x00000000
clean_loop:	cmp r0, r1

	cmp r0, r1 
    stmltia r0!,{r2}
    blt     clean_loop  	
	mov pc, lr