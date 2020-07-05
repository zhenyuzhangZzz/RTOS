#ifndef __ARP_H__
#define __ARP_H__

#include "dm9000.h"

#define ETHERTYPE_ARP 0x0806
#define ETHERTYPE_IP  0x0800
#define ARPHRD_ETHER  1 

#define ARPOP_REQUEST 1 
#define ARPOP_REPLY   2 

#define htons(n)    ((((unsigned short)(((n) & 0xff))) << 8) | (((n) & 0xff00) >> 8))

#pragma pack(1)

/* 根据 RFC 0826 修改*/
/* ethernet头 */
typedef struct
{
    unsigned char ether_dhost[6];                   /* 目地硬件地址 */
    unsigned char ether_shost[6];                   /* 源硬件地址 */
    unsigned short ether_type;                      /* 网络类型 */
}ether_head;

/* arp协议 */
typedef struct
{
    unsigned short ar_hrd; 							/* 硬件地址格式 */
    unsigned short ar_pro; 							/* 协议地址格式 */
    unsigned char ar_hln; 							/* 硬件地址长度(字节) */
    unsigned char ar_pln; 							/* 协议地址长度(字节) */
    unsigned short ar_op; 							/* 操作代码 */
    unsigned char arp_sha[6]; 						/* 源硬件地址 */
    unsigned char arp_spa[4]; 						/* 源协议地址 */
    unsigned char arp_tha[6]; 						/* 目地硬件地址 */
    unsigned char arp_tpa[4]; 						/* 目地协议地址 */
}arp_pkg;

/*以太网包，arp协议*/
typedef struct
{
    ether_head eth_hdr;
    arp_pkg arp;
}eth_arp_pkg;

#pragma pack()

extern unsigned char host_mac_addr[6];
extern unsigned char host_ip_addr[4];
extern unsigned char ip_addr[4];
extern unsigned char get_mac_flag;

void *memcpy(void *dest, const void *src, int n);
void *memset(void *s, int c, int n);
char *strcpy(char *dest, const char *src);
unsigned int strlen(const char *s);

int arp_request(unsigned char *dst_mac, unsigned char *src_mac, unsigned char *src_ip, unsigned char *dst_ip);
void arp_process(unsigned char *buffer);

#endif