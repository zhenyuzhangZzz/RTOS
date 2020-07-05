#ifndef __HALBOARDDSC_H__
#define __HALBOARDDSC_H__

typedef struct board_dscp
{
	spinlock_t bd_lock;								/*自旋锁*/
	struct list_head bd_list;						/*链表*/
	adr_t bd_krnlstartaddr;							/*内核起始地址*/
	adr_t bd_krnlendaddr;							/*内核结束地址*/
	mmap_dscp_t	*mmap_baseaddr;						/*内存位图基地址*/
	uint_t mmap_cnt;								/*内存位图个数*/
	adrspce_dscp_t *adrspce_baseaddr;				/*物理地址空间基地址*/
	uint_t adrspce_cnt;								/*物理地址空间块个数*/
	interrupt_dscp_t *irq_dscp_baseaddr;			/*中断描述符基地址*/
	uint_t irqsource_cnt;							/*中断源个数*/
}board_dscp_t;

void hal_board_dscp_init(void);
void hal_board_dscp_print(void);

#endif