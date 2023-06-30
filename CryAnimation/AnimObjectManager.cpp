////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   animobjectmanager.cpp
//  Version:     v1.00
//  Created:     14/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AnimObjectManager.h"

#include "AnimObject.h"
#include "AnimObjectLoader.h"

//////////////////////////////////////////////////////////////////////////
CAnimObjectManager::CAnimObjectManager()
{
	m_bResourcesLocked = false;
}

//////////////////////////////////////////////////////////////////////////
ICryCharInstance* CAnimObjectManager::MakeAnimObject( const char *animFile )
{
	CAnimObject* obj = new CAnimObject;
	obj->SetFileName(animFile);

	CAnimObjectLoader loader;
	if (!loader.Load( obj,animFile,animFile ))
	{
		// loading failed.
		delete obj;
		return 0;
	}
	m_objects.insert(obj);
	m_animObjects.insert(obj);
	if (m_bResourcesLocked)
		m_lockArray.push_back( obj );
	return obj;
}

bool CAnimObjectManager::RemoveCharacter( ICryCharInstance* obj )
{
	ObjectsSet::iterator it = m_objects.find(obj);
	if (it != m_objects.end())
	{
		// This is our character.
		m_objects.erase( it );
		m_animObjects.erase( (CAnimObject*)obj );
		return true;
	}
	return false;
}

// puts the size of the whole subsystem into this sizer object, classified,
// according to the flags set in the sizer
void CAnimObjectManager::GetMemoryUsage(class ICrySizer* pSizer)const
{
	// TODO:
}

//////////////////////////////////////////////////////////////////////////
void CAnimObjectManager::LockResources()
{
	m_bResourcesLocked = true;
	m_lockArray.reserve( m_animObjects.size() );
	for (AnimObjectsSet::iterator it = m_animObjects.begin(); it != m_animObjects.end(); ++it)
	{
		m_lockArray.push_back( *it );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimObjectManager::UnlockResources()
{
	m_lockArray.clear();
	m_bResourcesLocked = false;
}
