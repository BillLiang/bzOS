/**************************************************************************************************
 * @file			clock.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-8-27
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

/**************************************************************************************************
 *					clock_handler
 **************************************************************************************************
 * <Ring 0> This routine handles the clock interrupt generated by 8253/8254
 * 	    programmable interval timer.
 *
 * @param irq	The IRQ nr, unused here.
 *************************************************************************************************/
PUBLIC void clock_handler(int irq){
	ticks ++;
	p_proc_ready->ticks --;				/* 当前进程ticks递减 */

	if(key_pressed){
		inform_int(TASK_TTY);
	}

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

PUBLIC void init_clock(){
	/* 初始化 8253 PIT，修改时钟中断间隔 */
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (u8) (TIMER_FREQ / HZ));
	out_byte(TIMER0, (u8) ((TIMER_FREQ / HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);
}
/*==================================================================================================
  				milli_delay
==================================================================================================*/
PUBLIC void milli_delay(int milli_sec){
	int t = get_ticks();
	while(((get_ticks() - t) * 1000 / HZ) < milli_sec){}
}
