/*=================================================================================================
  				proc.h		Bill Liang	2014-8-26
=================================================================================================*/
typedef struct s_stackframe{
	u32	gs;		/*					*/
	u32	fs;		/*					*/
	u32	es;		/*					*/
	u32	ds;		/*					*/
	u32	edi;		/*					*/
	u32	esi;		/*它们都是save()压入			*/
	u32	ebp;		/*					*/
	u32	kernel_esp;	/*<-'popad'会无视这个			*/
	u32	ebx;		/*					*/
	u32	edx;		/*					*/
	u32	ecx;		/*					*/
	u32	eax;		/*					*/
	u32	retaddr;	// kernel.asm : save()的返回地址。
	u32	eip;		/*					*/
	u32	cs;		/*					*/
	u32	eflags;		/*这部分在中断期间被CPU压入栈		*/
	u32	esp;		/*					*/
	u32	ss;		/*					*/
}STACK_FRAME;

typedef struct s_proc{
	STACK_FRAME	regs;			/*  */

	u16		ldt_sel;		/* gdt selector giving ldt base and limit */
	DESCRIPTOR	ldts[LDT_SIZE];		/* 本地描述符表，进程的一部分 */

	int		ticks;			/* 进程运行的剩余ticks */
	int		priority;

	u32		pid;
	char		p_name[32];

	int		nr_tty;			/* 为进程指定TTY */
}PROCESS;

typedef struct s_task{				/* 任务，初始化进程用 */
	task_f	initial_eip;
	int	stacksize;
	char	name[32];
}TASK;

#define	proc2pid(x)	(x - proc_table)

/* 任务&进程的数量 */
#define	NR_TASKS	1
#define	NR_PROCS	3

/* 任务的栈 */
#define	STACK_SIZE_TESTA	0x8000
#define	STACK_SIZE_TESTB	0x8000
#define	STACK_SIZE_TESTC	0x8000
#define	STACK_SIZE_TTY		0x8000

#define	STACK_SIZE_TOTAL	STACK_SIZE_TESTA + \
				STACK_SIZE_TESTB + \
				STACK_SIZE_TESTC + \
				STACK_SIZE_TTY
