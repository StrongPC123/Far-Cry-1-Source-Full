// XTVersion.h Xtreme Toolkit version definitions file
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//
//////////////////////////////////////////////////////////////////////

#if !defined(__XTVERSION_H__)
#define __XTVERSION_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Xtreme Toolkit version definitions:
//////////////////////////////////////////////////////////////////////

// Summary: for determining version of XTLIB.DLL
#define _XTLIB_VERSION_         MAKELONG(1,3)
// Summary: Major Version Number
#define _XTLIB_VERSION_MAJOR    3
// Summary: Minor Version Number
#define _XTLIB_VERSION_MINOR    1
// Summary: Prefix for dlls/libs
#define _XTLIB_FILE_PREFIX      "XT3100"

//////////////////////////////////////////////////////////////////////
// Evaluation version definitions:
//////////////////////////////////////////////////////////////////////

//#ifndef _XT_DEMOMODE
//#define _XT_DEMOMODE						// defined in the evaluation library
//#endif //_XT_DEMOMODE

//////////////////////////////////////////////////////////////////////

#endif  // __XTVERSION_H__
