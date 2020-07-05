#ifndef __UART_H__
#define __UART_H__

#include "s5pv210reg.h"

#define NULL 0

/*
	用于实现可变参数
*/
#define va_list char *
#define _va_sizeof(TYPE) (((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#define va_start(AP, TARGET) (AP = (char *)(&TARGET) + _va_sizeof(TARGET))
#define va_arg(AP, TYPE) (AP += _va_sizeof(TYPE), *((TYPE *)(AP - _va_sizeof(TYPE))))
#define va_end(AP) (AP = (char *)NULL)


void uart_init(unsigned int baud);
unsigned char uart_in(void);
unsigned char uart_getchar();
int printf(const char *fmt, ...);
void uart_puts(char *str);
int uart_gets(void);
unsigned char *uart_get_buffer(void);

#endif
