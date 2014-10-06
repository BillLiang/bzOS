/**************************************************************************************************
 * @file			klib.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-8-23
 *************************************************************************************************/
#include "type.h"
#include "const.h"
#include "config.h"
#include "stdio.h"
#include "protect.h"
#include "fs.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "string.h"
#include "proto.h"
#include "global.h"

/* /usr/include/elf.h */
#include "elf.h"
/**************************************************************************************************
 *					get_boot_params
 **************************************************************************************************
 * <Ring 0~1>	The boot parameters have been saved by Loader.
 * 		We just read them out.
 *************************************************************************************************/
PUBLIC void get_boot_params(struct boot_params* pbp){
	/* Boot params should have been saved at BOOT_PARAM_ADDR */
	int* p = (int*) BOOT_PARAM_ADDR;
	assert(p[BI_MAG] == BOOT_PARAM_MAGIC);

	pbp->mem_size	= p[BI_MEM_SIZE];
	pbp->kernel_file= (u8*) p[BI_KERNEL_FILE];

	/**
	 * the kernel file should be a ELF executable, check its magic number.
	 * These definations are in /usr/include/elf.h
	 * #define ELFMAG	"\177ELF"
	 * #define SELFMAG	4
	 */
	assert(memcmp(pbp->kernel_file, ELFMAG, SELFMAG) == 0);
}
/**************************************************************************************************
 *					get_kernel_map
 **************************************************************************************************
 * <Ring 0~1> Parse the kernel file, get the memory range of the kernel image.
 *
 * @param b	Memory base of kernel.
 * @param l	Memory limit of kernel.
 *************************************************************************************************/
PUBLIC int get_kernel_map(u32* b, u32* l){
	struct boot_params bp;
	get_boot_params(&bp);

	Elf32_Ehdr* elf_header = (Elf32_Ehdr*) bp.kernel_file; 

	/* the kernel file should be in ELF format */
	if(memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0){
		return -1;
	}
	*b = ~0;
	u32 t =0;
	int i;
	for(i=0; i<elf_header->e_shnum; i++){	/* section header table entry count */
		Elf32_Shdr* section_header = 
			(Elf32_Shdr*) (bp.kernel_file + elf_header->e_shoff + i * elf_header->e_shentsize);

		if(section_header->sh_flags & SHF_ALLOC){ /* occupies memory during execution */
			int bottom	= section_header->sh_addr;
			int top		= section_header->sh_addr + section_header->sh_size;

			if(*b > bottom){
				*b = bottom;
			}
			if(t < top){
				t = top;
			}
		}
	}
	assert(*b < t);
	*l = t - *b - 1;

	return 0;
}
/**************************************************************************************************
 *					itoa
 **************************************************************************************************
 * Interger to ascii.
 * Example: 0000ABC0 --> ABC0
 *************************************************************************************************/
PUBLIC char* itoa(char* str, int num){
	int	i;
	char	ch;
	int	flag = 0;

	*str = '0';
	str ++;
	*str = 'x';
	str ++;
	if(num == 0){
		*str = '0';
		str ++;
	}else{
		for(i=28; i>=0; i-=4){
			ch = (num >> i) & 0xf;
			if(flag || ch > 0){
				flag = 1;
				ch += '0';
				if(ch > '9'){
					ch += 7;
				}
				*str = ch;
				str ++;
			}
		}
	}
	/*不要忘记了，C语言字符串结束符'\0'*/
	*str = 0;
	return str;
}
/*=================================================================================================
  					disp_int
================================================================================================ */
PUBLIC void disp_int(int input){
	char	output[16];
	itoa(output, input);
	disp_str(output);
}
/*=================================================================================================
  					delay	
================================================================================================ */
PUBLIC void delay(int times){
	int i, j, k;
	for(k=0; k<times; k++){
		for(i=0; i<100; i++){
			for(j=0; j<10000; j++){}
		}
	}
}
