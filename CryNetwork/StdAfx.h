// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__0F62BDE9_4784_4F7A_A265_B7D1A71883D0__INCLUDED_)
#define AFX_STDAFX_H__0F62BDE9_4784_4F7A_A265_B7D1A71883D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
// THIS MUST BE AT THE VERY BEGINING OF STDAFX.H FILE.
// Disable STL threading support, (makes STL faster)
//////////////////////////////////////////////////////////////////////////
#define _NOTHREADS
#define _STLP_NO_THREADS
//////////////////////////////////////////////////////////////////////////

#include <platform.h>

#ifndef _XBOX
#ifdef WIN32
#include <windows.h>
#define WIN32_LEAN_AND_MEAN

#endif
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#define USE_NEWPOOL
#include <CryMemoryManager.h>


#include <map>
#include <INetwork.h>
#include "DatagramSocket.h"
#include <CrySizer.h>
// TODO: reference additional headers your program requires here



#if defined(_DEBUG) && !defined(LINUX)
#include <crtdbg.h>
#endif




#if _MSC_VER > 1000
	#pragma warning( disable : 4786 )
	//#pragma warning( disable : 4716 )
#endif // _MSC_VER > 1000




#ifdef PS2
typedef int	BOOL;
#endif

#ifndef __NET_ASSERT
#define __NET_ASSERT

#if 1

#if defined(WIN64)

	#define NET_ASSERT(x) 

#elif defined(_XBOX)

	#define NET_ASSERT(x) 

#elif defined(PS2)
	#include <iostream.h>
	inline void NET_ASSERT(void * x)
	{
		if (!(x))  
		{
			cout <<	"Assertion Failed!!\n\nFile: " << __FILE__ <<"\n\nLine: " << __LINE__ << "\n";
			DEBUG_BREAK;
		}
	}
#elif defined(WIN32)
	#define NET_ASSERT(x)																									\
	{																													\
			if (!(x)) {																										\
			static char sAssertionMessage[500];																				\
																														\
			wsprintf(sAssertionMessage, "Assertion Failed!!\n\nFile: \"%s\"\n\nLine: %d\n", __FILE__, __LINE__);	\
			::MessageBox(NULL, sAssertionMessage, "Assertion Failed", MB_OK | MB_ICONERROR);						\
			DEBUG_BREAK;																\
			}																												\
	}   
#else

	#define NET_ASSERT(x) 

#endif

#endif //1
#endif

#ifndef NET____TRACE
#define NET____TRACE

// src and trg can be the same pointer (in place encryption)
// len must be in bytes and must be multiple of 8 byts (64bits).
// key is 128bit:  int key[4] = {n1,n2,n3,n4};
// void encipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k )
#define TEA_ENCODE( src,trg,len,key ) {\
	register unsigned int *v = (src), *w = (trg), *k = (key), nlen = (len) >> 3; \
	register unsigned int delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],d=k[3]; \
	while (nlen--) {\
	register unsigned int y=v[0],z=v[1],n=32,sum=0; \
	while(n-->0) { sum += delta; y += (z << 4)+a ^ z+sum ^ (z >> 5)+b; z += (y << 4)+c ^ y+sum ^ (y >> 5)+d; } \
	w[0]=y; w[1]=z; v+=2,w+=2; }}

// src and trg can be the same pointer (in place decryption)
// len must be in bytes and must be multiple of 8 byts (64bits).
// key is 128bit: int key[4] = {n1,n2,n3,n4};
// void decipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k)
#define TEA_DECODE( src,trg,len,key ) {\
	register unsigned int *v = (src), *w = (trg), *k = (key), nlen = (len) >> 3; \
	register unsigned int delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],d=k[3]; \
	while (nlen--) { \
	register unsigned int y=v[0],z=v[1],sum=0xC6EF3720,n=32; \
	while(n-->0) { z -= (y << 4)+c ^ y+sum ^ (y >> 5)+d; y -= (z << 4)+a ^ z+sum ^ (z >> 5)+b; sum -= delta; } \
	w[0]=y; w[1]=z; v+=2,w+=2; }}

// encode size ignore last 3 bits of size in bytes. (encode by 8bytes min)
#define TEA_GETSIZE( len ) ((len) & (~7))

#ifndef PS2
inline void __cdecl __NET_TRACE(const char *sFormat, ... )
{
	/*
	va_list vl;
	static char sTraceString[500];

	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);
	NET_ASSERT(strlen(sTraceString) < 500) 
	CryLogAlways( sTraceString );

	va_list vl;
	static char sTraceString[500];

	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);
	NET_ASSERT(strlen(sTraceString) < 500) 
	::OutputDebugString(sTraceString);*/
	
}
#else
inline void __NET_TRACE(const char *sFormat, ... )
{
	va_list vl;
	static char sTraceString[500];

	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);
	NET_ASSERT(strlen(sTraceString) < 500)
	cout << sTraceString;
	
}

#endif	//PS2

#if 1

#define NET_TRACE __NET_TRACE

#else

#define NET_TRACE 1?(void)0 : __NET_TRACE;

#endif //NET____TRACE

#endif //_DEBUG

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0F62BDE9_4784_4F7A_A265_B7D1A71883D0__INCLUDED_)
