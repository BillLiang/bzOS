/*=================================================================================================
  			proto.h			Bill Liang	2014-8-22
=================================================================================================*/
/* kliba.asm */
PUBLIC	void	out_byte(u16 port, u8 value);
PUBLIC	u8	in_byte(u16 port);
PUBLIC	void	port_read(u16 port, void* buf, int n);
PUBLIC	void	port_write(u16 port, void* buf, int n);
PUBLIC	void	disp_str(char* info);
PUBLIC	void	disp_color_str(char* info, int color);
PUBLIC	void	disable_irq(int irq);
PUBLIC	void	enable_irq(int irq);

/* protect.c */
PUBLIC	void	init_prot();
PUBLIC	u32	seg2linear(u16 seg);
PUBLIC	void	init_8259A();
PUBLIC	void	init_descriptor(DESCRIPTOR* p_desc, u32 base, u32 limit, u16 attribute);


/* klib.c */
PUBLIC	void	get_boot_params(struct boot_params* pbp);
PUBLIC	int	get_kernel_map(u32* b, u32* l);
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
void	Init();
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
PUBLIC	void	inform_int(int task_nr);
PUBLIC	void	dump_msg(const char* title, MESSAGE* m);
PUBLIC	void	dump_proc(PROCESS* p);

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

/* hd.c */
PUBLIC	void	task_hd();
PUBLIC	void	hd_handler(int irq);

/* fs/main.c */
PUBLIC	void	task_fs();
PUBLIC	int	rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_nr, void* buf);

PUBLIC	struct super_block* get_super_block(int dev);
PUBLIC	struct inode* get_inode(int dev, int num);
PUBLIC	void put_inode(struct inode* pinode);
PUBLIC	void sync_inode(struct inode* p);

/* fs/open.c */
PUBLIC	int	do_open();
PUBLIC	int	do_close();
/* fs/read_write.c */
PUBLIC	int	do_rdwt();
/* fs/link.c */
PUBLIC	int	do_unlink();

/* fs/misc.c */
PUBLIC	int	strip_path(char* filename, const char* pathname, struct inode** ppinode);
PUBLIC	int	search_file(char* path);

/* mm/main.c */
PUBLIC	void	task_mm();
PUBLIC	int	alloc_mem(int pid, int memsize);

/* mm/forkexit.c */
PUBLIC	int	do_fork();

/* lib/misc.c */
PUBLIC	void	spin(char* func_name);
