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

/**************************************************************************************************
 * 					task_fs
 **************************************************************************************************
 * <Ring 1> Main loop of Task FS.
 *************************************************************************************************/
PUBLIC void task_fs(){
	printl("Task FS begins.\n");

	MESSAGE driver_msg;
	driver_msg.type		= DEV_OPEN;
	driver_msg.DEVICE	= MINOR(ROOT_DEV);

	assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);

	send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

	spin("FS");
}
