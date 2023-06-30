// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__81DAABA0_0054_42BF_8696_D99BA6832D03__INCLUDED_)
#define AFX_STDAFX_H__81DAABA0_0054_42BF_8696_D99BA6832D03__INCLUDED_

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
// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif
#else
#include <xtl.h>
#endif

#include <stdlib.h>
#define USE_NEWPOOL
#include <CryMemoryManager.h>



#include "Cry_Math.h"
#include "Cry_XOptimise.h" // required by AMD64 compiler
#include "Cry_Camera.h"
// TODO: reference additional headers your program requires here

class CAISystem; 



CAISystem *GetAISystem();

//////////////////////////////////////////////////////////////////////////
// Report AI warnings to validator.
//////////////////////////////////////////////////////////////////////////
//! Reports an AI Warning to validator with WARNING severity.
void AIWarning( const char *format,... );
//! Reports an AI Warning to validator with ERROR severity.
void AIError( const char *format,... );


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__81DAABA0_0054_42BF_8696_D99BA6832D03__INCLUDED_)
