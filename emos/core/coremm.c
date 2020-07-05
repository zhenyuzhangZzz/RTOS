#include "lmosem.h"

private void krnl_mpl_init(krnl_mplmm_t *mplmm_ptr)
{
	spin_lock_init(&mplmm_ptr->kmpl_lock);

	init_list_head(&mplmm_ptr->kmpl_list);

	mplmm_ptr->kmpl_stus = 0;
	mplmm_ptr->kmpl_flag = 0;
	spin_lock_init(&mplmm_ptr->kmpl_pagelock);
	spin_lock_init(&mplmm_ptr->kmpl_bytelock);
	mplmm_ptr->kmpl_pagemplcnt = 0;
	mplmm_ptr->kmpl_bytemplcnt = 0;
	init_list_head(&mplmm_ptr->kmpl_pagelist);
	init_list_head(&mplmm_ptr->kmpl_bytelist);
	mplmm_ptr->kmpl_recentpagempl = NULL;
	mplmm_ptr->kmpl_recentbytempl = NULL;
}

void krnl_mm_init(void)
{
	krnl_mpl_init(&mplmm);
}

adr_t krnl_alloc(size_t size)
{
	adr_t rtn_addr = NULL;

	if(size == KRNL_ALLOC_MINSIZE || size > KRNL_ALLOC_MAXSIZE)
	{
		return NULL;
	}

	/*大于最小页面2K，调用页级分配函数*/
	if(size > KRNL_BYTES_MAXSIZE)
	{
		rtn_addr = krnl_page_alloc(size);
	}
	else
	{
		rtn_addr = krnl_bytes_alloc(size);
	}
	return rtn_addr;
}

bool_t krnl_free(adr_t addr, size_t size)
{
	bool_t rtn = FALSE;

	if(addr == NULL || size == KRNL_ALLOC_MINSIZE || size > KRNL_ALLOC_MAXSIZE)
	{
		return rtn;
	}

	if(size > KRNL_BYTES_MAXSIZE)
	{
		rtn = krnl_page_free(addr, size);
	}
	else
	{
		rtn = krnl_bytes_free(addr, size);
	}
	return rtn;
}

#if 0
typedef struct s_adrsz
{
    adr_t adr;
    size_t sz; 
}adrsz_t;


void testpgmpool(void)
{
    adrsz_t adsz[10];
    size_t alcsz=0x1000;
    adsz[0].sz=alcsz;
    adsz[0].adr=krnl_alloc(alcsz);
    adsz[1].sz=alcsz;
    adsz[1].adr=krnl_alloc(alcsz);
    alcsz=0x1500;
    adsz[2].sz=alcsz;
    adsz[2].adr=krnl_alloc(alcsz);
    adsz[3].sz=alcsz;
    adsz[3].adr=krnl_alloc(alcsz);
    alcsz=0x3000;
    adsz[4].sz=alcsz;
    adsz[4].adr=krnl_alloc(alcsz);
    adsz[5].sz=alcsz;
    adsz[5].adr=krnl_alloc(alcsz);
    alcsz=0x3200;
    adsz[6].sz=alcsz;
    adsz[6].adr=krnl_alloc(alcsz);
    adsz[7].sz=alcsz;
    adsz[7].adr=krnl_alloc(alcsz);
    alcsz=0x7000;
    adsz[8].sz=alcsz;
    adsz[8].adr=krnl_alloc(alcsz);
    adsz[9].sz=alcsz;
    adsz[9].adr=krnl_alloc(alcsz);
    for(int i=0;i<10;i++)
    {
        printk("adsz[%x] sz:%x adr:%x\n\r",i,adsz[i].sz,adsz[i].adr);
    }
    mplhead_t* retmhp;
    struct list_head *list;

    list_for_each(list, &mplmm.kmpl_pagelist)
    {
        retmhp=list_entry(list,mplhead_t,pml_list);
        printk("mplhead addr:0x%x mplhead.mpl_endaddr:0x%x mplhead.mpl_pagesize:0x%x mplhead.mpl_pagecnt:%d mplhead.mpl_usedpagecnt: %d\n",
                retmhp,retmhp->mpl_endaddr,retmhp->mpl_pagesize,retmhp->mpl_pagecnt, retmhp->mpl_usedpagecnt);
    }
    return;
}

void cmp_pageadrsz(adrsz_t* assp,uint_t nr)
{
    for(uint_t i=0;i<nr;i++)
    {
        for(uint_t j=0;j<nr;j++)
        {
            if(i!=j)
            {
                if(assp[i].adr==assp[j].adr)
                {
                    system_error("cmp adr start err");
                }
            }
        }
    }

    for(uint_t k=0;k<nr;k++)
    {
        for(uint_t h=0;h<nr;h++)
        {
            if(k!=h)
            {
                if((assp[k].adr+assp[k].sz)==(assp[h].adr+assp[h].sz))
                {
                    system_error("cmp adr end err");
                }
            }
        }
    }

    for(uint_t l=0;l<nr;l++)
    {
        for(uint_t m=0;m<nr;m++)
        {
            if(l!=m)
            {
                if((assp[l].adr>=(assp[m].adr))&&((assp[l].adr+assp[l].sz)<=(assp[m].adr+assp[m].sz)))
                {
                    system_error("cmp adr in err");
                }
            }
        }
    }
    return;
}

void testpagemgr()
{
    adrsz_t adsz[40];
    size_t alcsz=0x1000;
    for(;alcsz<0x6000;alcsz+=0x1000)
    {
        for(int i=0;i<40;i++)
        {
            adsz[i].sz=alcsz;
            adsz[i].adr=krnl_alloc(alcsz);
            if(adsz[i].adr==NULL)
            {
                system_error("testpagemgr kmempool_new err");
            }
            printk("adsz[%d] sz:%x adr:%x\n\r",i,adsz[i].sz,adsz[i].adr);
        }
        printk("kmpl_pagemplcnt:%d\n\r",mplmm.kmpl_pagemplcnt);
        cmp_pageadrsz(adsz,40);
        for(int j=0;j<40;j++)
        {
            if(krnl_free(adsz[j].adr,adsz[j].sz)==FALSE)
            {
                system_error("testpagemgr kmempool_delete err");
            }
            printk("delete adsz[%d] sz:%x adr:%x\n\r",j,adsz[j].sz,adsz[j].adr);
        }
        printk("kmpl_pagemplcnt:%d\n\r",mplmm.kmpl_pagemplcnt);
    }
   
    return;
}

void cmp_byteadrsz(adrsz_t* assp,uint_t nr)
{
    for(uint_t i=0;i<nr;i++)
    {
        for(uint_t j=0;j<nr;j++)
        {
            if(i!=j)
            {
                if(assp[i].adr==assp[j].adr)
                {
                    system_error("cmp adr start err");
                }
            }
        }
    }

    for(uint_t k=0;k<nr;k++)
    {
        for(uint_t h=0;h<nr;h++)
        {
            if(k!=h)
            {
                if((assp[k].adr+assp[k].sz)==(assp[h].adr+assp[h].sz))
                {
                    system_error("cmp adr end err");
                }
            }
        }
    }

    for(uint_t l=0;l<nr;l++)
    {
        for(uint_t m=0;m<nr;m++)
        {
            if(l!=m)
            {
                if((assp[l].adr>=(assp[m].adr))&&((assp[l].adr+assp[l].sz)<=(assp[m].adr+assp[m].sz)))
                {
                    system_error("cmp adr in err");
                }
            }
        }
    }
    u8_t* bytp=NULL,bytv=0;
    for(uint_t n=0;n<nr;n++)
    {
        bytp=(u8_t*)(assp[n].adr);
        bytv=(u8_t)(assp[n].adr&0xff);
        for(uint_t o=0;o<assp[n].sz;o++)
        {
            if(bytp[o]!=bytv)
            {
                system_error("cmp val err");
            }
        }
    }
    return;
}

void testobjsmgr(void)
{
    adrsz_t adsz[64];
    size_t alcsz=0x20;
    u8_t* adrbytp=NULL,bytval=0;

    for(;alcsz<0x50;alcsz+=0x10)
    {

        for(int i=0;i<64;i++)
        {
            adsz[i].sz=alcsz;
            adsz[i].adr=krnl_alloc(alcsz);
            if(adsz[i].adr==NULL)
            {
                system_error("testobjsmgr kmempool_new err");
            }
            printk("objs alloc adsz[%d] sz:%x adr:%x\n\r",i,adsz[i].sz,adsz[i].adr);
            adrbytp=(u8_t*)adsz[i].adr;
            bytval=(u8_t)(adsz[i].adr&0xff);
            for(size_t k=0; k<adsz[i].sz; k++)
            {
                adrbytp[k]=bytval;
            }
        }
        printk("kmpl_pagemplcnt:%d kmpl_bytemplcnt: %d\n\r",mplmm.kmpl_pagemplcnt, mplmm.kmpl_bytemplcnt);
        cmp_byteadrsz(adsz,64);
        for(int j=0;j<63;j++)
        {
            if(krnl_free(adsz[j].adr,adsz[j].sz)==FALSE)
            {
                system_error("testobjsmgr kmempool_delete err");
            }
            printk("objs delete adsz[%d] sz:%x adr:%x\n\r",j,adsz[j].sz,adsz[j].adr);
        }
        printk("kmpl_pagemplcnt:%d kmpl_bytemplcnt: %d\n\r",mplmm.kmpl_pagemplcnt, mplmm.kmpl_bytemplcnt);
    }
      
    return;
}
#endif
