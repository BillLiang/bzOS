#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "stdio.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "fs.h"
#include "global.h"
#include "proto.h"

/* irq handlers */
PUBLIC	irq_handler	irq_table[NR_IRQ];

/* Global process table */
PUBLIC	PROCESS		proc_table[NR_TASKS + NR_PROCS];

/* System tasks, running in ring1 */
PUBLIC	TASK	task_table[NR_TASKS] = {
	{task_tty,	STACK_SIZE_TTY,	"TTY"	},
	{task_sys,	STACK_SIZE_SYS,	"SYS"	},
	{task_hd,	STACK_SIZE_HD,	"HD"	},
	{task_fs,	STACK_SIZE_FS,	"FS"	},
	{task_mm,	STACK_SIZE_MM,	"MM"	}};

/* User processes, runnig in ring3 */
PUBLIC	TASK	user_proc_table[NR_NATIVE_PROCS] = {
	{Init,	STACK_SIZE_INIT,	"INIT"	},
	{TestA,	STACK_SIZE_TESTA,	"TestA"	},
	{TestB,	STACK_SIZE_TESTB,	"TestB"	},
	{TestC,	STACK_SIZE_TESTC,	"TestC"	}};

/* 全局任务栈 */
PUBLIC	char		task_stack[STACK_SIZE_TOTAL];

/* 系统调用 */
PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {sys_printx, sys_sendrec};

/* TTY */
PUBLIC	TTY		tty_table[NR_CONSOLES];
PUBLIC	CONSOLE		console_table[NR_CONSOLES];

/* File System */
/**************************************************************************************************
 * For dd_map[k].
 * 'k' is the device nr.
 * 'dd_map[k].driver_nr' is the driver nr.
 *
 * Remember to modify 'const.h' if the order is changed.
 *************************************************************************************************/
struct dev_drv_map dd_map[] = {
	{INVALID_DRIVER},	/* 0: Unused */
	{INVALID_DRIVER},	/* 1: Reserved for floppy driver */
	{INVALID_DRIVER},	/* 2: Reserved for cdrom driver */
	{TASK_HD},		/* 3: Hard disk */
	{TASK_TTY},		/* 4: TTY */
	{INVALID_DRIVER}	/* 5: Reserved for scsi disk driver */
};

/**************************************************************************************************
 * 6MB~7MB: buffer for FS
 *************************************************************************************************/
PUBLIC u8*		fsbuf		= (u8*)0x600000;
PUBLIC const int	FSBUF_SIZE	= 0x100000;
/**************************************************************************************************
 * 7MB~8MB: buffer for MM
 *************************************************************************************************/
PUBLIC u8*		mmbuf		= (u8*)0x700000;
PUBLIC const int	MMBUF_SIZE	= 0x100000;
