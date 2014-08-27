#define GLOBAL_VARIABLES_HERE

#include	"type.h"
#include	"const.h"
#include	"protect.h"
#include	"proto.h"
#include	"proc.h"
#include	"global.h"

/* 全局进程表 */
PUBLIC	PROCESS		proc_table[NR_TASKS];
PUBLIC	TASK		task_table[NR_TASKS] = {{TestA, STACK_SIZE_TESTA, "TestA"},
						{TestB, STACK_SIZE_TESTB, "TestB"}};

/* 全局任务栈 */
PUBLIC	char		task_stack[STACK_SIZE_TOTAL];
