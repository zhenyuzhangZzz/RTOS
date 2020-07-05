#ifndef __KRNLMM_H__
#define __KRNLMM_H__

#define KRNL_ALLOC_MAXSIZE			0x400000
#define KRNL_ALLOC_MINSIZE			0

#define KRNL_BYTES_MINSIZE			1
#define KRNL_BYTES_MAXSIZE			2048
#define KRNL_PAGE_SIZE 				4096
#define KRNL_PAGE_ALLOC_MINIZE		4096
#define KRNL_PAGE_ALLOC_MAXSIZE		(4096 * 31)

#define KRNL_BYTE_ALIGN_SIZE		4
#define MPLHEAD_PAGE_TYPE			0x01
#define MPLHEAD_BYTE_TYPE			0x02
#define MPLHEAD_PAGE_MG				0x03

/*页面描述符结构*/
typedef struct pglmap_t
{
	adr_t pgl_startaddr;
	struct pglmap_t *next;
}pglmap_t;

/*内存池结构*/
typedef struct mplhead
{
	spinlock_t mpl_lock;				
	struct list_head pml_list;
	uint_t mpl_headty;							/*类型：页级，字级*/
	adr_t mpl_startaddr;						/*内存池开始地址*/
	adr_t mpl_endaddr;							/*内存池结束地址*/
	adr_t mpl_firstbyteaddr;					/*第一个字级内存对象的地址, 页级内存不使用*/
	pglmap_t *mpl_firstpage;					/*内存池的第一个可使用的页面*/
	uint_t mpl_pagecnt;							/*内存池中页面个数*/
	uint_t mpl_aliobsz;							/*实际分配页面的大小，页级内存分配时等于mpl_pagesize*/
	uint_t mpl_pagesize;						/*页面大小*/
	uint_t mpl_nextpageoffset;					/*下一个对象指针的偏移*/
	uint_t mpl_usedpagecnt;						/*内存池中已经使用了的页面*/
	uint_t mpl_pagedscpcnt;						/*页面描述符个数，字级内存不使用*/
	pglmap_t *mpl_pagedscpaddr;					/*页面描述符基地址，字级内存不使用*/
}mplhead_t;

/*内核内存池管理结构*/
typedef struct krnl_mplmm
{
	spinlock_t kmpl_lock;					
	struct list_head kmpl_list;
	uint_t kmpl_stus;							/*状态*/
	uint_t kmpl_flag;							/*标记*/
	spinlock_t kmpl_pagelock;					/*页级内存池保护锁*/
	spinlock_t kmpl_bytelock;					/*字级内存池保护锁*/
	uint_t kmpl_pagemplcnt;					    /*页级内存池个数*/
	uint_t kmpl_bytemplcnt;						/*字级内存池个数*/
	struct list_head kmpl_pagelist;				/*挂载页级内存池的链表*/
	struct list_head kmpl_bytelist;				/*挂载字级内存池的链表*/
	struct list_head kmpl_headlist;				/*挂载内存池头的链表*/
	mplhead_t *kmpl_recentpagempl;				/*最近操作过的页级内存池*/
	mplhead_t *kmpl_recentbytempl;				/*最近操作过的字级内存池*/
	mplhead_t *kmpl_recentmplhead;				/*最近操作过的内存池头的内存池*/
}krnl_mplmm_t;

void krnl_mm_init(void);

adr_t krnl_alloc(size_t size);
bool_t krnl_free(adr_t addr, size_t size);
 
 
#endif