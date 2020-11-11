#include "string.h"

inline void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len)
{
    // 原理：指针取值覆盖
    for (; len != 0; len--)
    {
        *dest++ = *src++;
    }
}

inline void memset(void *dest, uint8_t val, uint32_t len)
{
    uint8_t *dst = (uint8_t *)dest;
    for (; len != 0; len--)
    {
        *dst++ = val;
    }
}

inline void bzero(void *dest, uint32_t len)
{
    memset(dest, 0, len);
}

inline int strcmp(const char *str1, const char *str2)
{
    while (*str1 && *str2 && *str1 == *str2)
    {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

inline char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;
    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0';
    return tmp;
}

inline char *strcat(char *dest, const char *src)
{
    char *cp = dest;
    while (*cp)
    {
        cp++;
    }
    while ((*cp++ = *src++))
        ;
    return dest;
}

inline int strlen(const char *src)
{
    const char *eos = src;
    while (*eos++)
        ;
    return (eos - src - 1);
}
