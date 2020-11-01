# 显示字符

## 文本模式显示规则

内存中的 `0xB8000~0xBFFFF` 区域，被映射到了显卡内存区，因此操作这部分内存的数据就会映射到显卡上。

控制这块内存的内容就相当于控制显卡内容。在PC上工作的显卡，在加电初始化后，默认为 80*25 文本模式，在这个模式下，屏幕被划分为 25 行，且每行可以显示 80 个字符，所以屏幕总共显示 2000 个字符。

这 2000 个字符便被映射到 `0xB8000~0xBFFFF` 内存区域，读写这片内存区域就是读写显存。

从 0xB8000 地址开始，每 `两个字节` 单位对应屏幕上的一个字符。

## 端口读写

端口读写对应汇编中的 `in/out` 指令，如：

``` s
outb src dst
```

`b` 代表 `byte` 字节，即输出一个字节。

outb 则向指定的地址（src）输出一个字节（dst）。

一直不是很理解这里究竟是如何工作的，所以反汇编了一下 `libs/common.o` 文件，查看了对应的汇编代码，如下：

``` s
// 端口写一个字节
inline void outb(uint16_t port, uint8_t value)
{
   0:   f3 0f 1e fb             endbr32 
   4:   55                      push   %ebp
   5:   89 e5                   mov    %esp,%ebp
   7:   83 ec 08                sub    $0x8,%esp
   a:   8b 45 08                mov    0x8(%ebp),%eax
   d:   8b 55 0c                mov    0xc(%ebp),%edx
  10:   66 89 45 fc             mov    %ax,-0x4(%ebp)
  14:   89 d0                   mov    %edx,%eax
  16:   88 45 f8                mov    %al,-0x8(%ebp)
    asm volatile("outb %1, %0" ::"dN"(port), "a"(value));
  19:   0f b7 55 fc             movzwl -0x4(%ebp),%edx
  1d:   0f b6 45 f8             movzbl -0x8(%ebp),%eax
  21:   ee                      out    %al,(%dx)
}
  22:   90                      nop
  23:   c9                      leave  
  24:   c3                      ret 
```

由于笔者汇编不好，所以不会详细解释其中内容，这里对于大概的解释一下 `outb` 函数的内容，对于前面 `0~5` 行，基本上是函数调用的模板汇编代码。

`endbr32` 保护函数栈， `push %ebp` 和 `move %esp, %ebp` 会将当前的 ebp 如栈，并且将 esp 的值赋给 ebp，这样 esp 和 ebp 指向了同一个位置，形成了一个新的函数栈。

然后移动 esp 将参数入栈，即 port 和 value，并将值赋给对应的寄存器，而 `out` 指令随后调用寄存器中的值，从而实现字节写入。

## 端口映射

由于 `in/out` 操作涉及到大量的寄存器操作，这里的寄存器指的是显卡寄存器，因此工程师们提出了一个通用的解决方案，将一个端口（0x3D4）作为内部寄存器的索引，再通过 0x3D5 端口来设置响应寄存器的值。

光标和显存不一样。它必须通过显卡的I/O端口开控制。

0x3d4和0x3d5两个端口可以用来读写显卡的内部寄存器。方法是先向0x3d4端口写入要访问的寄存器编号，再通过0x3d5端口来读写寄存器数据。存放光标位置的寄存器编号为14和15。

这个被称为 VGA 标准，虽然显卡发展的很快，但这些标准仍然得以保留，所以设置光标可以通过如下方式做到：

``` c
static void move_cursor()
{
    // 屏幕是 80 字节宽
    uint16_t cursorLocation = cursor_y * 80 + cursor_x;
    // 在这里用到的两个内部寄存器的编号为与，分别表示光标位置 1415
    // 的高位与地位。

    outb(0x3D4, 14);                  // 告诉 VGA 我们要设置光标的高字节
    outb(0x3D5, cursorLocation >> 8); // 发送高 8 位
    outb(0x3D4, 15);                  // 告诉 VGA 我们要设置光标的低字节
    outb(0x3D5, cursorLocation);      // 发送低 8 位
}
```

首先，通过操作 0x3D4 端口的值来告诉显卡设置光标的高、低位，然后操作 0x3D5 设置光标样式。如果还是不理解 VGA ，可参考 `简单的VGA字符模式驱动（二）` 。

## 参考

* IO函数：https://www.xuebuyuan.com/3230557.html。
* VGA 寄存器一览表：https://blog.csdn.net/weixin_33860722/article/details/85353277?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-1.channel_param&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-1.channel_param。
* 简单的VGA字符模式驱动（二）：https://blog.csdn.net/gemini_star/article/details/4438280。
* 端口与 in/out：https://blog.csdn.net/ccccdddxxx/article/details/6593533。
