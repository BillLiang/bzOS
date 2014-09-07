/**************************************************************************************************
 * @file			hd.h
 * @brief 			
 * @author			Bill Liang
 * @date			2014-9-6
 *************************************************************************************************/
#ifndef _BZOS_HD_H_
#define _BZOS_HD_H_
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
#define STATUS_CORR	0x04		/* 2_ Obsolete*/
#define STATUS_IDX	0x02		/* 1_ Obsolete*/
#define STATUS_ERR	0x01		/* 0_ Error. (an error occurred)*/


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

/* definitions */
#define HD_TIMEOUT		10000		/* in millisecond */
#define ATA_IDENTIFY		0xec
#define	ATA_READ		0x20
#define ATA_WRITE		0x30

/* data format for device register */
#define MAKE_DEVICE_REG(lba, drv, lba_highest)	(				\
						((lba) << 6)		 |	\
						((drv) << 4)		 |	\
						((lba_highest) & 0x0f)	 |	\
						0xa0)

#endif
