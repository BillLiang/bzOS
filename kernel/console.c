/*=================================================================================================
		 	 console.c			Bill Liang	2014-8-31
=================================================================================================*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proto.h"
#include "proc.h"
#include "global.h"
#include "console.h"
#include "tty.h"

PRIVATE void set_cursor(u32 position);
/**================================================================================================
 * 判断控制台是否是当前控制台。
 * @param p_con 控制台
 ================================================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con){
	return (p_con == &console_table[nr_current_console]);
}
/**================================================================================================
 * 向当前控制台输出一个字符。
 * @param p_con 控制台
 * @param ch	字符
 ================================================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch){
	/* 字符地址，指针类型为char */
	u8* p_vmem = (u8*) (V_MEM_BASE + p_con->cursor * 2);	
	
	*p_vmem ++ = ch;
	*p_vmem ++ = DEFAULT_CHAR_COLOR;
	p_con->cursor ++;

	set_cursor(p_con->cursor);
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
 * @param p_tty
 ================================================================================================*/
PUBLIC void init_screen(TTY* p_tty){
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;					/* 转化成双字节 */

	int con_v_mem_size			= v_mem_size / NR_CONSOLES;	/* 平均分配显存 */
	p_tty->p_console->original_addr		= con_v_mem_size * nr_tty;
	p_tty->p_console->current_start_addr	= p_tty->p_console->original_addr;
	p_tty->p_console->v_mem_limit		= con_v_mem_size;

	p_tty->p_console->cursor		= p_tty->p_console->original_addr;

	if(nr_tty == 0){
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}else{
		/* 打印控制台标号 */
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
		out_char(p_tty->p_console, ':');
	}
	set_cursor(p_tty->p_console->cursor);
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
	
	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}
/**================================================================================================
 * 滚动屏幕。
 * @param p_con
 * @param direction
 ================================================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction){
	if(direction == SCR_UP){
		if(p_con->current_start_addr > p_con->original_addr){
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}else if(direction == SCR_DN){
		if(p_con->current_start_addr + SCREEN_SIZE < p_con->original_addr + p_con->v_mem_limit){
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}else{
		return;
	}
	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}
