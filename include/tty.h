/*==================================================================================================
			tty.h				Bill Liang	2014-8-31
==================================================================================================*/
#ifndef	_BZOS_TTY_H_
#define	_BZOS_TTY_H_

#define	TTY_IN_BYTES		256		/* TTY输入缓存队列大小 */

typedef struct s_tty{
	u32*		p_inbuf_head;			/* 缓冲区下一个空闲位置 */
	u32*		p_inbuf_tail;			/* 键盘任务应处理的键值 */
	int		inbuf_count;			/* 缓冲区已经填满了多少值 */
	u32		in_buf[TTY_IN_BYTES];

	CONSOLE*	p_console;
}TTY;

#endif
