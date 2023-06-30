////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   safeobjectsarray.cpp
//  Version:     v1.00
//  Created:     28/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SafeObjectsArray.h"

//////////////////////////////////////////////////////////////////////////
CSafeObjectsArray::~CSafeObjectsArray()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////////
void CSafeObjectsArray::Clear()
{
	int num = m_objects.size();
	for (int i = 0; i < num; i++)
	{
		m_objects[i]->RemoveEventListener( functor(*this,&CSafeObjectsArray::OnTargetEvent) );
	}
	m_objects.clear();
}

//////////////////////////////////////////////////////////////////////////
void CSafeObjectsArray::Add( CBaseObject *obj )
{
	// Not add NULL object.
	if (!obj)
		return;

	// Check if object is unique in array.
	if (!stl::find( m_objects,obj ))
	{
		m_objects.push_back(obj);
		// Make reference on this object.
		obj->AddEventListener( functor(*this,&CSafeObjectsArray::OnTargetEvent) );
	}
}

//////////////////////////////////////////////////////////////////////////
void CSafeObjectsArray::Remove( CBaseObject *obj )
{
	// Find this object.
	if (stl::find_and_erase( m_objects,obj ))
	{
		obj->RemoveEventListener( functor(*this,&CSafeObjectsArray::OnTargetEvent) );
	}
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CSafeObjectsArray::Get( int index ) const
{
	assert( index >= 0 && index < m_objects.size() );
	return m_objects[index];
}

//////////////////////////////////////////////////////////////////////////
void CSafeObjectsArray::OnTargetEvent( CBaseObject *target,int event )
{
	if (event == CBaseObject::ON_DELETE)
	{
		Remove(target);
	}
}