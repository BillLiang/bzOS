#ifndef _BZOS_PROTECT_H_
#define _BZOS_PROTECT_H_

//存储段 / 系统描述符（共8个字节）
typedef struct s_descriptor{
	u16	limit_low;
	u16	base_low;
	u8	base_mid;
	u8	attr1;
	u8	limit_high_attr2;
	u8	base_high;
}DESCRIPTOR;

/*门描述符*/
typedef struct s_gate{
	u16	offset_low;
	u16	selector;
	u8	dcount;
	u8	attr;
	u16	offset_high;
}GATE;

typedef struct s_tss{
	u32	backlink;
	u32	esp0;
	u32	ss0;
	u32	esp1;
	u32	ss1;
	u32	esp2;
	u32	ss2;
	u32	cr3;
	u32	eip;
	u32	flags;
	u32	eax;
	u32	ecx;
	u32	edx;
	u32	ebx;
	u32	esp;
	u32	ebp;
	u32	esi;
	u32	edi;
	u32	es;
	u32	cs;		
	u32	ss;		
	u32	ds;		
	u32	fs;		
	u32	gs;		
	u32	ldt;
	u16	trap;
	u16	iobase;			/* I/O位图基址 >= TSS段界限，就表示没有I/O许可位图 */
}TSS;

/* GDT 中描述符的索引 */
#define	INDEX_DUMMY		0
#define	INDEX_FLAT_C		1
#define	INDEX_FLAT_RW		2
#define	INDEX_VIDEO		3
#define	INDEX_TSS		4
#define	INDEX_LDT_FIRST		5


/* 选择子，这些值都是在loader.asm定义好了的（参考上面的GDT索引），记住一个描述符的大小为8字节 */
#define	SELECTOR_DUMMY			0
#define	SELECTOR_FLAT_C			0x08
#define	SELECTOR_FLAT_RW		0x10
#define	SELECTOR_VIDEO			(0x18 + 3)

#define	SELECTOR_TSS			0x20		/* TSS 的描述符 */
#define	SELECTOR_LDT_FIRST		0x28		/* 第一个LDT */

#define	SELECTOR_KERNEL_CS		SELECTOR_FLAT_C
#define	SELECTOR_KERNEL_DS		SELECTOR_FLAT_RW
#define	SELECTOR_KERNEL_GS		SELECTOR_VIDEO

/* SA，选择子属性 */
#define	SA_RPL_MASK			0xfffc		/* 用于屏蔽选择子的RPL */
#define	SA_RPL0				0
#define	SA_RPL1				1
#define	SA_RPL2				2
#define	SA_RPL3				3

#define	SA_TI_MASK			0xfffb		/* 用于屏蔽TI位 */
#define	SA_TIG				0		/* 该选择子用于GDT */
#define	SA_TIL				4		/* 该选择在用于LDT */



/* 每个任务都有一个单独的LDT，每一个LDT中的描述符个数 */
#define	LDT_SIZE	2


/* 描述符类型说明 */
#define	DA_32				0x4000	/* 32位段 */
#define	DA_LIMIT_4K			0x8000	/* 段界限粒度为4k字节 */
#define	DA_DPL0				0x00	/* DPL=0 */
#define	DA_DPL1				0x20	/* DPL=1 */
#define	DA_DPL2				0x40	/* DPL=2 */
#define	DA_DPL3				0x60	/* DPL=3 */
/* 存储段 描述符类型说明 */
#define	DA_DR				0x90
#define	DA_DRW				0x92
#define	DA_DRWA				0x93
#define	DA_C				0x98
#define	DA_CR				0x9a
#define	DA_CCO				0x9c
#define	DA_CCOR				0x9e


/* 系统段 描述符类型说明 */
#define	DA_LDT				0x82	/* LDT表段类型 */
#define	DA_TaskGate			0x85	/* 任务门类型 */
#define	DA_386TSS			0x89	/* 386 任务状态段 */
#define	DA_386CGate			0x8c	/* 386 调用门 */
#define	DA_386IGate			0x8e	/* 386 中断门 */
#define	DA_386TGate			0x8f	/* 386 陷阱门 */

/*中断向量*/
#define	INT_VECTOR_DIVIDE		0x0
#define	INT_VECTOR_DEBUG		0x1
#define	INT_VECTOR_NMI			0x2
#define	INT_VECTOR_BREAKPOINT		0x3
#define	INT_VECTOR_OVERFLOW		0x4
#define	INT_VECTOR_BOUNDS		0x5
#define	INT_VECTOR_INVAL_OP		0x6
#define	INT_VECTOR_COPROC_NOT		0x7
#define	INT_VECTOR_DOUBLE_FAULT		0x8
#define	INT_VECTOR_COPROC_SEG		0x9
#define	INT_VECTOR_INVAL_TSS		0xA
#define	INT_VECTOR_SEG_NOT		0xB
#define	INT_VECTOR_STACK_FAULT		0xC
#define	INT_VECTOR_PROTECTION		0xD
#define	INT_VECTOR_PAGE_FAULT		0xE
#define	INT_VECTOR_COPROC_ERR		0x10

/*中断向量*/
#define	INT_VECTOR_IRQ0			0x20
#define	INT_VECTOR_IRQ8			0x28
/* 系统调用 */
#define	INT_VECTOR_SYS_CALL		0x90

/* 宏 */
/* 线性地址 --> 物理地址 */
#define	vir2phys(seg_base, vir)	(u32)((u32) seg_base + (u32) vir)
#endif
