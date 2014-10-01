/**************************************************************************************************
 * @file			kernel/main.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-8-26
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

PUBLIC int kernel_main(){
	disp_str("--------\"kernel_main\" begins--------\n");

	PROCESS*	p_proc		= proc_table;
	TASK*		p_task		= task_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	
	int 		i;

	u8		privilege;
	u8		rpl;
	int		eflags;

	int		prio;						/* priority */
	/* initialize the proc table */
	for(i=0; i<NR_TASKS + NR_PROCS; i++){
		if(i < NR_TASKS){					/* TASKS, running in Ring1 */
			p_task		= task_table + i;
			privilege	= PRIVILEGE_TASK;
			rpl		= RPL_TASK;
			eflags		= 0x1202;			/* IF=1, IOPL=1, bit 2 always 1 */
			prio		= 15;				/* default priority of a task */
		}else{							/* USER PROC, running in Ring3 */
			p_task		= user_proc_table + (i - NR_TASKS);
			privilege	= PRIVILEGE_USER;
			rpl		= RPL_USER;
			eflags		= 0x202;			/* IF=1, bit 2 always 1 */
			prio		= 5;
		}

		strcpy(p_proc->name, p_task->name);
		p_proc->pid = i;

		p_proc->ldt_sel = selector_ldt;						/* 当前进程的LDT在GDT中选择子 */
		/* 这里简单地把GDT的代码段描述符复制到LDT的第一个描述符中,然后下一句改变了该LDT描述符的DPL */
		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		/* 这里简单地把GDT的数据段描述符复制到LDT的第二个描述符中,然后下一句改变了该LDT描述符的DPL */
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;

		p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;		/* cs为指向LDT第一个描述符的选择子 */
		p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;		/* ds为指向LDT第二个描述符的选择子 */
		p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;		/* es为指向LDT第二个描述符的选择子 */
		p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;		/* fs为指向LDT第二个描述符的选择子 */
		p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;		/* ss为指向LDT第二个描述符的选择子 */
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;		/* gs仍然指向显存，只是改变了DPL让其在低特权级下运行 */
	
		p_proc->regs.eip = (u32) p_task->initial_eip;				/* eip为指向进程体 */
		p_proc->regs.esp = (u32) p_task_stack;					/* esp指向新的栈底 */
		p_proc->regs.eflags = eflags;

		p_proc->nr_tty = 0;							/* 默认TTY为0 */

		p_proc->flags		= 0;						/* this proc is runnable */
		p_proc->p_msg		= 0;						/* null message */
		p_proc->recv_from	= NO_TASK;
		p_proc->send_to		= NO_TASK;
		p_proc->has_int_msg	= 0;
		p_proc->q_sending	= 0;
		p_proc->next_sending	= 0;

		p_proc->ticks = p_proc->priority = prio;
	
		p_task_stack -= p_task->stacksize;
		p_proc ++;
		p_task ++;
		selector_ldt += (1 << 3);
	}
	proc_table[NR_TASKS + 0].nr_tty = 0;
	proc_table[NR_TASKS + 1].nr_tty = 1;
	proc_table[NR_TASKS + 2].nr_tty = 1;

	ticks = 0;
	k_reenter = 0;									/* 用于判断中断嵌套时中断是否重入 */
	p_proc_ready = proc_table;

	init_clock();
	init_keyboard();

	restart();

	while(TRUE){}
}
/*=================================================================================================
  					get_ticks()
=================================================================================================*/
PUBLIC int get_ticks(){
	MESSAGE	msg;

	reset_msg(&msg);
	msg.type	= GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}

/*=================================================================================================
  					一个进程体
=================================================================================================*/
void TestA(){
	int fd = open("/blah", O_CREAT);
	printf("fd: %d\n", fd);
	close(fd);
	spin("TestA");
}

/*=================================================================================================
  					一个进程体
=================================================================================================*/
void TestB(){
	while(TRUE){
		printf("B");
		milli_delay(2000);
	}
}
/*=================================================================================================
  					一个进程体
=================================================================================================*/
void TestC(){
	while(TRUE){
		printf("C");
		milli_delay(2000);
	}
}
/*=================================================================================================
  					panic
=================================================================================================*/
PUBLIC void panic(const char* fmt, ...){
	int i;
	char buf[256];

	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c >>panic<< %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile("ud2");
}
