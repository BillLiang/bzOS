/**************************************************************************************************
 * @file			config.h
 * @brief 			
 * @author			Bill Liang
 * @date			2014-9-10
 *************************************************************************************************/
/* minor number of boot partition */
#define MINOR_BOOT		MINOR_hd2a

/**
 * boot parameters are stored by the loader, thery should be there when kernel is running and
 * should not be overwritten since kernel might use them at any time.
 */
#define BOOT_PARAM_ADDR		0x900	/* physical address */
#define BOOT_PARAM_MAGIC	0xb007	/* magic number */
#define BI_MAG			0	/* index of magic number in BOOT_PARAM */
#define BI_MEM_SIZE		1	/* index of memory size in BOOT_PARAM */
#define BI_KERNEL_FILE		2	/* index of kernel file addr in BOOT_PARAM */
