////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   baselibrarymanager.cpp
//  Version:     v1.00
//  Created:     10/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BaseLibraryManager.h"

#include "BaseLibrary.h"
#include "BaseLibraryItem.h"
#include "ErrorReport.h"

//////////////////////////////////////////////////////////////////////////
// CBaseLibraryManager implementation.
//////////////////////////////////////////////////////////////////////////
CBaseLibraryManager::CBaseLibraryManager()
{
}

//////////////////////////////////////////////////////////////////////////
CBaseLibraryManager::~CBaseLibraryManager()
{
	ClearAll();
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::ClearAll()
{
	// Delete all items from all libraries.
	for (int i = 0; i < m_libs.size(); i++)
	{
		m_libs[i]->RemoveAllItems();
	}

	m_itemsMap.clear();
	m_libs.clear();
}

//////////////////////////////////////////////////////////////////////////
IDataBaseLibrary* CBaseLibraryManager::FindLibrary( const CString &library )
{
	for (int i = 0; i < m_libs.size(); i++)
	{
		if (stricmp(library,m_libs[i]->GetName()) == 0)
		{
			return m_libs[i];
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
IDataBaseItem* CBaseLibraryManager::FindItem( REFGUID guid ) const
{
	CBaseLibraryItem* pMtl = stl::find_in_map( m_itemsMap,guid,(CBaseLibraryItem*)0 );
	return pMtl;
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::SplitFullItemName( const CString &fullItemName,CString &libraryName,CString &itemName )
{
	int p;
	p = fullItemName.Find( '.' );
	if (p < 0)
	{
		libraryName = "";
		itemName = fullItemName;
		return;
	}
	libraryName = fullItemName.Mid(0,p);
	itemName = fullItemName.Mid(p+1);
}

//////////////////////////////////////////////////////////////////////////
IDataBaseItem* CBaseLibraryManager::FindItemByName( const CString &fullItemName )
{
	CString libraryName,itemName;
	SplitFullItemName( fullItemName,libraryName,itemName );
	if (libraryName.IsEmpty())
	{
		Error( _T("Cannot Find Item, Library name must be specified before \".\" : %s"),(const char*)fullItemName );
		return 0;
	}
	IDataBaseLibrary *lib = FindLibrary( libraryName );
	if (!lib)
	{
		Error( _T("Cannot Find Library: %s"),(const char*)libraryName );
		return 0;
	}
	return lib->FindItem( itemName );
}

//////////////////////////////////////////////////////////////////////////
IDataBaseItem* CBaseLibraryManager::CreateItem( IDataBaseLibrary *pLibrary )
{
	assert( pLibrary );

	// Add item to this library.
	TSmartPtr<CBaseLibraryItem> pItem = MakeNewItem();
	pLibrary->AddItem( pItem );
	pItem->GenerateId();
	return pItem;
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::DeleteItem( IDataBaseItem* pItem )
{
	assert( pItem );

	m_itemsMap.erase( pItem->GetGUID() );
	if (pItem->GetLibrary())
	{
		pItem->GetLibrary()->RemoveItem( pItem );
	}

	// Delete all objects from object manager that have 
	//GetIEditor()->GetObjectManager()->GetObjects( objects );
}

//////////////////////////////////////////////////////////////////////////
IDataBaseLibrary* CBaseLibraryManager::LoadLibrary( const CString &filename )
{
	// If library is already loaded ignore it.
	for (int i = 0; i < m_libs.size(); i++)
	{
		if (stricmp(filename,m_libs[i]->GetFilename()) == 0)
		{
			Error( _T("Loading Duplicate Library: %s"),(const char*)filename );
			return 0;
		}
	}

	TSmartPtr<CBaseLibrary> pLib = MakeNewLibrary();
	if (!pLib->Load( filename ))
	{
		Error( _T("Failed to Load Item Library: %s"),(const char*)filename );
		return 0;
	}
	if (FindLibrary(pLib->GetName()) != 0)
	{
		Error( _T("Loading Duplicate Library: %s"),(const char*)pLib->GetName() );
		return 0;
	}
	m_libs.push_back( pLib );
	return pLib;
}

//////////////////////////////////////////////////////////////////////////
IDataBaseLibrary* CBaseLibraryManager::AddLibrary( const CString &library )
{
	// Check if library with same name already exist.
	IDataBaseLibrary* pBaseLib = FindLibrary(library);
	if (pBaseLib)
		return pBaseLib;

	CBaseLibrary *lib = MakeNewLibrary();
	lib->SetName( library );

	// Set filename of this library.
	// Make a filename from name of library.
	CString filename = library;
	filename.Replace( ' ','_' );
	filename = GetLibsPath() + filename + ".xml";
	lib->SetFilename( filename );

	m_libs.push_back( lib );
	return lib;
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::DeleteLibrary( const CString &library )
{
	for (int i = 0; i < m_libs.size(); i++)
	{
		if (stricmp(library,m_libs[i]->GetName()) == 0)
		{
			CBaseLibrary *pLibrary = m_libs[i];
			// Check if not level library, they cannot be deleted.
			if (!pLibrary->IsLevelLibrary())
			{
				for (int j = 0; j < pLibrary->GetItemCount(); j++)
				{
					UnregisterItem( (CBaseLibraryItem*)pLibrary->GetItem(j) );
				}
				m_libs.erase( m_libs.begin() + i );
			}
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
IDataBaseLibrary* CBaseLibraryManager::GetLibrary( int index ) const
{
	assert( index >= 0 && index < m_libs.size() );
	return m_libs[index];
};

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::SaveAllLibs()
{
	for (int i = 0; i < GetLibraryCount(); i++)
	{
		// Check if library is modified.
		IDataBaseLibrary *pLibrary = GetLibrary(i);
		if (pLibrary->IsLevelLibrary())
			continue;
		if (pLibrary->IsModified())
		{
			if (pLibrary->Save())
			{
				pLibrary->SetModified(false);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::Serialize( XmlNodeRef &node,bool bLoading )
{
	CString rootNodeName = GetRootNodeName();
	if (bLoading)
	{
		CString libsPath = GetLibsPath();
		if (!libsPath.IsEmpty())
			CFileUtil::CreateDirectory( libsPath );

		XmlNodeRef libs = node->findChild( rootNodeName );
		if (libs)
		{
			for (int i = 0; i < libs->getChildCount(); i++)
			{
				// Load only library name.
				XmlNodeRef libNode = libs->getChild(i);
				if (strcmp(libNode->getTag(),"LevelLibrary") == 0)
				{
					m_pLevelLibrary->Serialize( libNode,bLoading );
				}
				else
				{
					CString libName;
					if (libNode->getAttr( "Name",libName ))
					{
						// Load this library.
						CString filename = libName;
						filename.Replace( ' ','_' );
						filename = GetLibsPath() + filename + ".xml";
						if (!FindLibrary(libName))
						{
							LoadLibrary( filename );
						}
					}
				}
			}
		}
	}
	else
	{
		// Save all libraries.
		XmlNodeRef libs = node->newChild( rootNodeName );
		for (int i = 0; i < GetLibraryCount(); i++)
		{
			IDataBaseLibrary* pLib = GetLibrary(i);
			if (pLib->IsLevelLibrary())
			{
				// Level libraries are saved in in level.
				XmlNodeRef libNode = libs->newChild( "LevelLibrary" );
				pLib->Serialize( libNode,bLoading );
			}
			else
			{
				// Save only library name.
				XmlNodeRef libNode = libs->newChild( "Library" );
				libNode->setAttr( "Name",pLib->GetName() );
			}
		}
		SaveAllLibs();
	}
}

//////////////////////////////////////////////////////////////////////////
CString CBaseLibraryManager::MakeUniqItemName( const CString &srcName )
{
	// Remove all numbers from the end of name.
	CString typeName = srcName;
	int len = typeName.GetLength();
	while (len > 0 && isdigit(typeName[len-1]))
		len--;

	typeName = typeName.Left(len);

	CString tpName = typeName;
	int num = 0;
	
	for (ItemsMap::iterator it = m_itemsMap.begin(); it != m_itemsMap.end(); ++it)
	{
		CBaseLibraryItem *pItem = it->second;
		const char *name = pItem->GetName();
		if (strncmp(name,tpName,len) == 0)
		{
			int n = atoi(name+len) + 1;
			num = MAX( num,n );
		}
	}
	CString str;
	str.Format( "%s%d",(const char*)typeName,num );
	return str;
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::Validate()
{
	for (ItemsMap::iterator it = m_itemsMap.begin(); it != m_itemsMap.end(); ++it)
	{
		it->second->Validate();
	}
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::RegisterItem( CBaseLibraryItem *pItem,REFGUID newGuid )
{
	assert(pItem);
	bool bNewItem = true;
	REFGUID oldGuid = pItem->GetGUID();
	if (!GuidUtil::IsEmpty(oldGuid))
	{
		bNewItem = false;
		m_itemsMap.erase( oldGuid );
	}
	if (GuidUtil::IsEmpty(newGuid))
		return;
	CBaseLibraryItem *pOldItem = stl::find_in_map( m_itemsMap,newGuid,(CBaseLibraryItem*)0 );
	if (!pOldItem)
	{
		pItem->m_guid = newGuid;
		m_itemsMap[newGuid] = pItem;
	}
	else
	{
		if (pOldItem != pItem)
		{
			ReportDuplicateItem( pItem,pOldItem );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::RegisterItem( CBaseLibraryItem *pItem )
{
	assert(pItem);
	if (GuidUtil::IsEmpty(pItem->GetGUID()))
		return;
	CBaseLibraryItem *pOldItem = stl::find_in_map( m_itemsMap,pItem->GetGUID(),(CBaseLibraryItem*)0 );
	if (!pOldItem)
	{
		m_itemsMap[pItem->GetGUID()] = pItem;
	}
	else
	{
		if (pOldItem != pItem)
		{
			ReportDuplicateItem( pItem,pOldItem );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::ReportDuplicateItem( CBaseLibraryItem *pItem,CBaseLibraryItem *pOldItem )
{
	CString sLibName;
	if (pOldItem->GetLibrary())
		sLibName = pOldItem->GetLibrary()->GetName();
	CErrorRecord err;
	err.error.Format( "Item %s with duplicate GUID to loaded item %s ignored",(const char*)pItem->GetFullName(),(const char*)pOldItem->GetFullName() );
	GetIEditor()->GetErrorReport()->ReportError( err );
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::UnregisterItem( CBaseLibraryItem *pItem )
{
	m_itemsMap.erase( pItem->GetGUID() );
}

//////////////////////////////////////////////////////////////////////////
CString CBaseLibraryManager::MakeFullItemName( IDataBaseLibrary *pLibrary,const CString &group,const CString &itemName )
{
	assert(pLibrary);
	CString name = pLibrary->GetName() + ".";
	if (!group.IsEmpty())
		name += group + ".";
	name += itemName;
	return name;
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::GatherUsedResources( CUsedResources &resources )
{
	for (int lib = 0; lib < GetLibraryCount(); lib++)
	{
		IDataBaseLibrary *pLib = GetLibrary(lib);
		for (int i = 0; i < pLib->GetItemCount(); i++)
		{
			pLib->GetItem(i)->GatherUsedResources( resources );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::OnNewDocument()
{
	ClearAll();
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::OnLoadDocument()
{
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::OnCloseDocument()
{
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryManager::OnMissionChange()
{
}
