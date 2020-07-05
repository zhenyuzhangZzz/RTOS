#ifndef __KRNLPAGEMM_H__
#define __KRNLPAGEMM_H__

#define PAGE_ALIGN(size)	ALIGN(size, KRNL_PAGE_SIZE)

adr_t krnl_page_alloc(size_t sizes);
bool_t krnl_page_free(adr_t addr, size_t size);

#endif