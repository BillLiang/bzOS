;	2014-8-2	Bill Liang
;一个简单单的loader。

org	0100h
	
	mov ax, 0b800h
	mov gs, ax
	mov ah, 0fh				;0000 : 黑底， 1111 : 白字。
	mov al, 'B'
	mov [gs : ((80 * 0 + 39) * 2)], ax	;屏幕第0行，第39列。

	jmp $
