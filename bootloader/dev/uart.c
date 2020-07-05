#include "uart.h"

volatile unsigned int UDIVSLOTn_NUM[] = {0x0000, 0x0080, 0x0808, 0x0888, 0x2222, 0x4924, 0x4A52, 0x54AA,
										 0x5555, 0xD555, 0xD5D5, 0xDDD5, 0xDDDD, 0xDFDD, 0xDFDF, 0xFFDF};

unsigned char uart_buffer[100];

/*************************************************************************
*  函数名称：uart_init
*  功能说明：初始化串口0
*  参数说明：baud        波特率，如9600、19200、56000、115200等
*  函数返回：无
*  修改时间：2014年10月20日19:41:23
*************************************************************************/
void uart_init(unsigned int baud)
{
	unsigned int DIV_VAL_DEC = 0;
	/*GPA0_0和GPA0_1配置为串口模式*/
	var_int(GPA0CON) &= ~0xFF;
	var_int(GPA0CON) |= 0x22;
	
	 /*设置数据位：8位, 停止位:1, 校验位:无, 收发方式: Normal mode operation*/
	var_int(ULCON0) = 0x3 | (0 << 2) | (0 << 3) | (0 << 6);
	/*串口工作方式: 轮询或者中断，时钟: PCLK*/
	var_int(UCON0) = 1 | (1 << 2) | (0 << 10);

	/*屏蔽发送和接收中断位*/
	var_int(UINTM0) = 0x01 | 0x01 << 2;

	/*关闭FIFO*/
	var_int(UFCON0) = 0x00;
	
	/*计算波特率*/
	var_int(UBRDIV0) = (unsigned int)(66000000/(baud * 16)-1);       		   					/*波特率计算的整数部分*/
	DIV_VAL_DEC = (unsigned int)((66000000.0/(baud * 16) - 66000000/(baud * 16)) * 16);		    /*小数部分转换成整数以便查表*/	
	var_int(UDIVSLOT0) = UDIVSLOTn_NUM[DIV_VAL_DEC];
}

/*************************************************************************
*  函数名称：uart_send_byte
*  功能说明：发送一字节数据
*  参数说明：UARTn       模块号（UART0~UART3） 
			 byte		要发生的一字节数据
*  函数返回：无
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
static void uart_send_byte(unsigned char byte)
{
	while (!(var_int(UTRSTAT0) & (1 << 2)));	/* 等待发送缓冲区为空 */
	var_byte(UTXH0) = byte;
}

/*************************************************************************
*  函数名称：uart_recv_byte
*  功能说明：接受一字节数据
*  参数说明：UARTn       模块号（UART0~UART3） 	
*  函数返回：接受的一字节数据
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
static unsigned char uart_recv_byte()
{
	while (!(var_int(UTRSTAT0) & 1));		/* 等待接收缓冲区有数据可读 */
	return var_byte(URXH0);					/* 接收一字节数据 */
}

unsigned char uart_in(void)
{
	return var_int(UTRSTAT0) & 0x01;
}
/*************************************************************************
*  函数名称：uart_putchar
*  功能说明：发送一个字符
*  参数说明：UARTn       模块号（UART0~UART3）
			 cabs		 要发送的字符
*  函数返回：无
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
void uart_putchar(int c)
{
	uart_send_byte(c);
	/* 如果只写'\n'，只是换行，而不会跳到下一行开头 */
	if (c == '\n')
		uart_send_byte('\r');
}

/*************************************************************************
*  函数名称：uart_recv_byte
*  功能说明：接受一个字符
*  参数说明：UARTn       模块号（UART0~UART3） 	
*  函数返回：接受的字符
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
unsigned char uart_getchar()
{
	unsigned char c;
	c = uart_recv_byte();
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
void uart_puts(char *str)
{
	char *p = str;
	while (*p)
		uart_putchar(*p++);
}

/*************************************************************************
*  函数名称：uart_gets
*  功能说明：获取字符串并回显
*  函数返回：获取的字符串个数
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
int uart_gets(void)
{
	unsigned char c;

	int rtn, buffer_ptr = 0;

	for(;;)
	{
		c = uart_getchar();

		if(c == '\b')							//退格
		{
			if(buffer_ptr > 0)
			{
				buffer_ptr--;
				uart_putchar(c);				
				uart_putchar(' ');				//输出空格
				uart_putchar(c);
			}
			continue;						
		}
		else if(c == '\r')
		{
			uart_buffer[buffer_ptr] = '\0'; 	//取代回车
			break;
		}

		uart_buffer[buffer_ptr++] = c;

		uart_putchar(c);
	}

	rtn = buffer_ptr;

	return rtn;
}

unsigned char *uart_get_buffer(void)
{
	return uart_buffer;
}

/*************************************************************************
*  函数名称：uart_put_int
*  功能说明：将整形发送到终端显示
*  参数说明：UARTn       模块号（UART0~UART3）
			 cabs		 要发送的整形
*  函数返回：无
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
static void uart_put_int(unsigned int v)
{
	int i;
	unsigned char a[10];
	unsigned char cnt = 0;
	
	if (v == 0)
	{
		uart_putchar('0');
		return;
	}

	while (v)
	{
		a[cnt++] = v % 10;
		v /= 10; 
	}

	for (i = cnt - 1; i >= 0; i--)
		uart_putchar(a[i] + 0x30);
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
static void uart_put_hex(unsigned char v, unsigned char small)
{
	/* 注意：必须用volatile修饰，否则会出错 */
	unsigned char h, l;		/* 高4位和第4位(这里按二进制算) */
	char *hex1 = "0123456789abcdef";		/* 这里放在数据段中 */
	char *hex2 = "0123456789ABCDEF";

	h = v >> 4;
	l = v & 0x0F;

	if (small)	/* 小写 */
	{
		uart_putchar(hex1[h]);	/* 高4位 */
		uart_putchar(hex1[l]);	/* 低4位 */
	}
	else		/* 大写 */
	{
		uart_putchar(hex2[h]);	/* 高4位 */
		uart_putchar(hex2[l]);	/* 低4位 */
	}
}

/*************************************************************************
*  函数名称：uart_put_int_hex
*  功能说明：将32位整形发送到终端, 以十六进制显示
*  参数说明：UARTn       模块号（UART0~UART3）
			 cabs		 要发送的整形
			 small		 小写或大写显示
*  函数返回：无
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
static void uart_put_int_hex(unsigned int v, unsigned char small)
{
	if (v >> 24)
	{
		uart_put_hex(v >> 24, small);
		uart_put_hex((v >> 16) & 0xFF, small);
		uart_put_hex((v >> 8) & 0xFF, small);
		uart_put_hex(v & 0xFF, small);
	}
	else if ((v >> 16) & 0xFF)
	{
		uart_put_hex((v >> 16) & 0xFF, small);
		uart_put_hex((v >> 8) & 0xFF, small);
		uart_put_hex(v & 0xFF, small);
	}
	else if ((v >> 8) & 0xFF)
	{
		uart_put_hex((v >> 8) & 0xFF, small);
		uart_put_hex(v & 0xFF, small);
	}
	else
		uart_put_hex(v & 0xFF, small);
}

/*************************************************************************
*  函数名称：printf
*  功能说明：格式化输出
*  参数说明：同C语言print用法
*  函数返回：int
*  修改时间：2014-6-25
*  备    注：
*************************************************************************/
int printf(const char *fmt, ...)
{
	va_list ap;
	char c;
	char *s;
	unsigned int d;
	unsigned char small;
	unsigned int count = 0;

	va_start(ap, fmt);
	while (*fmt)
	{
		small = 0;
		c = *fmt++;
		if (c == '%')
		{
			switch (*fmt++)
			{
			case 'c':              /* char */
				c = (char) va_arg(ap, int);
				uart_putchar(c);
				break;
			case 's':              /* string */
				s = va_arg(ap, char *);
				uart_puts(s);
				break;
			case 'd':              /* int */
			case 'u':
				d = va_arg(ap, int);
				uart_put_int(d);
				break;
			case 'x':
				small = 1;	// small
			case 'X':
				d = va_arg(ap, int);
				uart_put_int_hex(d, small);
				break;
			}
		}
		else
			uart_putchar(c);
		count++;
	}
	va_end(ap);
	return count;
}

/*用户不要关心*/
int raise(int signum)
{
	return 0;
}
