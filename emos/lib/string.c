#include "lib.h"

void *memcpy(void *dest, const void *src, int n)
{
	unsigned char *dest_p = (unsigned char  *)dest, *src_p = (unsigned char  *)src;

	while(dest == NULL || src == NULL)
		return NULL;

	while(n--)
		*dest_p++ = *src_p++;

	return dest;
}

char *strcpy(char *dest, const char *src)
{
	char *p = dest;

	if(dest == NULL || src == NULL)
		return NULL;

	do{
		*p = *src++;
	}while(*p++);

	return dest;
}

char *strlcpy(char *dest, const char *src, int n)
{
    char *p = dest;

    if(dest == NULL || src == NULL)
        return NULL;

    while(n--)
    {
        *p++ = *src++;
    }
    return dest;
}

void *memset(void *s, int c, int n)
{
	char  *p = (char  *)s;
    char tmp = (char)(c & 0xff);

	while(n--)
		*p++ = tmp;
	return s;
}

unsigned int strlen(const char *s)
{
	int cnt = 0;

	while(s == NULL)
		return 0;

	while(*s++ && ++cnt);
		
	return cnt;
}

int strcmp(const char *s1, const char *s2)
{
    if(s1 == NULL)
    	return *s2;
    else if(s2 == NULL)
    	return *s1;

    while(*s1 && *s2 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

int atoi(const char *str)
{
    int temp = 0, index = 10;

    const char *ptr = str;

    if(*str == '-' || *str == '+')
    {
        str++;
    }
    else if(str[0] == '0' && str[1] == 'x')
    {
        str += 2;
        index = 16;
    }

    while(*str)
    {
        if(*str < '0' || *str > '9')
        {
                break;
        }

        temp = temp * index + (*str - '0');
        str++;
    }

    if(*ptr == '-')
    {
        temp = - temp;
    }

    return temp;
}

char *itoa(int num, char *str, int radix)
{
        char index[] = "0123456789ABCDEF";
        char temp_c;
        unsigned int temp_num;
        unsigned char i = 0, j, k = 0;

        //十进制负数
        if(num < 0 && radix == 10)
        {
                temp_num = -num;
                str[i++] = '-';
        }
        else
                temp_num = (unsigned char)num;

        //按进制转换
        do
        {
                str[i++] = index[temp_num % (unsigned char)radix];
                temp_num = temp_num / radix;
        }while(temp_num);

        //添加结束符
        str[i] = '\0';

        //逆序
        if(str[0] == '-') k = 1;

        for(j = k; j <= (i - 1) / 2; j++)
        {
                temp_c = str[j];
                str[j] = str[i - 1 + k - j];
                str[i - 1 + k - j] = temp_c;
        }

        return str;
}

char *strstr(const char *src, const char *sub)
{
        const char *src_p, *sub_p;

        if(!*src || !*sub)
        {
                return (char *)src;
        }

        while(*src)
        {
                src_p = src;
                sub_p = sub;

                do
                {
                        if(!*sub_p)
                        {
                                return (char *)src;
                        }
                }while(*src_p++ == *sub_p++);

                src++;
        }

        return NULL;
}