/*=================================================================================================
  			string.h			Bill Liang	2014-8-22
=================================================================================================*/
PUBLIC	void*	memcpy(void* pDst, void* pSrc, int iSize);
PUBLIC	void	memset(void* pDst, char ch, int size);
PUBLIC	char*	strcpy(char* pDst, char* pSrc);
PUBLIC	int	strlen(char* p_str);

/**************************************************************************************************
 * 'phys_copy' and 'phys_set' are used only in the kernel, where segments are all flat (based on 0).
 * In the meanwhile, currently linear address space is mapped to the identical physical address space.
 * Therefore, a 'phys_copy' will be as same as a common copy, so does 'phys_set'.
 **************************************************************************************************/
#define	phys_copy	memcpy
#define	phys_set	memset
