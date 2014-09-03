/*==================================================================================================
			proc.c			Bill Liang	2014-8-29
==================================================================================================*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "string.h"
#include "proto.h"
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

/**************************************************************************************************
 * 					ldt_seg_linear
 **************************************************************************************************
 * <Ring 0~1> 计算给定进程特定段的线性地址
 * @param p	whose
 * @param idx	进程LDT中哪个描述符
 *************************************************************************************************/
PUBLIC int ldt_seg_linear(PROCESS* p_proc, int idx){
	DESCRIPTOR* d	 = &p_proc->ldts[idx];
	return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}

/**************************************************************************************************
 * 					va2la
 **************************************************************************************************
 * <Ring 0~1> 虚拟地址 -> 线性地址
 * @param pid	whose
 * @param va	虚拟地址（逻辑地址？）
 *************************************************************************************************/
PUBLIC void* va2la(int pid, void* va){
	PROCESS* p	= &proc_table[pid];
	u32 seg_base	= ldt_seg_linear(p, INDEX_LDT_RW);
	u32 la		= seg_base + (u32)va;

	if(pid < NR_TASKS + NR_PROCS){
		assert(la == (u32)va);
	}
	return (void*)la;
}
