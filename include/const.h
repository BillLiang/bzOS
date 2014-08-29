#ifndef _BZOS_CONST_H_
#define	_BZOS_CONST_H_

//函数类型
#define PUBLIC
#define PRIVATE	static

/*除了在global.c中，EXTERN被定义为extern*/
#define	EXTERN	extern

//GDT和IDT中描述符的个数
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
#define	KERBOARD_IRQ		1
#define	CASCADE_IRQ		2
#define	ETHER_IRQ		3
#define	SECONDARY_IRQ		3
#define	RS232_IRQ		4
#define	XT_WINT_IRQ		5
#define	FLOPPY_IRQ		6
#define	PRINTER_IRQ		7
#define	AT_WINT_IRQ		14

#endif
