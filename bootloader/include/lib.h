#ifndef __TYPE_H__
#define __TYPE_H__

#define NULL  0

void *memcpy(void *dest, const void *src, int n);

char *strcpy(char *dest, const char *src);

void *memset(void *s, int c, int n);

unsigned int strlen(const char *s);

int strcmp(const char *s1, const char *s2);


int atoi(const char *str);

char *itoa(int num, char *str, int radix);

char *strstr(const char *src, const char *sub);

#endif