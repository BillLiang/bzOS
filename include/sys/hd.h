/**************************************************************************************************
 * @file			hd.h
 * @brief 			
 * @author			Bill Liang
 * @date			2014-9-6
 *************************************************************************************************/
#ifndef _BZOS_HD_H_
#define _BZOS_HD_H_

/**************************************************************************************************
 * @brief	partition entry
 *
 * Master Boot Record (MBR):
 * 	Located at offset 0x1BE in the 1st sector of a disk.
 * 	MBR contains four 16-byte partition entries. Should end with 55h & AAh.
 *
 * Partition in MBR:
 * 	A PC hard disk can contain either as many as four primary partitions,
 * 	or 1-3 primaries and a single extended partition. Each of these partitions
 * 	are described by a 16-byte entry in the Partition Table
 * 	which is located in the MBR.
 *
 * Extended Partition:
 * 	It is essentially a link list with many tricks.
 *************************************************************************************************/
struct part_ent{
	u8	boot_ind;	/* Indicator 80h=可引导，00h=不可引导，其他=不合法 */

	u8	start_head;	/* 起始磁头号 */
	u8	start_sector;	/* 起始扇区号（0-5使用，6-7为起始柱面号的第8，9位） */
	u8	start_cyl;	/* 起始柱面号的低8位 */

	u8	sys_id;		/* 分区类型 */

	u8	end_head;	/* 结束磁头号 */
	u8	end_sector;	/* 结束扇区号（0-5使用，6-7为结束柱面号的第8，9位） */
	u8	end_cyl;	/* 结束柱面号的低8位 */

	u32	start_sect;	/* 起始扇区的LBA */
	u32	nr_sects;	/* 扇区数目 */

} PARTITION_ENTRY;
/**************************************************************************************************
 * I/O Ports used by hard disk controllers.
 *************************************************************************************************/
/* slave disk not supported yet, all master registers below. */
#define REG_DATA	0x1f0		/* Data */
#define REG_FEATURES	0x1f1		/* Feature, WRITE */
#define REG_ERROR	REG_FEATURES	/* Error, READ, it's quite complex. */

#define REG_NSECTOR	0x1f2		/* Sector Count */
#define REG_LBA_LOW	0x1f3		/* Sector Number / LBA Bits 0-7 */
#define REG_LBA_MID	0x1f4		/* Cylinder Low / LBA Bits 8-15 */
#define REG_LBA_HIGH	0x1f5		/* Cylinder Mid / LBA Bits 16-23 */

#define	REG_DEVICE	0x1f6		/* Drive | Head | LBA Bits 24-27 */
/* REG_STATUS: Any pending interrupt is cleared whenever this register is read. */
#define REG_STATUS	0x1f7		/* Status, READ */
#define REG_CMD		REG_STATUS	/* Command, WRITE */

/* Control Block Register */
#define REG_DEV_CTRL	0x3f6		/* Dvice Control, WRITE */
#define REG_ALT_STATUS	REG_DEV_CRTL	/* Alternate Status, READ */

#define REG_DRV_ADDR	0x3f7		/* Drive Address */
/*************************************************************************************************
 * STATUS for REG_STATUS
 ************************************************************************************************/
#define STATUS_BSY	0x80		/* 7_ If BSY=1, no other bits in the register are valid. */
#define STATUS_DRDY	0x40		/* 6_ Drive Ready. */
#define STATUS_DFSE	0x20		/* 5_ Device Fault / Stream Error. */
#define STATUS_DSC	0x10		/* 4_ Command Dependent. (formerly DSC bit) */
#define STATUS_DRQ	0x08		/* 3_ Data Request. (ready to transfer data) */
#define STATUS_CORR	0x04		/* 2_ Obsolete */
#define STATUS_IDX	0x02		/* 1_ Obsolete */
#define STATUS_ERR	0x01		/* 0_ Error. (an error occurred) */


/* Command Block Registers */
struct hd_cmd{
	u8	features;
	u8	count;
	u8	lba_low;
	u8	lba_mid;
	u8	lba_high;
	u8	device;
	u8	command;
};

struct part_info{
	u32	base;				/* start sector (SECTOR) */
	u32	size;				/* how many sectors in this partition */
};

/* main drive struct, one entry per drive */
struct hd_info{
	int			open_cnt;
	struct part_info	primary[NR_PRIM_PER_DRIVE];
	struct part_info	logical[NR_SUB_PER_DRIVE];
};

/* definitions */
#define HD_TIMEOUT		10000		/* in millisecond */
#define PARTITION_TABLE_OFFSET	0x1be
#define ATA_IDENTIFY		0xec
#define	ATA_READ		0x20
#define ATA_WRITE		0x30

/* data format for device register */
#define MAKE_DEVICE_REG(lba, drv, lba_highest)	(				\
						((lba) << 6)		 |	\
						((drv) << 4)		 |	\
						(lba_highest & 0xf)	 |	\
						0xa0)

#endif
