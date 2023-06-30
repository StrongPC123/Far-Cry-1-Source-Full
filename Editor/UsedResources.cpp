////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   UsedResources.cpp
//  Version:     v1.00
//  Created:     28/11/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UsedResources.h"
#include "ErrorReport.h"

#include <io.h>

//////////////////////////////////////////////////////////////////////////
CUsedResources::CUsedResources()
{
}

//////////////////////////////////////////////////////////////////////////
void CUsedResources::Add( const CString &resourceFileName )
{
	if (!resourceFileName.IsEmpty())
		files.insert( resourceFileName );
}

//////////////////////////////////////////////////////////////////////////
void CUsedResources::Validate( CErrorReport *report )
{
	_finddata_t fd;
	ICryPak *pPak = GetISystem()->GetIPak();
	// Validates that this file present here.
	for (ResourceFiles::iterator it = files.begin(); it != files.end(); ++it)
	{
		const CString &filename = *it;
		
		intptr_t fhandle = pPak->FindFirst( filename,&fd );
		if (fhandle != -1)
		{
			pPak->FindClose(fhandle);
		}
		else
		{
			// File not found.
			CErrorRecord err;
			err.error.Format( "Resource File %s not found,",(const char*)filename );
			err.severity = CErrorRecord::ESEVERITY_ERROR;
			err.flags |= CErrorRecord::FLAG_NOFILE;
			report->ReportError(err);
		}
	}
}
