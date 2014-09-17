;==================================================================================================
;			Kernel.asm			Bill Liang	2014-8-21
;==================================================================================================

%include	"sconst.inc"

extern	cstart					;导入外部函数
extern	exception_handler
extern	spurious_irq
extern	kernel_main
extern	disp_str
extern	clock_handler

extern	gdt_ptr					;导入全局变量
extern	idt_ptr
extern	tss
extern	disp_pos
extern	p_proc_ready
extern	k_reenter
extern	irq_table
extern	sys_call_table

bits	32

[section .bss]					;通常指用来存放程序中未初始化的全局变量和静态变量的一块内存。在程序执行之前BSS段会自动清0，所以未初始化的全局变量在程序执行之前已经置0了。
StackSpace			resb	2 * 1024
StackTop:					;栈顶

[section .text]

global	_start							;导出 _start

global	restart
global	sys_call

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

;对应8259A的中断例程
global	hwint00
global	hwint01
global	hwint02
global	hwint03
global	hwint04
global	hwint05
global	hwint06
global	hwint07
global	hwint08
global	hwint09
global	hwint10
global	hwint11
global	hwint12
global	hwint13
global	hwint14
global	hwint15

_start:
	mov	esp, StackTop					;把esp从Loader挪到Kernel中(堆栈在bss段中)

	mov	dword [disp_pos], 0
	
	sgdt	[gdt_ptr]					;从gdtr取值存放到[gdt_ptr]中，cstart()将会用到gdt_ptr
	call	cstart						;cstart()将会改变gdt_ptr，让它指向新的GDT
	lgdt	[gdt_ptr]					;设置新的GDT

	lidt	[idt_ptr]

	jmp	SELECTOR_KERNEL_CS : csinit			;此跳转强制使用刚刚初始化的结构
csinit:
	xor	eax, eax
	mov	ax, SELECTOR_TSS
	ltr	ax

	jmp	kernel_main

; 中断和异常 -- 硬件中断
;==================================================================================================
%macro	hwint_master	1
	call	save						;!!注意此时的esp指向为进程堆栈

	in	al, INT_M_CTLMASK				;不允许再发生当前中断
	or	al, (1 << %1)
	out	INT_M_CTLMASK, al

	mov	al, EOI
	out	INT_M_CTL, al

	sti							;开中断，允许中断嵌套
	push	%1						;把irq号进栈，作为函数参数
	call	[irq_table + 4 * %1]
	pop	ecx
	cli							;关中断

	in	al, INT_M_CTLMASK				;又允许时钟中断
	and	al, ~(1 << %1)
	out	INT_M_CTLMASK, al

	ret							;!!注意这里不是iretd，因为ret弹出刚刚压入的eip，之后会执行eip指向的代码。
%endmacro
;==================================================================================================

align	16
hwint00:							;irq0的中断例程（时钟）
	hwint_master	0

align	16
hwint01:							;irq1的中断例程（键盘）
	hwint_master	1

align	16
hwint02:							;irq2的中断例程（级联从8259A）
	hwint_master	2

align	16
hwint03:							;irq3的中断例程（串口2）
	hwint_master	3

align	16
hwint04:							;irq4的中断例程（串口1）
	hwint_master	4

align	16
hwint05:							;irq5的中断例程（XT winchester）
	hwint_master	5

align	16
hwint06:							;irq6的中断例程（软盘）
	hwint_master	6

align	16
hwint07:							;irq7的中断例程（打印机）
	hwint_master	7

;==================================================================================================
%macro	hwint_slave	1
	call	save

	in	al, INT_S_CTLMASK
	or	al, (1 << (%1 - 8))
	out	INT_S_CTLMASK, al

	mov	al, EOI						; 置master和slave的EOI位
	out	INT_M_CTL, al					; remember this, otherwise the hard disk will never respond to interrupt again.
	nop
	out	INT_S_CTL, al

	sti
	push	%1
	call	[irq_table + %1 * 4]
	pop	ecx

	cli
	in	al, INT_S_CTLMASK
	and	al, ~(1 << (%1 - 8))
	out	INT_S_CTLMASK, al
	ret
%endmacro
;==================================================================================================

align	16
hwint08:							;irq8的中断例程（实时时钟）
	hwint_slave	8

align	16
hwint09:							;irq9的中断例程（重定向IRQ2）
	hwint_slave	9

align	16
hwint10:							;irq10的中断例程（保留）
	hwint_slave	10

align	16
hwint11:							;irq11的中断例程（保留）
	hwint_slave	11

align	16
hwint12:							;irq12的中断例程（ps/2鼠标）
	hwint_slave	12

align	16
hwint13:							;irq13的中断例程（FPU异常）
	hwint_slave	13

align	16
hwint14:							;irq14的中断例程（AT winchester）
	hwint_slave	14

align	16
hwint15:							;irq15的中断例程（保留）
	hwint_slave	15

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

;==================================================================================================
;					save	
;==================================================================================================
save:
	pushad
	push	ds
	push	es
	push	fs
	push	gs

	mov	esi, edx				;因为接下来会破坏edx，edx在sys_call中作为参数传递。这里不能用push保存

	mov	dx, ss
	mov	ds, dx
	mov	es, dx
			
	mov	edx, esi

	mov	esi, esp				;这时eax为进程表的起始地址

	inc	dword [k_reenter]
	cmp	dword [k_reenter], 0
	jne	.1
	mov	esp, StackTop				;非重入中断，则切换到内核栈
	push	restart

	jmp	[esi + (RETADR - P_STACKBASE)]		;注意，这里其实相当于ret掉save函数，继续执行call save的下一条指令。
.1:							;!!因为call save的时候，CPU就把下一条指令的地址压入栈，此时的栈还不是
	push	restart_reenter				;!!内核栈（esp指向进程栈）,于是把返回地址保存在了RETADR处了。
	jmp	[esi + (RETADR - P_STACKBASE)]

;==================================================================================================
;					恢复（开启）进程的执行	
;==================================================================================================
restart:
	mov	esp, [p_proc_ready]
	lldt	[esp + P_LDT_SEL]
	lea	eax, [esp + P_STACKTOP]
	mov	dword [tss + TSS3_S_SP0], eax
restart_reenter:					;如果是中断重入那么什么也不做
	dec	dword [k_reenter]
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad

	add	esp, 4

	iretd

;==================================================================================================
;					系统调用	
;==================================================================================================
sys_call:
	call	save
	sti

	push	esi					;确保esi不被破坏

	push	dword [p_proc_ready]
	push	edx
	push	ecx
	push	ebx
	call	[sys_call_table + eax * 4]
	add	esp, 4 * 4

	pop	esi

	mov	[esi + (EAXREG - P_STACKBASE)], eax	;把返回值存放在进程表中，当进程恢复是能正确地pop eax
	cli
	ret
