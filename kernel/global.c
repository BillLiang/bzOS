#define GLOBAL_VARIABLES_HERE

#include	"type.h"
#include	"const.h"
#include	"protect.h"
#include	"proto.h"
#include	"proc.h"
#include	"global.h"

/* irq处理例程 */
PUBLIC	irq_handler	irq_table[NR_IRQ];

/* 全局进程表 */
PUBLIC	PROCESS		proc_table[NR_TASKS];
PUBLIC	TASK		task_table[NR_TASKS] = {{TestA, STACK_SIZE_TESTA, "TestA"},
						{TestB, STACK_SIZE_TESTB, "TestB"},
						{TestC, STACK_SIZE_TESTC, "TestC"}};
/* 全局任务栈 */
PUBLIC	char		task_stack[STACK_SIZE_TOTAL];
