/*=================================================================================================
  			proto.h			Bill Liang	2014-8-22
=================================================================================================*/
/* kliba.asm */
PUBLIC	void	out_byte(u16 port, u8 value);
PUBLIC	u8	in_byte(u16 port);
PUBLIC	void	disp_str(char* info);
PUBLIC	void	disp_color_str(char* info, int color);
PUBLIC	void	disable_irq(int irq);
PUBLIC	void	enable_irq(int irq);

/* protect.c */
PUBLIC	void	init_prot();
PUBLIC	u32	seg2phys(u16 seg);
PUBLIC	void	init_8259A();

/* klib.c */
PUBLIC	char*	itoa(char* str, int num);
PUBLIC	void	delay(int times);

/* kernel.asm */
void	restart();
PUBLIC	void	sys_call();

/* i8259.c */
PUBLIC	void	spurious_irq(int irq);
PUBLIC	void	put_irq_handler(int irq, irq_handler handler);

/* main.c */
PUBLIC	int	get_ticks();
void	TestA();
void	TestB();
void	TestC();
PUBLIC	void	panic(const char* fmt, ...);

/* clock.c */
PUBLIC	void	clock_handler(int irq);
PUBLIC	void	init_clock();
PUBLIC	void	milli_delay(int milli_sec);

/* syscall.asm */
PUBLIC	int	sendrec(int function, int src_dest, MESSAGE* msg);
PUBLIC	void	printx(char* s);

/* systask.c */
PUBLIC	void	task_sys();

/* proc.c */
PUBLIC	void	reset_msg(MESSAGE* msg);
PUBLIC	int	sys_sendrec(int function, int src_dest, MESSAGE* msg, PROCESS* proc);
PUBLIC	void	schedule();

/* keyboard.c */
PUBLIC	void	init_keyboard();
PUBLIC	void	keyboard_read(TTY* p_tty);

/* tty.c */
PUBLIC	void	task_tty();
PUBLIC	void	in_process(TTY* p_tty, u32 key);
PUBLIC	int	sys_printx(int _unused1, int _unused2, char* s, PROCESS* p_proc);

/* console.c */
PUBLIC	int	is_current_console(CONSOLE* p_con);
PUBLIC	void	out_char(CONSOLE* p_con, char ch);
PUBLIC	void	init_screen(TTY* p_tty);
PUBLIC	void	scroll_screen(CONSOLE* p_con, int direction);

/* printf.c */
PUBLIC	int	printf(const char* fmt, ...);

/* lib/misc.c */
PUBLIC	void	spin(char* func_name);
#define	printl	printf
