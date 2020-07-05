#ifndef __PRINTK_H__
#define __PRINTK_H__


#define system_error(fmt, ...)	{printk(fmt, ##__VA_ARGS__); for(;;);}

void printk(const s8_t *fmt, ...);
// void system_error(const s8_t *fmt, ...);

#endif
