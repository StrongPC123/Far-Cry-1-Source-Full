// stdafx.cpp : source file that includes just the standard includes
//	CryGame.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#ifdef WIN64
int __cdecl strcmpi (const char *a, const char *b)
{
	return stricmp (a,b);
}
#endif