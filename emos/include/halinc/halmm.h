#ifndef __HALMM_H__
#define __HALMM_H__

#define ADRSPCE_NOTYPE			0
#define ADRSPCE_IO          	1
#define ADRSPCE_SDRAM          	2
#define ADRSPCE_SROMC			3
#define ADRSPCE_NAND			4
#define ADRSPCE_LPASRAM			5
#define ADRSPCE_IROM_IRAM		6
#define ADRSPCE_SFRS			7

#define MEM_BLK_4M_SIZE			(0x400000)
#define MEM_BLK_CLASS			6											/*内存块类型*/
#define MEM_NO_USE				0
#define MEM_USE_FLAG(VAL, OFFSET)	(VAL << OFFSET)

#define MEM_BLK_16K				1 << 0
#define MEM_BLK_32K				2 << 0
#define MEM_BLK_4M				8 << 0
#define MEM_SUBBLK_128K			1 <<  4
#define MEM_SUBBLK_256K			2 <<  4
#define MEM_SUBBLK_512K			3 <<  4
#define MEM_SUBBLK_1M			4 <<  4
#define MEM_SUBBLK_2M			5 <<  4
#define MEM_SUBBLK_4M			6 <<  4

#define MEM_FLAG_MASK_128K		0xFFFFFFFF
#define MEM_FLAG_MASK_256K		0xFFFF
#define MEM_FLAG_MASK_512K		0xFF
#define MEM_FLAG_MASK_1M		0x0F
#define MEM_FLAG_MASK_2M		0x03
#define MEM_FLAG_MASK_4M		0x01

#define MEM_ALLOC_BIT_128K 		32
#define MEM_ALLOC_BIT_256K  	16
#define MEM_ALLOC_BIT_512K  	8
#define MEM_ALLOC_BIT_1M  		4
#define MEM_ALLOC_BIT_2M  		2
#define MEM_ALLOC_BIT_4M  		1

#define MEM_SIZE_128K			0x20000
#define MEM_SIZE_256K			0x40000
#define MEM_SIZE_512K			0x80000
#define MEM_SIZE_1M				0x100000
#define MEM_SIZE_2M				0x200000
#define MEM_SIZE_4M				0x400000

#define MEM_BLK_INVALID(size)		((size != MEM_SIZE_128K) && (size != MEM_SIZE_256K) && (size != MEM_SIZE_512K) &&\
								(size != MEM_SIZE_1M) && (size != MEM_SIZE_2M) && (size != MEM_SIZE_4M))	

#define MEM_FLAG_VAL(RV, SUBBLK_SIZE, BLKSIZE)	(RV|SUBBLK_SIZE|BLKSIZE)


#define INSERT_EMPTY_FLAG	1
#define INSERT_PART_FLAG	2
#define INSERT_FULL_FLAG	3

/*
*	内存位图描述符，用于描述一块4M的内存
*	内核内存可以分为4M, 2M, 1M, 512KB, 256KB, 128KB
*/
typedef struct mmap_dscp
{
	struct list_head map_list;
	spinlock_t map_lock;
	adr_t map_phy_startaddr;
	adr_t map_phy_endaddr;
	u32_t map_alloc_bit;		
	u32_t map_flag;			
}mmap_dscp_t;

/*
* 	链接分配内存的链表，用于快速访问
*/
typedef struct alloc_memory_list
{
	spinlock_t am_lock;					
	size_t	am_size;
	struct list_head am_full_list;
	struct list_head am_part_list;
	struct list_head am_empty_list;
}alloc_mem_list_t;

/*
*	内存位图快速查询结构
*/
typedef struct mmap_tlb
{
	struct list_head tlb_list;
	spinlock_t tlb_lock;
	uint_t freeblks;
	uint_t allocblks;
	alloc_mem_list_t tlb_allocmem_list[MEM_BLK_CLASS];
	/*[0]: 128KB [1]: 256KB, [2]: 512KB, [3]: 1M, [4]: 2M, [5]: 4M*/
}mmap_tlb_t;

/*
* 物理划分描述符
*/
typedef struct adrspce_dscp
{
	u32_t adrspce_flag;					/*物理划分的类型ADRSPCE_IO、ADRSPCE_SDRAM*/
	u32_t adrspce_iotype;				/*若为ADRSPCE_IO，则指示是什么类型的IO*/
	adr_t adrspce_startaddr;			/*地址空间起始地址*/
	adr_t adrspce_endaddr;				/*地址空间结束地址*/
}adrspce_dscp_t;



void hal_mm_init(void);
adr_t hal_memallocblks(size_t blksz);					/*块级内存分配函数， 必须是128K的整数倍，最多一次分配4M*/
bool_t hal_memfreeblks(adr_t addr, size_t blksz);		/*块级内存释放函数*/
void blk_mem_test2(void);
uint_t blk_print_4M_cnt(void);
#endif