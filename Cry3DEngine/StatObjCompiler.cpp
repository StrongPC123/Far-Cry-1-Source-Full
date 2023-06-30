////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjconstr.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: loading
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "MeshIdx.h"
#include "irenderer.h"

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

bool CStatObj::CompileInNeeded()
{
	if(!GetCVars()->e_ccgf_make_if_not_found)
		return true;

#ifdef WIN32

	char szCompiledFileName[MAX_PATH_LENGTH];
	MakeCompiledFileName(szCompiledFileName,MAX_PATH_LENGTH);

	CCGFHeader fileHeader;
	// read header from compiled first
	FILE * f = GetSystem()->GetIPak()->FOpen(szCompiledFileName, "rb");
	if(f)
	{
		int nReaded = GetSystem()->GetIPak()->FRead(&fileHeader, 1, sizeof(fileHeader), f);
		assert(nReaded == sizeof(CCGFHeader));
		GetSystem()->GetIPak()->FClose(f);
	}

	// get date of source file
	FILE* fTmp = GetSystem()->GetIPak()->FOpen(m_szFileName, "rb");
	if (!fTmp)
		return false;

	FILETIME ftSourceFileTime = GetSystem()->GetIPak()->GetModificationTime(fTmp);
	GetSystem()->GetIPak()->FClose(fTmp);
	if(ftSourceFileTime.dwHighDateTime == 0 && ftSourceFileTime.dwLowDateTime == 0)
		return false; // source file not found

#ifdef _DEBUG
	HANDLE _dbg_h = CreateFile (m_szFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	FILETIME _dbg_ft = {0,0};
	if (_dbg_h != INVALID_HANDLE_VALUE)
		GetFileTime (_dbg_h, &_dbg_ft, NULL, &_dbg_ft);

	SYSTEMTIME _dbg_st1, _dbg_st2;
	FileTimeToSystemTime(&fileHeader.SourceFileTime, &_dbg_st1);
	FileTimeToSystemTime(&ftSourceFileTime, &_dbg_st2);
#endif

	// compare date of source file and date of source file used to build this ccgf
	if(!CompareFileTime(&fileHeader.SourceFileTime, &ftSourceFileTime))
		if(!strcmp(fileHeader.szVersion,CCGF_FILE_VERSION))
			if(fileHeader.nStructuresCheckSummm == CCGFHeader::GetStructuresCheckSummm())
		{
			if(!fileHeader.nDataSize)
			{
				assert(m_szGeomName[0]);
				return false; // geom name was specified but not found in source sgf during compilation (valid state)
			}

			return true; // no recompilation required
		}

		// make command for execution
		char szRemoteCmdLine[512];
		snprintf(szRemoteCmdLine, sizeof(szRemoteCmdLine), 
			RC_EXECUTABLE " \"%s\" /GeomName=%s /Stripify=%d /LoadAdditinalInfo=%d /KeepInLocalSpace=%d /StaticCGF=1 /refresh",
			m_szFileName, m_szGeomName,
			int(m_eVertsSharing==evs_ShareAndSortForCache), int(m_bLoadAdditinalInfo), int(m_bKeepInLocalSpace)
			);

		STARTUPINFO si;
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		si.dwX = 100;
		si.dwY = 100;
		si.dwFlags = STARTF_USEPOSITION;

		PROCESS_INFORMATION pi;
		ZeroMemory( &pi, sizeof(pi) );

		// Start the child process. 
		GetLog()->UpdateLoadingScreen("Executing " RC_EXECUTABLE " for %s", m_szFileName);
		GetLog()->Log("\004Command line: %s", szRemoteCmdLine);

		if( !CreateProcess( NULL, // No module name (use command line). 
			szRemoteCmdLine,				// Command line. 
			NULL,             // Process handle not inheritable. 
			NULL,             // Thread handle not inheritable. 
			FALSE,            // Set handle inheritance to FALSE. 
			GetCVars()->e_ccgf_make_if_not_found == 2 ? 0 : CREATE_NO_WINDOW, // No creation flags. 
			NULL,             // Use parent's environment block. 
			NULL/*szFolderName*/,     // Set starting directory. 
			&si,              // Pointer to STARTUPINFO structure.
			&pi )             // Pointer to PROCESS_INFORMATION structure.
			) 
		{
			GetSystem()->Error( "CreateProcess failed: %s", szRemoteCmdLine);
		}

		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );

		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

		// check compiled file
		f = GetSystem()->GetIPak()->FOpen(szCompiledFileName, "rb");
		if(f)
		{
			int nReaded = GetSystem()->GetIPak()->FRead(&fileHeader, 1, sizeof(fileHeader), f);
			GetSystem()->GetIPak()->FClose(f);
			if(nReaded == sizeof(CCGFHeader))
			{
				if(	ftSourceFileTime.dwHighDateTime == fileHeader.SourceFileTime.dwHighDateTime &&
						ftSourceFileTime.dwLowDateTime	== fileHeader.SourceFileTime.dwLowDateTime)
				{
					if(strcmp(fileHeader.szVersion,CCGF_FILE_VERSION))
						GetSystem()->Error(" " RC_EXECUTABLE " version error\n"
							"File produced by resource compiler has wrong version [%s]\n"
							"Must be [%s]\n"
							"File name is %s", 
							fileHeader.szVersion, CCGF_FILE_VERSION, szCompiledFileName);
						
					return true;
				}
				else
				{
					GetSystem()->Error(" " RC_EXECUTABLE " execution error\n"
						"File produced by resource compiler has time stamp different from source file time\n"
						"File name is %s", szCompiledFileName);
				}
			}
		}
		else
			GetSystem()->Error(" " RC_EXECUTABLE " execution error\n"
			"Resource compiler was not able to produce CCGF file\n"
			"File name is %s", szCompiledFileName);

		return false;

#endif // WIN32
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif
