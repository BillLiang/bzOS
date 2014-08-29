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
PUBLIC	void	delay(int times);

/* kernel.asm */
void	restart();

/* i8259.c */
PUBLIC	void	spurious_irq(int irq);
PUBLIC	void	put_irq_handler(int irq, irq_handler handler);

/* main.c */
void	TestA();
void	TestB();
void	TestC();

/* clock.c */
PUBLIC	void	clock_handler(int irq);
