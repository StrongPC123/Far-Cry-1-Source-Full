////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   Win32specific.h
//  Version:     v1.00
//  Created:     31/03/2003 by Sergiy.
//  Compilers:   Visual Studio.NET
//  Description: Specific to Win32 declarations, inline functions etc.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CRY_COMMON_XBOX_SPECIFIC_HDR_
#define _CRY_COMMON_XBOX_SPECIFIC_HDR_

#ifdef __cplusplus
inline int IsHeapValid ()
{
	return true;
}
#endif
#endif