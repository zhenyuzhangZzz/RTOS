#include "lmosem.h"

/*检查mplhead是不是页级*/
private bool_t mplhead_isok_allocbytempl(mplhead_t *ptr, size_t size)
{
	if(ptr == NULL)
		return FALSE;

	/*表示未初始化或者页面已经使用完了*/
	if(ptr->mpl_firstbyteaddr == NULL)
		return FALSE;

	if(ptr->mpl_headty != MPLHEAD_BYTE_TYPE)
		return FALSE;

	if(ptr->mpl_startaddr >= ptr->mpl_endaddr)
		return FALSE;

	if(ptr->mpl_pagesize != size)
		return FALSE;

	if(ptr->mpl_usedpagecnt >= ptr->mpl_pagecnt)
		return FALSE;

	return TRUE;
}

/*寻找合适的字级内存池*/
private mplhead_t *krnl_find_suitable_allocbytempl(krnl_mplmm_t *mplmm_ptr, size_t sizes)
{
	mplhead_t *rtn_mplhead;
	struct list_head *tmp_list;

	if(mplmm_ptr == NULL)
		return NULL;

	/*寻找最近操作的mplhead_t*/
	if(mplmm_ptr->kmpl_recentbytempl != NULL)
	{
		if(mplhead_isok_allocbytempl(mplmm_ptr->kmpl_recentbytempl, sizes) == TRUE)
		{
			return mplmm_ptr->kmpl_recentbytempl;
		}
	}
	/*寻找page list*/
	tmp_list = &mplmm_ptr->kmpl_bytelist;

	list_for_each(tmp_list, &mplmm_ptr->kmpl_bytelist)
	{
		rtn_mplhead = list_entry(tmp_list, mplhead_t, pml_list);

		if(mplhead_isok_allocbytempl(rtn_mplhead, sizes) == TRUE)
		{
			return rtn_mplhead;
		}
	}

	return NULL;
}

private mplhead_t *krnl_mplhead_init(krnl_mplmm_t *mplmm_ptr, mplhead_t *mplhead_ptr, size_t pagesize, adr_t start, adr_t end)
{
	adr_t tmpaddr, *next_ptr;
	uint_t addr_offset;

	if(mplmm_ptr == NULL || mplhead_ptr == NULL)
		return NULL;

	/*分配的一个页面4K内存，判断地址是否对齐，长度是否出错*/
	if((start & (KRNL_PAGE_SIZE - 1)) != 0 || (end - start + 1) < (KRNL_PAGE_SIZE))
		return NULL;

	tmpaddr = start + sizeof(mplhead_t);

	if((tmpaddr & (KRNL_BYTE_ALIGN_SIZE - 1)) != 0)
		system_error("byte memory alloc addr is not align at 4byte!");
	
	/*设置mplhead_t结构*/
	spin_lock_init(&mplhead_ptr->mpl_lock);
	init_list_head(&mplhead_ptr->pml_list);
	mplhead_ptr->mpl_headty = MPLHEAD_BYTE_TYPE;			
	mplhead_ptr->mpl_startaddr = start;		
	mplhead_ptr->mpl_endaddr = end;		
	mplhead_ptr->mpl_firstbyteaddr	= tmpaddr;
	mplhead_ptr->mpl_pagecnt = 0;				
	mplhead_ptr->mpl_aliobsz = pagesize;
	mplhead_ptr->mpl_pagesize = pagesize;	
	mplhead_ptr->mpl_nextpageoffset = pagesize + sizeof(adr_t);
	mplhead_ptr->mpl_usedpagecnt = 0;				

	addr_offset = mplhead_ptr->mpl_nextpageoffset;

	for(; tmpaddr + addr_offset < end; tmpaddr += addr_offset, mplhead_ptr->mpl_pagecnt++)
	{
		next_ptr = (adr_t *)(tmpaddr + pagesize);

		*next_ptr = tmpaddr + addr_offset;
	}

	if(mplhead_ptr->mpl_pagecnt == 0)
		system_error("byte memory alloc mplhead_t init failed!");

	/*将最后一个页面的指针设置为*/
	next_ptr = (adr_t *)(tmpaddr - addr_offset + pagesize);
	*next_ptr = NULL;

	/*将内存池加入内存池管理结构,这里添加到头部*/
	list_add(&mplhead_ptr->pml_list, &mplmm_ptr->kmpl_bytelist);
	/*计数+1*/
	mplmm_ptr->kmpl_bytemplcnt++;
	return mplhead_ptr;
}

private mplhead_t *krnl_new_bytempl(krnl_mplmm_t *mplmm_ptr, size_t sizes)
{
	/*根据页级内存块请求的大小分配合适的块*/
	adr_t blkaddr;

	/*向页级内存池请求一个页面4K的内存*/
	blkaddr = krnl_page_alloc(KRNL_PAGE_SIZE);

	if(blkaddr == NULL)
		return NULL;

	/*
	*初始化内存块，写入mplhead_t和pglmap_t，默认取内存块的第一个页面（4KB）来存储
	*mplhead_t和pglmap_t，此处有待改进，可以新建新的内存池来动态实现，后面改进
	*/
	return krnl_mplhead_init(mplmm_ptr, (mplhead_t *)blkaddr, sizes, blkaddr, blkaddr + KRNL_PAGE_SIZE - 1);
}

/*根据内存池池页面描述符返回页面地址*/
private adr_t krnl_rtn_byteaddr(mplhead_t *mplhead_ptr)
{
	adr_t *adr_ptr;
	adr_t rtn_addr = NULL;

	if(mplhead_ptr == NULL)
		return NULL;

	if(mplhead_ptr->mpl_usedpagecnt >= mplhead_ptr->mpl_pagecnt)
	{
		return NULL;
	}

	if(mplhead_ptr->mpl_firstbyteaddr != NULL)
	{
		rtn_addr = mplhead_ptr->mpl_firstbyteaddr;
		adr_ptr = (adr_t *)(rtn_addr + mplhead_ptr->mpl_aliobsz);
		mplhead_ptr->mpl_firstbyteaddr = *adr_ptr;

		*adr_ptr = NULL;
		mplhead_ptr->mpl_usedpagecnt++;
	}

	return rtn_addr;
}

private adr_t krnl_byte_core_alloc(size_t size)
{
	cpuflg_t kmpl_flag, kmpl_byteflag;
	adr_t rtn_addr = NULL;
	mplhead_t *mpl_ptr;
	krnl_mplmm_t *mplmm_ptr = &mplmm;

	spin_lock_save(&mplmm_ptr->kmpl_lock, kmpl_flag);
	spin_lock_save(&mplmm_ptr->kmpl_bytelock, kmpl_byteflag);

	/*寻找符合大小的内存池并返回*/
	mpl_ptr = krnl_find_suitable_allocbytempl(mplmm_ptr, size);

	/*如果没找到合适的内存池，则取一块空闲的块级内存块并新建内存池*/
	if(mpl_ptr == NULL)
	{
		mpl_ptr = krnl_new_bytempl(mplmm_ptr, size);

		if(mpl_ptr == NULL)
		{
			goto alloc_tail;
		}
	}
	
	/*更新kmpl_recentpagempl*/
	mplmm_ptr->kmpl_recentbytempl = mpl_ptr;
	/*根据内存池结构返回内存池中空闲的页面*/
	rtn_addr = krnl_rtn_byteaddr(mpl_ptr);
alloc_tail:
	spin_unlock_restore(&mplmm_ptr->kmpl_bytelock, kmpl_byteflag);
	spin_unlock_restore(&mplmm_ptr->kmpl_lock, kmpl_flag);
	return rtn_addr;
}

adr_t krnl_bytes_alloc(size_t size)
{
	// size_t alingsz = BYTE_ALIGN(size);
	// printk("alingsz: %d\n", alingsz);
	return krnl_byte_core_alloc(size);
}


private bool_t mplhead_isok_freebytempl(mplhead_t *ptr, adr_t addr, size_t size)
{
	if(ptr == NULL)
		return FALSE;

	if(ptr->mpl_headty != MPLHEAD_BYTE_TYPE)
		return FALSE;

	if(ptr->mpl_aliobsz != size)
		return FALSE;

	if(ptr->mpl_usedpagecnt == 0 || ptr->mpl_pagecnt == 0)
		return FALSE;

	/*地址交叉*/
	if(addr < (ptr->mpl_startaddr + sizeof(mplhead_t)) || (addr + size - 1) > ptr->mpl_endaddr)
		return FALSE;

	return TRUE;
}

private mplhead_t *krnl_find_suitable_freebytempl(krnl_mplmm_t *mplmm_ptr, adr_t addr, size_t size)
{
	mplhead_t *rtn_mplhead;
	struct list_head *tmp_list;

	if(mplmm_ptr == NULL)
		return NULL;

	/*寻找最近操作的mplhead_t*/
	if(mplmm_ptr->kmpl_recentbytempl != NULL)
	{
		if(mplhead_isok_freebytempl(mplmm_ptr->kmpl_recentbytempl, addr, size) == TRUE)
		{
			return mplmm_ptr->kmpl_recentbytempl;
		}
	}
	/*寻找page list*/
	tmp_list = &mplmm_ptr->kmpl_bytelist;

	list_for_each(tmp_list, &mplmm_ptr->kmpl_bytelist)
	{
		rtn_mplhead = list_entry(tmp_list, mplhead_t, pml_list);

		if(mplhead_isok_freebytempl(rtn_mplhead, addr, size) == TRUE)
		{
			return rtn_mplhead;
		}
	}

	return NULL;
}

private bool_t krnl_free_bytempl(mplhead_t *mplhead_ptr, adr_t addr)
{
	adr_t *next_ptr;

	if(mplhead_ptr == NULL)
		return FALSE;

	if(addr + mplhead_ptr->mpl_aliobsz > mplhead_ptr->mpl_endaddr)
		return FALSE;

	next_ptr = (adr_t *)(addr + mplhead_ptr->mpl_aliobsz);

	/*再分配时已经断开了，若未断开则表示出错了*/
	if(*next_ptr != NULL)
		return FALSE;

	/*将addr代表的页面接入链表*/
	*next_ptr = mplhead_ptr->mpl_firstbyteaddr;
	mplhead_ptr->mpl_firstbyteaddr = addr;

	mplhead_ptr->mpl_usedpagecnt--;

	return TRUE;
}

private void krnl_delete_bytempl(krnl_mplmm_t *mplmm_ptr, mplhead_t *mplhead_ptr)
{
	uint_t fresize;

	if(mplmm_ptr == NULL || mplhead_ptr == NULL)
		return;

	/*内存池非空闲*/
	if(mplhead_ptr->mpl_usedpagecnt > 0)
		return;

	/*释放大小*/
	fresize = mplhead_ptr->mpl_endaddr + 1 - mplhead_ptr->mpl_startaddr;

	/*从链表删除*/
	list_del(&mplhead_ptr->pml_list);

	if(mplmm_ptr->kmpl_bytemplcnt > 0)
		mplmm_ptr->kmpl_bytemplcnt--;

	if(mplmm_ptr->kmpl_recentbytempl == mplhead_ptr)
		mplmm_ptr->kmpl_recentbytempl = NULL;

	if(krnl_page_free(mplhead_ptr->mpl_startaddr, fresize) == FALSE)
		system_error("delete byte poll failed!\n");
}

private bool_t krnl_byte_core_free(adr_t addr, size_t size)
{
	cpuflg_t kmpl_flag, kmpl_byteflag;
	mplhead_t *mpl_ptr;
	bool_t rtn;
	krnl_mplmm_t *mplmm_ptr = &mplmm;

	spin_lock_save(&mplmm_ptr->kmpl_lock, kmpl_flag);
	spin_lock_save(&mplmm_ptr->kmpl_bytelock, kmpl_byteflag);

	/*寻找符合大小的内存池并返回*/
	mpl_ptr = krnl_find_suitable_freebytempl(mplmm_ptr, addr, size);

	if(mpl_ptr == NULL)
	{
		rtn =  FALSE;
		goto free_tail;	
	}

	/*根据地址释放内存池页面*/
	if(krnl_free_bytempl(mpl_ptr, addr) == FALSE)
	{
		rtn =  FALSE;
		goto free_tail;	
	}

	/*检查页面是否已经释放完了，释放完了就删除内存池*/
	krnl_delete_bytempl(mplmm_ptr, mpl_ptr);

	rtn = TRUE;
free_tail:
	spin_unlock_restore(&mplmm_ptr->kmpl_bytelock, kmpl_byteflag);
	spin_unlock_restore(&mplmm_ptr->kmpl_lock, kmpl_flag);
	return rtn;
}

bool_t krnl_bytes_free(adr_t addr, size_t size)
{
	size_t alingsz = BYTE_ALIGN(size);

	return krnl_byte_core_free(addr, alingsz);;
}