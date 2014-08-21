;==================================================================================================
;			Kernel.asm		2014-8-21
;==================================================================================================

SELECTOR_KERNEL_CS		equ	8

extern	cstart					;导入外部函数
extern	gdt_ptr					;导入全局变量

[section .bss]					;通常指用来存放程序中未初始化的全局变量和静态变量的一块内存。在程序执行之前BSS段会自动清0，所以未初始化的全局变量在程序执行之前已经置0了。
StackSpace			resb	2 * 1024
StackTop:					;栈顶

[section .text]

global _start							;导出 _start

_start:
	mov	esp, StackTop					;把esp从Loader挪到Kernel中(堆栈在bss段中)
	
	sgdt	[gdt_ptr]					;从gdtr取值存放到[gdt_ptr]中，cstart()将会用到gdt_ptr
	call	cstart						;cstart()将会改变gdt_ptr，让它指向新的GDT
	lgdt	[gdt_ptr]					;设置新的GDT

	jmp	SELECTOR_KERNEL_CS : csinit
	;此跳转强制使用刚刚初始化的结构
csinit:
	push	0
	popfd							;Pop top of stack into EFLAGS

	hlt							;停机指令

