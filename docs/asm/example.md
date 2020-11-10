# 代码示例

编写一个简单的 C 语言程序 mstore.c，包含以下函数定义：

``` c
long mult2(long, long);

void multstore(long x, long y, long *dest)
{
    long t = mult2(x, y);
    *dest = t;
}
```

通过如下指令编译成汇编代码：

``` sh
gcc -Og -S mstore.c
```

在生成 mstore.s 后，可在复杂的汇编代码中找到如下的几行（只留下的核心部分）：

``` s
multstore:
    pushq	%rbx
    movq	%rdx, %rbx
    call	mult2
    movq	%rax, (%rbx)
    popq	%rbx
    ret
```

这段汇编代码与 C 语言代码对应。简单说明一下，寄存器 `%rbx` 和 `%rdx` 被用来传递函数参数， `(%rbx)` 对应内存，即当寄存器中的值当成指针访问内存，通过 `movq` 来赋值， `q` 表示四字，即 64 位。

通过如下命令，我们可以将 C 代码生成目标文件：

``` sh
gcc -Og -c mstore.c
```

这个命令会生成 `mstore.o` 目标文件，对于目标文件，**它是不可运行的**，必须通过 `ld` 链接后才可运行（后面内容）。

当然我们也可以通过 `objdump` 工具来反汇编目标文件：

``` sh
objdump -d mstore.o
```

结果如下：

``` 

0000000000000000 <multstore>:
   0:   f3 0f 1e fa             endbr64 
   4:   53                      push   %rbx
   5:   48 89 d3                mov    %rdx,%rbx
   8:   e8 00 00 00 00          callq  d <multstore+0xd>
   d:   48 89 03                mov    %rax,(%rbx)
  10:   5b                      pop    %rbx
  11:   c3                      retq  
```

反汇编的代码会给出机器码与汇编代码的对应关系。

添加如下主函数文件，main.c：

``` c
#include <stdio.h>

void multstore(long, long, long *);

int main()
{
    long d;
    multstore(2, 3, &d);
    printf("2*3 --> %ld\n", d);
    return 0;
}

long mult2(long a, long b)
{
    long s = a * b;
    return s;
}
```

通过如下命令生成可执行文件：

``` sh
gcc -Og -o prog main.c mstore.c
```

同样的，可以通过 `objdump` 来查看 prog 的反汇编代码：

``` sh
objdump -d prog
```
