/**************************************************************************************************
 * @file	read.c
 * @brief
 * @author	Bill Liang
 * @date	2014-10-2
 *************************************************************************************************/

#include "type.h"
#include "const.h"
#include "stdio.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "console.h"
#include "tty.h"
#include "proto.h"
#include "global.h"

/**************************************************************************************************
 * 					read
 **************************************************************************************************
 * Read from a file descriptor.
 *
 * @param fd	File descriptor.
 * @param buf	Buffer to accept the bytes read.
 * @param count	How many bytes to read.
 *
 * @return	On successful, the number of bytes read are returned.
 * 		On error, -1 is returned.
 *************************************************************************************************/
PUBLIC int read(int fd, void* buf, int count){
	MESSAGE msg;
	msg.type	= READ;
	msg.FD		= fd;
	msg.BUF		= buf;
	msg.CNT		= count;

	send_recv(BOTH, TASK_FS, &msg);

	return msg.CNT;
}
