////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   SelectionGroup.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CSelection group definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __SelectionGroup_h__
#define __SelectionGroup_h__

#if _MSC_VER > 1000
#pragma once
#endif

class CBaseObject;

#include "ObjectEvent.h"

/*!
 *	CSelectionGroup is a named selection group of objects.
 */
class CSelectionGroup
{
public:
	//! Set name of selection.
	void SetName( const CString &name ) { m_name = name; };
	//! Get name of selection.
	const CString& GetName() const { return m_name; };

	//! Adds object into selection list.
	void AddObject( CBaseObject *obj );
	//! Remove object from selection list.
	void RemoveObject( CBaseObject *obj );
	//! Remove all objects from selection.
	void RemoveAll();
	//! Check if object contained in selection list.
	bool IsContainObject( CBaseObject *obj );
	//! Return true if selection doesnt contain any object.
	bool IsEmpty() const;
	//! Check if all selected objects are of same type
	bool SameObjectType();
	//! Number of selected object.
	int	 GetCount() const;
	//! Get object at givven index.
	CBaseObject* GetObject( int index ) const;

	//! Get mass center of selected objects.
	Vec3	GetCenter() const;

	//! Get Bounding box of selection.
	BBox GetBounds() const;

	void	Copy( const CSelectionGroup &from );

	//! Remove from selection group all objects which have parent also in selection group.
	//! And save resulting objects to saveTo selection.
	void	FilterParents();
	//! Get number of child filtered objects.
	int GetFilteredCount() const { return m_filtered.size(); }
	CBaseObject* GetFilteredObject( int i ) const { return m_filtered[i]; }

	//////////////////////////////////////////////////////////////////////////
	// Operations on selection group.
	//////////////////////////////////////////////////////////////////////////
	//! Move objects in selection by offset.
	void Move( const Vec3 &offset,bool keepHeight,bool inWorldSpace );
	//! Rotate objects in selection by givven angle.
	void Rotate( const Vec3 &angles,bool inWorldSpace );
	//! Scale objects in selection by givven scale.
	void Scale( const Vec3 &scale,bool inWorldSpace );

	//////////////////////////////////////////////////////////////////////////
	//! Clone objects in this group and add cloned objects to new selection group.
	//! Only topmost parent  objects will be added to this selection group.
	void Clone( CSelectionGroup &newGroup );

	//! Same as Copy but will copy all objects from hierarchy of current selection to new selection group.
	void FlattenHierarchy( CSelectionGroup &newGroup );

	//! Pick new parent and attach selection to it.
	void PickAndAttach();

	// Send event to all objects in selection group.
	void SendEvent( ObjectEvent event );

private:
	CString m_name;
	typedef std::vector<TSmartPtr<CBaseObject> > Objects;
	Objects m_objects;
	// Objects set, for fast searches.
	std::set<CBaseObject*> m_objectsSet;

	//! Selection list with child objecs filtered out.
	std::vector<CBaseObject*> m_filtered;
};

#endif // __SelectionGroup_h__
