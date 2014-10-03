BaseOfLoader			equ	09000h			;Loader.bin被加载到的位置 ---- 段地址
OffsetOfLoader			equ	0100h			;			  ---- 偏移地址
BaseOfLoaderPhyAddr		equ	BaseOfLoader * 10h	;Loader.bin被加载到的位置 ---- 物理地址

BaseOfKernelFile		equ	7000h			;Kernel.bin被加载到的位置 ---- 段地址
OffsetOfKernelFile		equ	0h			;			  ---- 偏移地址
BaseOfKernelFilePhyAddr		equ	BaseOfKernelFile * 10h

KernelEntryPointPhyAddr		equ	1000h

PageDirBase		equ	100000h				;页目录开始地址 1M
PageTblBase		equ	101000h				;页表开始地址 1M + 4K
