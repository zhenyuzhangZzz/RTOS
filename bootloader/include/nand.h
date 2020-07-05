#ifndef _NAND_H_
#define _NAND_H_

#include "s5pv210reg.h"

/* K9F4G08U0B²ÎÊý */		
#define PAGE_SIZE				2048
#define NAND_BLOCK_SIZE			64

#define ECC_SIZE 				512
#define ECC_BYTE				13

typedef struct nand_id_msg
{
	unsigned char maker_id; 
	unsigned char device_id; 
	unsigned char id_3th;
	unsigned char id_4th;
	unsigned char id_5th;
}nand_id_msg;

void nand_read(unsigned char *buf, unsigned int addr, int size);
void nand_write(unsigned char *buf, unsigned int addr, unsigned int size);
void nand_write_hwecc(unsigned char *buf, unsigned int addr, unsigned int size);
void copy_code_to_ram(unsigned int uboot_size);

#endif 


