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
#include "PrefabManager.h"

#include "PrefabItem.h"
#include "PrefabLibrary.h"

#include "GameEngine.h"
#include "GameExporter.h"

#include "DataBaseDialog.h"
#include "PrefabDialog.h"

#define PREFABS_LIBS_PATH "Editor\\Prefabs\\"

//////////////////////////////////////////////////////////////////////////
// CPrefabManager implementation.
//////////////////////////////////////////////////////////////////////////
CPrefabManager::CPrefabManager()
{
	m_libsPath = PREFABS_LIBS_PATH;

	m_pLevelLibrary = (CBaseLibrary*)AddLibrary( "Level" );
	m_pLevelLibrary->SetLevelLibrary( true );
}

//////////////////////////////////////////////////////////////////////////
CPrefabManager::~CPrefabManager()
{
}

//////////////////////////////////////////////////////////////////////////
void CPrefabManager::ClearAll()
{
	CBaseLibraryManager::ClearAll();

	m_pLevelLibrary = (CBaseLibrary*)AddLibrary( "Level" );
	m_pLevelLibrary->SetLevelLibrary( true );
}

//////////////////////////////////////////////////////////////////////////
CBaseLibraryItem* CPrefabManager::MakeNewItem()
{
	return new CPrefabItem;
}
//////////////////////////////////////////////////////////////////////////
CBaseLibrary* CPrefabManager::MakeNewLibrary()
{
	return new CPrefabLibrary(this);
}
//////////////////////////////////////////////////////////////////////////
CString CPrefabManager::GetRootNodeName()
{
	return "PrefabsLibrary";
}
//////////////////////////////////////////////////////////////////////////
CString CPrefabManager::GetLibsPath()
{
	return m_libsPath;
}

//////////////////////////////////////////////////////////////////////////
void CPrefabManager::Serialize( XmlNodeRef &node,bool bLoading )
{
	CBaseLibraryManager::Serialize( node,bLoading );
}

//////////////////////////////////////////////////////////////////////////
void CPrefabManager::Export( XmlNodeRef &node )
{
}

//////////////////////////////////////////////////////////////////////////
CPrefabItem* CPrefabManager::MakeFromSelection()
{
	CBaseLibraryDialog *dlg = GetIEditor()->OpenDataBaseLibrary( EDB_PREFAB_LIBRARY );
	if (dlg && dlg->IsKindOf(RUNTIME_CLASS(CPrefabDialog)))
	{
		CPrefabDialog *pPrefabDialog = (CPrefabDialog*)dlg;
		return pPrefabDialog->GetPrefabFromSelection();
	}
	return 0;
}
