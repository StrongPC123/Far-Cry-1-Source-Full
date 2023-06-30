////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PrefabLibrary.cpp
//  Version:     v1.00
//  Created:     10/11/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PrefabLibrary.h"
#include "PrefabItem.h"

//////////////////////////////////////////////////////////////////////////
// CPrefabLibrary implementation.
//////////////////////////////////////////////////////////////////////////
bool CPrefabLibrary::Save()
{
	XmlNodeRef root = new CXmlNode( "PrefabsLibrary" );
	Serialize( root,false );
	bool bRes = root->saveToFile( GetFilename() );
	return bRes;
}

//////////////////////////////////////////////////////////////////////////
bool CPrefabLibrary::Load( const CString &filename )
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
void CPrefabLibrary::Serialize( XmlNodeRef &root,bool bLoading )
{
	if (bLoading)
	{
		// Loading.
		CString name = GetName();
		root->getAttr( "Name",name );
		SetName( name );
		for (int i = 0; i < root->getChildCount(); i++)
		{
			XmlNodeRef itemNode = root->getChild(i);
			// Only accept nodes with correct name.
			if (stricmp(itemNode->getTag(),"Prefab") != 0)
				continue;
			CBaseLibraryItem *pItem = new CPrefabItem;
			AddItem( pItem );

			CBaseLibraryItem::SerializeContext ctx( itemNode,bLoading );
			pItem->Serialize( ctx );
		}
		SetModified(false);
	}
	else
	{
		// Saving.
		root->setAttr( "Name",GetName() );
		// Serialize prototypes.
		for (int i = 0; i < GetItemCount(); i++)
		{
			XmlNodeRef itemNode = root->newChild( "Prefab" );
			CBaseLibraryItem::SerializeContext ctx( itemNode,bLoading );
			GetItem(i)->Serialize( ctx );
		}
	}
}