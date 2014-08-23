;==================================================================================================
;			Kernel.asm			Bill Liang	2014-8-21
;==================================================================================================

SELECTOR_KERNEL_CS		equ	8

extern	cstart					;导入外部函数
extern	exception_handler

extern	gdt_ptr					;导入全局变量
extern	idt_ptr
extern	disp_pos

[section .bss]					;通常指用来存放程序中未初始化的全局变量和静态变量的一块内存。在程序执行之前BSS段会自动清0，所以未初始化的全局变量在程序执行之前已经置0了。
StackSpace			resb	2 * 1024
StackTop:					;栈顶

[section .text]

global _start							;导出 _start

global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error

_start:
	mov	esp, StackTop					;把esp从Loader挪到Kernel中(堆栈在bss段中)

	mov	dword [disp_pos], 0
	
	sgdt	[gdt_ptr]					;从gdtr取值存放到[gdt_ptr]中，cstart()将会用到gdt_ptr
	call	cstart						;cstart()将会改变gdt_ptr，让它指向新的GDT
	lgdt	[gdt_ptr]					;设置新的GDT

	lidt	[idt_ptr]

	jmp	SELECTOR_KERNEL_CS : csinit
	;此跳转强制使用刚刚初始化的结构
csinit:
	;ud2
	jmp	0x40 : 0
	;push	0
	;popfd							;Pop top of stack into EFLAGS
	;jmp	$
	hlt							;停机指令

; 中断和异常 -- 异常
divide_error:
	push	0xFFFFFFFF	; no err code
	push	0		; vector_no	= 0
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	; no err code
	push	1		; vector_no	= 1
	jmp	exception
nmi:
	push	0xFFFFFFFF	; no err code
	push	2		; vector_no	= 2
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	; no err code
	push	3		; vector_no	= 3
	jmp	exception
overflow:
	push	0xFFFFFFFF	; no err code
	push	4		; vector_no	= 4
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	; no err code
	push	5		; vector_no	= 5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	; no err code
	push	6		; vector_no	= 6
	jmp	exception
copr_not_available:
	push	0xFFFFFFFF	; no err code
	push	7		; vector_no	= 7
	jmp	exception
double_fault:
	push	8		; vector_no	= 8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	; no err code
	push	9		; vector_no	= 9
	jmp	exception
inval_tss:
	push	10		; vector_no	= A
	jmp	exception
segment_not_present:
	push	11		; vector_no	= B
	jmp	exception
stack_exception:
	push	12		; vector_no	= C
	jmp	exception
general_protection:
	push	13		; vector_no	= D
	jmp	exception
page_fault:
	push	14		; vector_no	= E
	jmp	exception
copr_error:
	push	0xFFFFFFFF	; no err code
	push	16		; vector_no	= 10h
	jmp	exception

exception:
	call	exception_handler
	add	esp, 4*2	; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
	hlt
