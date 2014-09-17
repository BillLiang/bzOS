/**************************************************************************************************
 * @file	systask.c
 * @brief
 * @author	BillLiang
 * @date	2014-9-5
 *************************************************************************************************/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "fs.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "string.h"
#include "proto.h"
#include "global.h"

/**************************************************************************************************
 * 					task_sys
 **************************************************************************************************
 * <Ring 1> The main loop off TASK SYS.
 *************************************************************************************************/
PUBLIC void task_sys(){
	MESSAGE msg;				/* for loading the msg sent */
	while(TRUE){
		send_recv(RECEIVE, ANY, &msg);	/* get the msg */
		int src		= msg.source;
		switch(msg.type){
		case GET_TICKS:
			msg.RETVAL = ticks;
			send_recv(SEND, src, &msg);
			break;
		default:
			panic("unknown msg type");
			break;
		}
	}
}
