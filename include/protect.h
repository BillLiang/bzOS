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

/*选择子*/
#define	SELECTOR_DUMMY			0
#define	SELECTOR_FLAT_C			0x08
#define	SELECTOR_FLAT_RW		0x10
#define	SELECTOR_VIDEO			(0x18 + 3)

#define	SELECTOR_KERNEL_CS		SELECTOR_FLAT_C
#define	SELECTOR_KERNEL_DS		SELECTOR_FLAT_RW

/*系统段描述符类型说明*/
#define	DA_386IGate			0x8e	/* 386 中断门类型*/

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
#endif
