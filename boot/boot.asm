;------------------------------------------
;---	2014-7-27	(c)Bill Liang	---
;---		启动扇区		---
;------------------------------------------

org 7c00h						;BIOS将把启动扇区加载到 7c00h处并开始执行。
;------------------------------------------------------------------------------------------------
;常量
BaseOfStack			equ	7c00h
BaseOfLoader			equ	9000h		;Loader.bin被加载到的位置--段地址。
OffsetOfLoader			equ	0100h		;Loader.bin被加载到的位置--偏移地址。
;------------------------------------------------------------------------------------------------

;下面是FAT12引导扇区的格式。
	jmp short LABEL_START				;开始启动。
	nop						;长度3字节，一个段跳转指令。
%include	"fat12hdr.inc"
LABEL_START:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, BaseOfStack
	
	;清屏
	mov	ax, 0600h
	mov	bx, 0700h
	mov	cx, 0
	mov	dx, 0184fh
	int	10h

	mov	dh, 0
	call	DispStr

	;开始在软盘中寻找Loader.bin。
	xor	ah, ah
	xor	dl, dl
	int	13h					;BIOS 13H中断，复位软驱。
	
	mov	word	[wSectorNo],	SectorNoOfRootDirectory
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp	word	[wRootDirSizeForLoop], 0	;判断根目录区是否已经读完。
	jz	LABEL_NO_LOADERBIN
	dec	word	[wRootDirSizeForLoop]
	
	mov	ax, BaseOfLoader
	mov	es, ax
	mov	bx, OffsetOfLoader
	mov	ax, [wSectorNo]				;从第19个扇区（根目录区）开始。
	mov	cl, 1					;将1个扇区（1个根目录条目）读入es:bx中。
	call	ReadSector

	mov	si, LoaderFileName			;ds:si-->"LOADER  BIN"
	mov	di, OffsetOfLoader			;es:di-->BaseOfLoader : 0100h
	cld
	mov	dx, 10h					;每个扇区有16个更目录的条目。
LABEL_SEARCH_FOR_LOADERBIN:
	cmp	dx, 0					;如果此扇区没找到，进入下一个扇区。
	jz	LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR	;
	dec	dx
	mov	cx, 11
LABEL_CMP_FILENAME:
	cmp	cx, 0
	jz	LABEL_FILENAME_FOUND
	dec	cx
	lodsb						;ds:si字符加载到al
	cmp	al, byte [es : di]			;
	jz	LABEL_GO_ON
	jmp	LABEL_DIFFERENT

LABEL_GO_ON:
	inc	di
	jmp	LABEL_CMP_FILENAME

LABEL_DIFFERENT:
	and	di, 0ffe0h				;di &= e0h 为了让它指向本条目开头。
	add	di, 20h					;di += 20h 指向下一个目录条目。
	mov	si, LoaderFileName			;
	jmp	LABEL_SEARCH_FOR_LOADERBIN

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add	word	[wSectorNo], 1
	jmp	LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_LOADERBIN:
	mov	dh, 2
	call	DispStr
	jmp 	$

LABEL_FILENAME_FOUND:					;加载Loader
	mov	ax, RootDirSectors
	and	di, 0ffe0h				;di --> 当前条目的开始。
	add	di, 01ah				;di --> 首Sector
	mov	cx, word [es : di]
	push	cx
	add	cx, ax
	add	cx, DeltaSectorNo
	mov	ax, BaseOfLoader
	mov	es, ax
	mov	bx, OffsetOfLoader
	mov	ax, cx
	
LABEL_GOON_LOADING_FILE:
	push	ax
	push	bx
	mov	ah, 0eh
	mov	al, '.'
	mov	bl, 0fh
	int	10h
	pop	bx
	pop	ax

	mov	cl, 1
	call	ReadSector
	pop	ax
	call	GetFATEntry
	cmp	ax, 0fffh
	jz	LABEL_FILE_LOADED
	push	ax
	mov	dx, RootDirSectors
	add	ax, dx
	add	ax, DeltaSectorNo
	add	bx, [BPB_BytesPerSec]
	jmp	LABEL_GOON_LOADING_FILE
LABEL_FILE_LOADED:
	mov	dh, 1
	call	DispStr

	jmp	BaseOfLoader:OffsetOfLoader		;这一句正式跳转到已加载到内存中的LOADER.BIN的开始处，
							;启动扇区的使命到此结束。

;------------------------------------------------------------------------------------------------
;变量
wSectorNo			dw	0		;要读的扇区数。
wRootDirSizeForLoop		dw	RootDirSectors	;根目录占用的扇区数，在循环中会递减至零。
bOdd				db	0		;对待FAT扇区号是奇数还是偶数。
;------------------------------------------------------------------------------------------------
;字符串
LoaderFileName			db	'LOADER  BIN',0 ;LOADER.BIN的文件名。文件名8字节，扩展名3字节。
;为简化代码，下面字符串的长度均为MessageLength
MessageLength			equ	9
BootMessage			db	'Booting  '	;9字节，空格补足。序号0
Message1			db	'Ready    '	;序号1
Message2			db	'No Loader'	;序号2

;------------------------------------------------------------------------------------------------
;显示字符串。
DispStr:
	mov	ax, MessageLength
	mul	dh
	add	ax, BootMessage
	mov	bp, ax
	mov	ax, ds
	mov	es, ax					;es : bp = 串地址。
	mov	cx, MessageLength
	mov	ax, 1301h				;ah = 13h, al = 01h
	mov	bx, 0007h				;bh = 0页号为0， 黑底白字（bl = 07h）
	mov	dl, 0
	int	10h
	ret

;------------------------------------------------------------------------------------------------
;函数名：ReadSector
;------------------------------------------------------------------------------------------------
;描述：从第ax个Sector开始，将cl个Sector读入es:bx中。
;------------------------------------------------------------------------------------------------
;怎样由 扇区号（x） --> 柱面号， 起始扇区， 磁头号？
;
;				柱面号=y >> 1
;	x		商y--
;--------------- =		磁头号=y & 1
; 每磁道扇区数		
;			余z--	起始扇区=z + 1
;
ReadSector:
	push	bp
	mov	bp, sp
	sub	esp, 2					;腾出2个字节的堆栈区域保存要读的扇区数。

	mov	byte [bp - 2], cl
	push	bx
	mov	bl, [BPB_SecPerTrk]
	div	bl
	inc	ah
	mov	cl, ah					;起始扇区。
	mov	dh, al
	shr 	al, 1					
	mov	ch, al					;柱面号。
	and	dh, 1					;磁头号。
	pop	bx
	mov 	dl, [BS_DrvNum]				;驱动器号（0表示A盘）
.GoOnReading:
	mov 	ah, 2
	mov	al, byte [bp - 2]			;读al个扇区。
	int	13h
	jc	.GoOnReading				;出错CF置为1，这时就应该不停读，直到正确为止。

	add	esp, 2
	pop	bp

	ret
;------------------------------------------------------------------------------------------------
;	函数名：GetFATEntry
;------------------------------------------------------------------------------------------------
;	说明：
;		找到序列号ax的Sector在FAT中的条目，结果放在ax中。
;		需要注意的是，中间需要读FAT的扇区到es:bx处，所以函数一开始保存了es和bx
;------------------------------------------------------------------------------------------------
GetFATEntry:
	push	es
	push	bx
	push	ax
	mov	ax, BaseOfLoader			;
	sub	ax, 0100h				;在BaseOfLoader后面留出4K空间用于存放FAT
	mov	es, ax					;

	pop	ax
	mov	byte [bOdd], 0
	mov	bx, 3
	mul	bx
	mov	bx, 2
	div	bx
	cmp	dx, 0
	jz	LABEL_EVEN
	mov	byte [bOdd], 1
LABEL_EVEN:
	xor	dx, dx
	mov	bx, [BPB_BytesPerSec]
	div	bx

	push	dx
	mov	bx, 0
	add	ax, SectorNoOfFAT1
	mov	cl, 2
	call	ReadSector

	pop	dx
	add	bx, dx
	mov	ax, [es : bx]
	cmp	byte [bOdd], 1
	jnz	LABEL_EVEN_2
	shr	ax, 4
LABEL_EVEN_2:
	and	ax, 0fffh
LABEL_GET_FAT_ENTRY_OK:
	pop	bx
	pop	es
	ret


times 510-($-$$) db 0
dw 0aa55h						;启动扇区结束标记。
