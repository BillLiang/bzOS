#define GLOBAL_VARIABLES_HERE

#include	"type.h"
#include	"const.h"
#include	"protect.h"
#include	"proto.h"
#include	"proc.h"
#include	"global.h"

/* 全局进程表 */
PUBLIC	PROCESS		proc_table[NR_TASKS];

/* 全局任务栈 */
PUBLIC	char		task_stack[STACK_SIZE_TOTAL];
