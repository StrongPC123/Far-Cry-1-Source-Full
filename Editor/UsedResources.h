////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   UsedResources.h
//  Version:     v1.00
//  Created:     28/11/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __UsedResources_h__
#define __UsedResources_h__
#pragma once

class CErrorRecord;

//! Class passed to resource gathering functions
class CUsedResources
{
public:
	//////////////////////////////////////////////////////////////////////////
	CUsedResources();
	void Add( const CString &resourceFileName );

	//! Validate gathered resources.
	//! Reports warning if resource is not found.
	void Validate( CErrorReport *report );

public:
	typedef std::set<CString,stl::less_stricmp<CString> > ResourceFiles;
	ResourceFiles files;
};


#endif // __UsedResources_h__
