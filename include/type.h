#ifndef _BZOS_TYPE_H_
#define _BZOS_TYPE_H_

typedef unsigned int		u32;
typedef unsigned short		u16;
typedef unsigned char		u8;

typedef void (*irq_handler)(int irq);
typedef void (*int_handler)();

typedef void (*task_f)();

typedef	void* system_call;

typedef char* va_list;
#endif
