#include "uart.h"
#include "led.h"
#include "timer.h"
#include "dm9000.h"
#include "net.h"
#include "nand.h"

#define BOOTDELAY 		3
#define KRL_START_ADDR 	0x20008000
#define KRL_STORE_ADDR	0x100000
#define KEL_SIZE		0x40000

void (*krnl_fnc)(void) = (void (*)(void))KRL_START_ADDR;

//下载命令
/*
set serverip 192.168.0.20;set ipaddr 192.168.0.30; tftp 30008000 bootloader.bin; go 30008000
tftp 20000000 bootloaderhead.bin; nand erase 0 4000; nand write 20000000 0 4000
tftp 20000000 led_glide_210.bin; nand erase 0 1000; nand write 20000000 0 1000
*/

void nand_boot_linux(void)
{
	printf("\nloading kernel from 0x%x to 0x20008000...\n", KRL_STORE_ADDR);

	nand_read((unsigned char *)KRL_START_ADDR, KRL_STORE_ADDR, KEL_SIZE);

	printf("boot kernel ...\n");

	krnl_fnc();
}

void printinfo(void)
{
	printf("\n\n\n#########	 uboot	################\n\r");
	
	printf("1. download kernel or bin from TFTP to nand flash.  \n\r");
	printf("2. download program from TFTP to ram and run.\n\r");
	printf("3. boot kernel from nand flash.\n\r");
	printf("4. reverse led.\n\r");
	printf("uboot# ");
}

void autoboot(void)
{
	int i;

	char bootdelay = BOOTDELAY;

	printf("Hit any key to stop autoboot: ");

	while(1)
	{	
		printf("%d", bootdelay);

		if(bootdelay == 0)
		{
			nand_boot_linux();

			return;
		}

		for(i = 0; i < 10; i++)
		{
			delay_ms(100);	

			if(uart_in())				//串口有输入
			{
				uart_getchar();			//读取掉
				return;
			}
		}

		bootdelay--;

		uart_puts("\b");
	}
}

int main(void)
{	
	int command;

	int ledstatus = 0;

	/*初始化串口0,波特率115200*/
	uart_init(115200);

	/*led初始化*/
	led_init();

	/*网卡初始化*/
	dm9000_init();

	/*定时器0初始化做延时*/
	timer_init(TIME0, 0.001);		//1ms定时

	printinfo();

	/*自启动*/
	autoboot();

	while(1)
	{
		printinfo();

		command = uart_getchar();

		switch(command - '0')
		{
			case 1:
				tftp_download_nand(KRL_STORE_ADDR);					//tftp烧写内核
				break;
			case 2:	
				tftp_download_run();								//tftp下载调试内核
				break;
			case 3:
				nand_boot_linux();									//内核自启动
				break;
			case 4:
			    ledstatus = !ledstatus;
				led_change_status(ledstatus);						//测试
   				break;
			default:
				printf("enter error select!\r\n");
				break;
		}
	}
    return 0;    
}
