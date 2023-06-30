////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   fileutil.cpp
//  Version:     v1.00
//  Created:     5/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "FileUtil.h"

#include <io.h>

//////////////////////////////////////////////////////////////////////////
// the paths must have trailing slash
static bool ScanDirectoryRecursive( const CString &root,const CString &path,const CString &file,std::vector<CString> &files, bool recursive )
{
	__finddata64_t c_file;
	intptr_t hFile;

	bool anyFound = false;

	CString fullPath = root + path + file;
	if ( (hFile = _findfirst64( fullPath.GetString(), &c_file )) != -1L )
	{
		// Find the rest of the files.
		do {
			if (!(c_file.attrib & _A_SUBDIR))
			{
				anyFound = true;
				files.push_back( path + c_file.name );
			}
		}	while (_findnext64( hFile, &c_file ) == 0);
		_findclose( hFile );
	}

	if (recursive)
	{
		fullPath = root + path + "*.*";
		if( (hFile = _findfirst64( fullPath.GetString(), &c_file )) != -1L )
		{
			// Find directories.
			do {
				if (c_file.attrib & _A_SUBDIR)
				{
					// If recursive.
					if (c_file.name[0] != '.')
					{
						if (ScanDirectoryRecursive( root,path + c_file.name + "\\",file,files,recursive ))
							anyFound = true;
					}
				}
			}	while (_findnext64( hFile, &c_file ) == 0);
			_findclose( hFile );
		}
	}

	return anyFound;
}

//////////////////////////////////////////////////////////////////////////
bool FileUtil::ScanDirectory( const CString &path,const CString &file,std::vector<CString> &files, bool recursive )
{
	return ScanDirectoryRecursive(path,"",file,files,recursive );
}


//	Magic number explanation:
//	Both epochs are Gregorian. 1970 - 1601 = 369. Assuming a leap
//	year every four years, 369 / 4 = 92. However, 1700, 1800, and 1900
//	were NOT leap years, so 89 leap years, 280 non-leap years.
//	89 * 366 + 280 * 365 = 134744 days between epochs. Of course
//	60 * 60 * 24 = 86400 seconds per day, so 134744 * 86400 =
//	11644473600 = SECS_BETWEEN_EPOCHS.
//
//	This result is also confirmed in the MSDN documentation on how
//	to convert a time_t value to a win32 FILETIME.
static const __int64 SECS_BETWEEN_EPOCHS = 11644473600;
static const __int64 SECS_TO_100NS = 10000000; /* 10^7 */

// converts the FILETIME to the C Timestamp (compatible with dbghelp.dll)
DWORD FileUtil::FiletimeToUnixTime(const FILETIME& ft)
{
	return (DWORD)((((__int64&)ft) / SECS_TO_100NS) - SECS_BETWEEN_EPOCHS);
}

// converts the C Timestamp (compatible with dbghelp.dll) to FILETIME
FILETIME FileUtil::UnixTimeToFiletime(DWORD nCTime)
{
	__int64 time = (nCTime + SECS_BETWEEN_EPOCHS) * SECS_TO_100NS;
	return (FILETIME&)time;
}
