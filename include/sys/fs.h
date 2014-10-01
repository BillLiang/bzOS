/**************************************************************************************************
 * @file	fs.h
 * @brief	Header file for File System
 * @author	Bill Liang
 * @date	2014-9-8
 *************************************************************************************************/
#ifndef _BZOS_FS_H_
#define	_BZOS_FS_H_
/*
 * @brief	The File System on the hard disk.
 *
 * |------------------------------|
 * |				  |
 * |				  |
 * |		Data		  |	// Store data.
 * |				  |	
 * |				  |	
 * |	--------------------	  |
 * |	Root(belogs to data)	  |	// It's similar with FAT12.
 * |------------------------------|
 * |				  |
 * |	     inode_array	  |	// Store all inodes data struct, every one maintains information about
 * |				  |	// the file. (4096 inodes max, 32 bytes per inode.)
 * |------------------------------|
 * |				  |
 * |	      sector_map	  |	// Bit-map, which maps the usage condition of each sector.
 * |				  |
 * |------------------------------|
 * |	      inode_map		  |	// Bit-map, which maps the inode_array. Size: one sector.(4094 inodes max)
 * |------------------------------|
 * |	     super_block	  |	// Store metadata of the FS.
 * |------------------------------|
 * |	      boot_sector	  |	// Boot sector, of course.
 * |------------------------------|
 *
 */

struct dev_drv_map{
	int	driver_nr;		/* The proc nr of the device driver. */
};

/**
 * @brief 	Magic number of FS v1.0
 */
#define MAGIC_V1	0x111

/**************************************************************************************************
 * @struct	super_block.
 * @brief	The 2nd secotr of the FS on the disk. Stores metadata of the FS.
 *
 * Remember to change SUPER_BLOCK_SIZE if the members are changed.
 *************************************************************************************************/
struct super_block {
	u32	magic;			/* Magic number of the FS. */

	u32	nr_imap_sects;		/* How many inode-map sectors. */
	u32	nr_smap_sects;		/* How many sector-map sectors. */

	u32	nr_inodes;		/* How many inodes in inodes-array. */
	u32	nr_inode_sects;		/* How many inode sectors. */

	u32	root_inode;		/* Inode nr of root directory. */

	u32	n_1st_sect;		/* Number of the 1st data sector. */

	u32	nr_sects;		/* How many sectors of this FS. */
	/*******************************************************************/
	u32	inode_size;		/* INODE_SIZE */
	u32	inode_isize_off;	/* Offset of 'struct inode::i_size' */
	u32	inode_start_off;	/* Offset of 'struct inode::i_start_sect' */

	u32	dir_ent_size;		/* DIR_ENTRY_SIZE */
	u32	dir_ent_inode_off;	/* Offset of 'struct dir_entry::inode_nr' */
	u32	dir_ent_fname_off;	/* Offset of 'struct dir_entry::name' */

	/*
	 * The following items are only present in menory.
	 */
	int	sb_dev;			/* The super block's home device. */
};

/*
 * Note that this is the size of the struct in the device, NOT in memory!
 * The size in memory is larger because of some more members.
 */
#define SUPER_BLOCK_SIZE	56

/**************************************************************************************************
 * @brief	The 'start_sect' and 'nr_sects' locate the file in the device,
 * and the size show how many bytes is used.
 *
 * If 'size < (nr_sects * SECTOR_SIZE)', the rest bytes are wasted
 * and reserved for later writing.
 *************************************************************************************************/
struct inode {
	u32	i_mode;			/* Access mode, distinguish between data on disk and devices of PC */

	u32	i_size;			/* File size, in byte */
	u32	i_start_sect;		/* The first sector of the data */
	u32	i_nr_sects;		/* How many sectors the file occupies */

	u8	_unused[16];		/* Stuff for alignment */

	/*
	 * The following items are only present in memory
	 */
	int	i_dev;
	int	i_cnt;			/* How many procs share this inode */
	int	i_num;			/* inode nr */
};

/*
 * Note that this is the size of the struct in the device, NOT in memory!
 * The size in memory is larger because of some more members.
 */
#define INODE_SIZE		32

/**
 * @brief	Max length of a filename.
 */
#define MAX_FILENAME_LEN	12

/**************************************************************************************************
 * @brief	Directory Entry stored in file in the root directory.
 *************************************************************************************************/
struct dir_entry {
	int	inode_nr;		/* inode nr */	
	char	name[MAX_FILENAME_LEN];	/* filename */
};

/**
 * @brief	The size of directory entry in the device.
 *
 * It is as same as the size in memory.
 */
#define DIR_ENTRY_SIZE		sizeof(struct dir_entry)

/**
 * @bref	File Descriptor
 */
struct file_desc{
	int		fd_mode;	/* R or W */
	int		fd_pos;		/* Current position for R/W */
	struct inode*	fd_inode;	/* Pointer to the i-node. */
};


/**
 * Since all invocations of 'rw_sector()' in FS look similar
 * (most of the params are the same), we use this macro to make
 * code more readable.
 */
#define RD_SECT(dev, sect_nr) rw_sector(DEV_READ,		\
					dev,			\
					(sect_nr) * SECTOR_SIZE,\
					SECTOR_SIZE,		\
					TASK_FS,		\
					fsbuf);
#define WR_SECT(dev, sect_nr) rw_sector(DEV_WRITE,		\
					dev,			\
					(sect_nr) * SECTOR_SIZE,\
					SECTOR_SIZE,		\
					TASK_FS,		\
					fsbuf);


#endif
