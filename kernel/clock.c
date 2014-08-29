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
	ticks ++;
	p_proc_ready->ticks --;					/* 当前进程ticks递减 */
	/* 如果是中断重入，什么也不干 */
	if(k_reenter != 0){
		return;
	}
	/* 如果当前进程的ticks还没有降到零 */
	if(p_proc_ready->ticks > 0){
		return;
	}
	
	schedule();
}
/*==================================================================================================
  				milli_delay
==================================================================================================*/
PUBLIC void milli_delay(int milli_sec){
	int t = get_ticks();
	while(((get_ticks() - t) * 1000 / HZ) < milli_sec){}
}
