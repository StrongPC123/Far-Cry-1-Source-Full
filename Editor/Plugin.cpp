////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   plugin.cpp
//  Version:     v1.00
//  Created:     15/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Class Factory implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Plugin.h"

//////////////////////////////////////////////////////////////////////////
CClassFactory* CClassFactory::m_instance = 0;

//////////////////////////////////////////////////////////////////////////
// ClassFactory implementation.
//////////////////////////////////////////////////////////////////////////
CClassFactory::CClassFactory()
{
}

//////////////////////////////////////////////////////////////////////////
CClassFactory::~CClassFactory()
{
	for (int i = 0; i < m_classes.size(); i++)
	{
		m_classes[i]->Release();
	}
}

CClassFactory* CClassFactory::Instance()
{
	if (!m_instance)
	{
		m_instance = new CClassFactory;
	}
	return m_instance;
}

void CClassFactory::RegisterClass( IClassDesc *cls )
{
	assert( cls );
	m_classes.push_back( cls );
	m_guidToClass[cls->ClassID()] = cls;
	m_nameToClass[cls->ClassName()] = cls;
}

//////////////////////////////////////////////////////////////////////////
IClassDesc* CClassFactory::FindClass( const char *className ) const
{
	IClassDesc *cls = stl::find_in_map( m_nameToClass,className,(IClassDesc*)0 );
	return cls;
}

//////////////////////////////////////////////////////////////////////////
IClassDesc* CClassFactory::FindClass( const GUID& clsid ) const
{
	IClassDesc *cls = stl::find_in_map( m_guidToClass,clsid,(IClassDesc*)0 );
	return cls;
}

//! Get classes that matching specific requirements.
void CClassFactory::GetClassesBySystemID( ESystemClassID systemCLSID,std::vector<IClassDesc*> &classes )
{
	classes.clear();
	for (int i = 0; i < m_classes.size(); i++)
	{
		if (m_classes[i]->SystemClassID() == systemCLSID)
		{
			classes.push_back( m_classes[i] );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CClassFactory::GetClassesByCategory( const char* category,std::vector<IClassDesc*> &classes )
{
	classes.clear();
	for (int i = 0; i < m_classes.size(); i++)
	{
		if (stricmp(category,m_classes[i]->Category()) == 0)
		{
			classes.push_back( m_classes[i] );
		}
	}
}