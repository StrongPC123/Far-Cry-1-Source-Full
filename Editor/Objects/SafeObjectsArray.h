////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   safeobjectsarray.h
//  Version:     v1.00
//  Created:     28/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __safeobjectsarray_h__
#define __safeobjectsarray_h__
#pragma once

#include "BaseObject.h"

/** This class used as a safe collction of references to CBaseObject instances.
		Target objects in this collection can be safely removed or added,
		This object makes references to added objects and recieve back event when theose objects are deleted.
*/
class CSafeObjectsArray
{
public:
	CSafeObjectsArray() {};
	~CSafeObjectsArray();

	void Add( CBaseObject *obj );
	void Remove( CBaseObject *obj );

	bool IsEmpty() const { return m_objects.empty(); }
	int GetCount() const { return m_objects.size(); }
	CBaseObject* Get( int index ) const;

	// Clear array.
	void Clear();

private:
	void OnTargetEvent( CBaseObject *target,int event );

	//////////////////////////////////////////////////////////////////////////
	std::vector<CBaseObjectPtr> m_objects;
};

#endif // __safeobjectsarray_h__
