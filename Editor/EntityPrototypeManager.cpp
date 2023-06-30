////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   entityprototypemanager.cpp
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EntityPrototypeManager.h"

#include "EntityPrototype.h"
#include "EntityPrototypeLibrary.h"

#define ENTITY_LIBS_PATH "Editor\\EntityLibrary\\"

//////////////////////////////////////////////////////////////////////////
// CEntityPrototypeManager implementation.
//////////////////////////////////////////////////////////////////////////
CEntityPrototypeManager::CEntityPrototypeManager()
{
	m_libsPath = ENTITY_LIBS_PATH;

	m_pLevelLibrary = (CBaseLibrary*)AddLibrary( "Level" );
	m_pLevelLibrary->SetLevelLibrary( true );
}

//////////////////////////////////////////////////////////////////////////
CEntityPrototypeManager::~CEntityPrototypeManager()
{
}

//////////////////////////////////////////////////////////////////////////
void CEntityPrototypeManager::ClearAll()
{
	CBaseLibraryManager::ClearAll();

	m_pLevelLibrary = (CBaseLibrary*)AddLibrary( "Level" );
	m_pLevelLibrary->SetLevelLibrary( true );
}

//////////////////////////////////////////////////////////////////////////
CEntityPrototype* CEntityPrototypeManager::LoadPrototype( CEntityPrototypeLibrary *pLibrary,XmlNodeRef &node )
{
	assert( pLibrary );
	assert( node != NULL );

	CBaseLibraryItem::SerializeContext ctx(node,true);
	ctx.bCopyPaste = true;

	CEntityPrototype* prototype = new CEntityPrototype;
	pLibrary->AddItem( prototype );
	prototype->Serialize( ctx );
	return prototype;
}

//////////////////////////////////////////////////////////////////////////
CBaseLibraryItem* CEntityPrototypeManager::MakeNewItem()
{
	return new CEntityPrototype;
}
//////////////////////////////////////////////////////////////////////////
CBaseLibrary* CEntityPrototypeManager::MakeNewLibrary()
{
	return new CEntityPrototypeLibrary(this);
}
//////////////////////////////////////////////////////////////////////////
CString CEntityPrototypeManager::GetRootNodeName()
{
	return "EntityPrototypesLibs";
}
//////////////////////////////////////////////////////////////////////////
CString CEntityPrototypeManager::GetLibsPath()
{
	return m_libsPath;
}