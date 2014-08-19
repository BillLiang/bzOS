;FAT12磁盘的头。
;--------------------------------------------------------------------------------------------
BS_OEMName		db	'Liang_BZ'	;厂商名，必须为8个字节。
BPB_BytesPerSec 	dw	512		;每扇区字节数。
BPB_SecPerClus		db	1		;每簇扇区数。
BPB_RsvdSecCnt		dw	1		;Boot记录占用多少扇区。
BPB_NumFATs		db	2		;共有多少FAT表。
BPB_RootEntCnt		dw	224		;根目录文件数最大值。
BPP_TotSec16		dw 	2280		;逻辑扇区总数。
BPB_Media		db	0xF0		;媒体描述符。
BPB_FATSz16		dw	9		;每FAT扇区数。
BPB_SecPerTrk		dw	18		;每磁道扇区数。
BPP_NumHeads		dw	2		;磁头数（面数）。
BPB_HiddSec		dd	0		;隐藏扇区数。
BPB_TotSec32		dd	0		;wTotalSectorCount为0时这个值记录扇区数。
BS_DrvNum		db	0		;中断13的驱动器号。
BS_Reserved1		db	0		;保留位。
BS_BootSig		db	0x29		;扩展引导标记（29h）
BS_VolID		dd	0		;卷序列号。
BS_VolLab		db	'bzOS       '	;卷标，必须为11个字节。
BS_FileSysType		db	'FAT12   '	;文件系统类型，必须为8字节。

;--------------------------------------------------------------------------------------------
FATSz				equ	9
SectorNoOfRootDirectory		equ	19		;根目录的第一个扇区。
RootDirSectors			equ	14		;根目录占用空间。
SectorNoOfFAT1			equ	1		;FAT1的第一个扇区号 = BPB_RsvdSecCnt
DeltaSectorNo			equ	17
