// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__0B7BFEE0_95B3_4DD3_956A_33AD2ADB212D__INCLUDED_)
#define AFX_STDAFX_H__0B7BFEE0_95B3_4DD3_956A_33AD2ADB212D__INCLUDED_

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

#include <vector>
#include <list>
#include <map>
#include <algorithm>

#include "platform.h"

#if !defined(LINUX)
#include <assert.h>
#endif

#include "Cry_Math.h"
#include <IXml.h>
#include <IEntitySystem.h>
#include <IMovieSystem.h>
#include "smartptr.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0B7BFEE0_95B3_4DD3_956A_33AD2ADB212D__INCLUDED_)
