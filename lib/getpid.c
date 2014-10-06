/**************************************************************************************************
 * @file			getpid.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-10-6
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
#include "global.h"
#include "proto.h"
/**************************************************************************************************
 * 					getpid
 **************************************************************************************************
 * Get the PID.
 *
 * @return	The PID.
 *************************************************************************************************/
PUBLIC int getpid(){
	MESSAGE msg;
	msg.type = GET_PID;

	send_recv(BOTH, TASK_SYS, &msg);
	assert(msg.type == SYSCALL_RET);

	return msg.PID;
}
