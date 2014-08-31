/*==================================================================================================
			proc.c			Bill Liang	2014-8-29
==================================================================================================*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

PUBLIC int sys_get_ticks(){
	return ticks;
}
/*==================================================================================================
  				进程调度
==================================================================================================*/
PUBLIC void schedule(){
	PROCESS*	p;
	int		greatest_ticks = 0;
	while(!greatest_ticks){
		for(p=proc_table; p<proc_table + NR_TASKS + NR_PROCS; p++){
			if(p->ticks > greatest_ticks){
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}
		/* 当所有的进程的ticks都减到零时 */
		if(!greatest_ticks){
			for(p=proc_table; p<proc_table + NR_TASKS + NR_PROCS; p++){
				p->ticks = p->priority;
			}
		}
	}
}
