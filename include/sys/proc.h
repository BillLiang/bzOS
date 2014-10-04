/*=================================================================================================
  				proc.h		Bill Liang	2014-8-26
=================================================================================================*/
typedef struct s_stackframe{
	u32	gs;		/*					*/
	u32	fs;		/*					*/
	u32	es;		/*					*/
	u32	ds;		/*					*/
	u32	edi;		/*					*/
	u32	esi;		/* they are all pushed by kernel::save	*/
	u32	ebp;		/*					*/
	u32	kernel_esp;	/* popad will ignore this		*/
	u32	ebx;		/*					*/
	u32	edx;		/*					*/
	u32	ecx;		/*					*/
	u32	eax;		/*					*/
	u32	retaddr;	/***the return address of kernel::save***/
	u32	eip;		/*					*/
	u32	cs;		/*					*/
	u32	eflags;		/* this part will be pushed by CPU into	*/
	u32	esp;		/* stack during interruption		*/
	u32	ss;		/*					*/
}STACK_FRAME;

typedef struct s_proc{
	STACK_FRAME	regs;			/* process registers saved in stack frame */

	u16		ldt_sel;		/* gdt selector giving ldt base and limit */
	DESCRIPTOR	ldts[LDT_SIZE];		/* local descriptors for code and data, a part of the process */

	int		ticks;			/* remained ticks */
	int		priority;

	u32		pid;
	char		name[32];		/* name of the process */

	int		flags;			/* process flags. A proc is runnable if flags == 0 
						 * flags == SENDING, the proc is sending a message.
						 * flags == RECEIVING, the proc is receiving a message.
						 */
	MESSAGE*	p_msg;			/* point to message */
	int		recv_from;		/* pid: from whom this proc wants to receive the message */
	int		send_to;		/* pid: to whome this proc wants to send the meassage */

	int		has_int_msg;		/* nonzero if an INTERRUPT occurred when the task is not ready to deal with it */

	struct s_proc*	q_sending;		/* queue of procs sending messages to this proc */
	struct s_proc*	next_sending;		/*
						 * next proc in the sending queue
						 * e.g. A,B,C are sending message to this proc, then this.q_sending points to
						 * A, A.next_sending points to B, B.next_sending points to C.
						 */

	struct file_desc* filp[NR_FILES];	/* A set of pointers to file_desc_table */
}PROCESS;

typedef struct s_task{				/* task, for process initialization */
	task_f	initial_eip;
	int	stacksize;
	char	name[32];
}TASK;

#define	proc2pid(x)	(x - proc_table)

/* 任务&进程的数量 */
#define	NR_TASKS	4
#define	NR_PROCS	3

/* 任务的栈 */
#define	STACK_SIZE_TESTA	0x8000
#define	STACK_SIZE_TESTB	0x8000
#define	STACK_SIZE_TESTC	0x8000
#define	STACK_SIZE_TTY		0x8000
#define	STACK_SIZE_SYS		0x8000
#define	STACK_SIZE_HD		0x8000
#define	STACK_SIZE_FS		0x8000

#define	STACK_SIZE_TOTAL	STACK_SIZE_TESTA + \
				STACK_SIZE_TESTB + \
				STACK_SIZE_TESTC + \
				STACK_SIZE_TTY + \
				STACK_SIZE_SYS + \
				STACK_SIZE_HD + \
				STACK_SIZE_FS
