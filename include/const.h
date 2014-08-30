#ifndef _BZOS_CONST_H_
#define	_BZOS_CONST_H_

//函数类型
#define PUBLIC
#define PRIVATE	static

/*除了在global.c中，EXTERN被定义为extern*/
#define	EXTERN	extern

/* Boolean */
#define	TRUE		1
#define	FALSE		0

/* 颜色 */
#define	BLACK		0x0
#define	WHITE		0x7
#define	RED		0x4
#define	GREEN		0x2
#define	BLUE		0x1
#define	FLASH		0x80
#define	BRIGHT		0x8
#define	MAKE_COLOR(X,Y)	(X | Y)


/* GDT和IDT中描述符的个数 */
#define	GDT_SIZE	128
#define	IDT_SIZE	256

/* 权限 */
#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3
/* RPL */
#define	RPL_KRNL	SA_RPL0
#define	RPL_TASK	SA_RPL1
#define	RPL_USER	SA_RPL3

//8259A中断处理器的端口。
#define INT_M_CTL		0x20			//中断命令寄存器I/O地址（主片）
#define	INT_M_CTLMASK		0x21			//中断屏蔽寄存器I/O地址（主片）
#define	INT_S_CTL		0xa0
#define	INT_S_CTLMASK		0xa1

#define	NR_IRQ			16
#define CLOCK_IRQ		0
#define	KEYBOARD_IRQ		1
#define	CASCADE_IRQ		2
#define	ETHER_IRQ		3
#define	SECONDARY_IRQ		3
#define	RS232_IRQ		4
#define	XT_WINT_IRQ		5
#define	FLOPPY_IRQ		6
#define	PRINTER_IRQ		7
#define	AT_WINT_IRQ		14

/* 系统调用 */
#define	NR_SYS_CALL		1

/* 8253/8254 PIT (Programmable Interval Timer)可编程间隔计时器 */
#define	TIMER0			0x40			/* I/O端口计时器 Counter 0 */
#define TIMER_MODE		0x43			/* 8253模式控制寄存器I/O地址 */
#define	RATE_GENERATOR		0x34			/* 输入到 Counter 0 的数据，00-11-010-0 */
#define	TIMER_FREQ		1193182L		/* 计时器时钟频率 */
#define	HZ			100			

/* 键盘 */
/* 8042键盘控制器端口 */
#define KB_DATA			0x60			/* 数据缓冲区，读/写 */
#define	KB_CMD			0x64			/* 读：状态	写：发送命令 */

#endif
