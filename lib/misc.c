/*=================================================================================================
		  misc.c			Bill Liang	2014-9-2
=================================================================================================*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "fs.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "string.h"
#include "proto.h"
#include "global.h"

PUBLIC void spin(char* func_name){
	printl("\nspinning in %s ...\n", func_name);
	while(TRUE){}
}

PUBLIC void assertion_failure(char* exp, char* file, char* base_file, int line){
	printl("%c  assert(%s) failed: file: %s, base_file: %s, ln%d", MAG_CH_ASSERT, exp, file, base_file, line);
	
	/* we use a forever loop to prevent the proc from going on */
	spin("assertion_failure()");

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}
