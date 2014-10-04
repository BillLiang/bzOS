/*==================================================================================================
			tty.h				Bill Liang	2014-8-31
==================================================================================================*/
#ifndef	_BZOS_TTY_H_
#define	_BZOS_TTY_H_

#define	TTY_IN_BYTES		256		/* TTY输入缓存队列大小 */
#define TTY_OUT_BUF_LEN		2		/* tty output buffer size */

typedef struct s_tty{
	u32*	ibuf_head;			/* 缓冲区下一个空闲位置 */
	u32*	ibuf_tail;			/* 键盘任务应处理的键值 */
	int	ibuf_cnt;			/* 缓冲区已经填满了多少值 */
	u32	ibuf[TTY_IN_BYTES];		/* TTY input buffer */

	int	tty_caller;			/* who sends the MESSAGE to TTY (normally it would be FS) */
	int	tty_procnr;			/* who requests the data (we call it P) */
	void*	tty_req_buf;			/* the linear addr of buffer in P for storing the read chars */
	int	tty_left_cnt;			/* how much data P wants to receive */
	int	tty_trans_cnt;			/* how much data TTY has transferred to P */

	CONSOLE* console;
}TTY;

#endif
