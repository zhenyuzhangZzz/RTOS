#include "lmosem.h"

volatile adr_t uart_base; 
u32_t UDIVSLOTn_NUM[] = {0x0000, 0x0080, 0x0808, 0x0888, 0x2222, 0x4924, 0x4A52, 0x54AA,
								0x5555, 0xD555, 0xD5D5, 0xDDD5, 0xDDDD, 0xDFDD, 0xDFDF, 0xFFDF};

/*************************************************************************
*  函数名称：uart_init
*  功能说明：初始化串口，设置波特率
*  参数说明：UARTn       模块号（UART0~UART3）
*            baud        波特率，如9600、19200、56000、115200等
*  函数返回：无
*  修改时间：2018-6-25
*************************************************************************/
void uart_init (UARTn uratn, u32_t baud)
{
	u32_t DIV_VAL_DEC;                             /*小数部分*/   
 
	switch(uratn)        						/*将对应引脚配置成串口模式*/
	{
		case UART0:
			writel(GPA0CON, (readl(GPA0CON) & ~0xFF) | 0x2222);
			uart_base = UART0_BASE;
			break;
		case UART1:
			writel(GPA0CON, (readl(GPA0CON) & ~(0xFF << 16)) | (0x2222 << 16));
			uart_base = UART1_BASE;
			break;
		case UART2:
			writel(GPA1CON, (readl(GPA1CON) & ~0xFF) | 0x22);
			uart_base = UART2_BASE;
			break;
		case UART3:
			writel(GPA1CON, (readl(GPA1CON) & ~(0xFF << 8)) | (0x22 << 8));
			uart_base = UART3_BASE;
			break;
	}
	writel(uart_base + ULCON, 0x3 | (0 << 2) | (0 << 3) | (0 << 6));	/*设置起始位, 停止位, 校验位, 收发方式*/
	writel(uart_base + UCON, 1 | (1 << 2) | (0 << 10));					/*串口工作方式, 时钟*/
	writel(uart_base + UFCON, 0x01);									/*启动FIFO*/                                
	
	/*计算波特率*/
	writel(uart_base + UBRDIV, (u32_t)(66000000/(baud * 16)-1));						/*波特率计算的整数部分*/      		   			
	DIV_VAL_DEC = (u32_t)((66000000.0/(baud * 16) - 66000000/(baud * 16)) * 16);		/*小数部分转换成整数以便查表*/	
	writel(uart_base + UDIVSLOT, UDIVSLOTn_NUM[DIV_VAL_DEC]);
}

/*************************************************************************
*  函数名称：uart_send_byte
*  功能说明：发送一字节数据
*  参数说明：UARTn       模块号（UART0~UART3） 
			 byte		要发生的一字节数据
*  函数返回：无
*  修改时间：2018-6-25
*  备    注：
*************************************************************************/
void uart_send_byte(UARTn uratn, u8_t byte)
{	
	while (!(readl(uart_base + UTRSTAT) & (1 << 2)));			/* 等待发送缓冲区为空 */
	writel(uart_base + UTXH, byte);
}

/*************************************************************************
*  函数名称：uart_recv_byte
*  功能说明：接受一字节数据
*  参数说明：UARTn       模块号（UART0~UART3） 	
*  函数返回：接受的一字节数据
*  修改时间：2018-6-25
*  备    注：
*************************************************************************/
u8_t uart_recv_byte(UARTn uratn)
{

	while (!(readl(uart_base + UTRSTAT) & 1));				/* 等待接收缓冲区有数据可读 */
	return readb(uart_base + URXH);							/* 接收一字节数据 */		
}

void hal_uart_init(void)
{
	uart_init(WORK_UART, BAUDRATE);
	return;
}
