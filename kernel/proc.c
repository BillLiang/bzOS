/*==================================================================================================
			proc.c			Bill Liang	2014-8-29
==================================================================================================*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

PUBLIC int sys_get_ticks(){
	return ticks;
}
