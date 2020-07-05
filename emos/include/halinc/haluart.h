#ifndef __HALUART_H__
#define __HALUART_H__

typedef enum  UARTn
{//初始化默认配置       --TXD--      --RXD--     可以复用其他通道，请自行修改 uart_init
  UART0,    //           GPA0_0       GPA0_1
  UART1,    //           GPA0_4       GPA0_5
  UART2,    //           GPA1_0       GPA1_1
  UART3,    //           GPA1_2       GPA1_3
}UARTn;

void hal_uart_init(void);
void uart_send_byte(UARTn uratn, u8_t byte);
u8_t uart_recv_byte(UARTn uratn);

#endif