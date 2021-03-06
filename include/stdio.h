/**************************************************************************************************
 * @file			stdio.c
 * @brief 			
 * @author			Bill Liang
 * @date			2014-9-25
 *************************************************************************************************/

/* assertion */
#define	ASSERT
#ifdef	ASSERT
void assertion_failure(char* exp, char* file, char* base_file, int line);
#define assert(exp)	if(exp) ; \
	else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__);
#else
#define	assert(exp)
#endif

/* EXTERN is defined as extern except in global.c */
#define	EXTERN	extern

#define	STR_DEFAULT_LEN		1024

#define O_CREAT			1
#define O_RDWR			2

#define MAX_PATH		128

/**************************************************************************************************
 ***********************************
 * printf, printl, printx
 ***********************************
 *
 * printf:
 * 	USER_PROC --------> FS --------> TTY
 *
 **************************************************************************************************
 *
 * printl: variant-parameter-version printx
 * 	calls vsprintf, then printx (trap into kernel directly)
 *
 **************************************************************************************************
 *
 * printx: low level print without using IPC
 * 	USER_PROC --------------------> KERNEL (trap directly)
 *
 *************************************************************************************************/

/* printf.c */
PUBLIC	int	printf(const char* fmt, ...);
PUBLIC	int	printl(const char* fmt, ...);
PUBLIC	int	sprintf(char* buf, const char* fmt, ...);

/* library functions */
/* lib/getpid.c */
PUBLIC int getpid();

/* lib/open.c */
PUBLIC int open(const char* pathname, int flags);
/* lib/close.c */
PUBLIC int close(int fd);
/* lib/read.c */
PUBLIC int read(int fd, void* buf, int count);
/* lib/write.c */
PUBLIC int write(int fd, const void* buf, int count);
/* lib/unlink.c */
PUBLIC int unlink(const char* pathname);

/* lib/fork.c */
PUBLIC int fork();
