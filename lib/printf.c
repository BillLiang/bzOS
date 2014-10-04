/**************************************************************************************************
 * @file			printf.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-9-1
 *************************************************************************************************/

#include "type.h"
#include "const.h"
#include "stdio.h"
#include "protect.h"
#include "fs.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "string.h"
#include "proto.h"
#include "global.h"

PUBLIC int vsprintf(char* buf, const char* fmt, va_list args){
	char* 	p = buf;
	char	tmp[256];
	char*	str;
	int	value;
	int 	i;

	for(; *fmt; fmt++){
		if(*fmt != '%'){
			*buf ++ = *fmt;
			continue;
		}
		fmt ++;
		switch(*fmt){
		case 's':
			str = (char*) *((int*)args);
			while(*str){
				*buf ++ = *str ++;
			}
			args += 4;
			break;
		case 'd':
			value = *((int*) args);
			if(value < 0){
				*buf ++ = '-';
				value = -value;
			}
			i = 0;
			tmp[i] = (value % 10) + '0';
			i ++;
			value /= 10;
			while(value){
				tmp[i] = (value % 10) + '0';
				i ++;
				value /= 10;
			}
			for(i=i-1; i>=0; i--){
				*buf ++ = tmp[i];
			}
			args += 4;
			break;
		case 'c':
			*buf ++ = *((char*)args);
			args += 4;
			break;
		case 'x':
			itoa(tmp, *((int*) args));
			strcpy(buf, tmp);
			buf += strlen(tmp);
			args += 4;
			break;
		default:
			break;
		}
	}

	*buf	= 0;

	return (buf - p);
}
/**************************************************************************************************
 * 					printf
 **************************************************************************************************
 * The most famous one.
 *
 * @param fmt	The format string.
 *
 * @return	The number of chars printed.
 *************************************************************************************************/
PUBLIC int printf(const char* fmt, ...){
	int	i;
	char	buf[STR_DEFAULT_LEN];
	va_list arg = (va_list) ((char*)(&fmt) + 4);	/* 参数 */

	i = vsprintf(buf, fmt, arg);
	int c = write(1, buf, i);			/* file descriptor is 1 */

	assert(c == i);

	return i;
}
/**************************************************************************************************
 * 					printl
 **************************************************************************************************
 * Low level print
 *
 * @param fmt	The format string.
 *
 * @return	The number of chars printed.
 *************************************************************************************************/
PUBLIC int printl(const char* fmt, ...){
	int i;
	char buf[STR_DEFAULT_LEN];
	va_list arg = (va_list) ((char*)(&fmt) + 4);

	i = vsprintf(buf, fmt, arg);
	printx(buf);

	return i;
}
/**************************************************************************************************
 * 					sprintf
 *************************************************************************************************/
PUBLIC int sprintf(char* buf, const char* fmt, ...){
	va_list arg = (va_list)((char*)&fmt + 4);
	return vsprintf(buf, fmt, arg);
}
