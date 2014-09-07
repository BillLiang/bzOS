#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

/* irq处理例程 */
PUBLIC	irq_handler	irq_table[NR_IRQ];

/* 全局进程表 */
PUBLIC	PROCESS		proc_table[NR_TASKS + NR_PROCS];
/* 任务,运行在ring0 */
PUBLIC	TASK		task_table[NR_TASKS] = {{task_tty, STACK_SIZE_TTY, "TTY"},
						{task_sys, STACK_SIZE_SYS, "SYS"},
						{task_hd, STACK_SIZE_HD, "HD"},
						{task_fs, STACK_SIZE_FS, "FS"}};
/* 用户进程,运行在ring3 */
PUBLIC	TASK		user_proc_table[NR_PROCS] = {{TestA, STACK_SIZE_TESTA, "TestA"},
						   {TestB, STACK_SIZE_TESTB, "TestB"},
						   {TestC, STACK_SIZE_TESTC, "TestC"}};
/* 全局任务栈 */
PUBLIC	char		task_stack[STACK_SIZE_TOTAL];

/* 系统调用 */
PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {sys_printx, sys_sendrec};

/* TTY */
PUBLIC	TTY		tty_table[NR_CONSOLES];
PUBLIC	CONSOLE		console_table[NR_CONSOLES];
