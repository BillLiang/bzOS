/*==================================================================================================
			console.h				Bill Liang	2014-8-31
==================================================================================================*/
#ifndef	_BZOS_CONSOLE_H_
#define	_BZOS_CONSOLE_H_

#define	DEFAULT_CHAR_COLOR	0x07		/* 黑底白字 */

#define	SCR_UP			1
#define	SCR_DN			-1

#define	SCREEN_SIZE		(80 * 25)
#define	SCREEN_WIDTH		80

typedef struct s_console{
	u32	original_addr;		/* 当前控制台对应的显存位置 */
	u32	current_start_addr;	/* 当前控制台显示到了什么地方 */

	u32	v_mem_limit;		/* 当前控制台的显存大小 */
	u32	cursor;			/* 当前光标的位置 */
}CONSOLE;

#endif
