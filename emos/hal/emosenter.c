#include "lmosem.h"

LKHEAD_T void lmosemhal_start(void)
{
	/* 硬件层hal初始化 */
	hal_init();

	/* 内核层kernel初始化 */
	krnl_init();

	return;
}