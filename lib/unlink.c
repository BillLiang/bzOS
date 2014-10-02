/**************************************************************************************************
 * @file			unlink.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-10-2
 *************************************************************************************************/

#include "type.h"
#include "config.h"
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

/**************************************************************************************************
 * 						unlink
 **************************************************************************************************
 * Delete a file.
 *
 * @param pathname	The full path of the file to be deleted.
 *
 * @return		0: success, -1: error.
 *************************************************************************************************/
PUBLIC int unlink(const char* pathname){
	MESSAGE msg;
	msg.type	= UNLINK;
	msg.PATHNAME	= (void*) pathname;
	msg.NAME_LEN	= strlen(pathname);

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}

