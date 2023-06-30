////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   fileutil.h
//  Version:     v1.00
//  Created:     5/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __fileutil_h__
#define __fileutil_h__
#pragma once

namespace FileUtil
{
	// Find all files matching filespec.
	bool ScanDirectory( const CString &path,const CString &filespec,std::vector<CString> &files,bool recursive );
	// converts the FILETIME to the C Timestamp (compatible with dbghelp.dll)
	DWORD FiletimeToUnixTime(const FILETIME& ft);
	// converts the C Timestamp (compatible with dbghelp.dll) to FILETIME
	FILETIME UnixTimeToFiletime(DWORD nCTime);
};

#endif // __fileutil_h__
