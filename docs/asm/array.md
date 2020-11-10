# 探索数组汇编

首先在 `array.c` 上定义一个简单的数组：

``` c
#define N 16
typedef int fix_matrix[N][N];

int fix_prod_ele(fix_matrix A, fix_matrix B, long i, long k)
{
    long j;
    int result = 0;
    for (j = 0; j < N; j++)
    {
        result += A[i][j] * B[j][k];
    }
    return result;
}
```

``` s
fix_prod_ele:
.LFB0:
	endbr64
	movl	$0, %r10d
	movl	$0, %eax
.L2:
	cmpq	$15, %rax
	jg	.L4
	movq	%rdx, %r9
	salq	$6, %r9
	addq	%rdi, %r9
	movq	%rax, %r8
	salq	$6, %r8
	addq	%rsi, %r8
	movl	(%r8,%rcx,4), %r8d
	imull	(%r9,%rax,4), %r8d
	addl	%r8d, %r10d
	addq	$1, %rax
	jmp	.L2
.L4:
	movl	%r10d, %eax
	ret
```

对于数组的访问，本质上来说是通过偏移量来计算内存地址，动态数组同理，但是动态数组性能稍微差一点，必须通过变量来计算。
