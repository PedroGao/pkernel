#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_

#include "types.h"

// 端口写一个字节
void outb(uint16_t port, uint8_t value);

// 端口读一个字节
uint8_t inb(uint16_t port);

// 端口读一个字
uint16_t inw(uint16_t port);

// 开启中断
static void enable_intr()
{
    asm volatile("sti");
}

// 关闭中断
static void disable_intr()
{
    asm volatile("cli" ::
                     : "memory");
}

#endif // INCLUDE_COMMON_H_
