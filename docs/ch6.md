# 全局段描述符表

## 保护模式

在 80386 之前，CPU 只有一种模式，现在称之为 `实模式` 。

到了 80386，引入了保护模式，保护模式有一些新的特色, 用来增强多工和系统稳定度, 比如内存保护, 分页系
统, 以及硬件支持的虚拟内存等。

### 内存分段

内存分段的最主要原因是为了兼容老处理器的寻址模式，8086 只有 16 根地址线，而80386有32根地址线，因此分段机制可以在新处理器的基础上兼容老处理的寻址模式。

实模式对于内存段并没有访问控制, 任意的程序
可以修改任意地址的变量, 而保护模式需要对内存段的性质和允许的操作给出定义, 以实
现对特定内存段的访问检测和数据保护。考虑到各种属性和需要设置的操作, 32位保护模
式下对一个内存段的描述需要8个字节, 其称之为段描述符(Segment Descriptor)。段
描述符分为数据段描述符、指令段描述符和系统段描述符三种。

GDTR 全局描述符表寄存器，用来保存全局描述符表的信息。

GDTR 共 48 为，0~15 位表示 GDT 的边界位置，16~47位存放 GDT 的基地址。

16 位表示表的长度，2^16 是 65536 字节，每个描述符 8 个字节，因此最多可以创建
8192 个描述符。

### 进入保护模式

CPU 加电后就自动进入实模式，那么怎么进入保护模式了？ 在 CPU 内部有 5 个 32 位的
控制寄存器，分别是 CR0 到 CR3，以及 CR8。

其中 CR0 的寄存器的 PE 位 即 0 号位，就表示了 CPU 的运行状态，0 为实模式，
1 为保护模式，通过修改这个位就可以立即改变 CPU 的工作模式。

一旦 CR0 的 PE 位被修改了，CPU 就立即按照保护模式去寻址了，所以必须在进入保护模式前就在内存中放置好 GDT，然后设置好 GDTR 寄存器。

在实模式下只有 1M 的寻址空间，所以 GDT 必须在 1M 空间内。

分段机制其实不符合现在 CPU 的工作模式，但是为了兼容原来的 CPU，因此一直保留了分段机制。

因此现在 CPU 采取了平坦模式来绕过分段，即初始地址是 `0` ，段长是 `4G` ，因此访问段地址其实就是线性地址。

全局段描述符需要两个部分，分别是 GDTR 和 GDT，结构体定义如下：

``` c
typedef struct gdt_entry_t
{
    uint16_t limit_low;  // 段界限   15～0
    uint16_t base_low;   // 段基地址 15～0
    uint8_t base_middle; // 段基地址 23～16
    uint8_t access;      // 段存在位、描述符特权级、描述符类型、描述符子类别
    uint8_t granularity; // 其他标志、段界限 19～16
    uint8_t base_high;   // 段基地址 31～24
} __attribute__((packed)) gdt_entry_t;

// GDTR
typedef struct gdt_ptr_t
{
    uint16_t limit; // 全局描述符表限长
    uint32_t base;  // 全局描述符表 32位 基地址
} __attribute__((packed)) gdt_ptr_t;
```

gdt_ptr_t 是对应 GDTR 寄存器的值，而 gdt_entry_t 对应 GDT 中的一项，会被保存在内存中。

剩下的代码就是如何操作 GDTR 和内存来保存这些数据了，讲道理汇编不熟还真的挺麻烦。

初始化描述符表的核心代码在这：

``` c
void init_gdt()
{
    // 全局描述符表界限 e.g. 从 0 开始，所以总长要 - 1
    gdt_ptr.limit = sizeof(gdt_entry_t) * GDT_LENGTH - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    // 采用 Intel 平坦模型
    gdt_set_gate(0, 0, 0, 0, 0);                // 按照 Intel 文档要求，第一个描述符必须全 0
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // 指令段
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // 数据段
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // 用户模式代码段
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // 用户模式数据段

    // 加载全局描述符表地址到 GPTR 寄存器
    gdt_flush((uint32_t)&gdt_ptr);
}
```

首先是平坦模型段，然后是指令段，数据段，用户态的代码段和数据段，让每个段都专注于一件事情。

## 总结

本小节主要是完成在进入保护模式之前的，初始化全局段描述符表的操作，由于采取的是平坦模型，所以第一个段最大，其它段都是固定的数据、指令段。