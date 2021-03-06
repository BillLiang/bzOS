
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;               kliba.asm	Bill Liang		2014-8-22
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include	"sconst.inc"

;全局变量
extern	disp_pos

[SECTION .text]

; 导出函数
global	disp_str
global	disp_color_str
global	out_byte
global	in_byte

global	disable_irq
global	enable_irq
global	disable_int
global	enable_int

global	port_read
global	port_write
; ========================================================================
;                  void disp_str(char * info);
; ========================================================================
disp_str:
	push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo, 注意：之前进行了 push ebp 压栈操作，故这里是 +8
	mov	edi, [disp_pos]
	mov	ah, 0Fh
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[disp_pos], edi

	pop	ebp
	ret

; ========================================================================
;		void disp_color_str(char* info, int color);
; ========================================================================
disp_color_str:
	push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]
	mov	edi, [disp_pos]
	mov	ah, [ebp + 12]				;颜色
.1:
	lodsb
	test	al, al					;字符串结束，'\0'
	jz	.2
	cmp	al, 0ah					;是回车吗
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0ffh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs : edi], ax
	add	edi, 2
	jmp	.1
.2:
	mov	[disp_pos], edi
	pop	ebp
	ret

; ========================================================================
;		void out_byte(u16 port, u8 value);
; ========================================================================
out_byte:
	mov	edx, [esp + 4]				;16位端口
	mov	al, [esp + 8]				;8位值
	out	dx, al
	nop						;一点延迟
	nop
	ret
; ========================================================================

; ========================================================================
;		u8 in_byte(u16 port);
; ========================================================================
in_byte:
	mov	edx, [esp + 4]
	xor	eax, eax
	in	al, dx					;al存放返回值，8位
	nop
	nop
	ret
; ========================================================================
;		void port_read(u16 port, void* buf, int n);
; ========================================================================
port_read:
	mov	edx, [esp + 4]		; dx -> port
	mov	edi, [esp + 8]		; edi -> buf
	mov	ecx, [esp + 12]		; ecx -> n (count)
	shr	ecx, 1			; beacause we read a word everytime
	cld
	rep	insw			; input from port to string
	ret

; ========================================================================
;		void port_write(u16 port, void* buf, int n);
; ========================================================================
port_write:
	mov	edx, [esp + 4]		; dx -> port
	mov	esi, [esp + 8]		; esi -> buf
	mov	ecx, [esp + 12]		; ecx -> n (count)
	shr	ecx, 1			; beacause we write a word everytime
	cld
	rep	outsw			; output string to port
	ret

; ========================================================================
;		void disable_irq(int irq);
; ========================================================================
disable_irq:
	mov	ecx, [esp + 4]				;irq
	pushf						;保存flags
	cli						;关中断
	mov	ah, 1
	rol	ah, cl
	cmp	cl, 8
	jae	disable_8
disable_0:
	in	al, INT_M_CTLMASK
	test	al, ah					;是否已经disable该irq了？
	jnz	dis_already
	or	al, ah
	out	INT_M_CTLMASK, al
	popf
	mov	eax, 1					;返回true
	ret
disable_8:
	in	al, INT_S_CTLMASK
	test	al, ah
	jnz	dis_already
	or	al, ah
	out	INT_S_CTLMASK, al
	popf
	mov	eax, 1
	ret
dis_already:
	popf
	xor	eax, eax
	ret

; ========================================================================
;		void enable_irq(int irq);
; ========================================================================
enable_irq:
	mov	ecx, [esp + 4]
	pushf
	cli
	mov	ah, ~1
	rol	ah, cl
	cmp	cl, 8
	jae	enable_8
enable_0:
	in	al, INT_M_CTLMASK
	and	al, ah
	out	INT_M_CTLMASK, al
	popf
	ret
enable_8:
	in	al, INT_S_CTLMASK
	and	al, ah
	out	INT_S_CTLMASK, al
	popf
	ret
; ========================================================================
;		void disable_int();
; ========================================================================
disable_int:
	cli
	ret
; ========================================================================
;		void enable_int();
; ========================================================================
enable_int:
	sti
	ret
