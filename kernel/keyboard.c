/**************************************************************************************************
 * @file			keyboard.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-8-30
 *************************************************************************************************/

#include "type.h"
#include "const.h"
#include "stdio.h"
#include "protect.h"
#include "fs.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "string.h"
#include "proto.h"
#include "global.h"
#include "keyboard.h"
#include "keymap.h"

PRIVATE KB_INPUT	kb_in;

PRIVATE int		code_with_E0;
PRIVATE int		shift_l;
PRIVATE int		shift_r;
PRIVATE int		alt_l;
PRIVATE int		alt_r;
PRIVATE int		ctrl_l;
PRIVATE int		ctrl_r;
PRIVATE int		caps_lock;
PRIVATE int		num_lock;
PRIVATE int		scroll_lock;

PRIVATE int		column;

PRIVATE u8		get_byte_from_kbuf();
PRIVATE void		set_leds();

/*=================================================================================================
  					键盘中断处理例程
=================================================================================================*/
PUBLIC void keyboard_handler(int irq){
	u8 scan_code = in_byte(KB_DATA);
	/* 把数据放入缓冲区 */
	if(kb_in.count < KB_IN_BYTES){
		*(kb_in.p_head) = scan_code;
		kb_in.p_head ++;
		if(kb_in.p_head >= kb_in.buf + KB_IN_BYTES){
			kb_in.p_head = kb_in.buf;
		}
		kb_in.count ++;
	}
}

PUBLIC void init_keyboard(){
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;

	code_with_E0 = FALSE;
	shift_l = shift_r = FALSE;
	alt_l = alt_r = FALSE;
	ctrl_l = ctrl_r = FALSE;
	caps_lock = scroll_lock = FALSE;
	
	num_lock = TRUE;
	set_leds();

	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
	enable_irq(KEYBOARD_IRQ);
}

/*=================================================================================================
  					处理键盘输入
=================================================================================================*/
PUBLIC void keyboard_read(TTY* p_tty){
	u8	scan_code;
	int	make;
	u32	key = 0;
	u32*	keyrow;

	if(kb_in.count > 0){
		code_with_E0 = FALSE;
		scan_code = get_byte_from_kbuf();

		if(scan_code == 0xe1){
			int	i;
			u8	pausebreak_scode[]	= {0xe1, 0x1d, 0x45, 0xe1, 0x9d, 0xc5};
			int	is_pausebreak		= TRUE;
			for(i=1; i<6; i++){
				if(get_byte_from_kbuf() != pausebreak_scode[i]){
					is_pausebreak = FALSE;
					break;
				}
			}
			if(is_pausebreak){
				key = PAUSEBREAK;
			}
		}else if(scan_code == 0xe0){
			scan_code = get_byte_from_kbuf();

			/* PrintScreen被按下 */
			if(scan_code == 0x2a){
				if(get_byte_from_kbuf() == 0xe0){
					if(get_byte_from_kbuf() == 0x37){
						key = PRINTSCREEN;
						make = TRUE;
					}
				}
			}
			/* PrintScreen被释放 */
			if(scan_code == 0xb7){
				if(get_byte_from_kbuf() == 0xe0){
					if(get_byte_from_kbuf() == 0xaa){
						key = PRINTSCREEN;
						make = FALSE;
					}
				}
			}

			/* 非PrintScreen键 */
			if(key == 0){
				code_with_E0 = TRUE;
			}
		}

		if(key != PAUSEBREAK && key != PRINTSCREEN){
			/* 是Make Code还是Break Code ? */
			make = (scan_code & FLAG_BREAK) ? FALSE : TRUE;
			/* 定位到keymap的行 */
			keyrow = &keymap[(scan_code & 0x7f) * MAP_COLS];
			column = 0;

			int caps = shift_l || shift_r;

			if(caps_lock){
				if((keyrow[0] >= 'a') && (keyrow[0] <= 'z')){
					caps = !caps;
				}
			}
			
			if(caps){
				column = 1;
			}

			if(code_with_E0){
				column = 2;
				code_with_E0 = FALSE;
			}

			key = keyrow[column];

			switch(key){
			case SHIFT_L:
				shift_l = make;
				break;
			case SHIFT_R:
				shift_r = make;
				break;
			case CTRL_L:
				ctrl_l = make;
				break;
			case CTRL_R:
				ctrl_r = make;
				break;
			case ALT_L:
				alt_l = make;
				break;
			case ALT_R:
				alt_r = make;
				break;
			case CAPS_LOCK:
				if(make){
					caps_lock = !caps_lock;
					set_leds();
				}
				break;
			case NUM_LOCK:
				if(make){
					num_lock = !num_lock;
					set_leds();
				}
				break;
			case SCROLL_LOCK:
				if(make){
					scroll_lock = !scroll_lock;
					set_leds();
				}
				break;
			default:
				break;
			}

			if(make){
				int	pad = 0;

				if((key >= PAD_SLASH) && (key <= PAD_9)){
					pad = 1;
					switch(key){
					case PAD_SLASH:
						key = '/';
						break;
					case PAD_STAR:
						key = '*';
						break;
					case PAD_MINUS:
						key = '-';
						break;
					case PAD_PLUS:
						key = '+';
						break;
					case PAD_ENTER:
						key = ENTER;
						break;
					default:
						if(num_lock && (key >= PAD_0) && (key <= PAD_9)){
							key = key - PAD_0 + '0';
						}else if(num_lock && (key == PAD_DOT)){
							key = '.';
						}else{
							switch(key){
							case PAD_HOME:
								key = HOME;
								break;
							case PAD_END:
								key = END;
								break;
							case PAD_PAGEUP:
								key = PAGEUP;
								break;
							case PAD_PAGEDOWN:
								key = PAD_PAGEDOWN;
								break;
							case PAD_INS:
								key = INSERT;
								break;
							case PAD_UP:
								key = UP;
								break;
							case PAD_DOWN:
								key = DOWN;
								break;
							case PAD_LEFT:
								key = LEFT;
								break;
							case PAD_RIGHT:
								key = RIGHT;
								break;
							case PAD_DOT:
								key = DELETE;
								break;
							default:
								break;
							}
						}
						break;
					}
				}

				/* 组合按键信息（忽略Break Code） */
				key |= shift_l	? FLAG_SHIFT_L : 0;
				key |= shift_r	? FLAG_SHIFT_R : 0;
				key |= ctrl_l	? FLAG_CTRL_L : 0;
				key |= ctrl_r	? FLAG_CTRL_R : 0;
				key |= alt_l	? FLAG_ALT_L : 0;
				key |= alt_r	? FLAG_ALT_R : 0;
				/* 处理32位的按键信息 */
				in_process(p_tty, key);
			}
		}
	}
}
/*=================================================================================================
  					从缓冲区读入一个字节
=================================================================================================*/
PRIVATE	u8 get_byte_from_kbuf(){
	u8 scan_code;

	while(kb_in.count <= 0){}			/* 等待下一个字节的到来 */

	disable_int();
	scan_code = *(kb_in.p_tail);
	kb_in.p_tail ++;
	if(kb_in.p_tail >= kb_in.buf + KB_IN_BYTES){
		kb_in.p_tail = kb_in.buf;
	}
	kb_in.count --;
	enable_int();

	return scan_code;
}
/*=================================================================================================
  					kb_wait
=================================================================================================*/
PRIVATE void kb_wait(){
	u8	kb_state;
	do{
		kb_state = in_byte(KB_CMD);
	}while(kb_state & 0x02);
}
/*=================================================================================================
  					kb_ack
=================================================================================================*/
PRIVATE void kb_ack(){
	u8	kb_read;
	do{
		kb_read = in_byte(KB_DATA);
	}while(kb_read = !KB_ACK);
}
/*=================================================================================================
  					set_leds
=================================================================================================*/
PRIVATE void set_leds(){
	u8	leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;
	kb_wait();
	out_byte(KB_DATA, LED_CODE);
	kb_ack();
	kb_wait();
	out_byte(KB_DATA, leds);
	kb_ack();
}
