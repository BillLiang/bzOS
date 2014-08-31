/*=================================================================================================
		  main.c			Bill Liang	2014-8-26
=================================================================================================*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

PUBLIC int kernel_main(){
	disp_str("--------\"kernel_main\" begins--------\n");

	PROCESS*	p_proc		= proc_table;
	TASK*		p_task		= task_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	
	int 		i;

	/* 初始化进程表 */
	for(i=0; i<NR_TASKS; i++){
		strcpy(p_proc->p_name, p_task->name);
		p_proc->pid = i;

		p_proc->ldt_sel = selector_ldt;						/* 当前进程的LDT在GDT中选择子 */
		/* 这里简单地把GDT的代码段描述符复制到LDT的第一个描述符中,然后下一句改变了该LDT描述符的DPL */
		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		/* 这里简单地把GDT的数据段描述符复制到LDT的第二个描述符中,然后下一句改变了该LDT描述符的DPL */
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;

		p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;		/* cs为指向LDT第一个描述符的选择子 */
		p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;		/* ds为指向LDT第二个描述符的选择子 */
		p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;		/* es为指向LDT第二个描述符的选择子 */
		p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;		/* fs为指向LDT第二个描述符的选择子 */
		p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;		/* ss为指向LDT第二个描述符的选择子 */
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;		/* gs仍然指向显存，只是改变了DPL让其在低特权级下运行 */
	
		p_proc->regs.eip = (u32) p_task->initial_eip;					/* eip为指向进程体 */
		p_proc->regs.esp = (u32) p_task_stack;						/* esp指向新的栈底 */
		p_proc->regs.eflags = 0x1202;

		p_task_stack -= p_task->stacksize;
		p_proc ++;
		p_task ++;
		selector_ldt += (1 << 3);
	}
	for(i=0; i<NR_TASKS; i++){
		proc_table[i].ticks = proc_table[i].priority = 10;
	}

	ticks = 0;
	k_reenter = 0;									/* 用于判断中断嵌套时中断是否重入 */
	p_proc_ready = proc_table;

	init_clock();

	restart();

	while(1){}
}

/*=================================================================================================
  					一个进程体
=================================================================================================*/
void TestA(){
	int i = 0;
	while(1){
		//disp_color_str("A.", BRIGHT | MAKE_COLOR(BLACK, RED));
		milli_delay(200);
	}
}

/*=================================================================================================
  					一个进程体
=================================================================================================*/
void TestB(){
	int i = 0x100;
	while(1){
		//disp_color_str("B.", BRIGHT | MAKE_COLOR(BLACK, RED));
		milli_delay(200);
	}
}
/*=================================================================================================
  					一个进程体
=================================================================================================*/
void TestC(){
	int i = 0x2000;
	while(1){
		//disp_color_str("C.", BRIGHT | MAKE_COLOR(BLACK, RED));
		milli_delay(200);
	}
}
