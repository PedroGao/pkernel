# 运行 Hello World

本小节主要的主要目的是运行一个 `Hello World` 级别的内核。

## Makefile

一开始，我一直不理解，为什么 Makefile 中有这么一行：

``` Makefile
all: $(S_OBJECTS) $(C_OBJECTS) link update_image
```

在运行 `make` 命令时，为什么在后面非要加上一个子命令 `update_image` ：

``` Makefile
.PHONY:update_image
update_image:
	sudo mount floppy.img /mnt/kernel
	sudo cp hx_kernel /mnt/kernel/hx_kernel
	sleep 1
	sudo umount /mnt/kernel
```

至于 `update_image` 这条命令，会先将 floppy.img，即装有 `GRUB` 的软盘镜像挂载到 `/mnt/kernel` ，
然后将编译后的内核执行文件 `hx_kernel` 拷贝至该目录下，休眠1秒后，再将 `/mnt/kernel` unmount。

其实这个命令相当重要，因为 floppy.img 中虽然有 GRUB，但是对于编译后的内核文件 hx_kernel，我们无法直接将其内核文件挂载至镜像，通过 `mount` 和 `unmount` 命令，操作 floppy.img 镜像，将编译后的内核可执行文件装入镜像。

## ld

`scripts/kernel.ld` 是一段链接脚本，通过这个链接脚本来生成最后的内核执行文件，关于链接器，参考[ld](./ld.md)。

链接脚本中有一段其实挺重要的，这里来强调一下：

``` 

ENTRY(start)
SECTIONS
{
	. = 0x100000;
	.text :
```

`ENTRY` 用来制定可执行文件的入口，这里指定的是 `start` ， `start` 是 boot.s 启动脚本中的一个函数，因此当内核运行时，第一个执行的函数就是 `start` 函数。

第二个点，这里 `.text` 指的就是代码段，而上面一行是 `0x100000` ，意思是告诉连接器将代码段的开始地址设为 `0x100000` ，即代码段从 `0x100000` 开始，也就是 `1M` 。

为什么这个地方是 1M, 很简单，8086 实模式下的寻址地址就是 1M，在这1M中有 BIOS区，显存区，以及其他设备区，显然我们不能将代码放在 1M 里面，因此代码段需要放在 1M 后面。

## boot

操作系统的启动顺序其实是这样的，首先电源开启，CPU 重置，CS/IP 指向0xFFFFFFF0地址，注意此时是段地址+偏移地址，0xFFFFFFF0 实际在 BIOS 的尾部，因此第一条指令就是跳转，跳转到 BIOS 的前半段处，然后将运行权交给了 BIOS，BIOS 会进行自检，检查一下硬件是否正常，自检 OK 以后，BIOS 负责加载第一个扇区的 512 字节的数据。

这个第一扇区的 512 字节的内容其实就是 boot loader，为什么先加载 boot loader 而不是直接加载内核文件了？

因为 BIOS 只能加载 512 字节，而内核太大，所以加载 boot loader，然后将控制权交给了 boot loader。

在这里，并没有自造 boot loader，而是直接使用了成熟的 `GRUB` , 至于如何写 boot loader，可以参考 MIT6.828 中的 lab1。

boot loader 的内容大部分由 GRUB 来完成，但是 GRUB 是有标准的，因此在 boot.s 中需要指定一些字段来满足 GRUB 标准。如下：

``` s
; ----------------------------------------------------------------
;
; 	boot.s -- 内核从这里开始
;
;                 这里还有根据 GRUB Multiboot 规范的一些定义
;
; ----------------------------------------------------------------

MBOOT_HEADER_MAGIC 	equ 	0x1BADB002 	; Multiboot 魔数，由规范决定的

MBOOT_PAGE_ALIGN 	equ 	1 << 0    	; 0 号位表示所有的引导模块将按页(4KB)边界对齐
MBOOT_MEM_INFO 		equ 	1 << 1    	; 1 号位通过 Multiboot 信息结构的 mem_* 域包括可用内存的信息
						; (告诉GRUB把内存空间的信息包含在Multiboot信息结构中)

; 定义我们使用的 Multiboot 的标记
MBOOT_HEADER_FLAGS 	equ 	MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO

; 域checksum是一个32位的无符号值，当与其他的magic域(也就是magic和flags)相加时，
; 要求其结果必须是32位的无符号值 0 (即magic + flags + checksum = 0)
MBOOT_CHECKSUM 		equ 	- (MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

; 符合Multiboot规范的 OS 映象需要这样一个 magic Multiboot 头

; Multiboot 头的分布必须如下表所示：
; ----------------------------------------------------------
; 偏移量  类型  域名        备注
;
;   0     u32   magic       必需
;   4     u32   flags       必需 
;   8     u32   checksum    必需 
;
; 我们只使用到这些就够了，更多的详细说明请参阅 GNU 相关文档
;-----------------------------------------------------------

;-----------------------------------------------------------------------------
```

注意，这里都是汇编，其中有一个很重要的伪指令 `equ` , 相当于高级语言中的变量声明，如：

``` s
MBOOT_HEADER_MAGIC 	equ 	0x1BADB002 
```

声明 `MBOOT_HEADER_MAGIC` 值为 `0x1BADB002` ，equ其实是 `equal` 的简写。

在 boot.s 中关于每一行汇编代码都给出了详细的解释，无奈本人不会汇编，因此就不献丑了。

在 boot.s 中的 start 函数里，有这么一行代码：

``` s
call kern_entry	
```

此时 CPU 的控制权，将由 boot.s 交给 entry.c，因此这里的 kern_entry 是 entry.c 中的
入口函数。

## entry

终于来到了 entry.c 中了，这里已经是内核的入口了，这里并没有太多的信息，其实就是向显卡内存中输出一段字符。

``` c
int kern_entry()
{
    uint8_t *input = (uint8_t *)0xB8000;
    uint8_t color = (0 << 4) | (15 & 0x0F);

    *input++ = 'H';
    *input++ = color;
}
```

注意， `0xB8000` 是一个特殊的内存地址，它是显存的第一个地址，因此对该地址进行操作实际上就是操作显卡内存，因此这里才会输出一段字符。

## 总结

这小节，会通过操作显存输出一段 `Hello World` 字符，但是对于很多不懂计算机如何启动、BIOS 以及 boot loader 的人却是很好的一课。

操作系统启动流程：

``` 
电源通电 -> CPU RESET -> BIOS -> boot loader -> kernel
```

对笔者也收获巨大，后面继续～
