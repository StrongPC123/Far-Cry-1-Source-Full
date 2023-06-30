////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   XmlArchive.cpp
//  Version:     v1.00
//  Created:     30/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XmlArchive.h"
#include "PakFile.h"

//////////////////////////////////////////////////////////////////////////
// CXmlArchive
bool CXmlArchive::Load( const CString &file )
{
	bLoading = true;

	CFile cFile;
	if (!cFile.Open( file, CFile::modeRead))
	{
		CLogFile::FormatLine("Warning: Loading of %s failed",(const char*)file );
		return false;
	}
	CArchive ar(&cFile, CArchive::load);

	CString str;
	
	ar >> str;
	pNamedData->Serialize( ar );
			
	XmlParser parser;
	root = parser.parseBuffer( str );
	if (!root)
	{
		CLogFile::FormatLine("Warning: Loading of %s failed",(const char*)file );
	}

	if (root)
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CXmlArchive::Save( const CString &file )
{
	bLoading = false;
	if (!root)
		return;

	CFile cFile;
	// Open the file for writing, create it if needed
	if (!cFile.Open(file, CFile::modeCreate | CFile::modeWrite))
	{
		CLogFile::FormatLine("Warning: Saving of %s failed",(const char*)file );
		return;
	}
	// Create the archive object
	CArchive ar(&cFile, CArchive::store);

	CString xml = root->getXML();
	ar << xml;
	pNamedData->Serialize( ar );
}

//////////////////////////////////////////////////////////////////////////
bool CXmlArchive::SaveToPak( const CString &levelPath,CPakFile &pakFile )
{
	CString xml = root->getXML();
	// Save xml file.
	CString xmlFilename = "Level.editor_xml";
	pakFile.UpdateFile( xmlFilename,(void*)((const char*)xml),xml.GetLength() );

	pNamedData->Save( pakFile );
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlArchive::LoadFromPak( const CString &levelPath,CPakFile &pakFile )
{
	CString xmlFilename = levelPath + "Level.editor_xml";
	XmlParser parser;
	root = parser.parse( xmlFilename );
	if (!root)
		return false;

	pNamedData->Load( levelPath,pakFile );
	return true;
}
