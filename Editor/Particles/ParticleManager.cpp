////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   particlemanager.cpp
//  Version:     v1.00
//  Created:     17/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ParticleManager.h"

#include "ParticleItem.h"
#include "ParticleLibrary.h"

#include "GameEngine.h"
#include "GameExporter.h"

#define PARICLES_LIBS_PATH "Editor\\Particles\\"

//////////////////////////////////////////////////////////////////////////
// CParticleManager implementation.
//////////////////////////////////////////////////////////////////////////
CParticleManager::CParticleManager()
{
	m_libsPath = PARICLES_LIBS_PATH;

	m_pLevelLibrary = (CBaseLibrary*)AddLibrary( "Level" );
	m_pLevelLibrary->SetLevelLibrary( true );
}

//////////////////////////////////////////////////////////////////////////
CParticleManager::~CParticleManager()
{
}

//////////////////////////////////////////////////////////////////////////
void CParticleManager::ClearAll()
{
	CBaseLibraryManager::ClearAll();

	m_pLevelLibrary = (CBaseLibrary*)AddLibrary( "Level" );
	m_pLevelLibrary->SetLevelLibrary( true );
}

//////////////////////////////////////////////////////////////////////////
CBaseLibraryItem* CParticleManager::MakeNewItem()
{
	return new CParticleItem;
}
//////////////////////////////////////////////////////////////////////////
CBaseLibrary* CParticleManager::MakeNewLibrary()
{
	return new CParticleLibrary(this);
}
//////////////////////////////////////////////////////////////////////////
CString CParticleManager::GetRootNodeName()
{
	return "ParticleLibs";
}
//////////////////////////////////////////////////////////////////////////
CString CParticleManager::GetLibsPath()
{
	return m_libsPath;
}

//////////////////////////////////////////////////////////////////////////
void CParticleManager::Serialize( XmlNodeRef &node,bool bLoading )
{
	CBaseLibraryManager::Serialize( node,bLoading );
}

//////////////////////////////////////////////////////////////////////////
void CParticleManager::Export( XmlNodeRef &node )
{
	XmlNodeRef libs = node->newChild( "ParticlesLibrary" );
	for (int i = 0; i < GetLibraryCount(); i++)
	{
		IDataBaseLibrary* pLib = GetLibrary(i);
		if (pLib->IsLevelLibrary())
			continue;
		// Level libraries are saved in in level.
		XmlNodeRef libNode = libs->newChild( "Library" );
		libNode->setAttr( "Name",pLib->GetName() );
	}	
}

//////////////////////////////////////////////////////////////////////////
void CParticleManager::PasteToParticleItem( CParticleItem* pItem,XmlNodeRef &node,bool bWithChilds )
{
	assert( pItem );
	assert( node != NULL );

	CBaseLibraryItem::SerializeContext serCtx( node,true );
	serCtx.bCopyPaste = true;
	serCtx.bIgnoreChilds = !bWithChilds;
	pItem->Serialize( serCtx );
	pItem->GenerateIdRecursively();

	for (int i = 0; i < pItem->GetChildCount(); i++)
	{
		CParticleItem *pChildItem = pItem->GetChild(i);
		pChildItem->SetName( MakeUniqItemName(pChildItem->GetName()) );
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleManager::DeleteItem( CBaseLibraryItem* pItem )
{
	CParticleItem *pPartItem = ((CParticleItem*)pItem);
	if (pPartItem->GetParent())
		pPartItem->GetParent()->RemoveChild(pPartItem);

	CBaseLibraryManager::DeleteItem( pItem );
}
