/*=================================================================================================
		 	 tty.c			Bill Liang	2014-8-30
=================================================================================================*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "string.h"
#include "proto.h"
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
/*=================================================================================================
 *					tty_write 
=================================================================================================*/
PUBLIC void tty_write(TTY* p_tty, char* buf, int len){
	char*	p = buf;
	int	i = len;
	while(i){
		out_char(p_tty->p_console, *p++);
		i --;
	}
}
/*=================================================================================================
 *					sys_printx 
=================================================================================================*/
PUBLIC int sys_printx(int _unused1, int _unused2, char* s, PROCESS* p_proc){
	const char* p;
	char ch;

	char reenter_err[]	= "? k_reenter is incorrect for unknown reason";
	reenter_err[0]		= MAG_CH_PANIC;

	if(k_reenter == 0){		/* printx() called in ring 1~3 */
		p = va2la(proc2pid(p_proc), s);
	}else if(k_reenter > 0){	/* printx() called in ring 0, no linear-physical address mapping is needed */
		p = s;
	}else{				/* should not happen */
		p = reenter_err;
	}
	/**
	 * @note if assertion fails in any TASKS, the system will be halted;
	 * if it fails in a USER PROC, it'll return like any normal syscall does;
	 */
	if((*p == MAG_CH_PANIC) || (*p == MAG_CH_ASSERT && p_proc_ready < &proc_table[NR_TASKS])){
		disable_int();
		char* v		= (char*) V_MEM_BASE;
		const char* q	= p + 1;			/* skip the magic char */
		while(v < (char*) (V_MEM_BASE + V_MEM_SIZE)){	/* print the message everywhere */
			*v ++ = *q++;
			*v ++ = RED_CHAR;
			if(!*q){
				while(((int)v - V_MEM_BASE) % (SCREEN_WIDTH * 16)){
					v ++;
					*v ++ = GRAY_CHAR;	/* 这是要“冻结”屏幕的节奏么 */
				}
				q = p + 1;			/* skip the magic char */
			}
		}
		/* system will be halted */
		__asm__ __volatile__("hlt");
	}

	while((ch = *p ++) != 0){
		if(ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT){
			continue;
		}
		out_char(tty_table[p_proc->nr_tty].p_console, ch);
	}

	return 0;
}
