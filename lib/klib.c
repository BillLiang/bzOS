/*=================================================================================================
  			klib.c			Bill Liang	2014-8-23
================================================================================================ */
#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"
/*=================================================================================================
  				itoa	将整数转换成字符串
================================================================================================ */
/*0000ABC0 --> ABC0*/
PUBLIC char* itoa(char* str, int num){
	int	i;
	char	ch;
	int	flag = 0;

	*str = '0';
	str ++;
	*str = 'x';
	str ++;
	if(num == 0){
		*str = '0';
		str ++;
	}else{
		for(i=28; i>=0; i-=4){
			ch = (num >> i) & 0xf;
			if(flag || ch > 0){
				flag = 1;
				ch += '0';
				if(ch > '9'){
					ch += 7;
				}
				*str = ch;
				str ++;
			}
		}
	}
	/*不要忘记了，C语言字符串结束符'\0'*/
	*str = 0;
	return str;
}
/*=================================================================================================
  					disp_int
================================================================================================ */
PUBLIC void disp_int(int input){
	char	output[16];
	itoa(output, input);
	disp_str(output);
}
/*=================================================================================================
  					delay	
================================================================================================ */
PUBLIC void delay(int times){
	int i, j, k;
	for(k=0; k<times; k++){
		for(i=0; i<100; i++){
			for(j=0; j<10000; j++){}
		}
	}
}