/**************************************************************************************************
 * @file	write.c
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
 * 					write
 **************************************************************************************************
 * Write to a file descriptor.
 *
 * @param fd	File descriptor.
 * @param buf	Buffer including the bytes to write.
 * @param count	How many bytes to write.
 *
 * @return	On successful, the number of bytes written are returned.
 * 		On error, -1 is returned.
 *************************************************************************************************/
PUBLIC int write(int fd, const void* buf, int count){
	MESSAGE msg;
	msg.type	= WRITE;
	msg.FD		= fd;
	msg.BUF		= buf;
	msg.CNT		= count;

	send_recv(BOTH, TASK_FS, &msg);

	return msg.CNT;
}	
