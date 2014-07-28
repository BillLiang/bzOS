;	2014-7-27	(c)Bill Liang
;Boot Sector

org 7c00h		;tell the compiler to load the program at 7c00h
mov ax, cs
mov ds, ax
mov es, ax
call DispMessage
jmp $			;无限循环。

;-----------------------
;Display the boot message.
DispMessage:
	mov ax, BootMessage
	mov bp, ax		;es:bp=串地址。
	mov cx, 15		;cx=串长度。
	mov ax, 1301h		;ah=13h,显示字符串; al=01h,光标跟随移动。
	mov bx, 000ch		;bl=属性，黑底红字。
	mov dx, 0000h		;dh, dl=起始行，列。
	int 10h			;BIOS功能调用。
	ret

BootMessage:	db	"Welcome to bzOS"
times 510-($-$$) db 0
dw 0aa55h			;启动扇区结束标记。
