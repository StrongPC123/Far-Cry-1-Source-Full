////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   baselibrary.cpp
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BaseLibrary.h"
#include "BaseLibraryItem.h"
#include "BaseLibraryManager.h"

//////////////////////////////////////////////////////////////////////////
// CBaseLibrary implementation.
//////////////////////////////////////////////////////////////////////////
CBaseLibrary::CBaseLibrary( CBaseLibraryManager *pManager )
{
	m_pManager = pManager;
	m_bModified = false;
	m_bLevelLib = false;
}
	
//////////////////////////////////////////////////////////////////////////
CBaseLibrary::~CBaseLibrary()
{
	m_items.clear();
}

//////////////////////////////////////////////////////////////////////////
IDataBaseManager* CBaseLibrary::GetManager()
{
	return m_pManager;
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibrary::RemoveAllItems()
{
	AddRef();
	for (int i = 0; i < m_items.size(); i++)
	{
		// Clear library item.
		m_items[i]->m_library = NULL;
	}
	m_items.clear();
	Release();
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibrary::SetName( const CString &name )
{
	m_name = name;
	
	/*
	// Make a file name from name of library.
	CString path = Path::GetPath( m_filename );
	m_filename = m_name;
	m_filename.Replace( ' ','_' );
	m_filename = path + m_filename + ".xml";
	*/
	SetModified();
}

//////////////////////////////////////////////////////////////////////////
const CString& CBaseLibrary::GetName() const
{
	return m_name;
}

//////////////////////////////////////////////////////////////////////////
bool CBaseLibrary::Save()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBaseLibrary::Load( const CString &filename )
{
	m_filename = filename;
	SetModified(false);
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibrary::SetModified( bool bModified )
{
	m_bModified = bModified;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CBaseLibrary::AddItem( IDataBaseItem* item )
{
	CBaseLibraryItem *pLibItem = (CBaseLibraryItem*)item;
	// Check if item is already assigned to this library.
	if (pLibItem->m_library != this)
	{
		pLibItem->m_library = this;
		m_items.push_back( pLibItem );
		SetModified();
		m_pManager->RegisterItem( pLibItem );
	}
}

//////////////////////////////////////////////////////////////////////////
IDataBaseItem* CBaseLibrary::GetItem( int index )
{
	assert( index >= 0 && index < m_items.size() );
	return m_items[index];
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibrary::RemoveItem( IDataBaseItem* item )
{
	for (int i = 0; i < m_items.size(); i++)
	{
		if (m_items[i] == item)
		{
			m_items.erase( m_items.begin()+i );
			SetModified();
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
IDataBaseItem* CBaseLibrary::FindItem( const CString &name )
{
	for (int i = 0; i < m_items.size(); i++)
	{
		if (stricmp(m_items[i]->GetName(),name) == 0)
		{
			return m_items[i];
		}
	}
	return NULL;
}