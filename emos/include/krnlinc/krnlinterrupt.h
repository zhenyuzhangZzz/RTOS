#ifndef __KRNLINTERRUPT_H__
#define __KRNLINTERRUPT_H__


bool_t request_irq(unsigned int irq, irq_handler_t handler, void *device, unsigned long flags, const char *name, void *dev);
bool_t free_irq(unsigned int irq, void *dev_id);

void interrupt_test(void);

#endif