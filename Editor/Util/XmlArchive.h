////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   XmlArchive.h
//  Version:     v1.00
//  Created:     30/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Stores XML in MFC archive.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __XmlArchive_h__
#define __XmlArchive_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "NamedData.h"

class CPakFile;
/*!
 *	CXmlArcive used to stores XML in MFC archive.
 */
class CXmlArchive
{
public:
	XmlNodeRef	root;
	CNamedData* pNamedData;
	bool				bLoading;
	bool				bOwnNamedData;

	CXmlArchive() {
		bLoading = false;
		bOwnNamedData= true;
		pNamedData = new CNamedData;
	};
	explicit CXmlArchive( const CString &xmlRoot ) {
		bLoading = false;
		bOwnNamedData= true;
		pNamedData = new CNamedData;
		root = new CXmlNode(xmlRoot);
	};
	~CXmlArchive() {
		if (bOwnNamedData)
			delete pNamedData;
	};
	CXmlArchive( const CXmlArchive &ar ) { *this = ar; }
	CXmlArchive& operator=( const CXmlArchive &ar )
	{
		root = ar.root;
		pNamedData = ar.pNamedData;
		bLoading = ar.bLoading;
		bOwnNamedData = false;
    return *this;
	}
	
	bool Load( const CString &file );
	void Save( const CString &file );

	//! Save XML Archive to pak file.
	//! @return true if saved.
	bool SaveToPak( const CString &levelPath,CPakFile &pakFile );
	bool LoadFromPak( const CString &levelPath,CPakFile &pakFile );
};


#endif // __XmlArchive_h__
