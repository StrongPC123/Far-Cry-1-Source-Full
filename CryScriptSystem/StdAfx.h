// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__844E5BAB_B810_40FC_8939_167146C07AED__INCLUDED_)
#define AFX_STDAFX_H__844E5BAB_B810_40FC_8939_167146C07AED__INCLUDED_

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

// to avoid warnings when we've already defined this in the command line..
//#ifndef CRYXMLDOM_EXPORTS
//#define CRYXMLDOM_EXPORTS
//#endif

#ifndef _XBOX
#if defined(WIN32) || defined(WIN64)
#define WIN32_LEAN_AND_MEAN
//#define USE_MEM_POOL
#include <windows.h>
//#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
//#include <crtdbg.h>

#endif
#else
#include <xtl.h>
#endif

#ifdef PS2
#include "iostream.h"
//wrapper for VC specific function
inline void itoa(int n, char *str, int basen)
{
 	sprintf(str,"%d", n);
}
  
inline char *_strlwr(const char *str)
{
    return PS2strlwr(str);
}
#endif


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__844E5BAB_B810_40FC_8939_167146C07AED__INCLUDED_)
