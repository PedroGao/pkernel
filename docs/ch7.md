# 中断

中断就是，硬件发生了某个事件通知中断控制器，中断控制器再通知 CPU，CPU 从中断控制器拿到中断号，
根据这个中断号找到对应的中断处理程序并跳转，中断处理完成后再回到之前的执行流程。

Intel 处理器允许 256 个中断，中断范围是 0～255。

## 中断实现

中断处理程序运行在 `ring0` 层，所以中断处理程序拥有系统的全部权限，按照内存段描述符表的思路，
也有一个中断描述符表，和段描述符表一样存储在内存中，所以也有一个中断描述符表寄存器 IDTR 记录
这个表的起始地址。

## 中断处理流程

1. CPU在执行完当前程序的每一条指令后,都会去确认在执行刚才的指令过程中是否发送中断请求过来, 如果有那么CPU就会在相应的时钟脉冲到来时从总线上读取中断请求对应的中断向量。然后根据得到的中断向量为索引到IDT中找到该向量对应的中断描述符, 中断描述符里保存着中断处理函数的段选择子; 

2. CPU使用IDT查到的中断处理函数段选择子从GDT中取得相应的段描述符,段描述符里保存了中断处理函数的段基址和属性信息。此时CPU要进行一个很关键的特权检验的过程, 这个涉及到CPL、RPL和DPL的数值检验以及判断是否发生用户态到内核态的切换。如果发生了切换, 还要涉及到TSS段和用户栈和内核栈的切换; 60

3. 确认无误后CPU开始保存当前被打断的程序的现场(即一些寄存器的值),以便于将来恢复被打断的程序继续执行。这需要利用内核栈来保存相关现场信息, 即依次压入当前被打断程序使用的eflags、cs、eip、以及错误代码号(如果当前中断有错误代码); 

4. 最后CPU会从中断描述符中取出中断处理函数的起始地址并跳转过去执行。

由于中断处理程序可以分为三个部分，分别是现场保护，中断处理逻辑，现场恢复。而现场保护和现场恢复都是
可重用的。

代码主要核心逻辑如下：

``` c
// 初始化中断描述符表
void init_idt()
{	
	bzero((uint8_t *)&interrupt_handlers, sizeof(interrupt_handler_t) * 256);
	
	idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
	idt_ptr.base  = (uint32_t)&idt_entries;
	
	bzero((uint8_t *)&idt_entries, sizeof(idt_entry_t) * 256);

	// 0-32:  用于 CPU 的中断处理
	idt_set_gate( 0, (uint32_t)isr0,  0x08, 0x8E);
	idt_set_gate( 1, (uint32_t)isr1,  0x08, 0x8E);
	idt_set_gate( 2, (uint32_t)isr2,  0x08, 0x8E);
	idt_set_gate( 3, (uint32_t)isr3,  0x08, 0x8E);
}
```

首先 `idt_ptr` 是中断寄存器，base 是基地址，limit 是中断处理程序数量， `idt_set_gate` 来设置中断处理程序。

## 总结

中断处理寄存器和段全局描述符寄存器几乎一直，段全局描述符表和中断表结构也几乎一致，不一样的是里面的数据结构。

