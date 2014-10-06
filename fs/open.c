/**************************************************************************************************
 * @file			fs/open.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-9-25
 *************************************************************************************************/

#include "type.h"
#include "const.h"
#include "stdio.h"
#include "protect.h"
#include "fs.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "string.h"
#include "proto.h"
#include "global.h"
#include "hd.h"

PRIVATE struct inode*	create_file(char* path, int flags);
PRIVATE int 		alloc_imap_bit(int dev);
PRIVATE int 		alloc_smap_bit(int dev, int nr_sects_to_alloc);
PRIVATE struct inode* 	new_inode(int dev, int inode_nr, int start_sect);
PRIVATE void		new_dir_entry(struct inode* dir_inode, int nr_inode, char* filename);
/**************************************************************************************************
 * 					do_open
 **************************************************************************************************
 * Open a file and return the file descriptor.
 *
 * @return File descriptor if successful, otherwise a negative error code.
 *************************************************************************************************/
PUBLIC int do_open(){
	int fd	= -1;
	char pathname[MAX_PATH];
	/* get parameters from the message. */
	int flags	= fs_msg.FLAGS;		/* access mode */
	int name_len	= fs_msg.NAME_LEN;	/* length of filename */
	int src		= fs_msg.source;	/* caller proc nr */

	assert(name_len <= MAX_PATH);
	/* copy the pathname here */
	phys_copy((void*) va2la(TASK_FS, pathname), (void*) va2la(src, fs_msg.PATHNAME), name_len);

	pathname[name_len] = 0;

	/* find a free slot in PROCESS::filp[] */
	int i;
	for(i=0; i<NR_FILES; i++){
		if(pcaller->filp[i] == 0){
			fd = i;
			break;
		}
	}
	if((fd < 0) || (fd >= NR_FILES)){
		panic("filp[] is full (PID:%d)", proc2pid(pcaller));
	}
	
	/* find a free slot in f_desc_table[] */
	for(i=0; i<NR_FILE_DESC; i++){
		if(f_desc_table[i].fd_inode == 0){
			break;
		}
	}
	if(i >= NR_FILE_DESC){
		panic("f_desc_table[] is full (PID:%d)", proc2pid(pcaller));
	}

	int inode_nr	= search_file(pathname);

	struct inode* pin = 0;
	
	if(flags & O_CREAT){
		if(inode_nr){
			printl("file exits.\n");
			return -1;
		}else{
			pin = create_file(pathname, flags);
		}
	}else{/* read or write files. */
		assert(flags & O_RDWR);

		char filename[MAX_PATH];
		struct inode* dir_inode;

		if(strip_path(filename, pathname, &dir_inode) != 0){
			return -1;
		}
		pin = get_inode(dir_inode->i_dev, inode_nr);
	}
	/* hit the target */
	if(pin){
		/* connect proc with file_desc_table */
		pcaller->filp[fd]	= &f_desc_table[i];
		/* connect file_desc_table with inode */
		f_desc_table[i].fd_inode= pin;
		f_desc_table[i].fd_mode	= flags;
		f_desc_table[i].fd_cnt	= 1;
		f_desc_table[i].fd_pos	= 0;

		int imode = pin->i_mode & I_TYPE_MASK;
		if(imode == I_CHAR_SPECIAL){
			MESSAGE driver_msg;
			driver_msg.type		= DEV_OPEN;
			int dev			= pin->i_start_sect;
			driver_msg.DEVICE	= MINOR(dev);

			assert(MAJOR(dev) == 4);
			assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);

			send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);
		}else if(imode == I_DIRECTORY){
			assert(pin->i_num == ROOT_INODE);
		}else{
			assert(pin->i_mode == I_REGULAR);
		}
	}else{
		return -1;
	}
	return fd;
}
/**************************************************************************************************
 * 					create_file
 **************************************************************************************************
 * Create a file and return it's inode pointer.
 *
 * @param path	The full path of the new file.
 * @param flags	Attributes of the new file.
 *
 * @return	Pointer to inode of the new file if successed, otherwise 0.
 *************************************************************************************************/
PRIVATE struct inode* create_file(char* path, int flags){
	char filename[MAX_PATH];
	struct inode* dir_inode;

	if(strip_path(filename, path, &dir_inode) != 0){
		return 0;
	}

	int nr_inode		= alloc_imap_bit(dir_inode->i_dev);
	/* one file at least is 1MB... */
	int nr_free_sect	= alloc_smap_bit(dir_inode->i_dev, NR_DEFAULT_FILE_SECTS);

	struct inode* newino	= new_inode(dir_inode->i_dev, nr_inode, nr_free_sect);

	new_dir_entry(dir_inode, newino->i_num, filename);

	return newino;
}
/**************************************************************************************************
 * 					do_close
 **************************************************************************************************
 * Handle the message CLOSE.
 *
 * @return	0 if success.
 *************************************************************************************************/
PUBLIC int do_close(){
	int fd = fs_msg.FD;
	put_inode(pcaller->filp[fd]->fd_inode);
	pcaller->filp[fd]->fd_inode	= 0;
	pcaller->filp[fd]		= 0;
	return 0;

}
/**************************************************************************************************
 * 					alloc_imap_bit
 **************************************************************************************************
 * Allocate a bit in inode-map.
 *
 * @param dev	In which device the inode-map is located.
 *
 * @return	inode nr.
 *************************************************************************************************/
PRIVATE int alloc_imap_bit(int dev){
	int nr_inode	= 0;
	int i, j, k;

	int nr_imap_blk0	= 1 + 1;	/* boot sector & super block */
	struct super_block* sb	= get_super_block(dev);
	
	for(i=0; i<sb->nr_imap_sects; i++){
		RD_SECT(dev, nr_imap_blk0 + i);
		
		for(j=0; j<SECTOR_SIZE; j++){
			/* skip '11111111' bytes */
			if(fsbuf[j] == 0xff){
				continue;
			}
			/* skip '1' bits */
			for(k=0; ((fsbuf[j] >> k) & 1) != 0; k++){}

			/* i: sector index	j: byte index	k: bit index*/
			nr_inode = (i * SECTOR_SIZE + j) * 8 + k;
			fsbuf[j] |= (1 << k);

			/* write the bit to imap */
			WR_SECT(dev, nr_imap_blk0 + i);
			break;
		}
		return nr_inode;
	}
	/* no free bit in imap */
	panic("inode-map is probably full.\n");
	return 0;
}
/**************************************************************************************************
 * 					alloc_smap_bit
 **************************************************************************************************
 * Allocate a bit in sector-map.
 *
 * @param dev			In which device the sector-map is located.
 * @param nr_sects_to_alloc	How many sectors are allocated.
 *
 * @return	The 1st sector nr allocated.
 *************************************************************************************************/
PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc){
	int i, j, k;	/* i: sector index	j: byte index	k: bit index */
	int nr_free_sect	= 0;

	struct super_block* sb	= get_super_block(dev);
	int nr_smap_blk0	= 1 + 1 + sb->nr_imap_sects;

	for(i=0; i<sb->nr_smap_sects; i++){
		RD_SECT(dev, nr_smap_blk0 + i);

		for(j=0; j<SECTOR_SIZE && nr_sects_to_alloc > 0; j++){
			k = 0;
			if(!nr_free_sect){
				/* loop untill a free bit is found. */
				if(fsbuf[j] == 0xff){
					continue;
				}
				for(; ((fsbuf[j] >> k) & 1) != 0; k++){}

				nr_free_sect = (i * SECTOR_SIZE + j) * 8 + k - 1 + sb->n_1st_sect;
			}
			/* repeat till enough bits are set */
			for(; k < 8; k++){
				assert(((fsbuf[j] >> k) & 1) == 0);
				fsbuf[j] |= (1 << k);
				if(--nr_sects_to_alloc == 0){
					break;
				}
			}
		}
		/* free bit was found, write the bit to smap */
		if(nr_free_sect){
			WR_SECT(dev, nr_smap_blk0 + i);
		}

		if(nr_sects_to_alloc == 0){
			break;
		}
	}
	assert(nr_sects_to_alloc == 0);

	return nr_free_sect;
}
/**************************************************************************************************
 * 					new_inode
 **************************************************************************************************
 * Generate a new inode and write it to disk.
 *
 * @param dev		Home device of the inode.
 * @param nr_inode	inode nr.
 * @param start_sect	Start sector of the file pointed by the new inode.
 *
 * @return		Pointer of the new inode.
 *************************************************************************************************/
PRIVATE struct inode* new_inode(int dev, int nr_inode, int start_sect){
	struct inode* new_inode = get_inode(dev, nr_inode);
	
	new_inode->i_mode	= I_REGULAR;
	new_inode->i_size	= 0;
	new_inode->i_start_sect	= start_sect;
	new_inode->i_nr_sects	= NR_DEFAULT_FILE_SECTS;

	new_inode->i_dev	= dev;
	new_inode->i_cnt	= 1;
	new_inode->i_num	= nr_inode;

	/* write it to the inode array */
	sync_inode(new_inode);

	return new_inode;
}
/**************************************************************************************************
 * 					new_dir_entry
 **************************************************************************************************
 * Write a new entry into the directory.
 *
 * @param dir_inode	Inode of the directory.
 * @param nr_inode	Inode nr of the new file.
 * @param filename	File name of the new file.
 *************************************************************************************************/
PRIVATE void new_dir_entry(struct inode* dir_inode, int nr_inode, char* filename){
	/* Write the dir_entry */
	int nr_dir_blk0	= dir_inode->i_start_sect;
	int nr_dir_blks	= (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
	/* including unused slots (the file has been deleted but the slot is still there) */
	int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;

	int m = 0;
	int i, j;
	struct dir_entry* pde;
	struct dir_entry* new_de = 0;

	for(i=0; i<nr_dir_blks; i++){
		RD_SECT(dir_inode->i_dev, nr_dir_blk0 + i);

		pde = (struct dir_entry*) fsbuf;
		for(j=0; j<SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++){
			if(++m > nr_dir_entries){
				break;
			}
			if(pde->inode_nr == 0){	/* it's a free slot */
				new_de = pde;
				break;
			}
		}
		if(m > nr_dir_entries || new_de){/* all entries have been iterated or */
			break;			/* free slot is found */
		}
	}
	if(!new_de){				/* reached the end of the dir */
		new_de = pde;
		dir_inode->i_size += DIR_ENTRY_SIZE;
	}
	new_de->inode_nr = nr_inode;
	strcpy(new_de->name, filename);

	/* Write dir block -- ROOT dir block */
	WR_SECT(dir_inode->i_dev, nr_dir_blk0 + i);

	/* update dir inode */
	sync_inode(dir_inode);
}
