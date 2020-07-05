#include "lmosem.h"

// #define QUICKTABLE_GETNUM(x) (((x & 0x0f) == 0) ? (quick_find_ne\wtable[0][x >> 4]) : \
// 								quick_find_newtable[1][x & 0x0f])

// private u8_t quick_find_newtable[2][8] = 
// {
// 		//第一列数据，16的倍数
// 	    0xff, 4, 5, 4, 6, 4, 5, 4, 7, 4, 5, 4, 6, 4, 5, 4,
// 		//第一行数据，最后一个0xff为补全字节
// 	    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 0xff, 
// };

private char quick_find_table[] = 
{
	/* 00 */ 0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* 10 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* 20 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* 30 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* 40 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* 50 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* 60 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* 70 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* 80 */ 7,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* 90 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* A0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* B0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* C0 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* D0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* E0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	/* F0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
};

void bitmap_init(bitmap_t *map)
{
	map->count[0] = 0;
	map->count[1] = 0;
}

uint_t bitmap_bit_count(void)
{
	return 64;
}

void bitmap_set_bit(bitmap_t *map, uint_t pos)
{
	if(pos > 31)
		map->count[1] |= 1 << (pos - 32); 
	else
		map->count[0] |= 1 << pos;
}

void bitmap_clear_bit(bitmap_t *map, uint_t pos)
{
	if(pos > 31)
		map->count[1] &= ~(1 << (pos - 32)); 
	else
		map->count[0] &= ~(0x01 << pos);
}

uint_t bitmap_get_first_set(bitmap_t *map)
{
	u32_t count0 = map->count[0], count1 = map->count[1];

	if(count0)
	{
		if(count0 & 0xFF)
		{
			return quick_find_table[count0 & 0xFF];
		}
		else if(count0 & 0xFF00)
		{
			return quick_find_table[(count0 >> 8) & 0xFF] + 8;
		}
		else if(count0 & 0xFF0000)
		{
			return quick_find_table[(count0 >> 16) & 0xFF] + 16;
		}
		else if(count0 & 0xFF000000)
		{
			return quick_find_table[(count0 >> 24) & 0xFF] + 24;
		}
	}
	else if(count1)
	{
		if(count1 & 0xFF)
		{
			return quick_find_table[count1 & 0xFF] + 32;
		}
		else if(count1 & 0xFF00)
		{
			return quick_find_table[(count1 >> 8) & 0xFF] + 40;
		}
		else if(count1 & 0xFF0000)
		{
			return quick_find_table[(count1 >> 16) & 0xFF] + 48;
		}
		else if(count1 & 0xFF000000)
		{
			return quick_find_table[(count1 >> 24) & 0xFF] + 56;
		}
	}
	return bitmap_bit_count();
}
