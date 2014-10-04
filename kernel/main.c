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

	ticks = 0;
	k_reenter = 0;									/* 用于判断中断嵌套时中断是否重入 */
	p_proc_ready = proc_table;

	init_clock();

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
/**************************************************************************************************
 * 					TestA
 **************************************************************************************************
 * <Ring 3> A user process.
 *************************************************************************************************/
void TestA(){
	int fd;
	int i, n;
	const char filename[]	= "name";
	const char bufw[]	= "liangbizhi";
	const int rd_bytes	= 5;
	char bufr[rd_bytes];

	assert(rd_bytes <= strlen(bufw));

	/* create */
	fd = open(filename, O_CREAT | O_RDWR);
	assert(fd != -1);
	printl("File created. fd: %d\n", fd);

	/* write */
	n = write(fd, bufw, strlen(bufw));
	assert(n == strlen(bufw));

	/* close */
	close(fd);

	/* open */
	fd = open(filename, O_RDWR);
	assert(fd != -1);
	printl("File opened. fd: %d\n", fd);

	/* read */
	n = read(fd, bufr, rd_bytes);
	assert(n == rd_bytes);
	bufr[n] = 0;
	printl("%d bytes read: %s\n", n, bufr);

	/* close */
	close(fd);

	char* filenames[] = {"/foo", "/bar", "/baz"};

	/* create files */
	for(i=0; i<sizeof(filenames) / sizeof(filenames[0]); i++){
		fd = open(filenames[i], O_CREAT | O_RDWR);
		assert(fd != -1);
		printl("File created: %s (fd %d)\n", filenames[i], fd);
		close(fd);
	}

	char* rfilenames[] = {"/bar", "/foo", "/baz", "/dev_tty0"};

	/* remove files */
	for(i=0; i<sizeof(rfilenames) / sizeof(rfilenames[0]); i++){
		if(unlink(rfilenames[i]) == 0){
			printl("File removed: %s\n", rfilenames[i]);
		}else{
			printl("Failed to remove file: %s\n", rfilenames[i]);
		}
	}

	spin("TestA");
}

/**************************************************************************************************
 * 					TestB
 **************************************************************************************************
 * <Ring 3> A user process.
 *************************************************************************************************/
void TestB(){
	char tty_name[] = "/dev_tty1";

	int fd_stdin	= open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout	= open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[128];

	while(TRUE){
		printf("$ ");
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;

		if(strcmp(rdbuf, "hello") == 0){
			printf("hello world!\n");
		}else{
			if(rdbuf[0]){
				printf("{%s}\n", rdbuf);
			}
		}
	}

	assert(0);	/* never happend */
}
/**************************************************************************************************
 * 					TestC
 **************************************************************************************************
 * <Ring 3> A user process.
 *************************************************************************************************/
void TestC(){
	spin("TestC");
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
