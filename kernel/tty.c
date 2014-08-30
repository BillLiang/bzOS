/*=================================================================================================
		 	 tty.c			Bill Liang	2014-8-30
=================================================================================================*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "keyboard.h"

PUBLIC void task_tty(){
	while(TRUE){
		keyboard_read();
	}
}

/*=================================================================================================
  				处理32位组合信息按键
=================================================================================================*/
PUBLIC void in_process(u32 key){
	char	output[2] = {'\0', '\0'};
	/* 如果该按键是可显示字符 */
	if(!(key & FLAG_EXT)){
		output[0] = key & 0xff;
		disp_str(output);
	}
}
