////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   MaterialManager.cpp
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MaterialManager.h"

#include "Material.h"
#include "MaterialLibrary.h"
#include "ErrorReport.h"

#define MATERIALS_LIBS_PATH "Editor\\Materials\\"

//////////////////////////////////////////////////////////////////////////
// CMaterialManager implementation.
//////////////////////////////////////////////////////////////////////////
CMaterialManager::CMaterialManager()
{
	m_libsPath = MATERIALS_LIBS_PATH;

	m_pLevelLibrary = (CBaseLibrary*)AddLibrary( "Level" );
	m_pLevelLibrary->SetLevelLibrary( true );
}

//////////////////////////////////////////////////////////////////////////
CMaterialManager::~CMaterialManager()
{
}

//////////////////////////////////////////////////////////////////////////
void CMaterialManager::ClearAll()
{
	ZeroStruct(m_currentMaterialGUID);
	CBaseLibraryManager::ClearAll();

	m_pLevelLibrary = (CBaseLibrary*)AddLibrary( "Level" );
	m_pLevelLibrary->SetLevelLibrary( true );
}

//////////////////////////////////////////////////////////////////////////
CMaterial* CMaterialManager::LoadMaterial( CMaterialLibrary *pLibrary,XmlNodeRef &node,bool bNewGuid )
{
	assert( pLibrary );
	assert( node != NULL );

	CMaterial* material = new CMaterial;
	pLibrary->AddItem( material );

	CBaseLibraryItem::SerializeContext serCtx( node,true );
	serCtx.bCopyPaste = true;
	serCtx.bUniqName = true;
	material->Serialize( serCtx );
	if (bNewGuid)
		material->GenerateIdRecursively();

	return material;
}

//////////////////////////////////////////////////////////////////////////
void CMaterialManager::Export( XmlNodeRef &node )
{
	XmlNodeRef libs = node->newChild( "MaterialsLibrary" );
	for (int i = 0; i < GetLibraryCount(); i++)
	{
		IDataBaseLibrary* pLib = GetLibrary(i);
		// Level libraries are saved in in level.
		XmlNodeRef libNode = libs->newChild( "Library" );

		// Export library.
		libNode->setAttr( "Name",pLib->GetName() );
	}
}

//////////////////////////////////////////////////////////////////////////
int CMaterialManager::ExportLib( CMaterialLibrary *pLib,XmlNodeRef &libNode )
{
	int num = 0;
	// Export library.
	libNode->setAttr( "Name",pLib->GetName() );
	libNode->setAttr( "File",pLib->GetFilename() );
	libNode->setAttr( "SandboxVersion",(const char*)GetIEditor()->GetFileVersion().ToFullString() );
	// Serialize prototypes.
	for (int j = 0; j < pLib->GetItemCount(); j++)
	{
		CMaterial *pMtl = (CMaterial*)pLib->GetItem(j);
		// Only export parent materials.
		if (pMtl->GetParent())
			continue;

		// Do not export unused materials.
		if (!pMtl->IsUsed())
			continue;

		XmlNodeRef itemNode = libNode->newChild( "Material" );
		CBaseLibraryItem::SerializeContext ctx( itemNode,false );
		pMtl->Serialize( ctx );
		num += 1 + pMtl->GetSubMaterialCount();
	}
	return num;
}

//////////////////////////////////////////////////////////////////////////
void CMaterialManager::SetCurrentMaterial( CMaterial *pMtl )
{
	if (pMtl)
	{
		m_currentMaterialGUID = pMtl->GetGUID();
	}
	else
		ZeroStruct(m_currentMaterialGUID);
}

//////////////////////////////////////////////////////////////////////////
CMaterial* CMaterialManager::GetCurrentMaterial() const
{
	GUID nullGuid;
	ZeroStruct(nullGuid);
	if (m_currentMaterialGUID != nullGuid)
		return (CMaterial*)FindItem( m_currentMaterialGUID );
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CBaseLibraryItem* CMaterialManager::MakeNewItem()
{
	return new CMaterial;
}
//////////////////////////////////////////////////////////////////////////
CBaseLibrary* CMaterialManager::MakeNewLibrary()
{
	return new CMaterialLibrary(this);
}
//////////////////////////////////////////////////////////////////////////
CString CMaterialManager::GetRootNodeName()
{
	return "MaterialsLibs";
}
//////////////////////////////////////////////////////////////////////////
CString CMaterialManager::GetLibsPath()
{
	return m_libsPath;
}

//////////////////////////////////////////////////////////////////////////
void CMaterialManager::ReportDuplicateItem( CBaseLibraryItem *pItem,CBaseLibraryItem *pOldItem )
{
	CString sLibName;
	if (pOldItem->GetLibrary())
		sLibName = pOldItem->GetLibrary()->GetName();
	CErrorRecord err;
	err.pMaterial = (CMaterial*)pOldItem;
	err.error.Format( "Material %s with duplicate GUID to loaded material %s ignored",(const char*)pItem->GetFullName(),(const char*)pOldItem->GetFullName() );
	GetIEditor()->GetErrorReport()->ReportError( err );
}

//////////////////////////////////////////////////////////////////////////
void CMaterialManager::Serialize( XmlNodeRef &node,bool bLoading )
{
	CBaseLibraryManager::Serialize( node,bLoading );
	if (bLoading)
	{
		if (!FindLibrary("Shared"))
		{
			LoadLibrary( GetLibsPath() + "\\Shared.xml" );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialManager::OnNewDocument()
{
	CBaseLibraryManager::OnNewDocument();
	SetCurrentMaterial( 0 );
	if (!FindLibrary("Shared"))
	{
		LoadLibrary( GetLibsPath() + "\\Shared.xml" );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialManager::OnLoadDocument()
{
	CBaseLibraryManager::OnLoadDocument();
	SetCurrentMaterial( 0 );
	if (!FindLibrary("Shared"))
	{
		LoadLibrary( GetLibsPath() + "\\Shared.xml" );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialManager::OnCloseDocument()
{
	CBaseLibraryManager::OnCloseDocument();
	SetCurrentMaterial( 0 );
}

//////////////////////////////////////////////////////////////////////////
void CMaterialManager::OnMissionChange()
{
	CBaseLibraryManager::OnMissionChange();
	SetCurrentMaterial( 0 );
}
