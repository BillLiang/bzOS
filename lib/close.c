/**************************************************************************************************
 * @file			close.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-10-01
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

/**************************************************************************************************
 * 					close
 **************************************************************************************************
 * Close a file descriptor.
 *
 * @param fd	File descriptor.
 *
 * @return	0 if successful, otherwise -1.
 *************************************************************************************************/
PUBLIC int close(int fd){
	MESSAGE msg;
	msg.type	= CLOSE;
	msg.FD		= fd;

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}
