#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include "s5pv210reg.h"

typedef void(*irq_handle_p)(void);

void enable_interrupts(void);
void disenable_interrupts(void);

void exit10_init(irq_handle_p irq_handle);

#endif