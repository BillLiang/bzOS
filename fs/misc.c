/**************************************************************************************************
 * @file			fs/misc.c
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
#include "fs.h"
/**************************************************************************************************
 * 					search_file
 **************************************************************************************************
 * Search the file and return the inode_nr.
 *
 * @param path		The full path of the file to search.
 * 
 * @return		Pointer to the i-node of the file if successful, otherwise 0.
 *************************************************************************************************/
PUBLIC int search_file(char* path){
	int i, j;
	char filename[MAX_PATH];
	memset(filename, 0, MAX_FILENAME_LEN);
	
	struct inode* dir_inode;
	/* dir_inode now is root_dir */
	if(strip_path(filename, path, &dir_inode) != 0){
		return 0;
	}
	if(filename[0] == 0){	/* path: '/' */
		return dir_inode->i_num;
	}
	/* Search the dir for the file. */
	int nr_dir_blk0		= dir_inode->i_start_sect;
	int nr_dir_blks		= (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	int nr_dir_entries	= dir_inode->i_size / DIR_ENTRY_SIZE;

	int m = 0;
	struct dir_entry* pde;
	for(i=0; i<nr_dir_blks; i++){
		RD_SECT(dir_inode->i_dev, nr_dir_blk0 + i);
		pde = (struct dir_entry*) fsbuf;
		for(j=0; j<SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++){
			if(memcmp(filename, pde->name, MAX_FILENAME_LEN) == 0){
				return pde->inode_nr;
			}
			if(++m > nr_dir_entries){
				break;
			}
		}
		/* all entries have been iterated */
		if(m > nr_dir_entries){
			break;
		}
	}
	/* file not found */
	return 0;

}

/**************************************************************************************************
 * 					strip_path
 **************************************************************************************************
 * Get the basename from the fullpath.
 *
 * In bzOS, all files are stored in the root directory.
 * There is no sub-folder thing.
 *
 * @param[out] filename		The string for the result.
 * @param[in] pathname		The full pathname.
 * @param[out] ppinode		The pointer of the dir's inode will be stored here.
 *
 * @return 0 if success, otherwise the pathname is not valid.
 *************************************************************************************************/
PUBLIC int strip_path(char* filename, const char* pathname, struct inode** ppinode){
	const char* s	 = pathname;
	char* t		 = filename;

	if(s == 0){
		return -1;
	}
	if(*s == '/'){
		s ++;
	}
	while(*s){
		if(*s == '/'){
			return -1;
		}
		*t ++ = *s ++;
		/* if filename is too long, just truncate it. */
		if(t - filename >= MAX_FILENAME_LEN){
			break;
		}
	}
	*t = 0;

	*ppinode = root_inode;

	return 0;
}
