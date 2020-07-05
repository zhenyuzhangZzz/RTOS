#include "lmosem.h"

/*检查mplhead是不是页级*/
private bool_t mplhead_isok_allocpagempl(mplhead_t *ptr, size_t size)
{
	if(ptr == NULL)
		return FALSE;

	if(ptr->mpl_headty != MPLHEAD_PAGE_TYPE)
		return FALSE;

	if(ptr->mpl_startaddr >= ptr->mpl_endaddr)
		return FALSE;

	if(ptr->mpl_pagesize != size)
		return FALSE;

	if(ptr->mpl_usedpagecnt >= ptr->mpl_pagecnt)
		return FALSE;

	return TRUE;
}

/*寻找合适的页级内存池*/
private mplhead_t *krnl_find_suitable_allocpagempl(krnl_mplmm_t *mplmm_ptr, size_t sizes)
{
	mplhead_t *rtn_mplhead;
	struct list_head *tmp_list;

	if(mplmm_ptr == NULL)
		return NULL;

	/*寻找最近操作的mplhead_t*/
	if(mplmm_ptr->kmpl_recentpagempl != NULL)
	{
		if(mplhead_isok_allocpagempl(mplmm_ptr->kmpl_recentpagempl, sizes) == TRUE)
		{
			return mplmm_ptr->kmpl_recentpagempl;
		}
	}
	/*寻找page list*/
	tmp_list = &mplmm_ptr->kmpl_pagelist;

	list_for_each(tmp_list, &mplmm_ptr->kmpl_pagelist)
	{
		rtn_mplhead = list_entry(tmp_list, mplhead_t, pml_list);

		if(mplhead_isok_allocpagempl(rtn_mplhead, sizes) == TRUE)
		{
			return rtn_mplhead;
		}
	}

	return NULL;
}

private mplhead_t *krnl_mplhead_init(krnl_mplmm_t *mplmm_ptr, mplhead_t *mplhead_ptr, size_t pagesize, adr_t start, adr_t end)
{
	adr_t tmpaddr;
	pglmap_t *pglmap_ptr;

	uint_t pglmap_cnt;

	if(mplmm_ptr == NULL || mplhead_ptr == NULL)
		return NULL;

	/*内存未对齐，页面不够分配一页 */
	if((start & (KRNL_PAGE_SIZE - 1)) != 0 || (end - start + 1) < (pagesize + KRNL_PAGE_SIZE))
		return NULL;

	/*初始化mplhead_t结构*/
	spin_lock_init(&mplhead_ptr->mpl_lock);
	init_list_head(&mplhead_ptr->pml_list);
	mplhead_ptr->mpl_headty = MPLHEAD_PAGE_TYPE;			
	mplhead_ptr->mpl_startaddr = start;		
	mplhead_ptr->mpl_endaddr = end;		
	mplhead_ptr->mpl_firstpage	= NULL;
	mplhead_ptr->mpl_pagecnt = 0;				
	mplhead_ptr->mpl_aliobsz = pagesize;
	mplhead_ptr->mpl_pagesize = pagesize;	
	mplhead_ptr->mpl_usedpagecnt = 0;				
	mplhead_ptr->mpl_pagedscpcnt = 0;					
	mplhead_ptr->mpl_pagedscpaddr = (pglmap_t *)((adr_t)mplhead_ptr + sizeof(mplhead_t));

	tmpaddr = mplhead_ptr->mpl_startaddr + KRNL_PAGE_SIZE;
	pglmap_ptr = mplhead_ptr->mpl_pagedscpaddr;

	for(pglmap_cnt = 0; tmpaddr+pagesize-1 <= end; tmpaddr += pagesize, pglmap_cnt++)
	{
		pglmap_ptr[pglmap_cnt].pgl_startaddr = tmpaddr;
		pglmap_ptr[pglmap_cnt].next = &pglmap_ptr[pglmap_cnt + 1];
	}

	/*表示有页面建立*/
	if(pglmap_cnt > 0)
	{
		/*最后一个页面指向空*/
		pglmap_ptr[pglmap_cnt - 1].next = NULL;							
		mplhead_ptr->mpl_firstpage = &pglmap_ptr[0];
	}
	else
	{
		pglmap_cnt = 0;
		pglmap_ptr[0].pgl_startaddr = NULL;
		pglmap_ptr[pglmap_cnt].next = NULL;
	}

	/*更新页面描述符计数和页面计数*/
	mplhead_ptr->mpl_pagedscpcnt = pglmap_cnt;
	mplhead_ptr->mpl_pagecnt = pglmap_cnt;
	/*将内存池加入内存池管理结构,这里添加到头部*/
	list_add(&mplhead_ptr->pml_list, &mplmm_ptr->kmpl_pagelist);
	/*计数+1*/
	mplmm_ptr->kmpl_pagemplcnt++;
	return mplhead_ptr;
}

private mplhead_t *krnl_new_pagempl(krnl_mplmm_t *mplmm_ptr, size_t sizes)
{
	/*根据页级内存块请求的大小分配合适的块*/
	uint_t blksize;
	adr_t blkaddr;

	uint_t pagenr = sizes / KRNL_PAGE_SIZE;   				/*基础块的倍数*/
	
	if(pagenr < 1)
		return NULL;

	// if(pagenr <= 2)
	// 	blksize = MEM_SIZE_128K;
	// else if(pagenr <= 16)
	// 	blksize = MEM_SIZE_256K;
	// else
		blksize = MEM_SIZE_128K;								/*此时一个128K只能分配出一个页面*/

	/*请求内存块*/
	blkaddr = hal_memallocblks(blksize);

	if(blkaddr == NULL)
		return NULL;

	/*
	*初始化内存块，写入mplhead_t和pglmap_t，默认取内存块的第一个页面（4KB）来存储
	*mplhead_t和pglmap_t，此处有待改进，可以新建新的内存池来动态实现，后面改进
	*/
	return krnl_mplhead_init(mplmm_ptr, (mplhead_t *)blkaddr, sizes, blkaddr, blkaddr + blksize - 1);
}

/*根据线程池页面描述符返回页面地址*/
private adr_t krnl_rtn_pageaddr(mplhead_t *mplhead_ptr)
{
	pglmap_t *tmp_map;
	adr_t rtn_addr = NULL;

	if(mplhead_ptr == NULL)
		return NULL;

	if(mplhead_ptr->mpl_usedpagecnt >= mplhead_ptr->mpl_pagecnt)
	{
		return NULL;
	}

	if(mplhead_ptr->mpl_firstpage != NULL)
	{
		tmp_map = mplhead_ptr->mpl_firstpage;

		rtn_addr = tmp_map->pgl_startaddr;
		mplhead_ptr->mpl_firstpage = tmp_map->next;
		tmp_map->next = NULL;

		mplhead_ptr->mpl_usedpagecnt++;
	}

	return rtn_addr;
}

private adr_t krnl_page_core_alloc(size_t size)
{
	cpuflg_t kmpl_flag, kmpl_pageflag;
	adr_t rtn_addr = NULL;
	mplhead_t *mpl_ptr;
	krnl_mplmm_t *mplmm_ptr = &mplmm;

	spin_lock_save(&mplmm_ptr->kmpl_lock, kmpl_flag);
	spin_lock_save(&mplmm_ptr->kmpl_pagelock, kmpl_pageflag);

	/*寻找符合大小的内存池并返回*/
	mpl_ptr = krnl_find_suitable_allocpagempl(mplmm_ptr, size);

	/*如果没找到合适的内存池，则取一块空闲的块级内存块并新建内存池*/
	if(mpl_ptr == NULL)
	{
		mpl_ptr = krnl_new_pagempl(mplmm_ptr, size);

		if(mpl_ptr == NULL)
		{
			goto alloc_tail;
		}
	}
	/*更新kmpl_recentpagempl*/
	mplmm_ptr->kmpl_recentpagempl = mpl_ptr;
	/*根据内存池结构返回内存池中空闲的页面*/
	rtn_addr = krnl_rtn_pageaddr(mpl_ptr);
alloc_tail:
	spin_unlock_restore(&mplmm_ptr->kmpl_pagelock, kmpl_pageflag);
	spin_unlock_restore(&mplmm_ptr->kmpl_lock, kmpl_flag);
	return rtn_addr;
}

private adr_t krnl_blks_alloc(size_t size)
{
	size_t tmpsize = NULL;

	if(size <= MEM_SIZE_128K)
		tmpsize = MEM_SIZE_128K;
	else if(size <= MEM_SIZE_256K)
		tmpsize = MEM_SIZE_256K;
	else if(size <= MEM_SIZE_512K)
		tmpsize = MEM_SIZE_512K;
	else if(size <= MEM_SIZE_1M)
		tmpsize = MEM_SIZE_1M;
	else if(size <= MEM_SIZE_2M)
		tmpsize = MEM_SIZE_2M;
	else if(size <= MEM_SIZE_4M)
		tmpsize = MEM_SIZE_4M;

	return hal_memallocblks(tmpsize);
}

adr_t krnl_page_alloc(size_t size)
{
	size_t alingsz = PAGE_ALIGN(size);

	if(alingsz > KRNL_PAGE_ALLOC_MAXSIZE)
	{
		return krnl_blks_alloc(alingsz);
	}
	return krnl_page_core_alloc(alingsz);
}

private bool_t mplhead_isok_freepagempl(mplhead_t *ptr, adr_t addr, size_t size)
{
	if(ptr == NULL)
		return FALSE;

	if(ptr->mpl_headty != MPLHEAD_PAGE_TYPE)
		return FALSE;

	if(ptr->mpl_pagesize != size)
		return FALSE;

	if(ptr->mpl_usedpagecnt == 0 || ptr->mpl_pagecnt == 0)
		return FALSE;

	
	if(addr < (ptr->mpl_startaddr + KRNL_PAGE_SIZE) || (addr + size - 1) > ptr->mpl_endaddr)
		return FALSE;
	return TRUE;
}

private mplhead_t *krnl_find_suitable_freepagempl(krnl_mplmm_t *mplmm_ptr, adr_t addr, size_t size)
{
	mplhead_t *rtn_mplhead;
	struct list_head *tmp_list;

	if(mplmm_ptr == NULL)
		return NULL;

	/*寻找最近操作的mplhead_t*/
	if(mplmm_ptr->kmpl_recentpagempl != NULL)
	{
		if(mplhead_isok_freepagempl(mplmm_ptr->kmpl_recentpagempl, addr, size) == TRUE)
		{
			return mplmm_ptr->kmpl_recentpagempl;
		}
	}
	/*寻找page list*/
	tmp_list = &mplmm_ptr->kmpl_pagelist;

	list_for_each(tmp_list, &mplmm_ptr->kmpl_pagelist)
	{
		rtn_mplhead = list_entry(tmp_list, mplhead_t, pml_list);

		if(mplhead_isok_freepagempl(rtn_mplhead, addr, size) == TRUE)
		{
			return rtn_mplhead;
		}
	}

	return NULL;
}

private bool_t krnl_free_pagempl(mplhead_t *mplhead_ptr, adr_t addr)
{
	pglmap_t *pgl_ptr = NULL;

	if(mplhead_ptr == NULL)
		return FALSE;

	for(uint_t i = 0; i < mplhead_ptr->mpl_pagecnt; i++)
	{
		if(mplhead_ptr->mpl_pagedscpaddr[i].pgl_startaddr == addr)
		{
			pgl_ptr = &mplhead_ptr->mpl_pagedscpaddr[i];

			/*表示链没有断开, 页面未使用*/
			if(mplhead_ptr->mpl_pagedscpaddr[i].next != NULL)
				return FALSE;
			break;
		}
	}

	/*未找到*/
	if(pgl_ptr == NULL)
		return FALSE;

	/*将该块页面插入到未使用的页面头部*/
	pgl_ptr->next = mplhead_ptr->mpl_firstpage;
	mplhead_ptr->mpl_firstpage = pgl_ptr;

	mplhead_ptr->mpl_usedpagecnt--;

	return TRUE;
}

private void krnl_delete_pagempl(krnl_mplmm_t *mplmm_ptr, mplhead_t *mplhead_ptr)
{
	uint_t fresize;

	if(mplmm_ptr == NULL || mplhead_ptr == NULL)
		return;

	/*内存池非空闲*/
	if(mplhead_ptr->mpl_usedpagecnt > 0)
		return;

	fresize = mplhead_ptr->mpl_endaddr + 1 - mplhead_ptr->mpl_startaddr;

	/*从链表删除*/
	list_del(&mplhead_ptr->pml_list);

	if(mplmm_ptr->kmpl_pagemplcnt > 0)
		mplmm_ptr->kmpl_pagemplcnt--;

	if(mplmm_ptr->kmpl_recentpagempl == mplhead_ptr)
		mplmm_ptr->kmpl_recentpagempl = NULL;

	if(hal_memfreeblks(mplhead_ptr->mpl_startaddr, fresize) == FALSE)
		system_error("delete page poll failed!\n");
}

private bool_t krnl_page_core_free(adr_t addr, size_t size)
{
	cpuflg_t flag;
	mplhead_t *mpl_ptr;
	bool_t rtn;
	krnl_mplmm_t *mplmm_ptr = &mplmm;

	spin_lock_save(&mplmm_ptr->kmpl_pagelock, flag);

	/*寻找符合大小的内存池并返回*/
	mpl_ptr = krnl_find_suitable_freepagempl(mplmm_ptr, addr, size);

	if(mpl_ptr == NULL)
	{
		rtn =  FALSE;
		goto free_tail;	
	}

	/*根据地址释放内存池页面*/
	if(krnl_free_pagempl(mpl_ptr, addr) == FALSE)
	{
		rtn =  FALSE;
		goto free_tail;	
	}

	/*检查页面是否已经释放完了，释放完了就删除内存池*/
	krnl_delete_pagempl(mplmm_ptr, mpl_ptr);

	rtn = TRUE;
free_tail:
	spin_unlock_restore(&mplmm_ptr->kmpl_pagelock, flag);
	return rtn;
}

private adr_t krnl_blks_free(adr_t addr, size_t size)
{
	size_t tmpsize = NULL;

	if(size <= MEM_SIZE_128K)
		tmpsize = MEM_SIZE_128K;
	else if(size <= MEM_SIZE_256K)
		tmpsize = MEM_SIZE_256K;
	else if(size <= MEM_SIZE_512K)
		tmpsize = MEM_SIZE_512K;
	else if(size <= MEM_SIZE_1M)
		tmpsize = MEM_SIZE_1M;
	else if(size <= MEM_SIZE_2M)
		tmpsize = MEM_SIZE_2M;
	else if(size <= MEM_SIZE_4M)
		tmpsize = MEM_SIZE_4M;

	return hal_memfreeblks(addr, tmpsize);
}

bool_t krnl_page_free(adr_t addr, size_t size)
{
	size_t alingsz = PAGE_ALIGN(size);

	if(alingsz > KRNL_PAGE_ALLOC_MAXSIZE)
	{
		return krnl_blks_free(addr, alingsz);
	}
	return krnl_page_core_free(addr, alingsz);
}