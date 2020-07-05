#include "dm9000.h"
#include "uart.h"
#include "interrupt.h"
#include "net.h"

unsigned char netbuffer[DM9000_PKT_MAX];

unsigned char mac_addr[6] = {0x10,0x02,0x03,0x04,0x05,0x06}; 	//MAC addr

void dm9000_irq(void);

static void dm9000_pre_init(void)
{
	unsigned int smc_bw_conf, smc_bc_conf;

	/* Ethernet needs bus width of 16 bits */
	smc_bw_conf = SMC_DATA16_WIDTH(1) | SMC_BYTE_ADDR_MODE(1);
	smc_bc_conf = SMC_BC_TACS(0x0) | SMC_BC_TCOS(0x1) | SMC_BC_TACC(0x2)
			| SMC_BC_TCOH(0x1) | SMC_BC_TAH(0x0)
			| SMC_BC_TACP(0x0) | SMC_BC_PMC(0x0);

	/* Select and configure the SROMC bank */
	var_int(SROM_BW) &= ~(0x0F << 4);
	var_int(SROM_BW) |= smc_bw_conf;

	var_int(SROM_BC1) = smc_bc_conf;
}

static unsigned char DM9000_ior(unsigned short reg)
{
	var_word(DM9000_IO) = reg;
	return var_byte(DM9000_DATA);
}

/*
   Write a byte to I/O port
*/
static void DM9000_iow(unsigned short reg, unsigned short value)
{
	var_word(DM9000_IO) = reg;
	var_word(DM9000_DATA) = value;
}

/* General Purpose dm9000 reset routine */
static void dm9000_reset(void)
{
	/* Reset DM9000,
	   see DM9000 Application Notes V1.22 Jun 11, 2004 page 29 */

	/* DEBUG: Make all GPIO0 outputs, all others inputs */
	DM9000_iow(DM9000_GPCR, GPCR_GPIO0_OUT);
	/* Step 1: Power internal PHY by writing 0 to GPIO0 pin */
	DM9000_iow(DM9000_GPR, 0);
	/* Step 2: Software reset */
	DM9000_iow(DM9000_NCR, (NCR_LBK_INT_MAC | NCR_RST));

	while (DM9000_ior(DM9000_NCR) & 1);

	DM9000_iow(DM9000_NCR, 0);
	DM9000_iow(DM9000_NCR, (NCR_LBK_INT_MAC | NCR_RST)); /* Issue a second reset */

	while (DM9000_ior(DM9000_NCR) & 1);

    DM9000_iow(DM9000_NCR, 0);

	/* Check whether the ethernet controller is present */
	if ((DM9000_ior(DM9000_PIDL) != 0x0) ||
	    (DM9000_ior(DM9000_PIDH) != 0x90))
		printf("ERROR: resetting DM9000 -> not responding\n");
}

/*
  Search DM9000 board, allocate space and register it
*/
int dm9000_probe(void)
{
	unsigned int id_val;
	id_val = DM9000_ior(DM9000_VIDL);
	id_val |= DM9000_ior(DM9000_VIDH) << 8;
	id_val |= DM9000_ior(DM9000_PIDL) << 16;
	id_val |= DM9000_ior(DM9000_PIDH) << 24;
	if (id_val == DM9000_ID) {
		printf("\ndm9000 i/o: 0x%x, id: 0x%x \n", CONFIG_DM9000_BASE,
		       id_val);
		return 0;
	} else {
		printf("\ndm9000 not found at 0x%08x id: 0x%08x\n",
		       CONFIG_DM9000_BASE, id_val);
		return -1;
	}
}

/*
	dm9000初始化, 参考uboot的dm9000x.c
*/
void dm9000_init(void)
{
	unsigned char oft;
	short i;

	/*1. 初始化SROM bank1, 字宽，时序*/
	dm9000_pre_init();

	/*2. 复位设备 */
	dm9000_reset();

	/*3. 外部中断初始化 */
	exit10_init(dm9000_irq);

	/*4. 捕获设备 */
	if (dm9000_probe() < 0)
		return;

	/*5. 初始化dm9000到合适模式*/
	/* Program operating register, only internal phy supported */
	DM9000_iow(DM9000_NCR, 0x0);
	/* TX Polling clear */
	DM9000_iow(DM9000_TCR, 0);
	/* Less 3Kb, 200us */
	DM9000_iow(DM9000_BPTR, BPTR_BPHW(3) | BPTR_JPT_600US);
	/* Flow Control : High/Low Water */
	DM9000_iow(DM9000_FCTR, FCTR_HWOT(3) | FCTR_LWOT(8));
	/* SH FIXME: This looks strange! Flow Control */
	DM9000_iow(DM9000_FCR, 0x0);
	/* Special Mode */
	DM9000_iow(DM9000_SMCR, 0);
	/* clear TX status */
	DM9000_iow(DM9000_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END);
	/* Clear interrupt status */
	DM9000_iow(DM9000_ISR, ISR_ROOS | ISR_ROS | ISR_PTS | ISR_PRS);

	/*6. 填充mac地址寄存器 */
	/* fill device MAC address registers */
	for (i = 0, oft = DM9000_PAR; i < 6; i++, oft++)
		DM9000_iow(oft, mac_addr[i]);
	for (i = 0, oft = 0x16; i < 8; i++, oft++)
		DM9000_iow(oft, 0x00);								//屏蔽多播包

	/*7. 激活dm9000*/
	DM9000_iow(DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);
	/* Enable TX/RX interrupt mask */
	DM9000_iow(DM9000_IMR, IMR_PAR);
}


void dm9000_tx(unsigned char *packet, unsigned int length)
{
	unsigned int i;
	unsigned char status;
	/* 1.禁止发送接收中断*/
	DM9000_iow(DM9000_IMR, 0x80);

	/*2. Clear Tx bit in ISR */
	// DM9000_iow(DM9000_ISR, IMR_PTM); 

	/* Set TX length to DM9000 */
	DM9000_iow(DM9000_TXPLL, length & 0xff);
	DM9000_iow(DM9000_TXPLH, (length >> 8) & 0xff);

	/* Move data to DM9000 TX RAM */
	var_word(DM9000_IO) = DM9000_MWCMD;		/* Prepare for TX-data */

	/* push the data to the TX-fifo */

	for(i = 0; i < length; i+=2)
	{
		var_word(DM9000_DATA) = packet[i] | (packet[i + 1] << 8);
	}

	/* Issue TX polling command */
	DM9000_iow(DM9000_TCR, TCR_TXREQ); /* Cleared after TX complete */

	/*等待发送结束*/
    while(1)
    {
       status = DM9000_ior(DM9000_TCR);
       if((status & 0x01) == 0x00) break;	
    }

    //DM9000_iow(DM9000_ISR, IMR_PTM); /* Clear Tx bit in ISR */
    DM9000_iow(DM9000_NSR,0x2c);
    /* 打开接收中断*/
	DM9000_iow(DM9000_IMR, 0x81);
}

int dm9000_rx(unsigned char *buffer)
{

	unsigned char rxbyte;

	unsigned short RxStatus, RxLen = 0;

	volatile unsigned short tmp, i;

	if (!(DM9000_ior(DM9000_ISR) & 0x01)) /* Rx-ISR bit must be set. */
		return 0;

	DM9000_iow(DM9000_ISR, 0x01); /* clear PR status latched in bit 0 */

		/* There is _at least_ 1 package in the fifo, read them all */
	for (;;) {
		DM9000_ior(DM9000_MRCMDX);	/* Dummy read */

		/* Get most updated data,
		   only look at bits 0:1, See application notes DM9000 */
		rxbyte = var_byte(DM9000_DATA) & 0x03;

		/* Status check: this byte must be 0 or 1 */
		if (rxbyte > DM9000_PKT_RDY) {
			DM9000_iow(DM9000_RCR, 0x00);	/* Stop Device */
			DM9000_iow(DM9000_ISR, 0x80);	/* Stop INT request */
			printf("DM9000 error: status check fail: 0x%x\n",
				rxbyte);
			return 0;
		}

		if (rxbyte != DM9000_PKT_RDY)
			return 0; /* No packet received, ignore */

		/* A packet ready now  & Get status/length */

        RxStatus = DM9000_ior(DM9000_MRCMD);
		RxLen = var_word(DM9000_DATA);

		/* Move data from DM9000 */
		/* Read received packet from RX SRAM */
		for(i = 0; i < RxLen; i+=2)
		{
			tmp = var_word(DM9000_DATA);
           	buffer[i] = tmp & 0xff;
           	buffer[i+1] = (tmp>>8) & 0xff;	
		}

		if ((RxStatus & 0xbf00) || (RxLen < 0x40)
			|| (RxLen > DM9000_PKT_MAX)) {
			if (RxStatus & 0x100) {
				printf("rx fifo error\n");
			}
			if (RxStatus & 0x200) {
				printf("rx crc error\n");
			}
			if (RxStatus & 0x8000) {
				printf("rx length error\n");
			}
			if (RxLen > DM9000_PKT_MAX) {
				printf("rx length too big\n");
				dm9000_reset();
			}
		} else {
            return RxLen;
		}
	}
}

void dm9000_irq(void)
{
	int netrecvbyte = dm9000_rx(netbuffer);
	
	if(netrecvbyte)
		net_handle(netbuffer, netrecvbyte);									//数据接收正确调用通用的以太网包处理函数

    var_int(VIC0ADDRESS) = 0;
    var_int(EXT_INT_1_PEND) = 0xff;
}