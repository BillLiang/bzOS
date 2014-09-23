/**************************************************************************************************
 * @file			main.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-9-7
 *************************************************************************************************/
#include "type.h"
#include "config.h"
#include "const.h"
#include "protect.h"
#include "fs.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "string.h"
#include "proto.h"
#include "global.h"
#include "hd.h"

PRIVATE void init_fs();
PRIVATE void mkfs();

/**************************************************************************************************
 * 					task_fs
 **************************************************************************************************
 * <Ring 1> Main loop of Task FS.
 *************************************************************************************************/
PUBLIC void task_fs(){
	init_fs();
	spin("FS");
}

/**************************************************************************************************
 * 					init_fs
 **************************************************************************************************
 * <Ring 1> Do some preparation.
 *************************************************************************************************/
PRIVATE void init_fs(){
	MESSAGE driver_msg;
	driver_msg.type		= DEV_OPEN;
	driver_msg.DEVICE	= MINOR(ROOT_DEV);

	assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);

	send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

	mkfs();
}

/**************************************************************************************************
 * 					mkfs
 **************************************************************************************************
 * <Ring 1> Make an available bzOS FS in the disk. It will
 * 	- Write a super block to sector 1.
 * 	- Create three special files: dev_tty0, dev_tty1, dev_tty2.
 * 	- Create the inode map.
 * 	- Create the sector map.
 * 	- Create the inodes of the files.
 * 	- Create '/', the root directory.
 *************************************************************************************************/
PRIVATE void mkfs(){
	MESSAGE	driver_msg;
	int i, j;
	int bits_per_sect	= SECTOR_SIZE * 8;

	/* get the geometry of ROOTDEV */
	struct part_info geo;
	driver_msg.type		= DEV_IOCTL;
	driver_msg.DEVICE	= MINOR(ROOT_DEV);
	driver_msg.REQUEST	= DIOCTL_GET_GEO;
	driver_msg.BUF		= &geo;
	driver_msg.PROC_NR	= TASK_FS;
	
	assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);

	send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

	printl("dev size: %x sectors\n", geo.size);

	/******************************************************************************************
	 * Super Block
	 *****************************************************************************************/
	struct super_block sb;
	sb.magic		= MAGIC_V1;

	sb.nr_inodes		= bits_per_sect;
	sb.nr_inode_sects	= sb.nr_inodes * INODE_SIZE / SECTOR_SIZE;
	sb.nr_sects		= geo.size;	/* partition size */
	sb.nr_imap_sects	= 1;
	sb.nr_smap_sects	= sb.nr_sects / bits_per_sect + 1;
	sb.n_1st_sect		= 1 + 1 + /* boot sector & super block */
				  sb.nr_imap_sects + sb.nr_smap_sects + sb.nr_inode_sects;
	sb.root_inode		= ROOT_INODE;
	sb.inode_size		= INODE_SIZE;
	
	struct inode x;
	sb.inode_isize_off	= (int)&x.i_size - (int)&x;
	sb.inode_start_off	= (int)&x.i_start_sect - (int)&x;

	sb.dir_ent_size		= DIR_ENTRY_SIZE;
	
	struct dir_entry de;
	sb.dir_ent_inode_off	= (int)&de.inode_nr - (int)&de;
	sb.dir_ent_fname_off	= (int)&de.name - (int)&de;

	memset(fsbuf, 0x90, SECTOR_SIZE);
	memcpy(fsbuf, &sb, SUPER_BLOCK_SIZE);
	/* write the super block using fsbuf */
	WR_SECT(ROOT_DEV, 1);
	/* 这里全是字节偏移量，后面加上00，然后* 2，其实就是（扇区*扇区字节大小） */
	printl("devbase: %x00, sb: %x00, imap: %x00, smap: %x00\n"
		"        inodes: %x00, 1st_sector: %x00\n",
		geo.base * 2,
		(geo.base + 1) * 2,
		(geo.base + 2) * 2,
		(geo.base + 2 + sb.nr_imap_sects) * 2,
		(geo.base + 2 + sb.nr_imap_sects + sb.nr_smap_sects) * 2,
		(geo.base + sb.n_1st_sect) *2
		);

	/******************************************************************************************
	 * inode map
	 *****************************************************************************************/
	memset(fsbuf, 0, SECTOR_SIZE);
	/* there are NR_CONSOLES character devices here */
	for(i=0; i<NR_CONSOLES + 2; i++){
		fsbuf[0] |= 1 << i;
	}

	assert(fsbuf[0] == 0x1f);/**
				  * 0001 1111:
				  * bit 0: reserved.
				  * bit 1: the first inode, which indicates '/'.
				  * bit 2: /dev_tty0
				  * bit 3: /dev_tty1
				  * bit 4: /dev_tty2
				  */
	WR_SECT(ROOT_DEV, 2);

	/******************************************************************************************
	 * sector map
	 *****************************************************************************************/
	memset(fsbuf, 0, SECTOR_SIZE);
	int nr_sects	= NR_DEFAULT_FILE_SECTS + 1;	/* bit 0 is reserved, root directory */
	for(i=0; i<nr_sects / 8; i++){			/* Notify this division */
		fsbuf[i] = 0xff;
	}
	for(j=0; j<nr_sects % 8; j++){
		fsbuf[i] |= (1 << j);
	}

	WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects);

	/* zeromemory the rest sector-map */
	memset(fsbuf, 0, SECTOR_SIZE);
	for(i=1; i<sb.nr_smap_sects; i++){
		WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + i);
	}
	/******************************************************************************************
	 * inodes
	 *****************************************************************************************/
	/* inode of '/' */
	memset(fsbuf, 0, SECTOR_SIZE);
	struct inode* pi	= (struct inode*) fsbuf;
	pi->i_mode		= I_DIRECTORY;
	pi->i_size		= DIR_ENTRY_SIZE * 4;/* 4 files:
						      * '.'.
						      * 'dev_tty0', 'dev_tty1', 'dev_tty2'
						      */
	pi->i_start_sect	= sb.n_1st_sect;
	pi->i_nr_sects		= NR_DEFAULT_FILE_SECTS;
	/* inode of '/dev_tty0~2' */
	for(i=0; i<NR_CONSOLES; i++){
		pi		= (struct inode*) (fsbuf + (INODE_SIZE * (i + 1)));
		pi->i_mode	= I_CHAR_SPECIAL;
		pi->i_size	= 0;
		pi->i_start_sect= MAKE_DEV(DEV_CHAR_TTY, i);
		pi->i_nr_sects	= 0;
	}
	WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + sb.nr_smap_sects);
	/******************************************************************************************
	 * '/'
	 *****************************************************************************************/
	memset(fsbuf, 0, SECTOR_SIZE);
	struct dir_entry* pde	= (struct dir_entry*)fsbuf;
	pde->inode_nr		= 1;
	strcpy(pde->name, ".");

	/* dir entries of '/dev_tty0~2' */
	for(i=0; i<NR_CONSOLES; i++){
		pde ++;
		pde->inode_nr	= i + 2;
		sprintf(pde->name, "dev_tty%d", i);	/* do not use 'vsprintf' */
	}
	WR_SECT(ROOT_DEV, sb.n_1st_sect);

}

/**************************************************************************************************
 * 					rw_sector
 **************************************************************************************************
 * <Ring 1> R/W a sector via messaging with the corresponding driver.
 *
 * @param io_type	DEV_READ or DEV_WRITE.
 * @param dev		device nr.
 * @param pos		Byte offset from/to where to r/w.
 * @param bytes		r/w count in bytes.
 * @param proc_nr	To whom the buffer belongs.
 * @param buf		r/w buffer.
 *
 * @return 0 if success.
 *************************************************************************************************/
PUBLIC int rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_nr, void* buf){
	MESSAGE driver_msg;
	driver_msg.type		= io_type;
	driver_msg.DEVICE	= MINOR(dev);
	driver_msg.POSITION	= pos;
	driver_msg.BUF		= buf;
	driver_msg.CNT		= bytes;
	driver_msg.PROC_NR	= proc_nr;

	assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);

	send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);

	return 0;
}

