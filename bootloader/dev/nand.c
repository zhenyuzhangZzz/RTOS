#include "nand.h"
#include "uart.h"

/* 延时函数 */
void delay(volatile unsigned int count)														
{
    unsigned i,j;
	for(i = 0;i < 10; i++)
		for(j = 0; j < count; j++);
        
}

void wait_idle()
{
	while(!(var_int(NFSTAT) & (1 << 0)));
}

void nand_select_chip()
{
	var_int(NFCONT) &= ~(1<<1);
}

void nand_deselect_chip()
{
	var_int(NFCONT) |= (1<<1);
}

void write_cmd(int cmd)
{
	var_int(NFCMMD) = cmd;
}

void write_addr(unsigned int addr)
{
	unsigned int col = addr % PAGE_SIZE;		/* 页内偏移 */
	unsigned int row = addr / PAGE_SIZE;		/* 页地址 */
	
	var_int(NFADDR) = col & 0xFF;
	var_int(NFADDR) = (col >> 8) & 0x0F;
	var_int(NFADDR) = row & 0xFF;
	var_int(NFADDR) = (row >> 8) & 0xFF;
	var_int(NFADDR) = (row >> 16) & 0x07;
}

unsigned char read_data()
{
	return var_byte(NFDATA);
}

void write_data(unsigned char data)
{
	var_byte(NFDATA) = data;
}

void nand_reset()
{
    nand_select_chip();
    write_cmd(0xff);  // 复位命令
    wait_idle();
    nand_deselect_chip();
}

unsigned char read_nand_status()
{
	unsigned char ret;

	// 1. 发出片选信号  
	nand_select_chip();

	// 2. 读状态  
	write_cmd(0x70);

	ret = read_data();

	// 3. 取消片选  
	nand_deselect_chip();
	return ret;
}

void nand_init()
{

	var_int(NFCONF) =	(0x1 << 23) |	/* Disable 1-bit and 4-bit ECC */
			/* 下面3个时间参数稍微比计算出的值大些（我这里依次加1），否则读写不稳定 */
			(0x3 << 12) |	/* 7.5ns * 2 > 12ns tALS tCLS */
			(0x2 << 8) | 	/* (1+1) * 7.5ns > 12ns (tWP) */
			(0x1 << 4) | 	/* (0+1) * 7.5 > 5ns (tCLH/tALH) */
			(0x0 << 3) | 	/* SLC NAND Flash */
			(0x0 << 2) |	/* 2KBytes/Page */
			(0x1 << 1);		/* 5 address cycle */
			
	
	/*
	 * var_int(NFCONT)[1]:为0时表示禁止片选，为1时表示使能片选
	 * var_int(NFCONT)[0]:为0时表示禁止NAND FLASH控制器，反之使能
 	 */
	var_int(NFCONT) |= (1<<1)|(1<<0);

	/* 设置相应管脚用于Nand Flash控制器*/
	
	var_int(MP0_1CON) &= ~(0xFFFF << 8);
	var_int(MP0_1CON) |= (0x3333 << 8);
	var_int(MP0_3CON) = 0x22222222;
	var_int(MP0_6CON) = 0x22222222;

	/* 复位NAND Flash */
	nand_reset();

	/*使能硬件ecc*/
	return;
}

/* 读ID */
void nand_read_id()
{
	nand_id_msg nand_id;
	/* 片选 */
	nand_select_chip();		
	
	/* 发90命令 */
	write_cmd(0x90);
	
	/* 写00地址 */
	write_addr(0x00);

	nand_id.maker_id  = read_data();
	nand_id.device_id = read_data();
	nand_id.id_3th    = read_data();
	nand_id.id_4th 	  = read_data();
	nand_id.id_5th    = read_data();

	printf("\nmaker_id = %x,device_id = %x\r\n",nand_id.maker_id,nand_id.device_id);
	
	nand_deselect_chip();
}

unsigned char nand_erase(unsigned int addr)
{
	unsigned int temp_addr = addr, row;

	if (temp_addr & (PAGE_SIZE - 1))
	{
		printf("page not aligned, addr must be mulriple 0f 0x800!");
		return -1;
	}

	//获得row地址，即页地址r  
	row = temp_addr / PAGE_SIZE;

	// 1. 发出片选信号  
	nand_select_chip();
	
	/* 2. 擦除：第一个周期发命令0x60，
	 *   第二个周期发块地址，
	 *   第三个周期发命令0xd0 
	 */ 
	write_cmd(0x60);

	// Row Address A12~A19	
	var_int(NFADDR) = row & 0xff;							

	// Row Address A20~A27  
	var_int(NFADDR) = (row >> 8) & 0xff;

	// Row Address A28~A30  
	var_int(NFADDR) = (row >> 16) & 0x0f;		

	write_cmd(0xd0);

	wait_idle();
	
	if(read_nand_status()&0x01)
	{
		/* 取消片选信号 */
		nand_deselect_chip();					
		printf("\nFail to erase block!\r\n");
		return -1;
	}
	else
	{
		/* 取消片选信号 */
		nand_deselect_chip();	 					
		printf("\nSuccess to erase block\r\n");
		return 0;
	}
}

/* 
	从NAND FLASH第addr页开始读size到buf指定的地址
	addr: 页号(0 -- 524287)
 */
void nand_read(unsigned char *buf, unsigned int addr, int size)
{
	unsigned int i, j;

	unsigned int temp_addr = addr;

	if(temp_addr & (PAGE_SIZE - 1))
	{
		printf("page not aligned, addr must be mulriple 0f 0x800!");
		return;
	}

	/* 选中芯片 */
	nand_select_chip();

	for(i = temp_addr; i < (temp_addr + size);) 
	{
		/* 发出READ0命令 */
		write_cmd(0x00);

		/* Write Address */
		write_addr(i);
		write_cmd(0x30);		
		wait_idle();

		for(j = 0; j < PAGE_SIZE; j++, i++) 
		{
			buf[j] = read_data();	
		}
		buf += 1 << 11;
	}

	nand_deselect_chip();	
}
/* 
	从buf指定的地址写数据到nand flash的addr页开始，写size大小
*/
void nand_write(unsigned char *buf, unsigned int addr, unsigned int size)
{
	unsigned int i, j;

	unsigned int temp_addr = addr;

	if (temp_addr & (PAGE_SIZE - 1))
	{
		printf("page not aligned, addr must be mulriple 0f 0x800!");
		return;
	}	
		
	/*先擦除才能写入*/
	for(i = 0; i < size / PAGE_SIZE + 1; i++)
	{
		nand_erase(temp_addr);
		temp_addr += PAGE_SIZE;
	}

	temp_addr = addr;

	/* 选中芯片 */
	nand_select_chip();

	for(i = temp_addr; i < temp_addr + size; ) 
	{
		/* 发出wirte0命令 */
		write_cmd(0x80);

		/* Write Address */
		write_addr(i);
		
		wait_idle();

		for(j = 0; j < PAGE_SIZE; j++, i++) 
		{
			write_data(buf[j]);
		}

		write_cmd(0x10);	

		wait_idle();

		buf += 1 << 11;
	}

	nand_deselect_chip();	
}

void copy_code_to_ram(unsigned int uboot_size)
{
	extern int _start;

	unsigned char *buf = (unsigned char *)((int)&_start - 16);
	
	//从0页开始读，读uboot_size大小
	nand_read(buf, 0, uboot_size);
}



/***************************************当需要从nand启动时，需要写入ECC，如下是ecc写入代码****************************************************/

void nand_enable_hwecc(void)
{
	var_int(NFCONF) = var_int(NFCONF) | (0x3 << 23);

	/* set 8/12/16bit Ecc direction to Encoding */
	var_int(NFECCCONT) = var_int(NFECCCONT) | (0x1 << 16);
	/* clear 8/12/16bit ecc encode done */
	var_int(NFECCSTAT) = var_int(NFECCSTAT) | (0x1 << 25);

	/* Initialize main area ECC decoder/encoder */
	var_int(NFCONT) = var_int(NFCONT) | (0x1 << 5);
	
	/* The ECC message size(For 512-byte message, you should set 511)
	* 8-bit ECC/512B */
	var_int(NFECCCONF) = (511 << 16) | 0x3;

	var_int(NFSTAT) = var_int(NFSTAT) | (0x1 << 4) | (0x1 << 5);
	
	/* Initialize main area ECC decoder/ encoder */
	var_int(NFECCCONT) = var_int(NFECCCONT) | (0x1 << 2);

	/* Unlock Main area ECC   */
	var_int(NFCONT) = var_int(NFCONT) & ~(0x1 << 7);
}

/* 
	[0~1]: 用于保存坏块标记,必须为0xff，表示块未损坏
	[2~11]:	为free, 
	[12~63]	: 保存ecc
	因为设置处理器每512字节产生13byte的ecc码， nandflash一页2048字节，分4次写，
	每次写512byte得到13字节存入nand_oob_64,然后一次写入obb区
*/
static unsigned char nand_oob_64[64] = {0x00};

static void nand_calculate_ecc(unsigned char *ecc_code)
{
	unsigned int nfeccprgecc0 = 0, nfeccprgecc1 = 0, nfeccprgecc2 = 0, nfeccprgecc3 = 0;

	/* Lock Main area ECC */
	var_int(NFCONT) = var_int(NFCONT) | (0x1 << 7);
	
	/* 读取13 Byte的Ecc Code */
	nfeccprgecc0 = var_int(NFECCPRGECC0);
	nfeccprgecc1 = var_int(NFECCPRGECC1);
	nfeccprgecc2 = var_int(NFECCPRGECC2);
	nfeccprgecc3 = var_int(NFECCPRGECC3);

	ecc_code[0] = nfeccprgecc0 & 0xff;
	ecc_code[1] = (nfeccprgecc0 >> 8) & 0xff;
	ecc_code[2] = (nfeccprgecc0 >> 16) & 0xff;
	ecc_code[3] = (nfeccprgecc0 >> 24) & 0xff;
	ecc_code[4] = nfeccprgecc1 & 0xff;
	ecc_code[5] = (nfeccprgecc1 >> 8) & 0xff;
	ecc_code[6] = (nfeccprgecc1 >> 16) & 0xff;
	ecc_code[7] = (nfeccprgecc1 >> 24) & 0xff;
	ecc_code[8] = nfeccprgecc2 & 0xff;
	ecc_code[9] = (nfeccprgecc2 >> 8) & 0xff;
	ecc_code[10] = (nfeccprgecc2 >> 16) & 0xff;
	ecc_code[11] = (nfeccprgecc2 >> 24) & 0xff;
	ecc_code[12] = nfeccprgecc3 & 0xff;
}

/*
	nand flash带ecc写入函数
*/
void nand_write_hwecc(unsigned char *buf, unsigned int addr, unsigned int size)
{
	unsigned int i;

	int j, k;

	unsigned int temp_addr = addr;

	int eccsteps = PAGE_SIZE / ECC_SIZE;		//4

	int eccbytes = ECC_BYTE;					//13

	int eccsize = ECC_SIZE;						//512

	unsigned char *p = buf, *ecc_start = &nand_oob_64[12];

	if (temp_addr & (PAGE_SIZE - 1))
	{
		printf("page not aligned, addr must be mulriple 0f 0x800!");
		return;
	}

	/*先擦除才能写入*/
	for(i = 0; i < size / PAGE_SIZE + 1; i++)
	{
		nand_erase(temp_addr);
		temp_addr += PAGE_SIZE;
	}

	temp_addr = addr;

	// printf("\neccsteps: %d eccsize: %d eccbytes = %d start: 0x%x\n", eccsteps, eccsize, eccbytes, (int)buf);

	for(i = 0; i < 12; i++)
		nand_oob_64[i] = 0xff;

	/* 选中芯片 */
	nand_select_chip();

	for(i = temp_addr; i < temp_addr + size; ) 
	{
		/*1. 写一页数据 2048 byte*/
		/* 发出wirte命令 */
		write_cmd(0x80);

		write_addr(i);
		
		wait_idle();

		for (j = 0; eccsteps; eccsteps--, j += eccbytes, p += eccsize, i+=ECC_SIZE)
		{
			/*1. 使能ecc*/
			nand_enable_hwecc();

			/*2. 写入512byte数据*/
			for(k = 0; k < ECC_SIZE; k++)
				write_data(p[k]);

			/*3. 获取硬件计算的ecc值*/
			nand_calculate_ecc(&ecc_start[j]);
		}

		/*写入ecc*/
		for(k = 0; k < 64; k++)
		{
			write_data(nand_oob_64[k]);
		}

		write_cmd(0x10);	

		wait_idle();

		eccsteps = PAGE_SIZE / ECC_SIZE;
	}

	nand_deselect_chip();
}