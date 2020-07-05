#ifndef __KRNLBITMAP_H__
#define __KRNLBITMAP_H__

typedef struct bitmap
{
	u32_t count[2];				/*64个优先级*/
}bitmap_t;

void bitmap_init(bitmap_t *map);
uint_t bitmap_bit_count(void);
void bitmap_set_bit(bitmap_t *map, uint_t pos);
void bitmap_clear_bit(bitmap_t *map, uint_t pos);
uint_t bitmap_get_first_set(bitmap_t *map);

#endif