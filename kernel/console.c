/**************************************************************************************************
 * @file			console.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-8-31
 *************************************************************************************************/

#include "type.h"
#include "const.h"
#include "stdio.h"
#include "protect.h"
#include "fs.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "proto.h"
#include "global.h"
#include "console.h"
#include "tty.h"

PRIVATE void set_cursor(u32 position);
PRIVATE void flush(CONSOLE* console);
/**================================================================================================
 * 判断控制台是否是当前控制台。
 * @param console 控制台
 ================================================================================================*/
PUBLIC int is_current_console(CONSOLE* console){
	return (console == &console_table[nr_current_console]);
}
/**================================================================================================
 * 向当前控制台输出一个字符。
 * @param console 控制台
 * @param ch	字符
 ================================================================================================*/
PUBLIC void out_char(CONSOLE* console, char ch){
	/* 字符地址，指针类型为char */
	u8* p_vmem = (u8*) (V_MEM_BASE + console->cursor * 2);
	
	switch(ch){
	case '\n':
		/* 如果不是最后一行 */
		if(console->cursor < console->original_addr + console->v_mem_limit - SCREEN_WIDTH * 2){
			console->cursor = console->original_addr + 
					SCREEN_WIDTH * ((console->cursor - console->original_addr) / SCREEN_WIDTH + 1);
		}
		break;
	case '\b':
		if(console->cursor > console->original_addr){
			console->cursor --;
			*(p_vmem - 2) = ' ';
			*(p_vmem - 1) = DEFAULT_CHAR_COLOR;
		}
		break;
	default:
		/* 防止字符越界（不是很准确） */
		if(console->cursor < console->original_addr + console->v_mem_limit - SCREEN_WIDTH - 1){
			*p_vmem ++ = ch;
			*p_vmem ++ = DEFAULT_CHAR_COLOR;
			console->cursor ++;
		}
		break;
	}
	/* 字符超出屏幕则滚屏 */
	if(console->cursor >= console->current_start_addr + SCREEN_SIZE){
		scroll_screen(console, SCR_DN);
	}
	/* 刷新 */
	flush(console);
}
/**================================================================================================
 * 设置当前光标位置。
 * @param position
 ================================================================================================*/
PRIVATE void set_cursor(u32 position){
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xff);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xff);
	enable_int();
}
/**================================================================================================
 * 为TTY初始化窗口。
 * @param tty
 ================================================================================================*/
PUBLIC void init_screen(TTY* tty){
	int nr_tty = tty - tty_table;
	tty->console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;					/* 转化成双字节 */

	int con_v_mem_size			= v_mem_size / NR_CONSOLES;	/* 平均分配显存 */
	tty->console->original_addr		= con_v_mem_size * nr_tty;
	tty->console->current_start_addr	= tty->console->original_addr;
	tty->console->v_mem_limit		= con_v_mem_size;

	tty->console->cursor		= tty->console->original_addr;

	if(nr_tty == 0){
		tty->console->cursor = disp_pos / 2;
		disp_pos = 0;
	}else{
		/* 打印控制台标号 */
		const char prompt[] = "[tty?]\n";
		const char* p = prompt;
		
		for(; *p; p++){
			out_char(tty->console, *p == '?' ? nr_tty + '0' : *p);
		}
	}
	set_cursor(tty->console->cursor);
}
/**================================================================================================
 * 切换控制台。
 * @param addr
 ================================================================================================*/
PRIVATE void set_video_start_addr(u32 addr){
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xff);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xff);
	enable_int();
}
/**================================================================================================
 * 切换控制台。
 * @param nr_console
 ================================================================================================*/
PUBLIC void select_console(int nr_console){
	if(nr_console < 0 || nr_console >= NR_CONSOLES){
		return;
	}
	nr_current_console = nr_console;
	
	flush(&console_table[nr_console]);
}
/**================================================================================================
 * 滚动屏幕。
 * @param console
 * @param direction
 ================================================================================================*/
PUBLIC void scroll_screen(CONSOLE* console, int direction){
	if(direction == SCR_UP){
		if(console->current_start_addr > console->original_addr){
			console->current_start_addr -= SCREEN_WIDTH;
		}
	}else if(direction == SCR_DN){
		if(console->current_start_addr + SCREEN_SIZE + SCREEN_WIDTH < console->original_addr + console->v_mem_limit){
			console->current_start_addr += SCREEN_WIDTH;
		}
	}else{
		return;
	}
	flush(console);
}
/**================================================================================================
 * 刷新屏幕。
 * @param console
 ================================================================================================*/
PRIVATE void flush(CONSOLE* console){
	if(is_current_console(console)){
		set_cursor(console->cursor);
		set_video_start_addr(console->current_start_addr);
	}
}
