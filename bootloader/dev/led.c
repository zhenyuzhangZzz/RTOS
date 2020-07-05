#include "led.h"


void led_init(void)
{
	LEDCON |= 0x00011000;
}

void led_change_status(int status)
{
	if(status)
		LEDDAT |= 0x00000018;
	else
		LEDDAT &= ~(0x03 << 3);
}

