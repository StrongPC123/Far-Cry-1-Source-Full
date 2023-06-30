////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   baselibraryitem.cpp
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BaseLibraryItem.h"
#include "BaseLibrary.h"
#include "BaseLibraryManager.h"

//////////////////////////////////////////////////////////////////////////
// CBaseLibraryItem implementation.
//////////////////////////////////////////////////////////////////////////
CBaseLibraryItem::CBaseLibraryItem()
{
	m_library = 0;
	GenerateId();
}

CBaseLibraryItem::~CBaseLibraryItem()
{
}

//////////////////////////////////////////////////////////////////////////
CString CBaseLibraryItem::GetFullName() const
{
	CString name;
	if (m_library)
		name = m_library->GetName() + ".";
	name += m_name;
	return name;
}

//////////////////////////////////////////////////////////////////////////
CString CBaseLibraryItem::GetGroupName()
{
	CString str = GetName();
	int p = str.Find('.');
	if (p >= 0)
	{
		return str.Mid(0,p);
	}
	return "";
}

//////////////////////////////////////////////////////////////////////////
CString CBaseLibraryItem::GetShortName()
{
	CString str = GetName();
	int p = str.Find('.');
	if (p >= 0)
	{
		return str.Mid(p+1);
	}
	return str;
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryItem::SetName( const CString &name )
{
	m_name = name;
}

//////////////////////////////////////////////////////////////////////////
const CString& CBaseLibraryItem::GetName() const
{
	return m_name;
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryItem::GenerateId()
{
	GUID guid;
	CoCreateGuid( &guid );
	SetGUID(guid);
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryItem::SetGUID( REFGUID guid )
{
	if (m_library)
	{
		((CBaseLibraryManager*)m_library->GetManager())->RegisterItem( this,guid );
	}
	m_guid = guid;
}

//////////////////////////////////////////////////////////////////////////
void CBaseLibraryItem::Serialize( SerializeContext &ctx )
{
	//assert(m_library);
	
	XmlNodeRef node = ctx.node;
	if (ctx.bLoading)
	{
		CString name = m_name;
		// Loading
		node->getAttr( "Name",name );

		if (!ctx.bUniqName)
		{
			SetName( name );
		}
		else
		{
			SetName( GetLibrary()->GetManager()->MakeUniqItemName(name) );
		}

		if (!ctx.bCopyPaste)
		{
			GUID guid;
			if (node->getAttr( "Id",guid ))
				SetGUID(guid);
		}
	}
	else
	{
		// Saving.
		node->setAttr( "Name",m_name );
		node->setAttr( "Id",m_guid );
	}
}

//////////////////////////////////////////////////////////////////////////
IDataBaseLibrary* CBaseLibraryItem::GetLibrary() const
{
	return m_library;
}