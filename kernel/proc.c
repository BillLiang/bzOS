/**************************************************************************************************
 * @file			proc.c
 * @brief 
 * @author			Bill Liang
 * @date			2014-8-29
 *************************************************************************************************/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "string.h"
#include "proto.h"
#include "global.h"

/**************************************************************************************************
 * 					schedule
 **************************************************************************************************
 * <Ring 0> choose one proc to run.
 *************************************************************************************************/
PUBLIC void schedule(){
	PROCESS*	p;
	int		greatest_ticks = 0;
	while(!greatest_ticks){
		for(p=proc_table; p<proc_table + NR_TASKS + NR_PROCS; p++){
			if(p->flags == 0){		/* if the proc is runnable (unblocked) */
				if(p->ticks > greatest_ticks){
					greatest_ticks = p->ticks;
					p_proc_ready = p;
				}
			}
		}
		/* 当所有的进程的ticks都减到零时 */
		if(!greatest_ticks){
			for(p=proc_table; p<proc_table + NR_TASKS + NR_PROCS; p++){
				if(p->flags == 0){		/* if the proc is runnable (unblocked) */
					p->ticks = p->priority;
				}
			}
		}
	}
}





/**************************************************************************************************
 * 					ldt_seg_linear
 **************************************************************************************************
 * <Ring 0~1> Calculate the linear address of a certain segment of a given proc.
 * @param p	whose
 * @param idx	Which descriptor in the LDT of the proc.
 *************************************************************************************************/
PUBLIC int ldt_seg_linear(PROCESS* p_proc, int idx){
	DESCRIPTOR* d	 = &p_proc->ldts[idx];
	return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}

/**************************************************************************************************
 * 					va2la
 **************************************************************************************************
 * <Ring 0~1> Virtual addr -> linear addr
 * @param pid	Whose
 * @param va	Virtual addr(or logical addr?)
 *************************************************************************************************/
PUBLIC void* va2la(int pid, void* va){
	PROCESS* p	= &proc_table[pid];
	u32 seg_base	= ldt_seg_linear(p, INDEX_LDT_RW);
	u32 la		= seg_base + (u32)va;

	if(pid < NR_TASKS + NR_PROCS){
		assert(la == (u32)va);
	}
	return (void*)la;
}
/**************************************************************************************************
 * 					reset_msg
 **************************************************************************************************
 * <Ring 0~3> Clear up a MESSAGE by setting each byte to 0
 *
 * @param msg	The meassage to be cleared.
 *************************************************************************************************/
PUBLIC void reset_msg(MESSAGE* msg){
	memset(msg, 0, sizeof(MESSAGE));
}
/**************************************************************************************************
 * 					block
 **************************************************************************************************
 * <Ring 0> This routine is called after 'flags' has been set (flags != 0).
 * It calls 'schedule()' to choose another proc as the 'p_proc_ready'.
 *
 * @attention This routine does not change 'flags'. Make sure the 'flags' of the proc to be blocked
 * has been set properly.
 *
 * @param proc		The proc to be blocked.
 *************************************************************************************************/
PRIVATE void block(PROCESS* proc){
	assert(proc->flags);
	schedule();
}
/**************************************************************************************************
 * 					unblock
 **************************************************************************************************
 * <Ring 0> This is a dummy routine. It does nothing actually. When it is called, the 'flags'
 * should have been cleared (flags == 0).
 *
 * @param proc	The unblocked proc.
 *************************************************************************************************/
PRIVATE void unblock(PROCESS* proc){
	assert(proc->flags == 0);
}
/**************************************************************************************************
 * 					deadlock
 **************************************************************************************************
 * <Ring 0> Check whether it is safe to send a message from src to dest.
 * The routine will detect if the messaging graph contains a cycle.
 * For instance, if we have procs trying to send message like this:
 * A -> B -> C -> A, then a deadlock occurs, because all of them will wait forever.
 * If no cycles detected, it is considered as safe.
 *
 * @param src	Who wants to send message.
 * @param dest	To whom the message is sent.
 *
 * @return	0 if success.
 *************************************************************************************************/
PRIVATE int deadlock(int src, int dest){
	PROCESS*	p = proc_table + dest;
	while(TRUE){
		if(p->flags & SENDING){				/* if the proc is sending a message */
			if(p->send_to == src){			/* That is it! A cycle. Let's print the chain. */
				p = proc_table + dest;
				printl("T_T %s", p->name);
				do{
					assert(p->p_msg);	/* of course, the message is not null */
					p = proc_table + p->send_to;
					printl("->%s", p->name);
				}while(p != proc_table + src);
				printl("T_T");

				return 1;
			}
			p = proc_table + p->send_to;
		}else{
			break;
		}
	}
	return 0;
}





/**************************************************************************************************
 * 					msg_send
 **************************************************************************************************
 * <Ring 0> Send a message to the dest proc. If dest is blocked waiting for the message,
 * copy the message to it and unblock dest. 
 * Otherwise the caller will be blocked and appended to the dest's sending queue.
 *
 * @param current	The caller, the sender.
 * @param dest		To whom the message is sent.
 * @param msg		Pointer to the MESSAGE struct.
 *
 * @return		0 if success.
 *************************************************************************************************/
PRIVATE int msg_send(PROCESS* current, int dest, MESSAGE* msg){
	PROCESS* sender		= current;
	PROCESS* p_dest		= proc_table + dest;

	assert(proc2pid(sender) != dest);

	if(deadlock(proc2pid(sender), dest)){			/* Is there deadlock chain? */
		panic(">>DEADLOCK<< %s->%s", sender->name, p_dest->name);
	}
	/* dest is waiting for that msg or ANY msg. */
	if((p_dest->flags & RECEIVING) && (p_dest->recv_from == proc2pid(sender) || p_dest->recv_from == ANY)){
		assert(p_dest->p_msg);
		assert(msg);
		
		phys_copy(va2la(dest, p_dest->p_msg), va2la(proc2pid(sender), msg), sizeof(MESSAGE));

		p_dest->p_msg		= 0;
		p_dest->flags		&= ~RECEIVING;		/* dest has received the message */
		p_dest->recv_from	= NO_TASK;
		unblock(p_dest);

		assert(p_dest->flags == 0);
		assert(p_dest->p_msg == 0);
		assert(p_dest->recv_from == NO_TASK);
		assert(p_dest->send_to == NO_TASK);

		assert(sender->flags == 0);
		assert(sender->p_msg == 0);
		assert(sender->recv_from == NO_TASK);
		assert(sender->send_to == NO_TASK);
	}else{	/* However, dest is not waiting for the message */
		sender->flags		|= SENDING;
		assert(sender->flags == SENDING);
		sender->send_to		= dest;
		sender->p_msg		= msg;

		/* append to the sending queue */
		PROCESS* p;
		if(p_dest->q_sending){
			p = p_dest->q_sending;
			while(p->next_sending){
				p = p->next_sending;
			}
			p->next_sending = sender;
		}else{
			p_dest->q_sending = sender;
		}
		sender->next_sending = 0;

		block(sender);

		assert(sender->flags == SENDING);
		assert(sender->p_msg != 0);
		assert(sender->recv_from == NO_TASK);
		assert(sender->send_to == dest);
	}
	return 0;
}
/**************************************************************************************************
 * 					msg_receive
 **************************************************************************************************
 * <Ring 0> Try to get a message from the src proc.
 * If src is blocked sending the message, copy the message from it and unblock src.
 * Otherwise the caller will be blocked.
 *
 * @param current	The caller, the proc who wanna receive.
 * @param src		From whom the message will be received.
 * @param m		Pointer to the MESSAGE struct.
 *
 * @return		0 if success.
 *************************************************************************************************/
PRIVATE int msg_receive(PROCESS* current, int src, MESSAGE* m){
	PROCESS* p_who_wanna_recv	= current;
	PROCESS* p_from			= 0;
	PROCESS* prev			= 0;	/* the prior of p_from in the sending queue. */

	int copyok			= FALSE;
	
	assert(proc2pid(p_who_wanna_recv) != src);

	/*
	 * There is an interrupt needs p_who_wanna_recv's handling and p_who_wanna_recv is ready
	 * to handle it.
	 */
	if((p_who_wanna_recv->has_int_msg) && ((src == ANY) || (src == INTERRUPT))){
		MESSAGE msg;
		reset_msg(&msg);
		msg.source	= INTERRUPT;
		msg.type	= HARD_INT;

		assert(m);

		phys_copy(va2la(proc2pid(p_who_wanna_recv), m), &msg, sizeof(MESSAGE));
		
		p_who_wanna_recv->has_int_msg = 0;

		assert(p_who_wanna_recv->flags == 0);
		assert(p_who_wanna_recv->p_msg == 0);
		assert(p_who_wanna_recv->send_to == NO_TASK);
		assert(p_who_wanna_recv->has_int_msg == 0);

		return 0;
	}

	/*
	 * Arrive here if no interrupt for p_who_wanna_recv.
	 *
	 * p_who_wanna_recv is ready to receive messages from ANY proc, we'll check the sending queue
	 * and pick the first proc in it.
	 */
	if(src == ANY){
		if(p_who_wanna_recv->q_sending){
			p_from		= p_who_wanna_recv->q_sending;
			copyok		= TRUE;

			assert(p_who_wanna_recv->flags == 0);
			assert(p_who_wanna_recv->p_msg == 0);
			assert(p_who_wanna_recv->recv_from == NO_TASK);
			assert(p_who_wanna_recv->send_to == NO_TASK);
			assert(p_who_wanna_recv->q_sending != 0);

			assert(p_from->flags == SENDING);
			assert(p_from->p_msg != 0);
			assert(p_from->recv_from == NO_TASK);
			assert(p_from->send_to == proc2pid(p_who_wanna_recv));
		}
	}else{/* p_who_wanna_recv wants to receive a message from a certain proc: src. */
		p_from		= &proc_table[src];
		if((p_from->flags & SENDING) && (p_from->send_to == proc2pid(p_who_wanna_recv))){
			copyok		= TRUE;
			PROCESS* p	= p_who_wanna_recv->q_sending;

			assert(p);	/* p_from must have been appended to the queue */

			while(p){
				assert(p_from->flags & SENDING);
				if(proc2pid(p) == src){
					p_from = p;
					break;
				}
				prev	= p;
				p	= p->next_sending;	
			}

			assert(p_who_wanna_recv->flags == 0);
			assert(p_who_wanna_recv->p_msg == 0);
			assert(p_who_wanna_recv->recv_from == NO_TASK);
			assert(p_who_wanna_recv->send_to == NO_TASK);
			assert(p_who_wanna_recv->q_sending != 0);

			assert(p_from->flags == SENDING);
			assert(p_from->p_msg != 0);
			assert(p_from->recv_from == NO_TASK);
			assert(p_from->send_to == proc2pid(p_who_wanna_recv));
		}
	}
	/*
	 * It's determined from which proc the message will be copied.
	 * Note that this proc must have been waiting for this moment in the queue,
	 * so we should remove it from the queue.
	 */
	if(copyok){
		if(p_from == p_who_wanna_recv->q_sending){	/* the 1st one. */
			assert(prev == 0);
			
			p_who_wanna_recv->q_sending	= p_from->next_sending;
			p_from->next_sending		= 0;
		}else{
			assert(prev);

			prev->next_sending	= p_from->next_sending;
			p_from->next_sending	= 0;
		}

		assert(m);
		assert(p_from->p_msg);
		/* It's time to copy the message. */
		phys_copy(va2la(proc2pid(p_who_wanna_recv), m), va2la(proc2pid(p_from), p_from->p_msg), sizeof(MESSAGE));

		p_from->p_msg		= 0;
		p_from->send_to		= NO_TASK;
		p_from->flags		&= ~SENDING;
		unblock(p_from);
	}
	/*
	 * Unfortunately, nobody is sending any message.
	 * Set flags so that p_who_wanna_recv will not be shceduled util it is unblocked.
	 */
	else{
		p_who_wanna_recv->flags		|= RECEIVING;
		p_who_wanna_recv->p_msg		= m;
		if(src == ANY){
			p_who_wanna_recv->recv_from	= ANY;
		}else{
			p_who_wanna_recv->recv_from	= proc2pid(p_from);
		}

		block(p_who_wanna_recv);

		assert(p_who_wanna_recv->flags == RECEIVING);
		assert(p_who_wanna_recv->p_msg != 0);
		assert(p_who_wanna_recv->recv_from != NO_TASK);
		assert(p_who_wanna_recv->send_to == NO_TASK);
		assert(p_who_wanna_recv->has_int_msg == 0);
	}

	return 0;
}





/**************************************************************************************************
 * 					inform_int
 **************************************************************************************************
 * <Ring 0> Inform a proc that an interrupt has occurred.
 * 
 * @param task_nr	The task which will be informed.	
 *************************************************************************************************/
PUBLIC void inform_int(int task_nr){
	PROCESS* p = proc_table + task_nr;

	if((p->flags & RECEIVING) && (p->recv_from == INTERRUPT || p->recv_from == ANY)){
		p->p_msg->source	= INTERRUPT;
		p->p_msg->type		= HARD_INT;
		p->p_msg		= 0;
		p->has_int_msg		= 0;
		p->flags		&= ~RECEIVING;
		p->recv_from		= NO_TASK;
		
		assert(p->flags == 0);
		
		unblock(p);

		assert(p->p_msg == 0);
		assert(p->recv_from == NO_TASK);
		assert(p->send_to == NO_TASK);
	}else{
		p->has_int_msg		= 1;
	}
}




/**************************************************************************************************
 * 					sys_sendrec
 **************************************************************************************************
 * <Ring 0> The core routine of system call 'sendrec'.
 * 
 * @param function	SEND, RECEIVE, BOTH
 * @param src_dest	To / From whom the message is transferred
 * @param msg		Pointer to the MESSAGE struct
 * @param proc		The caller process
 *
 * @return		0 if success
 *************************************************************************************************/
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* msg, PROCESS* proc){
	assert(k_reenter == 0);			/* make sure we are not in ring0 */
	assert((src_dest >= 0 && src_dest < NR_TASKS + NR_PROCS) ||
		src_dest == ANY ||
		src_dest == INTERRUPT);

	int ret		= 0;
	int caller	= proc2pid(proc);
	MESSAGE* mla	= (MESSAGE*) va2la(caller, msg);
	mla->source	= caller;		/* from caller */

	assert(mla->source != src_dest);

	/**
	 * Actually we have the third message type: BOTH. However, it is not allowed to be passed
	 * to the kernel directly. Kernel doesn't know it at all. It is transformed into a SEND
	 * followed by RECEIVE by 'send_recv()'.
	 */
	if(function == SEND){
		ret = msg_send(proc, src_dest, msg);
		if(ret != 0){
			return ret;
		}
	}else if(function == RECEIVE){
		ret = msg_receive(proc, src_dest, msg);
		if(ret != 0){
			return ret;
		}
	}else{
		panic("{sys_sendrec} invalid function: %d (SEND: %d, RECEIVE: %d).", function, SEND, RECEIVE);
	}
	return 0;
}
/**************************************************************************************************
 * 					send_recv
 **************************************************************************************************
 * <Ring 1~3> IPC syscall.
 * 
 * It is an encapsulation of 'sendrec', invoking 'sendrec' directly should be avoided.
 *
 * @param function	SEND, RECEIVE, BOTH
 * @param src_dest	The caller's proc_nr
 * @param msg		Pointer to the MESSAGE struct
 *
 * @return		always 0
 *************************************************************************************************/
PUBLIC int send_recv(int function, int src_dest, MESSAGE* msg){
	int ret		= 0;
	if(function == RECEIVE){		/* if proc wants to receive message, then clear its meassge struct */
		memset(msg, 0, sizeof(MESSAGE));
	}

	switch(function){
	case BOTH:
		ret = sendrec(SEND, src_dest, msg);
		if(ret == 0){
			ret = sendrec(RECEIVE, src_dest, msg);
		}
		break;
	case SEND:
	case RECEIVE:
		ret = sendrec(function, src_dest, msg);
		break;
	default:
		assert((function == BOTH) || (function == SEND) || (function == RECEIVE));
		break;
	}
	return ret;
}




/**************************************************************************************************
 * 					dump_msg
 *************************************************************************************************/
PUBLIC void dump_msg(const char* title, MESSAGE* m){
	int packed = FALSE;
	printl("{%s}<0x%x>{%ssrc:%s(%d),%stype:%d,%s(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)%s}%s",
			title,
			(int)m,
			packed ? "" : "\n        ",
			proc_table[m->source].name,
			m->source,
			packed ? "  " : "\n        ",
			m->type,
			packed ? "  " : "\n        ",
			m->u.m3.m3i1,
			m->u.m3.m3i2,
			m->u.m3.m3i3,
			m->u.m3.m3i4,
			(int) m->u.m3.m3p1,
			(int) m->u.m3.m3p2,
			packed ? "" : "\n",
			packed ? "" : "\n"
			);
}
/**************************************************************************************************
 * 					dump_proc
 *************************************************************************************************/
PUBLIC void dump_proc(PROCESS* p){
	char info[STR_DEFAULT_LEN];
	int i;
	int text_color		= MAKE_COLOR(GREEN, RED);
	
	int dump_len		= sizeof(PROCESS);

	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, 0);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, 0);

	vsprintf(info, "byte dump of proc_table[%d]:\n", p - proc_table);
	disp_color_str(info, text_color);
	for(i=0; i<dump_len; i++){
		vsprintf(info, "%x.", ((unsigned char*)p)[i]);
		disp_color_str(info, text_color);
	}

	disp_color_str("\n\n", text_color);
	vsprintf(info, "ANY: 0x%x.\n", ANY);		disp_color_str(info, text_color);
	vsprintf(info, "NO_TASK: 0x%x.\n", NO_TASK);	disp_color_str(info, text_color);
	disp_color_str("\n", text_color);

	vsprintf(info, "ldt_sel: 0x%x.  ", p->ldt_sel);		disp_color_str(info, text_color);
	vsprintf(info, "ticks: 0x%x.  ", p->ticks);		disp_color_str(info, text_color);
	vsprintf(info, "priority: 0x%x.  ", p->priority);	disp_color_str(info, text_color);
	vsprintf(info, "pid: 0x%x.  ", p->pid);			disp_color_str(info, text_color);
	vsprintf(info, "name: 0x%s.  ", p->name);		disp_color_str(info, text_color);
	disp_color_str("\n", text_color);
	vsprintf(info, "flags: 0x%x.  ", p->flags);		disp_color_str(info, text_color);
	vsprintf(info, "recv_from: 0x%x.  ", p->recv_from);	disp_color_str(info, text_color);
	vsprintf(info, "send_to: 0x%x.  ", p->send_to);		disp_color_str(info, text_color);
	vsprintf(info, "nr_tty: 0x%x.  ", p->nr_tty);		disp_color_str(info, text_color);
	disp_color_str("\n", text_color);
	vsprintf(info, "has_int_msg: 0x%x.  ", p->has_int_msg);	disp_color_str(info, text_color);
}
