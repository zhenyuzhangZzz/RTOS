#ifndef __LED_H__
#define __LED_H__

#include "s5pv210reg.h"

#define LEDCON (*(volatile unsigned long*)GPC0CON)
#define LEDDAT (*(volatile unsigned long*)GPC0DAT)


void led_init(void);

void led_change_status(int status);

#endif // LED_H
