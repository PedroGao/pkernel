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
    