/*=================================================================================================
		 	 tty.c			Bill Liang	2014-8-30
=================================================================================================*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "keyboard.h"

#define TTY_FIRST	(tty_table)
#define	TTY_END		(tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);

PUBLIC void task_tty(){
	TTY*		p_tty;

	init_keyboard();

	for(p_tty=TTY_FIRST; p_tty<TTY_END; p_tty++){
		init_tty(p_tty);
	}

	select_console(0);

	while(TRUE){
		for(p_tty=TTY_FIRST; p_tty<TTY_END; p_tty++){
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}
/*=================================================================================================
  					init_tty
=================================================================================================*/
PRIVATE void init_tty(TTY* p_tty){
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;
	
	init_screen(p_tty);
}
/*=================================================================================================
  					tty_do_read
=================================================================================================*/
PRIVATE void tty_do_read(TTY* p_tty){
	if(is_current_console(p_tty->p_console)){
		keyboard_read(p_tty);
	}	
}
/*=================================================================================================
 *					tty_do_write 
=================================================================================================*/
PRIVATE void tty_do_write(TTY* p_tty){
	if(p_tty->inbuf_count > 0){
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail ++;
		if(p_tty->p_inbuf_tail >= p_tty->in_buf + TTY_IN_BYTES){
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count --;

		out_char(p_tty->p_console, ch);
	}
}
/*=================================================================================================
 *					put_key 
=================================================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key){
	if(p_tty->inbuf_count < TTY_IN_BYTES){
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head ++;
		if(p_tty->p_inbuf_head >= p_tty->in_buf + TTY_IN_BYTES){
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count ++;
	}
}
/*=================================================================================================
 * 处理32位的组合键值
 * @param p_tty		对应的TTY
 * @param key		32位组合键值
=================================================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key){
	char	output[2] = {'\0', '\0'};
	/* 如果该按键是可显示字符 */
	if(!(key & FLAG_EXT)){
		/* 把当前字符放入相应的TTY */
		if(p_tty->inbuf_count < TTY_IN_BYTES){
			*(p_tty->p_inbuf_head) = key;
			p_tty->p_inbuf_head ++;
			if(p_tty->p_inbuf_head >= p_tty->in_buf + TTY_IN_BYTES){
				p_tty->p_inbuf_head = p_tty->in_buf;
			}
			p_tty->inbuf_count ++;
		}
	}else{
		int raw_code = key & MASK_RAW;
		switch(raw_code){
		case ENTER:
			put_key(p_tty, '\n');
			break;
		case BACKSPACE:
			put_key(p_tty, '\b');
			break;
		case UP:
			if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)){
				scroll_screen(p_tty->p_console, SCR_UP);
			}
			break;
		case DOWN:
			if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)){
				scroll_screen(p_tty->p_console, SCR_DN);
			}
			break;
		case F1:
		case F2:
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			/* ctrl + f1~f12 用于切换控制台 */
			if(key & FLAG_CTRL_L || key & FLAG_CTRL_R){
				select_console(raw_code - F1);
			}
			break;
		default:
			break;
		}
	}
}
