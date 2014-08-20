BaseOfLoader		equ	09000h
OffsetOfLoader		equ	0100h

BaseOfLoaderPhyAddr	equ	BaseOfLoader * 10h	;Loader.bin被加载到的位置 ---- 物理地址

BaseOfKernelFile	equ	08000h
OffsetOfKernelFile	equ	0h
