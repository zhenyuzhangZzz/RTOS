#ifndef __NET_H__
#define __NET_H__

#include "arp.h"

#pragma pack(1)

//IP头部，总长度20字节
typedef struct
{
	unsigned char vhl;					//首部长度和版本
	unsigned char tos;					//服务类型
	unsigned short tot_len;				//总长度
	unsigned short id;					//标志
	unsigned short frag_off;			//分片偏移
	unsigned char ttl;					//生存时间
	unsigned char protocol;				//协议
	unsigned short chk_sum;				//检验和
	unsigned char srcaddr[4];			//源IP地址
	unsigned char dstaddr[4];			//目的IP地址
}ip_head;

//UDP头部，总长度8字节
typedef struct
{
	unsigned short src_port;			//源端口号
	unsigned short dst_port;			//目的端口号
	unsigned short uhl;					//udp包长度
	unsigned short chk_sum;				//16位udp检验和
}udp_head;

typedef struct 
{
	unsigned short opcode;				//操作码
	unsigned short block_num;			//块号
	unsigned char data[0];				//数据指针
}tftp_data_pkg;

#pragma pack()

#define PROTOCOL_UDP		17

#define DOWNLOAD_WAIT		0
#define DOWNLOAD_SUCCESS	1
#define DOWNLOAD_ERROR		2

void net_handle(unsigned char *buffer, int len);

//下载程序到内存并运行
void tftp_download_run(void);
//下载程序到内存并写入flash
void tftp_download_nand(unsigned int krladdr);
#endif