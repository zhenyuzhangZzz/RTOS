#include "arp.h"
#include "uart.h"
#include "timer.h"

eth_arp_pkg arp_buffer;

unsigned char host_mac_addr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
unsigned char host_ip_addr[4] = {192, 168, 0, 20};						//主机ip
unsigned char ip_addr[4] = {192, 168, 0, 30};							//dm9000 ip addr
unsigned char get_mac_flag = 0;											// = 1表示已经获取到mac

/*

	判断广播包是不是自己的ip
	是： rtn = 1, 否： rnt = 0
*/
int is_my_ip(unsigned char *ip)
{
	int i, rtn = 0;
	for(i = 0; i < 4; i++)
	{
		if(ip[i] != ip_addr[i])
			break;
	}	

	if(i == 4)
		rtn = 1;
	return rtn;
}

/*发送arp请求*/
int arp_request(unsigned char *dst_mac, unsigned char *src_mac, unsigned char *src_ip, unsigned char *dst_ip)
{
	unsigned int i;
	
	int rtn = -1;
	//char *p = (char *)&arp_buffer;

	get_mac_flag = 0;
	
	/*1. 封装arp包 */
	memcpy(arp_buffer.eth_hdr.ether_dhost, dst_mac, 6);

	memcpy(arp_buffer.eth_hdr.ether_shost, src_mac, 6);

	arp_buffer.eth_hdr.ether_type = htons(ETHERTYPE_ARP);

	arp_buffer.arp.ar_hrd = htons(ARPHRD_ETHER);
	arp_buffer.arp.ar_pro = htons(ETHERTYPE_IP);

	arp_buffer.arp.ar_hln = 6;
    arp_buffer.arp.ar_pln = 4;
    arp_buffer.arp.ar_op = htons(ARPOP_REQUEST);
    memcpy (arp_buffer.arp.arp_sha, src_mac, 6);
    memcpy (arp_buffer.arp.arp_spa, src_ip, 4);
    memcpy (arp_buffer.arp.arp_tpa, dst_ip, 4);

    // for(i = 0; i < sizeof(arp_buffer); i++)
    // 	printf("%d ", p[i]);

    // printf("szie: %d\n", sizeof(arp_buffer));

    i = 0;
	/*2. 发送arp包*/
	while(i < 5)					
	{
		dm9000_tx((unsigned char *)&arp_buffer, sizeof(arp_buffer));
		delay_ms(3000);															//发五次，每次等2s

		if(get_mac_flag == 1)
			break;

		i++;
		printf("request again\n");
	}

	if(get_mac_flag == 1)
		rtn = 0;

	return rtn;
}

/*arp响应包*/
void arp_reply(eth_arp_pkg *arp_ptr)
{
	/*1. 封装arp包 */
	memcpy(arp_buffer.eth_hdr.ether_dhost, arp_ptr->arp.arp_sha, 6);

	memcpy(arp_buffer.eth_hdr.ether_shost, mac_addr, 6);

	arp_buffer.eth_hdr.ether_type = htons(ETHERTYPE_ARP);

	arp_buffer.arp.ar_hrd = htons(ARPHRD_ETHER);
	arp_buffer.arp.ar_pro = htons(ETHERTYPE_IP);

	arp_buffer.arp.ar_hln = 6;
    arp_buffer.arp.ar_pln = 4;
    arp_buffer.arp.ar_op = htons(ARPOP_REPLY);
    memcpy (arp_buffer.arp.arp_sha, mac_addr, 6);
    memcpy (arp_buffer.arp.arp_spa, ip_addr, 4);
    memcpy (arp_buffer.arp.arp_tha, arp_ptr->arp.arp_sha, 6);
    memcpy (arp_buffer.arp.arp_tpa, arp_ptr->arp.arp_spa, 4);

    /*2. 发送arp包*/
	dm9000_tx((unsigned char *)&arp_buffer, sizeof(arp_buffer));
}

/*接收arp包*/
void arp_process(unsigned char *buffer)
{
	int i;
	/*1. 转化为arp包类型 */
	eth_arp_pkg *arp_ptr = (eth_arp_pkg *)buffer;

	if(!is_my_ip(arp_ptr->arp.arp_tpa))
		return;


	 switch(htons(arp_ptr->arp.ar_op))
	 {
	 	case ARPOP_REQUEST:				/*arp请求包,则发送arp响应包*/
	 		arp_reply(arp_ptr);
	 		break;

	 	case ARPOP_REPLY:				/*arp响应包*/
	 		
	 		if(get_mac_flag == 1)
	 			break;

	 		memcpy(host_mac_addr, arp_ptr->arp.arp_sha, 6);		//拷贝主机mac地址

	 		printf("\nmac addr: ");

	 		for(i = 0; i < 6; i++)
	 			printf("%x ", host_mac_addr[i]);

	 		printf("\n");

	 		get_mac_flag = 1;

	 		break;

	 	default:
	 		break;
	 }
}