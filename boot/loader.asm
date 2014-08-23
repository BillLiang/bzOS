;	2014-8-10
;Loader加载内核到内存。并且进入保护模式，开启分页机制。
;================================================================================================

org	0100h

jmp	LABEL_START

%include	"fat12hdr.inc"
%include	"load.inc"
%include	"pm.inc"
;GDT
;				     段基址	     段界限	属性	
LABEL_GDT:		Descriptor	0,		0,	0
LABEL_DESC_FLAT_C:	Descriptor	0,	  0fffffh,	DA_CR | DA_32 | DA_LIMIT_4K	;0-4G
LABEL_DESC_FLAT_RW:	Descriptor	0,	  0fffffh,	DA_DRW | DA_32 | DA_LIMIT_4K	;0-4G
LABEL_DESC_VIDEO:	Descriptor 0b8000h,	   0ffffh,	DA_DRW | DA_DPL3

GdtLen		equ	$ - LABEL_GDT
GdtPtr		dw	GdtLen - 1
		dd	BaseOfLoaderPhyAddr + LABEL_GDT

;GDT选择子
SelectorFlatC		equ	LABEL_DESC_FLAT_C	-	LABEL_GDT
SelectorFlatRW		equ	LABEL_DESC_FLAT_RW	-	LABEL_GDT
SelectorVideo		equ	LABEL_DESC_VIDEO	-	LABEL_GDT + SA_RPL3

BaseOfStack		equ	0100h

LABEL_START:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, BaseOfStack
	
	mov	dh, 0						;'Loading  '
	call	DispStrRealMode

	;我们要打开分页机制，打开之前要知道可使用内存情况，为页目录表（Page Directory Table）和页表（Page Table）合理分配内存大小。
	mov	ebx, 0						;ebx = 后续值， 开始时需为0
	mov	di, _MemChkBuf					;es : di指向一个地址范围描述符结构（ARDS）
.MemChkLoop:
	mov	eax, 0e820h
	mov	ecx, 20						;ecx = ARDS的大小， 20个字节。
	mov	edx, 0534d4150h					;edx = 'SMAP'
	int	15h
	jc	.MemChkFail
	add	di, 20
	inc	dword [_dwMCRNumber]				;dwMCRNumber = ARDS的个数
	cmp	ebx, 0
	jne	.MemChkLoop
	jmp	.MemChkOK
.MemChkFail:
	mov	dword [_dwMCRNumber], 0
.MemChkOK:
	;下面在软盘根目录寻找Kernel.bin
	mov	word [wSectorNo], SectorNoOfRootDirectory
	xor	ah, ah
	xor	dl, dl
	int	13h
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp	word [wRootDirSizeForLoop], 0
	jz	LABEL_NO_KERNELBIN
	dec	word [wRootDirSizeForLoop]
	mov	ax, BaseOfKernelFile
	mov	es, ax
	mov	bx, OffsetOfKernelFile
	mov	ax, [wSectorNo]
	mov	cl, 1
	call	ReadSector
	
	mov	si, KernelFileName
	mov	di, OffsetOfKernelFile
	cld
	mov	dx, 10h
LABEL_SEARCH_FOR_KERNELBIN:
	cmp	cx, 0
	jz	LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR
	dec	dx
	mov	cx, 11
LABEL_CMP_FILENAME:
	cmp	cx, 0
	jz	LABEL_FILENAME_FOUND
	dec	cx
	lodsb
	cmp	al, byte [es : di]
	jz	LABEL_GO_ON
	jmp	LABEL_DIFFERENT
	
LABEL_GO_ON:
	inc	di
	jmp	LABEL_CMP_FILENAME

LABEL_DIFFERENT:
	and	di, 0ffe0h
	add	di, 20h
	mov	si, KernelFileName
	jmp	LABEL_SEARCH_FOR_KERNELBIN
	
LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add	word [wSectorNo], 1
	jmp	LABEL_SEARCH_IN_ROOT_DIR_BEGIN
	
LABEL_NO_KERNELBIN:
	mov	ah, 2
	call	DispStrRealMode

	jmp	$						;如果没有找到KERNEL.BIN
	
LABEL_FILENAME_FOUND:
	mov	ax, RootDirSectors
	and	di, 0fff0h
	
	push	eax
	mov	eax, [es : di + 01ch]				;保存KERNEL.BIN文件大小。
	mov	dword [dwKernelSize], eax
	pop	eax
	
	add	di, 01ah
	mov	cx, word [es : di]
	push	cx
	add	cx, ax
	add	cx, DeltaSectorNo
	mov	ax, BaseOfKernelFile
	mov	es, ax
	mov	bx, OffsetOfKernelFile
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
	call	KillMotor

	mov	dh, 1
	call	DispStrRealMode

	;下面准备跳入保护模式
	lgdt	[GdtPtr]

	;关中断
	cli

	;打开地址线A20
	in	al, 92h
	or	al, 00000010b
	out	92h, al

	;准备切换到保护模式
	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax

	;进入保护模式
	jmp	dword SelectorFlatC : (BaseOfLoaderPhyAddr + LABEL_PM_START)
	
;--------------------------------------------------------------------------------------------
;变量
;--------------------------------------------------------------------------------------------
wRootDirSizeForLoop	dw	RootDirSectors
wSectorNo		dw	0
bOdd			db	0
dwKernelSize		dd	0

;--------------------------------------------------------------------------------------------
;字符串
;--------------------------------------------------------------------------------------------
KernelFileName		db	'KERNEL  BIN', 0
MessageLength		equ	9
LoadMessage		db	'Loading  '
Message1		db	'Ready    '
Message2		db	'NO KERNEL'

;----------------------------------------------------------------------------
; 函数名: DispStrRealMode
;----------------------------------------------------------------------------
; 运行环境：
;	实模式。
; 作用:
;	显示一个字符串, 函数开始时 dh 中应该是字符串序号(0-based)
DispStrRealMode:
	mov	ax, MessageLength
	mul	dh
	add	ax, LoadMessage
	mov	bp, ax			; ┓
	mov	ax, ds			; ┣ ES:BP = 串地址
	mov	es, ax			; ┛
	mov	cx, MessageLength	; CX = 串长度
	mov	ax, 01301h		; AH = 13,  AL = 01h
	mov	bx, 0007h		; 页号为0(BH = 0) 黑底白字(BL = 07h)
	mov	dl, 0
	add	dh, 3			; 从第 3 行往下显示
	int	10h			; int 10h
	ret
;----------------------------------------------------------------------------
; 函数名: ReadSector
;----------------------------------------------------------------------------
; 作用:
;	从序号(Directory Entry 中的 Sector 号)为 ax 的的 Sector 开始, 将 cl 个 Sector 读入 es:bx 中
ReadSector:
	; -----------------------------------------------------------------------
	; 怎样由扇区号求扇区在磁盘中的位置 (扇区号 -> 柱面号, 起始扇区, 磁头号)
	; -----------------------------------------------------------------------
	; 设扇区号为 x
	;                           ┌ 柱面号 = y >> 1
	;       x           ┌ 商 y ┤
	; -------------- => ┤      └ 磁头号 = y & 1
	;  每磁道扇区数     │
	;                   └ 余 z => 起始扇区号 = z + 1
	push	bp
	mov	bp, sp
	sub	esp, 2			; 辟出两个字节的堆栈区域保存要读的扇区数: byte [bp-2]

	mov	byte [bp-2], cl
	push	bx			; 保存 bx
	mov	bl, [BPB_SecPerTrk]	; bl: 除数
	div	bl			; y 在 al 中, z 在 ah 中
	inc	ah			; z ++
	mov	cl, ah			; cl <- 起始扇区号
	mov	dh, al			; dh <- y
	shr	al, 1			; y >> 1 (其实是 y/BPB_NumHeads, 这里BPB_NumHeads=2)
	mov	ch, al			; ch <- 柱面号
	and	dh, 1			; dh & 1 = 磁头号
	pop	bx			; 恢复 bx
	; 至此, "柱面号, 起始扇区, 磁头号" 全部得到 ^^^^^^^^^^^^^^^^^^^^^^^^
	mov	dl, [BS_DrvNum]		; 驱动器号 (0 表示 A 盘)
.GoOnReading:
	mov	ah, 2			; 读
	mov	al, byte [bp-2]		; 读 al 个扇区
	int	13h
	jc	.GoOnReading		; 如果读取错误 CF 会被置为 1, 这时就不停地读, 直到正确为止

	add	esp, 2
	pop	bp

	ret

;----------------------------------------------------------------------------
; 函数名: GetFATEntry
;----------------------------------------------------------------------------
; 作用:
;	找到序号为 ax 的 Sector 在 FAT 中的条目, 结果放在 ax 中
;	需要注意的是, 中间需要读 FAT 的扇区到 es:bx 处, 所以函数一开始保存了 es 和 bx
GetFATEntry:
	push	es
	push	bx
	push	ax
	mov	ax, BaseOfKernelFile	; ┓
	sub	ax, 0100h		; ┣ 在 BaseOfKernelFile 后面留出 4K 空间用于存放 FAT
	mov	es, ax			; ┛
	pop	ax
	mov	byte [bOdd], 0
	mov	bx, 3
	mul	bx			; dx:ax = ax * 3
	mov	bx, 2
	div	bx			; dx:ax / 2  ==>  ax <- 商, dx <- 余数
	cmp	dx, 0
	jz	LABEL_EVEN
	mov	byte [bOdd], 1
LABEL_EVEN:;偶数
	xor	dx, dx			; 现在 ax 中是 FATEntry 在 FAT 中的偏移量. 下面来计算 FATEntry 在哪个扇区中(FAT占用不止一个扇区)
	mov	bx, [BPB_BytesPerSec]
	div	bx			; dx:ax / BPB_BytsPerSec  ==>	ax <- 商   (FATEntry 所在的扇区相对于 FAT 来说的扇区号)
					;				dx <- 余数 (FATEntry 在扇区内的偏移)。
	push	dx
	mov	bx, 0			; bx <- 0	于是, es:bx = (BaseOfKernelFile - 100):00 = (BaseOfKernelFile - 100) * 10h
	add	ax, SectorNoOfFAT1	; 此句执行之后的 ax 就是 FATEntry 所在的扇区号
	mov	cl, 2
	call	ReadSector		; 读取 FATEntry 所在的扇区, 一次读两个, 避免在边界发生错误, 因为一个 FATEntry 可能跨越两个扇区
	pop	dx
	add	bx, dx
	mov	ax, [es:bx]
	cmp	byte [bOdd], 1
	jnz	LABEL_EVEN_2
	shr	ax, 4
LABEL_EVEN_2:
	and	ax, 0FFFh

LABEL_GET_FAT_ENRY_OK:

	pop	bx
	pop	es
	ret
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
; 函数名: KillMotor
;----------------------------------------------------------------------------
; 作用:
;	关闭软驱马达
KillMotor:
	push	dx
	mov	dx, 03f2h
	mov	al, 0
	out	dx, al
	pop	dx
	ret
;----------------------------------------------------------------------------

;从此以后的代码在保护模式下执行----------------------------------------------
;32位代码段，由实模式跳入----------------------------------------------------
[section .s32]

align	32

[bits 32]

LABEL_PM_START:
	mov	ax, SelectorVideo
	mov	gs, ax
	;初始化各个寄存器
	mov	ax, SelectorFlatRW
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	ss, ax
	mov	esp, TopOfStack

	push	szMemChkTitle
	call	DispStr
	add	esp, 4

	call	DispMemInfo
	call	SetupPaging

	;mov	ah, 0fh
	;mov	al, 'P'
	;mov	[gs : ((80 * 0 + 39) * 2)], ax

	call	InitKernel				;加载ELF文件中的内核代码到内存	
	
	;!!!正式进入内核了!!!
	jmp	SelectorFlatC : KernelEntryPointPhyAddr


; ------------------------------------------------------------------------
; 显示 AL 中的数字
; ------------------------------------------------------------------------
DispAL:
	push	ecx
	push	edx
	push	edi

	mov	edi, [dwDispPos]

	mov	ah, 0Fh			; 0000b: 黑底    1111b: 白字
	mov	dl, al
	shr	al, 4
	mov	ecx, 2
.begin:
	and	al, 01111b
	cmp	al, 9
	ja	.1
	add	al, '0'
	jmp	.2
.1:
	sub	al, 0Ah
	add	al, 'A'
.2:
	mov	[gs:edi], ax
	add	edi, 2

	mov	al, dl
	loop	.begin
	;add	edi, 2

	mov	[dwDispPos], edi

	pop	edi
	pop	edx
	pop	ecx

	ret
; DispAL 结束-------------------------------------------------------------


; ------------------------------------------------------------------------
; 显示一个整形数
; ------------------------------------------------------------------------
DispInt:
	mov	eax, [esp + 4]
	shr	eax, 24
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 16
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 8
	call	DispAL

	mov	eax, [esp + 4]
	call	DispAL

	mov	ah, 07h			; 0000b: 黑底    0111b: 灰字
	mov	al, 'h'
	push	edi
	mov	edi, [dwDispPos]
	mov	[gs:edi], ax
	add	edi, 4
	mov	[dwDispPos], edi
	pop	edi

	ret
; DispInt 结束------------------------------------------------------------

; ------------------------------------------------------------------------
; 显示一个字符串
; ------------------------------------------------------------------------
DispStr:
	push	ebp
	mov	ebp, esp
	push	ebx
	push	esi
	push	edi

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [dwDispPos]
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
	mov	[dwDispPos], edi

	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret
; DispStr 结束------------------------------------------------------------

; ------------------------------------------------------------------------
; 换行
; ------------------------------------------------------------------------
DispReturn:
	push	szReturn
	call	DispStr			;printf("\n");
	add	esp, 4

	ret
; DispReturn 结束---------------------------------------------------------


; ------------------------------------------------------------------------
; 内存拷贝，仿 memcpy
; ------------------------------------------------------------------------
; void* MemCpy(void* es:pDest, void* ds:pSrc, int iSize);
; ------------------------------------------------------------------------
MemCpy:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	esi, [ebp + 12]	; Source
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	al, [ds:esi]		; ┓
	inc	esi			; ┃
					; ┣ 逐字节移动
	mov	byte [es:edi], al	; ┃
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:
	mov	eax, [ebp + 8]	; 返回值

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回
; MemCpy 结束-------------------------------------------------------------


;============================================================================
;显示内存信息
DispMemInfo:
	push	esi
	push	edi
	push	ecx

	mov	esi, MemChkBuf
	mov	ecx, [dwMCRNumber]			;每次得到一个ARDS
.loop:
	mov	edx, 5					;每次得到一个ARDS中的成员
	mov	edi, ARDStruct
.1:
	push	dword [esi]
	call	DispInt					;显示一个成员
	pop	eax
	stosd
	add	esi, 4
	dec	edx
	cmp	edx, 0
	jnz	.1
	call	DispReturn
	cmp	dword [dwType], 1			;如果这个内存段是一段可以被OS使用的RAM
	jne	.2
	mov	eax, [dwBaseAddrLow]
	add	eax, [dwLengthLow]
	cmp	eax, [dwMemSize]
	jb	.2
	mov	[dwMemSize], eax
.2:	
	loop	.loop

	call	DispReturn
	push	szRAMSize
	call	DispStr					;打印字符串"RAM size:"
	add	esp, 4

	push	dword [dwMemSize]
	call	DispInt
	add	esp, 4

	pop	ecx
	pop	edi
	pop	esi
	ret

;================================================================================================
;启动分页机制
;================================================================================================
SetupPaging:
	;根据内存大小计算应初始化多少PDE以及多少页表（Page Table）
	xor	edx, edx
	mov	eax, [dwMemSize]
	mov	ebx, 400000h
	div	ebx				;edx:eax被除数。ax = 商， dx = 余数
	mov	ecx, eax
	test	edx, edx			;是否整除
	jz	.no_remainder
	inc	ecx				;如果不能整除，就增加一个页表
.no_remainder:
	push	ecx				;暂存页表个数
	;为简化处理，所有线性地址对应相等的物理地址，并且不考虑内存空洞
	
	;初始化页目录
	mov	ax, SelectorFlatRW
	mov	es, ax
	mov	edi, PageDirBase
	xor	eax, eax
	mov	eax, PageTblBase | PG_P | PG_USU | PG_RWW
.1:
	stosd
	add	eax, 4096
	loop	.1

	;初始化所有页表
	pop	eax
	mov	ebx, 1024
	mul	ebx				;eax * ebx，PTE个数 = 页表个数 * 1024
	mov	ecx, eax
	mov	edi, PageTblBase
	xor	eax, eax
	mov	eax, PG_P | PG_USU | PG_RWW
.2:
	stosd
	add	eax, 4096
	loop	.2

	mov	eax, PageDirBase
	mov	cr3, eax
	mov	eax, cr0
	or	eax, 80000000h
	mov	cr0, eax			;置cr0的PG位
	jmp	short .3
.3:
	nop

	ret

;================================================================================================
;将Kernel.bin的内容经过整理后放到新的位置。
;遍历ELF文件每一个Program Header，根据Program Header中的信息确定把什么放进内存，放在哪里，以及放多少。
;================================================================================================
InitKernel:
	xor	esi, esi
	mov	cx, word [BaseOfKernelFilePhyAddr + 2ch]			;ecx <-- ELFHeader中的e_phnum，指明有多少个Program Header
	movzx	ecx, cx								;带零扩展传送
	mov	esi, [BaseOfKernelFilePhyAddr + 1ch]				;esi <-- ELFHeader中的e_phoff，Program Header Table在文件中的偏移量。
	add	esi, BaseOfKernelFilePhyAddr
.Begin:
	mov	eax, [esi + 0]
	cmp	eax, 0								;PT_NULL
	jz	.NoAction
	push	dword [esi + 10h]						;Program Header中p_filesz，段在文件中的长度
	mov	eax, [esi + 4h]
	add	eax, BaseOfKernelFilePhyAddr
	push	eax								;Program Header中p_offset，段的第一个字节在文件中的偏移
	push	dword [esi + 8h]						;Program Header中p_vaddr，段的第一个字节在内存中的虚拟地址
	call	MemCpy								;复制文件内容到内存
	add	esp, 12								;
.NoAction:
	add	esi, 20h							;下个Program Header
	dec	ecx
	jnz	.Begin

	ret
;================================================================================================


[section .data1]

align	32

LABEL_DATA:
;============================================================================
;实模式下使用这些符号
;============================================================================
;字符串
_szMemChkTitle:			db	'BaseAddrL BaseAddrH LengthLow LengthHigh   Type', 0ah, 0
_szRAMSize:			db	'RAM size:', 0
_szReturn:			db	0ah, 0
;变量
_dwMCRNumber:			dd	0					;Memory Check Result
_dwDispPos:			dd	(80 * 6 + 0) * 2			;第6行，第0列
_dwMemSize:			dd	0

_ARDStruct:
	_dwBaseAddrLow:		dd	0
	_dwBaseAddrHigh:	dd	0
	_dwLengthLow:		dd	0
	_dwLengthHigh:		dd	0
	_dwType:		dd	0
_MemChkBuf:	times	256	db	0
;============================================================================
;保护模式下使用这些符号（全是equ哦）
;============================================================================
szMemChkTitle			equ	BaseOfLoaderPhyAddr + _szMemChkTitle
szRAMSize			equ	BaseOfLoaderPhyAddr + _szRAMSize
szReturn			equ	BaseOfLoaderPhyAddr + _szReturn

dwDispPos			equ	BaseOfLoaderPhyAddr + _dwDispPos
dwMemSize			equ	BaseOfLoaderPhyAddr + _dwMemSize
dwMCRNumber			equ	BaseOfLoaderPhyAddr + _dwMCRNumber
ARDStruct			equ	BaseOfLoaderPhyAddr + _ARDStruct
	dwBaseAddrLow		equ	BaseOfLoaderPhyAddr + _dwBaseAddrLow
	dwBaseAddrHigh		equ	BaseOfLoaderPhyAddr + _dwBaseAddrHigh
	dwLengthLow		equ	BaseOfLoaderPhyAddr + _dwLengthLow
	dwLengthHigh		equ	BaseOfLoaderPhyAddr + _dwLengthHigh
	dwType			equ	BaseOfLoaderPhyAddr + _dwType
MemChkBuf			equ	BaseOfLoaderPhyAddr + _MemChkBuf

;堆栈就在数据段的末尾
StackSpace:	times	1024	db	0
TopOfStack	equ	BaseOfLoaderPhyAddr + $
;[section .data1]结束========================================================
