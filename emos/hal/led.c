#include "lmosem.h"


void led_init(void)
{
	write_io(GPC0CON, read_io(GPC0CON) | 0x00011000);
}

void led_change_status(s32_t status)
{
	if(status)
		write_io(GPC0DAT, read_io(GPC0DAT) | 0x00000018);
	else
		write_io(GPC0DAT, read_io(GPC0DAT) & ~(0x03 << 3));
}

