.section .lmosem_hal.vector, "ax"
.code 32
.align 4
.extern hal_undefine
.extern hal_swi
.extern hal_prefetch_abrot
.extern hal_data_abrot
.extern no_use
.extern hal_irq
.extern hal_fiq
.global vector
vector:
	ldr pc, hal_start_l									@复位异常
	ldr pc, hal_undefine_l								@未定义指令异常
	ldr pc, hal_swi_l									@软中断，用于系统调用陷入内核
	ldr pc, hal_prefetch_abrot_l						@预取指令异常
	ldr pc, hal_data_abrot_l							@数据访问终止异常
	ldr pc, no_use_l								
	ldr pc, hal_irq_l									@外部中断
	ldr pc, hal_fiq_l									@快速中断

hal_start_l: .word _start
hal_undefine_l: .word hal_undefine
hal_swi_l: .word hal_swi
hal_prefetch_abrot_l: .word hal_prefetch_abrot
hal_data_abrot_l: .word hal_data_abrot
no_use_l: .word no_use
hal_irq_l: .word hal_irq
hal_fiq_l: .word hal_fiq
