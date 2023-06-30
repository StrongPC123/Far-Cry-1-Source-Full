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
#include "MaterialLibrary.h"
#include "Material.h"

//////////////////////////////////////////////////////////////////////////
// CMaterialLibrary implementation.
//////////////////////////////////////////////////////////////////////////
bool CMaterialLibrary::Save()
{
	CString filename = GetFilename();
	if (filename.IsEmpty())
	{
		return false;
	}

	XmlNodeRef root = new CXmlNode( "MaterialLibrary" );
	Serialize( root,false );
	bool bRes = root->saveToFile( GetFilename() );

	return bRes;
}

//////////////////////////////////////////////////////////////////////////
bool CMaterialLibrary::Load( const CString &filename )
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
void CMaterialLibrary::Serialize( XmlNodeRef &root,bool bLoading )
{
	if (bLoading)
	{
		// Loading.
		CString name = GetName();
		root->getAttr( "Name",name );
		SetName( name );
		for (int i = 0; i < root->getChildCount(); i++)
		{
			CMaterial *material = new CMaterial;
			AddItem( material );
			XmlNodeRef itemNode = root->getChild(i);
			CBaseLibraryItem::SerializeContext ctx( itemNode,bLoading );
			material->Serialize( ctx );
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
			CMaterial *pMtl = (CMaterial*)GetItem(i);
			// Save materials with parents under thier parent xml node.
			if (pMtl->GetParent())
				continue;

			XmlNodeRef itemNode = root->newChild( "Material" );
			CBaseLibraryItem::SerializeContext ctx( itemNode,bLoading );
			GetItem(i)->Serialize( ctx );
		}
	}
}