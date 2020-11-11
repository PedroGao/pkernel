# 调试

## 字符串基础类库

这部分主要是指针的操作，涉及到指针的偏移，赋值和字符串操作。

选个函数说明一下：

``` c
inline void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len)
{
    // 原理：指针取值覆盖
    for (; len != 0; len--)
    {
        *dest++ = *src++;
    }
}
```

这里， `dest` 是目标地址， `src` 是源地址，通过 `*` 解引用符来操作内存，然后 `++` 移到下一个地址（内存）处；循环结束后，内存拷贝完成。

## 内核打印函数

### 可变长参数

在 C 语言中，有一个 `printf` 函数，可以接收任意长的参数，那么可变长参数是如何实现的呢？

GCC 提供了 `___builtin_va_list` 可变长参数一系列的宏，如下：

``` c
typedef ___builtin_va_list va_list;

#define va_start(ap, last) (__builtin_va_start(ap, last))
#define va_arg(ap, type) (__builtin_va_arg(ap, type))
#define va_end(ap)
```

`va_start` 用于变长参数开始， `va_arg` 用于取出下一个类型的参数， `va_end` 会结束当前的变长参数。

如果不使用 GCC 提供的变长参数宏，也可以通过如下方式来简单实现变长参数：

``` c
#define va_list char *
#define va_start(p, first) (p = (va_list)&first + sizeof(first))
#define va_arg(p, next) (*(next*)((p += sizeof(next) ) − sizeof(next)))
#define va_end(p) (p = (va_list)NULL)
```

`va_list` 即可变长参数的类型是 `char*` ——字符指针，va_start 和 va_arg 都是通过指针类型和类型大小来取出相应的参数，注意：可变参数必须建立在参数是连续空间地址之上。

多说一句，参数是如何传递以及如何销毁的了？

C 语言默认的调用约定是 `cdec1` ，规定调用者 `从右到左` 向栈里压入参数，在函数调用完毕后，在移动栈顶来销毁这些参数。比如 `func(1,2,3,4)` ，编译成汇编代码如下：

``` s
push 4
push 3
push 2
push 1
call func
sub esp, 16
```

这里在函数调用前，将 `1,2,3,4` 四个参数分别入栈，在调用 `func` 完毕后，通过移动 `esp` 栈顶来销毁这四个参数。

## 函数调用栈

打印函数栈，至少需要知道栈中的数据，因此我们新建一个全局变量 `glb_mboot_ptr` ，指向 multiboot_t 结构体，这个结构体会在 GRUB 调用内核前获取硬件信息和内核本身信息。

哇，这个其实内容巨大，远远超过我能 hold 的范围，主要涉及 BIOS、驱动内存和函数栈的使用，当然还有 elf 这种东西，信息量巨大，一章的内容恐怖如斯。

因此我也不准备全部弄清楚，慢慢的积累，慢慢的学习，所以只是记录一下我所理解的部分。

关于 multiboot_t 和 elf，我没有太多的了解，简单说一下。

multiboot_t 是用来存储内核上的一些信息的全局变量，而 elf 是 GRUB 读取文件的格式，也就是我们内核文件的格式。注意：这很重要，因为 boot loader 加载内核会先读取内核文件，这个内核文件在硬盘里的格式是 elf 的，elf 是 linux 下的一种可执行文件，但是在 windows 下是另外一种，但是这个教程中的内核是 elf 格式的，这将贯穿教程的始终。

`GRUB` 在载入内核之后, 会读取 `ELF` 并把相关的信息组织成结构体放在
multiboot_t结构, 并把结构体指针放在ebx寄存器里传递给内核。其multiboot_t结构的
addr成员便指向的是elf_section_header_t类型的结构体数组, num成员是这个结构体数
组的成员个数。

下面再说函数调用栈，也是重点，前面一堆不理解也没关系。

重点：当函数调用发生 `panic` 的时候怎么拿到函数调用栈并打印了？

原理很简单，当发生函数调用的时候会被当前函数的地址压栈，所以函数调用链本身就已经在栈中了，因此只需要通过 `ebp` 和 `eip` 两个寄存器就可以拿到函数地址，并从 `glb_mboot_ptr` 拿到函数名打印。

这里， `ebp` 是栈底， `eip` 是下一条指令地址，而栈中的函数返回地址恰好在栈底的上方，因此通过 `ebp` 和 `eip` 可还原出真实的调用链。

如下：

``` c
void print_stack_trace()
{
    uint32_t *ebp, *eip;

    asm volatile("mov %%ebp, %0"
                 : "=r"(ebp));
    while (ebp)
    {
        eip = ebp + 1;
        printk("   [0x%x] %s\n", *eip, elf_lookup_symbol(*eip, &kernel_elf));
        ebp = (uint32_t *)*ebp;
    }
}
```

## 总结

这个小节不推荐死磕，理解即可，有很多汇编代码的应用，信息量巨大，一时难以接受。
