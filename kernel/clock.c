/*==================================================================================================
			clock.c				Bill Liang	2014-8-27
==================================================================================================*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

PUBLIC void clock_handler(int irq){
	disp_str("^");
	/* 如果是中断重入，什么也不干 */
	if(k_reenter != 0){
		disp_str("!");
		return;
	}

	p_proc_ready ++;
	if(p_proc_ready >= proc_table + NR_TASKS){
		p_proc_ready = proc_table;
	}
}
