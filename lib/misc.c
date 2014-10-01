/**************************************************************************************************
 * @file			lib/misc.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-9-2
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

/**************************************************************************************************
 * 					memcmp
 **************************************************************************************************
 * Compare memory areas.
 *
 * @param s1	The 1st area.
 * @param s2	The 2nd area.
 * @param n	The first n bytes will be compared.
 *
 * @return	An integer less than, equal to, or greater than zero if the first n bytes of s1 is
 * 		found, respectively, to be less than, to match, or be greater than the first n
 * 		bytes of s2.
 *************************************************************************************************/
PUBLIC int memcmp(const void* s1, const void* s2, int n){
	if((s1 == 0) || (s2 == 0)){
		return (s1 - s2);
	}

	const char* p1 = (const char*)s1;
	const char* p2 = (const char*)s2;
	int i;
	for(i=0; i<n; i++, p1++, p2++){
		if(*p1 != *p2){
			return (*p1 - *p2);
		}
	}
	return 0;
}
/**************************************************************************************************
 * 					strcmp
 **************************************************************************************************
 * Compare two strings.
 *
 * @param s1	The 1st string.
 * @param s2	The 2nd string.
 *
 * @return	An integer less than, equal to, or greater than zero if s1 (or the first n bytes
 * 		thereof) is found, respectively, to be less than, to match, or be greater than s2.
 *************************************************************************************************/
PUBLIC int strcmp(const char* s1, const char* s2){
	if((s1 == 0) || (s2 == 0)){
		return (s1 - s2);
	}

	const char* p1 = s1;
	const char* p2 = s2;

	for(; *p1 && *p2; p1++, p2++){
		if(*p1 != *p2){
			break;
		}
	}
	return (*p1 - *p2);
}
/**************************************************************************************************
 * 					strcat
 **************************************************************************************************
 * Concatenate two strings.
 *
 * @param s1	The 1st string.
 * @param s2	The 2nd string.
 *
 * @return	Pointer to the 1st string.
 *************************************************************************************************/
PUBLIC char* strcat(char* s1, const char* s2){
	if((s1 == 0) || (s2 == 0)){
		return 0;
	}

	char* p1 = s1;
	for(; *p1; p1++){}

	const char* p2 = s2;
	for(; *p2; p1++, p2++){
		*p1 = *p2;
	}
	*p1 = 0;
	
	return s1;
}

/**************************************************************************************************
 * 					spin
 *************************************************************************************************/
PUBLIC void spin(char* func_name){
	printl("\nspinning in %s ...\n", func_name);
	while(TRUE){}
}

/**************************************************************************************************
 * 					assertion_failure
 **************************************************************************************************
 * Invoked by assert().
 *
 * @param exp		The failure expression itself.
 * @param file		__FILE__
 * @param base_file	__BASE_FILE__
 * @param line		__LINE__
 *************************************************************************************************/
PUBLIC void assertion_failure(char* exp, char* file, char* base_file, int line){
	printl("%c  assert(%s) failed: file: %s, base_file: %s, ln%d", MAG_CH_ASSERT, exp, file, base_file, line);
	
	/* we use a forever loop to prevent the proc from going on */
	spin("assertion_failure()");

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}
