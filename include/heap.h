#ifndef INCLUDE_HEAP_H_
#define INCLUDE_HEAP_H_

#include "types.h"

// 堆起始地址，一般在 BSS 段的结束位置
#define HEAP_START 0xE0000000

// 内存块管理结构，堆的基本分配单位是块
typedef struct header
{
    struct header *prev;    // 块的头指针
    struct header *next;    // 尾指针
    uint32_t allocated : 1; // 是否已经被申请，这里是位域定义，节省内存，只会占一个 bit
    uint32_t length : 32;   // 内存块的长度
} header_t;

// 初始化堆
void init_heap();

// 内存申请
void *kmalloc(uint32_t len);

// 内存释放
void kfree(void *p);

// 测试内核堆申请与释放
void test_heap();

#endif //INCLUDE_HEAP_H_