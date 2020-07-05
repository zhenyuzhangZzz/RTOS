#include "net.h"
#include "uart.h"
#include "nand.h"
#include "lib.h"

char filename[30] = "bootloaderhead.bin";						//默认文件名

volatile unsigned int current_block = 1;						//当前块号
volatile unsigned int download_flag;							//下载完成标记
volatile unsigned char *download_addr;							//下载地址
volatile unsigned int total_len;								//文件长度

/*
	检验和的计算：
	1 把16bit的“首部检验和”字段置为零
	2 在IP数据报首部，以16位为单位切分成一段一段的字，计算所有字（仅首部）之和，并把求和所溢出数加到最低位上。
	3 把步骤2得到的结果求反码（按位取反），得到检验和
	4 把检验和存储在检验和字段中
*/
static unsigned short ip_checksum(unsigned char *ptr, int len)
{
    unsigned int sum = 0;
    unsigned short *p = (unsigned short *)ptr;

    while (len > 1)
    {
        sum += *p++;
        len -= 2;
    }
    
    if(len == 1)
        sum += *(unsigned char *)p;
    
    while(sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);
    
    return (unsigned short)((~sum)&0xffff);
}

/*发出tftp请求*/
static void tftp_request(void)
{
	unsigned char *tftp_pkg;
	int rtn;
	udp_head *udp_hdr;
	ip_head *ip_hdr;
	ether_head *eth_hdr;

	unsigned int tftp_len = 0;

	/*1. 先发出arp包获取pc mac地址*/
	rtn = arp_request(host_mac_addr, mac_addr, ip_addr, host_ip_addr);

	/*arp请求失败*/
	if(rtn == -1)
	{
		download_flag = DOWNLOAD_ERROR;
		printf("tftp server busy!\n");
		return;
	}

	/*2. 发送tftp请求包 */

	/*2.1 填充tftp请求包包*/
	tftp_pkg = &netbuffer[sizeof(ether_head) + sizeof(ip_head) + sizeof(udp_head)]; 
																	//eth_head + ip_head + udp_headd: 14 + 20 + 8 = 42

	tftp_pkg[0] = 0x00;												//opcode: RRQ 0x01，转换为大端即网络字节序 
	tftp_pkg[1] = 0x01;

	tftp_len += 2;

	strcpy((char *)&tftp_pkg[2], filename);							//文件名
	tftp_len += strlen(filename);									

	tftp_pkg[tftp_len] = 0;											//0结尾
	tftp_len += 1;

	strcpy((char *)&tftp_pkg[tftp_len], "octet");					//模式：netascii和octet
	tftp_len += strlen("octet");

	tftp_pkg[tftp_len] = 0;											//0结尾
	tftp_len += 1;

	/*2.2 填充udp头*/
	udp_hdr = (udp_head *)(tftp_pkg - sizeof(udp_head));

	udp_hdr->src_port = htons(1234);								//随便填，不占用已有协议端口即可
	udp_hdr->dst_port = htons(69);									//tftp端口为69
	udp_hdr->uhl = htons(tftp_len + sizeof(udp_head));				//udp包长度
	udp_hdr->chk_sum = htons(0x00);									//校验和

	/*3.3 填充ip头 */
	ip_hdr = (ip_head *)((unsigned char *)udp_hdr - sizeof(ip_head));

	ip_hdr->vhl = 0x45;      										//版本ipv4：4，首部20字节（以4字节为单位）：5，即0x45
	ip_hdr->tos = 0x00;												//弃用
	ip_hdr->tot_len =  htons(tftp_len + \
						sizeof(udp_head) + sizeof(ip_head));  		//ip报文长度

	ip_hdr->id = htons(0x00);										//第一个报文，id = 0
	ip_hdr->frag_off = htons(0x4000);								//偏移, 用于分片
	ip_hdr->ttl = 0xff;												//报文有效时间
	ip_hdr->protocol = PROTOCOL_UDP;								//上层协议，ICMP：1；IGMP：2；TCP：6；EGP：8；UDP：17
	ip_hdr->chk_sum = 0;
	memcpy(ip_hdr->srcaddr, ip_addr, 4);							//源ip地址
	memcpy(ip_hdr->dstaddr, host_ip_addr, 4);						//目的ip地址

	ip_hdr->chk_sum = ip_checksum((unsigned char *)ip_hdr, sizeof(ip_head));	//校验和

	/*3.4 填充mac包头部 */
	eth_hdr = (ether_head *)((unsigned char *)ip_hdr - sizeof(ether_head));
	memcpy(eth_hdr->ether_dhost, host_mac_addr, 6);
	memcpy(eth_hdr->ether_shost, mac_addr, 6);
	eth_hdr->ether_type = htons(ETHERTYPE_IP);

	/*4.4 发送出去 */
	dm9000_tx((unsigned char *)eth_hdr, sizeof(ether_head) + sizeof(ip_head) + sizeof(udp_head) + tftp_len);
}	

static void tftp_send_ack(unsigned short port)
{
	unsigned char *tftp_pkg;

	udp_head *udp_hdr;
	ip_head *ip_hdr;
	ether_head *eth_hdr;

	unsigned int tftp_len = 0;

	/*2.1 填充tftp请求包包*/
	tftp_pkg = &netbuffer[sizeof(ether_head) + sizeof(ip_head) + sizeof(udp_head)]; 
																	//eth_head + ip_head + udp_headd: 14 + 20 + 8 = 42

	tftp_pkg[0] = 0x00;												//opcode: RRQ 0x01，转换为大端即网络字节序 
	tftp_pkg[1] = 0x04;

	tftp_pkg[2] = (current_block & 0xff00) >> 8;					//块号
	tftp_pkg[3] = current_block & 0xff;
	
	tftp_len = 4;

	/*2.2 填充udp头*/
	udp_hdr = (udp_head *)(tftp_pkg - sizeof(udp_head));

	udp_hdr->src_port = htons(1234);								//随便填，不占用已有协议端口即可
	udp_hdr->dst_port = htons(port);								//tftp服务器目的端口
	udp_hdr->uhl = htons(tftp_len + sizeof(udp_head));				//udp包长度
	udp_hdr->chk_sum = htons(0x00);									//校验和

	/*3.3 填充ip头 */
	ip_hdr = (ip_head *)((unsigned char *)udp_hdr - sizeof(ip_head));

	ip_hdr->vhl = 0x45;      										//版本ipv4：4，首部20字节（以4字节为单位）：5，即0x45
	ip_hdr->tos = 0x00;												//弃用
	ip_hdr->tot_len =  htons(tftp_len + \
						sizeof(udp_head) + sizeof(ip_head));  		//ip报文长度

	ip_hdr->id = htons(0x00);										//第一个报文，id = 0
	ip_hdr->frag_off = htons(0x4000);								//偏移, 用于分片
	ip_hdr->ttl = 0xff;												//报文有效时间
	ip_hdr->protocol = PROTOCOL_UDP;								//上层协议，ICMP：1；IGMP：2；TCP：6；EGP：8；UDP：17
	ip_hdr->chk_sum = 0;
	memcpy(ip_hdr->srcaddr, ip_addr, 4);							//源ip地址
	memcpy(ip_hdr->dstaddr, host_ip_addr, 4);						//目的ip地址

	ip_hdr->chk_sum = ip_checksum((unsigned char *)ip_hdr, sizeof(ip_head));	//校验和

	/*3.4 填充mac包头部 */
	eth_hdr = (ether_head *)((unsigned char *)ip_hdr - sizeof(ether_head));
	memcpy(eth_hdr->ether_dhost, host_mac_addr, 6);
	memcpy(eth_hdr->ether_shost, mac_addr, 6);
	eth_hdr->ether_type = htons(ETHERTYPE_IP);

	/*4.4 发送出去 */
	dm9000_tx((unsigned char *)eth_hdr, sizeof(ether_head) + sizeof(ip_head) + sizeof(udp_head) + tftp_len);
}

static void tftp_process(unsigned char *buffer, int len, unsigned short sport)
{
	int i;

	tftp_data_pkg *tftp_ptr;

	tftp_ptr = (tftp_data_pkg *)(buffer + sizeof(ether_head) + sizeof(ip_head) + sizeof(udp_head));

	//tftp包长度
	len = len - sizeof(ether_head) - sizeof(ip_head) - sizeof(udp_head) - 4 - 4;  							//最后4字节是以太网尾部

	//数据块
	if(htons(tftp_ptr->opcode) == 3)
	{
		if(htons(tftp_ptr->block_num) == current_block)
		{
			printf("###");

			//存储下载内容
			for(i = 0; i < len; i++)
				download_addr[i] = tftp_ptr->data[i];

			download_addr += len;

			//记录文件长度
			total_len += len;

			//发送ack
			tftp_send_ack(sport);

			current_block++;

			/*最后一个包*/
			if(len < 512)
			{
				current_block = 1;

				download_flag = DOWNLOAD_SUCCESS;
			}
		}
	}
	else if(htons(tftp_ptr->opcode) == 5)
	{
		current_block = 1;
		download_flag = DOWNLOAD_ERROR;
	}
}

static void udp_process(unsigned char *buffer, int len)
{
	udp_head *udp_hdr;

	udp_hdr = (udp_head *)(buffer + sizeof(ether_head) + sizeof(ip_head));

	tftp_process(buffer, len, htons(udp_hdr->src_port));
}

/*ip包处理*/
static void ip_process(unsigned char *buffer, int len)
{
	ip_head *ip_hdr;

	ip_hdr = (ip_head *)(buffer + sizeof(ether_head));

	//指处理ip帧里是udp协议的包
	switch(ip_hdr->protocol)
	{
		case PROTOCOL_UDP:

			udp_process(buffer, len);

			break;

		default:
			break;
	}
}

static int tftp_download(void)
{
	int rtn = 0, addr;

	/*1. 获取文件名和下载地址*/
	printf("\nfile name: ");

	uart_gets();

	strcpy(filename, (char *)uart_get_buffer());

	printf("\naddr: ");

	uart_gets();

	addr = atoi((char *)uart_get_buffer());

	download_addr = (unsigned char *)addr;
	total_len = 0;

	/*2.发送tftp请求*/
	tftp_request();

	/*3. 等待下载完成*/
	while(download_flag == DOWNLOAD_WAIT);

	download_addr = (unsigned char *)addr;					//将下载地址复位

	if(download_flag == DOWNLOAD_SUCCESS)
	{
		printf("\nDownloaded file at 0x%x, size = %d bytes!\n", addr, total_len);
	}
	else if(download_flag == DOWNLOAD_ERROR)
	{
		printf("\nDownload fail, please check filename or tftp server!\n");
		total_len = 0;									//错误则复位文件长度
		rtn = -1;
	}

	download_flag = DOWNLOAD_WAIT;						//复位下载标志

	return rtn;
}


/*以太网包处理函数*/
void net_handle(unsigned char *buffer, int len)
{
	int type;		//以太网包类型

	ether_head *eth_head = (ether_head *)buffer;

	type = htons(eth_head->ether_type);

	switch(type)
	{
		case ETHERTYPE_ARP:				//arp包
			arp_process(buffer);
			break;
		case ETHERTYPE_IP:				//ip包
			ip_process(buffer, len);
			break;
		default:						//其他包不处理
			break;
	}
}

void tftp_download_run(void)
{
	int rtn;

	void (*func)(void);

	/*1. 下载文件 */
	rtn = tftp_download();

	if(rtn < 0)
		return;

	/*跳转*/
	func = (void (*)(void))download_addr;

	func();
}

void tftp_download_nand(unsigned int krladdr)
{
	int rtn, nand_write_addr;

	/*1. 下载文件 */
	rtn = tftp_download();

	if(rtn < 0)
		return;

	/*2. 写数据到nand flash*/
	printf("write addr: %x len: %d\n", download_addr, total_len);

	printf("nand write addr(default: 0x%x): ", krladdr);

	rtn = uart_gets();

	if(rtn == 0)
		nand_write_addr = krladdr;
	else
		nand_write_addr = atoi((char *)uart_get_buffer());

	//有数据可写
	if(total_len)				
	{
		// if(nand_write_addr == 0)
		// 	nand_write_hwecc((unsigned char *)download_addr, nand_write_addr, total_len);	//从0页开始写，写total_len, 带ecc写
		// else
		// 	nand_write((unsigned char *)download_addr, nand_write_addr, total_len);

		nand_write_hwecc((unsigned char *)download_addr, nand_write_addr, total_len);		//从0页开始写，写total_len, 带ecc写
	}																					
		
	printf("nand write finished\n");

}