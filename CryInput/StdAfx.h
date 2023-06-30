// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__448E473C_48A4_4BA1_8498_3F9DAA9FE6A4__INCLUDED_)
#define AFX_STDAFX_H__448E473C_48A4_4BA1_8498_3F9DAA9FE6A4__INCLUDED_

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

// Insert your headers here

#include "platform.h"

#ifdef WIN32
#include <windows.h>
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <dinput.h>
#endif

//////////////////////////////////////////////////////////////////////
#define USE_NEWPOOL
#include <CryMemoryManager.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include <ITimer.h>
#include <IInput.h>
// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__448E473C_48A4_4BA1_8498_3F9DAA9FE6A4__INCLUDED_)
