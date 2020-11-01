# 系统调用

> 从汇编层面上来探索系统调用是如何实现的

## C语言的 Hello World

``` c
#include <stdio.h>

int main()
{
    char *str = "Hello World\n";
    printf("%s", str);
    return 0;
}
```

``` c
#include <unistd.h>

int main()
{
    int stdout_fd = 1;
    char *str = "Hello World\n";
    int length = 13;
    write(stdout_fd, str, length);
    return 0;
}
```

反汇编

## 汇编

每一个系统调用都对应一个数字，如 exit(1)，当进行 exit 调用时，需先将 1 存储到寄存器 `%eax` 中，
然后通过中断指令 `int 0x80` 来触发用户态到内核态的切换，然后内核会根据寄存器中的值来进行对应的系统调用。

注意：这里其实有一个系统调用的映射表。

``` s
.section .data

msg:
    .ascii "Hello World\n"

.section .text
.global _start

_start:
    # write 的第三个参数时 13
    movl $13, %edx
    # write 的第二个参数 msg
    movl $msg, %ecx
    # write 的第一个参数 fd:1
    movl $1, %ebx
    # write 系统调用的标识 4 
    movl $4, %eax
    # 执行 write 系统调用
    int $0x80

    # exit status 0 
    movl $0, %ebx
    # 函数 exit 标识
    movl $1, %eax
    # 执行 exit 系统调用
    int $0x80
```

``` sh
as hello3.s -o hello.o
ld hello.o -o a.out
```
