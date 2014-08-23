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

/*权限*/
#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3

//8259A中断处理器的端口。
#define INT_M_CTL		0x20			//中断命令寄存器I/O地址（主片）
#define	INT_M_CTLMASK		0x21			//中断屏蔽寄存器I/O地址（主片）
#define	INT_S_CTL		0xa0
#define	INT_S_CTLMASK		0xa1

#endif
