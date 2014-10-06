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

/**************************************************************************************************
 * 					kernel_main
 **************************************************************************************************
 * jump from kernel.asm::_start.
 *************************************************************************************************/
PUBLIC int kernel_main(){
	disp_str("--------\"kernel_main\" begins--------\n");

	PROCESS*	p_proc		= proc_table;
	TASK*		p_task;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	
	int 		i, j, eflags, prio;

	u8		privilege;
	u8		rpl;
	/* initialize the proc table */
	for(i=0; i<NR_TASKS + NR_PROCS; i++, p_proc++, p_task++){
		if(i >= NR_TASKS + NR_NATIVE_PROCS){
			p_proc->flags = FREE_SLOT;
			continue;

		}
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
		p_proc->parent	= NO_TASK;

		/* base, limit & attribute */
		if(strcmp(p_task->name, "INIT") != 0){
			p_proc->ldts[INDEX_LDT_C]	= gdt[SELECTOR_KERNEL_CS >> 3];
			p_proc->ldts[INDEX_LDT_RW]	= gdt[SELECTOR_KERNEL_DS >> 3];
			/* change the DPLs */
			p_proc->ldts[INDEX_LDT_C].attr1	= DA_C | privilege << 5;
			p_proc->ldts[INDEX_LDT_RW].attr1= DA_DRW | privilege << 5;
		}else{	/* it is INIT process! */
			u32 k_base;
			u32 k_limit;
			int ret = get_kernel_map(&k_base, &k_limit);
			assert(ret == 0);

			init_descriptor(&p_proc->ldts[INDEX_LDT_C],
					0, /* bytes before the entry point are useless for
					    * the INIT process, doesn't matter.
					    */
					(k_base + k_limit) >> LIMIT_4K_SHIFT,
					DA_32 | DA_LIMIT_4K | DA_C | privilege << 5);
			init_descriptor(&p_proc->ldts[INDEX_LDT_RW],
					0, /* bytes before the entry point are useless for
					    * the INIT process, doesn't matter.
					    */
					(k_base + k_limit) >> LIMIT_4K_SHIFT,
					DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);
		}

		p_proc->regs.cs = INDEX_LDT_C << 3 | SA_TIL | rpl;		/* cs为指向LDT第一个描述符的选择子 */
		p_proc->regs.ds =
			p_proc->regs.es =
			p_proc->regs.fs =
			p_proc->regs.ss = INDEX_LDT_RW << 3 | SA_TIL | rpl;	/* ds, es, fs, ss为指向LDT第二个描述符的选择子 */
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;	/* gs仍然指向显存，只是改变了DPL让其在低特权级下运行 */
	
		p_proc->regs.eip = (u32) p_task->initial_eip;	/* eip为指向进程体 */
		p_proc->regs.esp = (u32) p_task_stack;		/* esp指向新的栈底 */
		p_proc->regs.eflags = eflags;

		p_proc->flags		= 0;			/* this proc is runnable */
		p_proc->p_msg		= 0;			/* null message */
		p_proc->recv_from	= NO_TASK;
		p_proc->send_to		= NO_TASK;
		p_proc->has_int_msg	= 0;
		p_proc->q_sending	= 0;
		p_proc->next_sending	= 0;

		p_proc->ticks = p_proc->priority = prio;
	
		p_task_stack -= p_task->stacksize;

		for(j=0; j<NR_FILES; j++){
			p_proc->filp[j] = 0;
		}
	}

	ticks = 0;
	k_reenter = 0;			/* 用于判断中断嵌套时中断是否重入 */
	p_proc_ready = proc_table;

	init_clock();

	restart();

	while(TRUE){}
}
/**************************************************************************************************
 * 					get_ticks
 *************************************************************************************************/
PUBLIC int get_ticks(){
	MESSAGE	msg;

	reset_msg(&msg);
	msg.type	= GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}
/**************************************************************************************************
 * 					Init
 **************************************************************************************************
 * The hen.
 *************************************************************************************************/
void Init(){
	int fd_stdin = open("/dev_tty0", O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR);
	assert(fd_stdout == 1);

	printf("Init() is running...\n");

	int pid = fork();
	if(pid != 0){
		printf("parent is running, child pid: %d\n", pid);
		spin("parent");
	}else{
		printf("child is running, pid: %d\n", getpid());
		spin("child");
	}
}
/**************************************************************************************************
 * 					TestA
 **************************************************************************************************
 * <Ring 3> A user process.
 *************************************************************************************************/
void TestA(){
	while(TRUE){}
}

/**************************************************************************************************
 * 					TestB
 **************************************************************************************************
 * <Ring 3> A user process.
 *************************************************************************************************/
void TestB(){
	while(TRUE){}
}
/**************************************************************************************************
 * 					TestC
 **************************************************************************************************
 * <Ring 3> A user process.
 *************************************************************************************************/
void TestC(){
	while(TRUE){}
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
