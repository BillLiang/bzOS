/**************************************************************************************************
 * @file			start.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-8-21
 *************************************************************************************************/

#include "type.h"
#include "config.h"
#include "const.h"
#include "stdio.h"
#include "protect.h"
#include "fs.h"
#include "console.h"
#include "tty.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

PUBLIC void cstart(){
	
	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n--------\"cstart\" begins--------\n");

	/* 将LOADER中的GDT复制到新的GDT中 */
	memcpy(&gdt,					/* New GDT */
		(void*) (*((u32*)(&gdt_ptr[2]))),	/* Base of Old GDT */
		*((u16*)(&gdt_ptr[0])) + 1);		/* Limit of Old GDT */

	/* 把gdt_ptr中的内容换成新的GDT的基地址和界限 */
	u16* p_gdt_limit = (u16*)(&gdt_ptr[0]);		/* 0~15: Limit */
	u32* p_gdt_base = (u32*)(&gdt_ptr[2]);		/* 16~47: Base */
	*p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
	*p_gdt_base = (u32)&gdt;

	/*初始化IDT*/
	u16* p_idt_limit = (u16*)(&idt_ptr[0]);		/* 0~15: Limit */
	u32* p_idt_base = (u32*)(&idt_ptr[2]);		/* 16~47: Base */
	*p_idt_limit = IDT_SIZE * sizeof(GATE) - 1;
	*p_idt_base = (u32)&idt;

	init_prot();

	disp_str("--------\"cstart\" ends--------\n");
}
