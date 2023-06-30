////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   entityprototypelibrary.cpp
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EntityPrototypeLibrary.h"

#include "EntityPrototype.h"

//////////////////////////////////////////////////////////////////////////
// CEntityPrototypeLibrary implementation.
//////////////////////////////////////////////////////////////////////////
bool CEntityPrototypeLibrary::Save()
{
	CString filename = GetFilename();
	if (filename.IsEmpty())
	{
		return false;
	}

	XmlNodeRef root = new CXmlNode( "EntityPrototypeLibrary" );
	Serialize( root,false );
	bool bRes = root->saveToFile( GetFilename() );

	return bRes;
}

//////////////////////////////////////////////////////////////////////////
bool CEntityPrototypeLibrary::Load( const CString &filename )
{
	if (filename.IsEmpty())
		return false;
	SetFilename( filename );
	XmlParser parser;
	XmlNodeRef root = parser.parse( filename );
	if (!root)
		return false;

	Serialize( root,true );
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CEntityPrototypeLibrary::Serialize( XmlNodeRef &root,bool bLoading )
{
	if (bLoading)
	{
		// Loading.
		CString name = GetName();
		root->getAttr( "Name",name );
		SetName( name );
		for (int i = 0; i < root->getChildCount(); i++)
		{
			CEntityPrototype *prototype = new CEntityPrototype;
			AddItem( prototype );
			XmlNodeRef itemNode = root->getChild(i);
			CBaseLibraryItem::SerializeContext ctx( itemNode,bLoading );
			prototype->Serialize( ctx );
		}
		SetModified(false);
	}
	else
	{
		// Saving.
		root->setAttr( "Name",GetName() );
		root->setAttr( "SandboxVersion",(const char*)GetIEditor()->GetFileVersion().ToFullString() );
		// Serialize prototypes.
		for (int i = 0; i < GetItemCount(); i++)
		{
			XmlNodeRef itemNode = root->newChild( "EntityPrototype" );
			CBaseLibraryItem::SerializeContext ctx( itemNode,bLoading );
			GetItem(i)->Serialize( ctx );
		}
	}
}