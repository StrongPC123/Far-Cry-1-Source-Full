#ifndef __UBISOFT_MEMORY_H
#define __UBISOFT_MEMORY_H

#if defined(WIN32)
#	define GS_WIN32
#else
#	define GS_LINUX
#endif

extern "C"  void * __stdcall ExtAlloc_Malloc(unsigned int lSize);
extern "C"  void __stdcall ExtAlloc_Free(void *ptr);
extern "C"  void * __stdcall ExtAlloc_Realloc(void *ptr, unsigned int uiSize);

#endif
