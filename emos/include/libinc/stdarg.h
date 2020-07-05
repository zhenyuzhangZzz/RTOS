#ifndef __STDARG_H__
#define __STDARG_H__

/*用于实现可变参数*/
#define va_list s8_t *

#define _va_sizeof(TYPE) (((sizeof(TYPE) + sizeof(s32_t) - 1) / sizeof(s32_t)) * sizeof(s32_t))				//保证TYPE是四字节对齐的

#define va_start(AP, TARGET) (AP = (s8_t *)(&TARGET) + _va_sizeof(TARGET))									//取TARGET的地址转换为char *后 + TARGET大小，AP指向第一个可变参数
#define va_arg(AP, TYPE) (AP += _va_sizeof(TYPE), *((TYPE *)(AP - _va_sizeof(TYPE))))						//AP指向下一个可变参数，同时返回这一个可变参数的值
#define va_end(AP) (AP = (s8_t *)NULL)

#endif