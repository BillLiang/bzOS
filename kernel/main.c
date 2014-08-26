/*=================================================================================================
			main.c			Bill Liang	2014-8-26
=================================================================================================*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

PUBLIC int kernel_main(){
	disp_str("--------\"kernel_main\" begins--------");

	/* 初始化进程表 */
	PROCESS* p_proc = proc_table;
	/* 由于现在的Proc_table数组只是有一个元素，所以p_proc就是p_proc[0]啦 */
	p_proc->ldt_sel = SELECTOR_LDT_FIRST;						/* 当前进程的LDT在GDT中选择子 */
	/* 这里简单得把GDT的代码段描述符复制到LDT的第一个描述符中,然后下一句改变了该LDT描述符的DPL */
	memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
	p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
	/* 这里简单得把GDT的数据段描述符复制到LDT的第二个描述符中,然后下一句改变了该LDT描述符的DPL */
	memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
	p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;

	p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;		/* cs为指向LDT第一个描述符的选择子 */
	p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;		/* ds为指向LDT第二个描述符的选择子 */
	p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;		/* es为指向LDT第二个描述符的选择子 */
	p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;		/* fs为指向LDT第二个描述符的选择子 */
	p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;		/* ss为指向LDT第二个描述符的选择子 */
	p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;		/* gs仍然指向显存，只是改变了DPL让其在低特权级下运行 */

	p_proc->regs.eip = (u32) TestA;							/* eip为指向TestA */
	p_proc->regs.esp = (u32) task_stack + STACK_SIZE_TOTAL;				/* esp指向新的栈底 */
	p_proc->regs.eflags = 0x1202;
	
	p_proc_ready = proc_table;

	restart();

	while(1){}
}

/*=================================================================================================
  					一个简单的进程体
=================================================================================================*/
void TestA(){
	int i = 0;
	while(1){
		disp_str("A");
		disp_int(i ++);
		disp_str(".");
		delay(1);
	}
}
