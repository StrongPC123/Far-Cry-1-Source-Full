
#ifndef FONTCRYPAK_H
#define FONTCRYPAK_H

//////////////////////////////////////////////////////////////////////////
// really annoying since the LUA library can't link any C++ code...

#define USE_CRYPAK

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

FILE	*CryPakOpen(const char *szFile,const char *szMode);
void	CryPakClose(FILE	*fp);
int 	CryPakFFlush(FILE	*fp);
int 	CryPakFTell(FILE	*fp);
int   CryPakFSeek(FILE *handle, long seek, int mode);
int   CryPakUngetc(int c, FILE *handle);
int   CryPakGetc(FILE *handle);
size_t CryPakFRead(void *data, size_t length, size_t elems, FILE *handle);
size_t CryPakFWrite(void *data, size_t length, size_t elems, FILE *handle);
int   CryPakFEof(FILE *handle);
//int   CryPakFScanf(FILE *handle, const char *format, ...);
int   CryPakFPrintf(FILE *handle, const char *format, ...);
char *CryPakFGets(char *str, int n, FILE *handle);

#ifdef __cplusplus
}
#endif

#endif
