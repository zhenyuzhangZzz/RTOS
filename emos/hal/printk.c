#include "lmosem.h"

/*************************************************************************
*  函数名称：uart_putchar
*  功能说明：发送一个字符
*  参数说明：UARTn       模块号（UART0~UART3）
			 cabs		 要发送的字符
*  函数返回：无
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
void uart_putchar(UARTn uratn, u8_t c)
{
	uart_send_byte(uratn, c);
	/* 如果只写'\n'，只是换行，而不会跳到下一行开头 */
	if (c == '\n')
		uart_send_byte(uratn, '\r');
}

/*************************************************************************
*  函数名称：uart_recv_byte
*  功能说明：接受一个字符
*  参数说明：UARTn       模块号（UART0~UART3） 	
*  函数返回：接受的字符
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
s32_t uart_getchar(UARTn uratn)
{
	s32_t c;
	c = uart_recv_byte(uratn);
	return c;
}

/*************************************************************************
*  函数名称：uart_putchar
*  功能说明：发送一个字符串
*  参数说明：UARTn       模块号（UART0~UART3）
			 cabs		 要发送的字符串
*  函数返回：无
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
void uart_puts(UARTn uratn, s8_t *str)
{
	s8_t *p = str;
	while (*p)
		uart_putchar(uratn, *p++);
}

/*************************************************************************
*  函数名称：uart_put_s32_t
*  功能说明：将整形发送到终端显示
*  参数说明：UARTn       模块号（UART0~UART3）
			 cabs		 要发送的整形
*  函数返回：无
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
private void uart_put_int(UARTn uratn, u32_t v)
{
	s32_t i;

	s8_t a[10];
	u8_t cnt = 0;
	
	if (v == 0)
	{
		uart_putchar(uratn, '0');
		return;
	}

	while (v)
	{
		a[cnt++] = (s8_t)(v % 10);
		v /= 10; 
	}

	for (i = cnt - 1; i >= 0; i--)
		uart_putchar(uratn, a[i] + '0');

	/* 整数0-9的ASCII分别为0x30-0x39 */
}

/*************************************************************************
*  函数名称：uart_put_hex
*  功能说明：将8位整形发送到终端, 以十六进制显示
*  参数说明：UARTn       模块号（UART0~UART3）
			 cabs		 要发送的整形
			 small		 小写或大写显示
*  函数返回：无
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
private void uart_put_hex(UARTn uratn, u8_t v, u8_t small)
{
	u8_t h, l;								/* 高4位和第4位(这里按二进制算) */
	s8_t *hex1 = "0123456789abcdef";		/* 这里放在数据段中 */
	s8_t *hex2 = "0123456789ABCDEF";

	h = v >> 4;
	l = v & 0x0F;

	if (small)	/* 小写 */
	{
		uart_putchar(uratn, hex1[h]);	/* 高4位 */
		uart_putchar(uratn, hex1[l]);	/* 低4位 */
	}
	else		/* 大写 */
	{
		uart_putchar(uratn, hex2[h]);	/* 高4位 */
		uart_putchar(uratn, hex2[l]);	/* 低4位 */
	}
}

/*************************************************************************
*  函数名称：uart_put_s32_t_hex
*  功能说明：将32位整形发送到终端, 以十六进制显示
*  参数说明：UARTn       模块号（UART0~UART3）
			 cabs		 要发送的整形
			 small		 小写或大写显示
*  函数返回：无
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
private void uart_put_int_hex(UARTn uratn, u32_t v, u8_t small)
{
	if (v >> 24)
	{
		uart_put_hex(uratn, (u8_t)(v >> 24), small);
		uart_put_hex(uratn, (v >> 16) & 0xFF, small);
		uart_put_hex(uratn, (v >> 8) & 0xFF, small);
		uart_put_hex(uratn, v & 0xFF, small);
	}
	else if ((v >> 16) & 0xFF)
	{
		uart_put_hex(uratn, (v >> 16) & 0xFF, small);
		uart_put_hex(uratn, (v >> 8) & 0xFF, small);
		uart_put_hex(uratn, v & 0xFF, small);
	}
	else if ((v >> 8) & 0xFF)
	{
		uart_put_hex(uratn, (v >> 8) & 0xFF, small);
		uart_put_hex(uratn, v & 0xFF, small);
	}
	else
		uart_put_hex(uratn, v & 0xFF, small);
}

/*************************************************************************
*  函数名称：prs32_tf
*  功能说明：格式化输出
*  参数说明：同C语言prs32_t用法
*  函数返回：s32_t
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
void printk(const s8_t *fmt, ...)
{
	va_list ap;
	s8_t c, *s;
	u8_t small;
	u32_t d;
	cpuflg_t flag;

	local_fiqirq_disable(flag);

	va_start(ap, fmt);

	while (*fmt)
	{
		small = 0;
		c = *fmt++;
		if (c == '%')
		{
			switch (*fmt++)
			{
			case 'c':              				/* s8_t */
				c = (char) va_arg(ap, s32_t);
				uart_putchar(WORK_UART, c);
				break;
			case 's':              				/* string */
				s = va_arg(ap, s8_t *);
				uart_puts(WORK_UART, s);
				break;
			case 'd':              				
			case 'u':
				d = va_arg(ap, u32_t);
				uart_put_int(WORK_UART, d);
				break;
			case 'x':
				small = 1;						//small
			case 'X':
				d = va_arg(ap, u32_t);
				uart_put_int_hex(WORK_UART, d, small);
				break;
			}
		}
		else
			uart_putchar(WORK_UART, c);
	}
	va_end(ap);
	local_fiqirq_restore(flag);
}