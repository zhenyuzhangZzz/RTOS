#ifndef __KRNLBYTESMM_H__
#define __KRNLBYTESMM_H__


#define BYTE_ALIGN(size)	ALIGN(size, KRNL_BYTE_ALIGN_SIZE)

adr_t krnl_bytes_alloc(size_t size);
bool_t krnl_bytes_free(adr_t addr, size_t size);

#endif