#include "lmosem.h"

void mmap_dscp_init(mmap_dscp_t *dp, adr_t start, adr_t end, u32_t alloc_bit, u32_t flag)
{
	spin_lock_init(&dp->map_lock);

	init_list_head(&dp->map_list);

	dp->map_phy_startaddr = start;
	dp->map_phy_endaddr = end;
	dp->map_alloc_bit = alloc_bit;
	dp->map_flag = flag;
}

private uint_t mmap_dscp_4M_init(mmap_dscp_t *dp, adr_t start, adr_t end, uint_t crt_index)
{
	uint_t index = crt_index;
	adr_t tmpaddr = start;

	for(; start < end; start+= MEM_BLK_4M_SIZE, index++)
	{
		if(start + MEM_BLK_4M_SIZE <= end)
		{
			tmpaddr = start + MEM_BLK_4M_SIZE - 1;
		}else
		{
			tmpaddr = end;
		}

		mmap_dscp_init(dp + index, start, tmpaddr, MEM_USE_FLAG(0, 0), MEM_FLAG_VAL(0, MEM_SUBBLK_4M, MEM_BLK_4M));
	}
	return index;
}

private void hal_mmap_dscp_init(board_dscp_t *bd_ptr)
{
	adrspce_dscp_t *adr_dp = bd_ptr->adrspce_baseaddr;
	uint_t adr_dp_cnt = bd_ptr->adrspce_cnt;

	uint_t index = 0;

	/*从描述符中找是内存类型的描述符*/
	for(uint_t i = 0; i < adr_dp_cnt; i++)
	{
		if(adr_dp[i].adrspce_flag == ADRSPCE_SDRAM)
		{
			index = mmap_dscp_4M_init(bd_ptr->mmap_baseaddr, adr_dp[i].adrspce_startaddr, adr_dp[i].adrspce_endaddr, index);
		}
	}
	/*更新内存位图描述符计数*/
	bd_ptr->mmap_cnt = index;

	/*更新内核结束地址，此时应该包含描述符大小*/
	bd_ptr->bd_krnlendaddr = (adr_t)bd_ptr->mmap_baseaddr + sizeof(mmap_dscp_t) * index;
}

private void alloc_mem_list_init(alloc_mem_list_t *list, size_t size)
{
	spin_lock_init(&list->am_lock);
	list->am_size = size;
	init_list_head(&list->am_full_list);
	init_list_head(&list->am_part_list);
	init_list_head(&list->am_empty_list);
}

void mmap_tlb_init(mmap_tlb_t *tlb_ptr)
{
	size_t size;

	spin_lock_init(&tlb_ptr->tlb_lock);

	init_list_head(&tlb_ptr->tlb_list);

	tlb_ptr->freeblks = 0;
	tlb_ptr->allocblks = 0;

	size = MEM_SIZE_128K;

	for(uint_t i = 0; i < MEM_BLK_CLASS; i++, size += size)
	{
		alloc_mem_list_init(&tlb_ptr->tlb_allocmem_list[i], size);
	}
}

void bd_dscp_insert_tlb(mmap_dscp_t *dp, mmap_tlb_t *tlb_ptr, uint_t am_flag)
{
	/*取描述符*/
	u8_t type = (u8_t)((dp->map_flag & 0xF0) >> 4);

	switch(am_flag)
	{
		case INSERT_EMPTY_FLAG:
			list_add_tail(&dp->map_list, &tlb_ptr->tlb_allocmem_list[type - 1].am_empty_list);
			break;
		case INSERT_PART_FLAG:
			list_add_tail(&dp->map_list, &tlb_ptr->tlb_allocmem_list[type - 1].am_part_list);
			break;
		case INSERT_FULL_FLAG:
			list_add_tail(&dp->map_list, &tlb_ptr->tlb_allocmem_list[type - 1].am_full_list);
			break;
		default:
			break;
	}
}

private void hal_mmap_tlb_init(board_dscp_t *bd_ptr, mmap_tlb_t *tlb_ptr)
{
	uint_t index = bd_ptr->mmap_cnt;

	/*初始化tlb结构*/
	mmap_tlb_init(tlb_ptr);
	/*将描述符根据类型插入快表中*/
	for(uint_t i = 1; i < index; i++)
	{
		bd_dscp_insert_tlb(&bd_ptr->mmap_baseaddr[i], tlb_ptr, INSERT_EMPTY_FLAG);
	}
}

private void hal_verify_krnl_dscp(board_dscp_t *bd_ptr, mmap_tlb_t *tlb_ptr)
{
	/*默认将内核放在第一个4M开始的内存，即描述符为0的内存是不能使用的*/
	adr_t krnl_startaddr = bd_ptr->bd_krnlstartaddr;
	adr_t krnl_endaddr = bd_ptr->bd_krnlendaddr;

	if(krnl_endaddr - krnl_startaddr > MEM_SIZE_4M || \
		krnl_startaddr < bd_ptr->mmap_baseaddr[0].map_phy_startaddr || krnl_endaddr > bd_ptr->mmap_baseaddr[0].map_phy_endaddr)
	{
		system_error("kernl size and location error!\n");
	}

	bd_ptr->mmap_baseaddr[0].map_alloc_bit = MEM_USE_FLAG(1, 0);						//表示这块内存已经被使用了
	bd_ptr->mmap_baseaddr[0].map_flag = MEM_FLAG_VAL(0, MEM_SUBBLK_4M, MEM_BLK_4M);

	bd_dscp_insert_tlb(&bd_ptr->mmap_baseaddr[0], tlb_ptr, INSERT_FULL_FLAG);
}


void hal_mm_init(void)
{
	/*1、初始化内存位图描述符 */
	hal_mmap_dscp_init(&bd);

	/*2、初始化访问快表 */
	hal_mmap_tlb_init(&bd, &mmap_tlb);	

	/*3. 修改内核在内存位图中的信息*/
	hal_verify_krnl_dscp(&bd, &mmap_tlb);
}

/*
* 寻找alloc_mem_list_t并返回
*/
private alloc_mem_list_t *hal_find_suitable_alloclist(mmap_tlb_t *tlb_ptr, alloc_mem_list_t **list, size_t blksz)
{
	alloc_mem_list_t *rtn_list = NULL;

	*list = NULL;

	for(uint_t i = 0; i < MEM_BLK_CLASS; i++)
	{
		if(tlb_ptr->tlb_allocmem_list[i].am_size == blksz)
		{
			rtn_list = &tlb_ptr->tlb_allocmem_list[i];
			*list = rtn_list;
			break;
		}
	}

	/*在快表中找到了合适大小的链表*/
	if(rtn_list != NULL)
	{
		/*部分分配链和空链不为空，则表示可以分配内存*/
		if(list_empty_careful(&rtn_list->am_part_list) == FALSE)
		{
			return rtn_list;
		}
		if(list_empty_careful(&rtn_list->am_empty_list) == FALSE)
		{
			return rtn_list;
		}

		/*表示上述类型的内存不存在，则去4M内存中去取来一个新的4M内存，并返回*/
		if(list_empty_careful(&tlb_ptr->tlb_allocmem_list[MEM_BLK_CLASS-1].am_empty_list) == FALSE)
		{
			rtn_list = &tlb_ptr->tlb_allocmem_list[MEM_BLK_CLASS-1];				
		}
	}
	return rtn_list;
}

private mmap_dscp_t *hal_find_suitable_allocdscp(alloc_mem_list_t *list)
{
	mmap_dscp_t *rtn_dscp = NULL;

	if(list == NULL)
		return NULL;

	/*若是4M的，不会有部分链，4M是一整块, 要么被用了，要么为空*/
	if(list_empty_careful(&list->am_part_list) == FALSE)
	{
		rtn_dscp = list_entry(list->am_part_list.next, mmap_dscp_t, map_list);
	}
	if(list_empty_careful(&list->am_empty_list) == FALSE)
	{
		rtn_dscp = list_entry(list->am_empty_list.next, mmap_dscp_t, map_list);
	}
	return rtn_dscp;
}

private adr_t hal_allocblks(uint_t mflag, uint_t mask, uint_t bits, alloc_mem_list_t *src_list, alloc_mem_list_t *dest_list)
{
	adr_t ret_adr;
	mmap_dscp_t *dscp;
	cpuflg_t flag;
	uint_t i;

	if(bits > 32)
		return NULL;

	/*寻找描述符*/
	dscp = hal_find_suitable_allocdscp(src_list);

	if(dscp == NULL)
	{
		return NULL;
	}

	spin_lock_save(&dscp->map_lock, flag);

	/*寻找未使用的次级内存块*/
	for(i = 0; i < bits; i++)
	{
		if(((dscp->map_alloc_bit >> i) & 0x01) == 0)
		{	
			break;
		}
	}

	if(i == bits)
		return NULL;

	ret_adr = dscp->map_phy_startaddr + dest_list->am_size * i;

	if(ret_adr < dscp->map_phy_startaddr || ret_adr > dscp->map_phy_endaddr)
		return NULL;

	/*改变描述符标记，不管src_list是否等于dest_list*/
	dscp->map_alloc_bit |= (1 << i);

	dscp->map_flag &= ~0xf0;						/*清除内存块标记*/	
	dscp->map_flag |= mflag;			

	/*表示内存已经使用完了*/
	if((dscp->map_alloc_bit & mask) == mask)
	{
		list_move_tail(&dscp->map_list, &dest_list->am_full_list);
	}
	else
	{
		list_move_tail(&dscp->map_list, &dest_list->am_part_list);
	}
	spin_unlock_restore(&dscp->map_lock, flag);
	return ret_adr;
}

private adr_t hal_find_allocblks(alloc_mem_list_t *src_list, alloc_mem_list_t *dest_list, size_t blksz)
{
	adr_t ret_adr;
	switch(blksz)
	{
		case MEM_SIZE_128K:
			ret_adr = hal_allocblks(MEM_SUBBLK_128K, MEM_FLAG_MASK_128K, MEM_ALLOC_BIT_128K, src_list, dest_list);
			break;
		case MEM_SIZE_256K:
			ret_adr = hal_allocblks(MEM_SUBBLK_256K, MEM_FLAG_MASK_256K, MEM_ALLOC_BIT_256K, src_list, dest_list);
			break;		
		case MEM_SIZE_512K:
			ret_adr = hal_allocblks(MEM_SUBBLK_512K, MEM_FLAG_MASK_512K, MEM_ALLOC_BIT_512K, src_list, dest_list);
			break;		
		case MEM_SIZE_1M:
			ret_adr = hal_allocblks(MEM_SUBBLK_1M, MEM_FLAG_MASK_1M, MEM_ALLOC_BIT_1M, src_list, dest_list);
			break;		
		case MEM_SIZE_2M:
			ret_adr = hal_allocblks(MEM_SUBBLK_2M, MEM_FLAG_MASK_2M, MEM_ALLOC_BIT_2M, src_list, dest_list);
			break;		
		case MEM_SIZE_4M:
			ret_adr = hal_allocblks(MEM_SUBBLK_4M, MEM_FLAG_MASK_4M, MEM_ALLOC_BIT_4M, src_list, dest_list);
			break;
		default:
			ret_adr = NULL;
	}
	return ret_adr;
}

adr_t hal_memallocblks(size_t blksz)
{
	adr_t ret_adr;
	cpuflg_t flag;
	mmap_tlb_t *tlb_ptr = &mmap_tlb;
	alloc_mem_list_t *rnt_list, *moveto_list;

	if(MEM_BLK_INVALID(blksz))
	{
		return NULL;
	}

	spin_lock_save(&tlb_ptr->tlb_lock, flag);

	/* 
	*根据blksz找到tlb中合适的rnt_list, moveto_list表示即将分配的内存链表
	*当rnt_list为4M类型时，两者不相等
	*/
	rnt_list = hal_find_suitable_alloclist(tlb_ptr, &moveto_list, blksz);

	/*根据alloc_mem_list_t结构找到合适的mmap_dscp_t并返回地址*/
	ret_adr = hal_find_allocblks(rnt_list, moveto_list, blksz);

	spin_unlock_restore(&tlb_ptr->tlb_lock, flag);

	return ret_adr;
}

private mmap_dscp_t *hal_find_suitable_freedscp(alloc_mem_list_t *list, adr_t freeaddr)
{
	mmap_dscp_t *rtn_dscp = NULL;
	struct list_head *tmp_list = NULL;

	if(list == NULL)
		return NULL;

	if(list_empty_careful(&list->am_part_list) == FALSE)
	{
		list_for_each(tmp_list, &list->am_part_list)
		{
			rtn_dscp = list_entry(tmp_list, mmap_dscp_t, map_list);

			if(freeaddr < rtn_dscp->map_phy_endaddr && freeaddr >= rtn_dscp->map_phy_startaddr)
			{
				return rtn_dscp;
			}
		}	
	}
	if(list_empty_careful(&list->am_full_list) == FALSE)
	{
		list_for_each(tmp_list, &list->am_full_list)
		{
			rtn_dscp = list_entry(tmp_list, mmap_dscp_t, map_list);

			if(freeaddr < rtn_dscp->map_phy_endaddr && freeaddr >= rtn_dscp->map_phy_startaddr)
			{
				return rtn_dscp;
			}
		}	
	}
	return rtn_dscp;
}

private bool_t hal_freeblks(adr_t freeaddr, uint_t mflag, uint_t mask, alloc_mem_list_t *src_list, alloc_mem_list_t *dest_list)
{
	cpuflg_t flag;
	mmap_dscp_t *dscp;
	uint_t used_bits;

	/*寻找描述符*/
	dscp = hal_find_suitable_freedscp(src_list, freeaddr);

	if(dscp == NULL)
	{
		return FALSE;
	}	

	spin_lock_save(&dscp->map_lock, flag);

	used_bits = (freeaddr - dscp->map_phy_startaddr) / src_list->am_size;

	if(used_bits > 31)
	{
		return FALSE;
	}

	/*如果标志位没被设置，表示出错了*/
	if(((dscp->map_alloc_bit >> used_bits) & 0x01) != 0x01)
	{
		return FALSE;
	}

	/*复位使用标志*/
	dscp->map_alloc_bit &= ~(1 << used_bits);

	/*4M的内存已经全部释放了*/
	if((dscp->map_alloc_bit & mask) == 0)
	{
		dscp->map_flag &= ~0xf0;
		dscp->map_flag |= mflag;
		list_move_tail(&dscp->map_list, &dest_list->am_empty_list);
	}

	spin_unlock_restore(&dscp->map_lock, flag);

	return TRUE;
}


private bool_t hal_find_freeblks(adr_t freeaddr, alloc_mem_list_t *src_list, alloc_mem_list_t *dest_list, size_t blksz)
{
	bool_t retstus;
	switch(blksz)
	{
		case MEM_SIZE_128K:
			retstus = hal_freeblks(freeaddr, MEM_SUBBLK_4M, MEM_FLAG_MASK_128K, src_list, dest_list);
			break;
		case MEM_SIZE_256K:
			retstus = hal_freeblks(freeaddr, MEM_SUBBLK_4M, MEM_FLAG_MASK_256K, src_list, dest_list);
			break;		
		case MEM_SIZE_512K:
			retstus = hal_freeblks(freeaddr, MEM_SUBBLK_4M, MEM_FLAG_MASK_512K, src_list, dest_list);
			break;		
		case MEM_SIZE_1M:
			retstus = hal_freeblks(freeaddr, MEM_SUBBLK_4M, MEM_FLAG_MASK_1M, src_list, dest_list);
			break;		
		case MEM_SIZE_2M:
			retstus = hal_freeblks(freeaddr, MEM_SUBBLK_4M, MEM_FLAG_MASK_2M, src_list, dest_list);
			break;		
		case MEM_SIZE_4M:
			retstus = hal_freeblks(freeaddr, MEM_SUBBLK_4M, MEM_FLAG_MASK_4M, src_list, dest_list);
			break;
		default:
			retstus = FALSE;
	}
	return retstus;
}

/*
* 寻找alloc_mem_list_t并返回
*/
private alloc_mem_list_t *hal_find_suitable_freelist(mmap_tlb_t *tlb_ptr, size_t blksz)
{
	alloc_mem_list_t *rtn_list = NULL;

	for(uint_t i = 0; i < MEM_BLK_CLASS; i++)
	{
		if(tlb_ptr->tlb_allocmem_list[i].am_size == blksz)
		{
			rtn_list = &tlb_ptr->tlb_allocmem_list[i];
			break;
		}
	}

	/*在快表中找到了合适大小的链表*/
	if(rtn_list != NULL)
	{
		/*在部分链和全使用完的链中查找*/
		if(list_empty_careful(&rtn_list->am_part_list) == FALSE)
		{
			return rtn_list;
		}
		if(list_empty_careful(&rtn_list->am_full_list) == FALSE)
		{
			return rtn_list;
		}
		/*到这里说明不存在要释放的内存*/
		rtn_list = NULL;
	}
	return rtn_list;
}

bool_t hal_memfreeblks(adr_t freeaddr, size_t blksz)
{
	cpuflg_t flag;
	bool_t rtnstus;
	mmap_tlb_t *tlb_ptr = &mmap_tlb;
	alloc_mem_list_t *rnt_list, *moveto_list;

	if(freeaddr == NULL || MEM_BLK_INVALID(blksz))
	{
		return FALSE;
	}

	moveto_list = &tlb_ptr->tlb_allocmem_list[MEM_BLK_CLASS-1];

	spin_lock_save(&tlb_ptr->tlb_lock, flag);

	rnt_list = hal_find_suitable_freelist(tlb_ptr, blksz);

	if(rnt_list == NULL)
	{
		return FALSE;
	}

	rtnstus = hal_find_freeblks(freeaddr, rnt_list, moveto_list, blksz);

	spin_unlock_restore(&tlb_ptr->tlb_lock, flag);

	return rtnstus;
}

uint_t blk_print_4M_cnt(void)
{
	struct list_head *head, *tmp_list;
	//mmap_dscp_t *rtn_dscp;
	uint_t rtn = 0;

	head = &mmap_tlb.tlb_allocmem_list[MEM_BLK_CLASS-1].am_empty_list;

	list_for_each(tmp_list, head)
	{
		rtn++;
	}
	// rtn_dscp = list_entry(head->prev, mmap_dscp_t, map_list);
	// printk("last dscp: 0x%x	", rtn_dscp);
	return rtn;
}

#if 0
void blk_mem_test1(void)
{
	adr_t addr, tmpaddr[12];
	size_t blksz;

	addr = hal_memallocblks(MEM_SIZE_128K);

	if(addr == NULL)
	{
		system_error("alloc error!\n");
	}

	printk("4M cnt: %d\n", blk_print_4M_cnt());

	if(hal_memfreeblks(addr, MEM_SIZE_128K) == FALSE)
	{
		system_error("free error!\n");
	}
	printk("4M cnt: %d\n", blk_print_4M_cnt());

	for(uint_t i = 0; i < 6; i++)
	{
		blksz = MEM_SIZE_128K << i;

		for(uint_t j = 0; j < 2; j++)
		{
			tmpaddr[i*2+j] = hal_memallocblks(blksz);
			printk("alloc size: 0x%x, addr: 0x%x\n", blksz, tmpaddr[i*2+j]);
		}
	}
	printk("4M cnt: %d\n", blk_print_4M_cnt());

	for(uint_t i = 0; i < 6; i++)
	{
		blksz = MEM_SIZE_128K << i;

		for(uint_t j = 0; j < 2; j++)
		{
			if(hal_memfreeblks(tmpaddr[i*2+j], blksz) == TRUE)
			{
				printk("4M cnt: %d, alloc size: 0x%x, addr: 0x%x\n", blk_print_4M_cnt(), blksz, tmpaddr[i*2+j]);
			}
			else
			{
				system_error("free error!\n");
			}
		}
	}
}
#endif

void blk_mem_test2(void)
{
	adr_t addr, tmpaddr[12];
	size_t blksz;
	uint_t i, j;

	for(i = 0; i < 6; i++)
	{
		blksz = MEM_SIZE_128K << i;

		for(j = 0; j < 2; j++)
		{
			tmpaddr[j] = hal_memallocblks(blksz);

			printk("alloc blk(%d): alloc size: 0x%x, addr: 0x%x\n", j,blksz, tmpaddr[j]);
		}

		for(j = 0; j < 2; j++)
		{
			if(hal_memfreeblks(tmpaddr[j], blksz) == TRUE)
			{
				printk("free blk(%d): free size: 0x%x, addr: 0x%x\n", j, blksz, tmpaddr[j]);
			}
			else
			{
				system_error("free error!\n");
			}
		}
		printk("-----------------------------------------------------------------------------------\n");
	}

	addr = hal_memallocblks(MEM_SIZE_128K);

	if(addr == NULL)
		system_error("alloc error!\n");

	printk("4M cnt: %d, alloc size: 0x%x, addr: 0x%x\n", blk_print_4M_cnt(), blksz, addr);

}

