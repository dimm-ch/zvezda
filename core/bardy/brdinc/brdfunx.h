/***************************************************
*
* BRDFUNX.H
*
* Some useful Functions
*
* (C) Instr.Sys. by Ekkore Mar, 2003
*
****************************************************/


#ifndef	__BRDFUNX_H_
#define	__BRDFUNX_H_


//
//======= Macros
//

#if defined (__linux__) || defined (__LINUX__)
#define HAlloc(size) malloc(size)
#define HFree(addr) free(addr)
#else
#define HAlloc(size) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, (size) )
#define HFree(ptr)   HeapFree( GetProcessHeap(), 0, (ptr) )
#endif



#endif	// __BRDFUNX_H_

//
//  End of File
//
