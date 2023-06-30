#include "stdafx.h"

#ifndef NOT_USE_UBICOM_SDK

#if defined(WIN32)
#	define GS_WIN32
#else
#	define GS_LINUX
#endif

#include "UbisoftMemory.h"

extern "C"
{
void * __stdcall ExtAlloc_Malloc(unsigned int lSize) {return malloc(lSize);} ;
void __stdcall ExtAlloc_Free(void *ptr) {free(ptr);};
void * __stdcall ExtAlloc_Realloc(void *ptr, unsigned int uiSize) {return realloc(ptr,uiSize);};
}
#endif // NOT_USE_UBICOM_SDK