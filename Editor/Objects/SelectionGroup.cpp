////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   SelectionGroup.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CSelectionGroup implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SelectionGroup.h"

#include "BaseObject.h"
#include "ObjectManager.h"

//////////////////////////////////////////////////////////////////////////
void CSelectionGroup::AddObject( CBaseObject *obj )
{
	if (!IsContainObject(obj))
	{
		m_objects.push_back(obj);
		m_objectsSet.insert(obj);
		m_filtered.clear();
	}
}

//////////////////////////////////////////////////////////////////////////
void CSelectionGroup::RemoveObject( CBaseObject *obj )
{
	for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		if (*it == obj)
		{
			m_objects.erase(it);
			m_objectsSet.erase(obj);
			m_filtered.clear();
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CSelectionGroup::RemoveAll()
{
	m_objects.clear();
	m_objectsSet.clear();
	m_filtered.clear();
}
	
bool CSelectionGroup::IsContainObject( CBaseObject *obj )
{
	return (m_objectsSet.find(obj) != m_objectsSet.end());
}

//////////////////////////////////////////////////////////////////////////
bool CSelectionGroup::IsEmpty() const
{
	return m_objects.empty();
}

//////////////////////////////////////////////////////////////////////////
bool CSelectionGroup::SameObjectType()
{
	if (IsEmpty())
		return false;
	CBaseObjectPtr pFirst=(*(m_objects.begin()));
	for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		if ((*it)->GetRuntimeClass()!=pFirst->GetRuntimeClass())
			return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
int CSelectionGroup::GetCount() const
{
	return m_objects.size();
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CSelectionGroup::GetObject( int index ) const
{
	ASSERT( index >= 0 && index < m_objects.size() );
	return m_objects[index];
}

//////////////////////////////////////////////////////////////////////////
void CSelectionGroup::Copy( const CSelectionGroup &from )
{
	m_name = from.m_name;
	m_objects = from.m_objects;
	m_objectsSet = from.m_objectsSet;
	m_filtered = from.m_filtered;
}

//////////////////////////////////////////////////////////////////////////
Vec3	CSelectionGroup::GetCenter() const
{
	Vec3 c(0,0,0);
	for (int i = 0; i < GetCount(); i++)
	{
		c += GetObject(i)->GetWorldPos();
	}
	c /= GetCount();
	return c;
}

//////////////////////////////////////////////////////////////////////////
BBox CSelectionGroup::GetBounds() const
{
	BBox b;
	BBox box;
	box.Reset();
	for (int i = 0; i < GetCount(); i++)
	{
		GetObject(i)->GetBoundBox( b );
		box.Add( b.min );
		box.Add( b.max );
	}
	return box;
}

//////////////////////////////////////////////////////////////////////////
void CSelectionGroup::FilterParents()
{
	if (!m_filtered.empty())
		return;

	m_filtered.reserve( m_objects.size() );
	for (int i = 0; i < m_objects.size(); i++)
	{
		CBaseObject *obj = m_objects[i];
		CBaseObject *parent = obj->GetParent();
		bool bParentInSet = false;
		while (parent)
		{
			if (m_objectsSet.find(parent) != m_objectsSet.end())
			{
				bParentInSet = true;
				break;
			}
			parent = parent->GetParent();
		}
		if (!bParentInSet)
		{
			m_filtered.push_back(obj);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CSelectionGroup::Move( const Vec3 &offset,bool keepHeight,bool inWorldSpace )
{
	if (offset.x == 0 && offset.y == 0 && offset.z == 0)
		return;

	FilterParents();

	Vec3 newPos;
	for (int i = 0; i < GetFilteredCount(); i++)
	{
		CBaseObject *obj = GetFilteredObject(i);
		
		Matrix44 wtm = obj->GetWorldTM();
		Vec3 wp = wtm.GetTranslationOLD();
		newPos = wp + offset;
		if (keepHeight)
		{
			// Make sure object keeps it height.
			float height = wp.z - GetIEditor()->GetTerrainElevation( wp.x,wp.y );
			newPos.z = GetIEditor()->GetTerrainElevation( newPos.x,newPos.y ) + height;
		}
		wtm.SetTranslationOLD( newPos );
		
		obj->SetWorldTM( wtm );
		//obj->SetPos(newPos);	// this is better i guess
	}
}

//////////////////////////////////////////////////////////////////////////
void CSelectionGroup::Rotate( const Vec3 &angles,bool inWorldSpace )
{
	if (angles.x == 0 && angles.y == 0 && angles.z == 0)
		return;
	/*
	if (sel.GetCount() == 1)
	{
		// If rotating only one object, assume angles are absolute angles of this object.
	
		CBaseObject *obj = sel.GetObject(0);
		Quat q1,q2;
		q1.SetEulerAngles( obj->GetAngles()*PI/180.0f );
		q2.SetEulerAngles( angles*PI/180.0f );
		q1 = q1 * q2;
		Vec3 angles = q1.GetEulerAngles() * 180.0f/PI;
	
		//obj->SetAngles( angles );
		return;
	}
*/

	// Rotate selection about selection center.
	Vec3 center = GetCenter();

	Matrix44 rotateTM;
	rotateTM.SetIdentity();
	//rotateTM.RotateMatrix_fix( angles );
	rotateTM=Matrix44::CreateRotationZYX(-angles*gf_DEGTORAD)*rotateTM; //NOTE: angles in radians and negated 

	Matrix44 ToOrigin;
	Matrix44 FromOrigin;

	ToOrigin.SetIdentity();
	FromOrigin.SetIdentity();

	if (inWorldSpace)
	{
		ToOrigin.SetTranslationOLD( -center );
		FromOrigin.SetTranslationOLD( center );
	}

	FilterParents();

	for (int i = 0; i < GetFilteredCount(); i++)
	{
		CBaseObject *obj = GetFilteredObject(i);
		
		Matrix44 m = obj->GetWorldTM();
		if (inWorldSpace)
		{
			m = m * ToOrigin * rotateTM * FromOrigin;
		}
		else
		{
			m = rotateTM * m;
		}
		obj->SetWorldTM( m );
	}
}

//////////////////////////////////////////////////////////////////////////
void CSelectionGroup::Scale( const Vec3 &scale,bool inWorldSpace )
{
	if (scale.x == 1 && scale.y == 1 && scale.z == 1)
		return;

	Vec3 scl = scale;
	if (scl.x == 0) scl.x = 0.01f;
	if (scl.y == 0) scl.y = 0.01f;
	if (scl.z == 0) scl.z = 0.01f;

	// Scale selection relative to selection center.
	Vec3 center = GetCenter();

	Matrix44 scaleTM;
	scaleTM.SetIdentity();

	scaleTM=Matrix33::CreateScale( Vec3d(scl.x,scl.y,scl.z) ) * scaleTM;

	Matrix44 ToOrigin;
	Matrix44 FromOrigin;

	ToOrigin.SetIdentity();
	FromOrigin.SetIdentity();

	if (inWorldSpace)
	{
		ToOrigin.SetTranslationOLD( -center );
		FromOrigin.SetTranslationOLD( center );
	}

	FilterParents();

	for (int i = 0; i < GetFilteredCount(); i++)
	{
		CBaseObject *obj = GetFilteredObject(i);
		
		Matrix44 m = obj->GetWorldTM();
		if (inWorldSpace)
			m = m * ToOrigin * scaleTM * FromOrigin;
		else
			m = scaleTM * m;
		obj->SetWorldTM( m );
	}
}

//////////////////////////////////////////////////////////////////////////
void CSelectionGroup::Clone( CSelectionGroup &newGroup )
{
	IObjectManager *pObjMan = GetIEditor()->GetObjectManager();
	assert( pObjMan );

	int i;
	CObjectCloneContext cloneContext;

	FilterParents();

	//////////////////////////////////////////////////////////////////////////
	// Clone every object.
	for (i = 0; i < GetFilteredCount(); i++)
	{
		CBaseObject *pFromObject = GetFilteredObject(i);
		CBaseObject *newObj = pObjMan->CloneObject( pFromObject );
		assert( newObj );

		cloneContext.AddClone( pFromObject,newObj );
		newGroup.AddObject( newObj );
	}

	//////////////////////////////////////////////////////////////////////////
	// Only after everything was cloned, call PostClone on all cloned objects.
	for (i = 0; i < GetFilteredCount(); i++)
	{
		CBaseObject *pFromObject = GetFilteredObject(i);
		CBaseObject *pClonedObject = newGroup.GetObject(i);
		pClonedObject->PostClone( pFromObject,cloneContext );
	}
}

//////////////////////////////////////////////////////////////////////////
static void RecursiveFlattenHierarchy( CBaseObject *pObj,CSelectionGroup &newGroup )
{
	if (pObj->CheckFlags(OBJFLAG_PREFAB))
		return;
	
	newGroup.AddObject( pObj );

	for (int i = 0; i < pObj->GetChildCount(); i++)
	{
		RecursiveFlattenHierarchy( pObj->GetChild(i),newGroup );
	}
}

//////////////////////////////////////////////////////////////////////////
void CSelectionGroup::FlattenHierarchy( CSelectionGroup &newGroup )
{
	for (int i = 0; i < GetCount(); i++)
	{
		RecursiveFlattenHierarchy( GetObject(i),newGroup );
	}
}

//////////////////////////////////////////////////////////////////////////
class CAttachToParentPickCallback : public IPickObjectCallback
{
public:
	CAttachToParentPickCallback() { m_bActive = true; };
	//! Called when object picked.
	virtual void OnPick( CBaseObject *picked )
	{
		CUndo undo( "Attach Selection" );

		CSelectionGroup *selGroup = GetIEditor()->GetSelection();
		selGroup->FilterParents();

		for (int i = 0; i < selGroup->GetFilteredCount(); i++)
		{
			if (ChildIsValid( picked,selGroup->GetFilteredObject(i) ))
				picked->AttachChild( selGroup->GetFilteredObject(i) );
		}
		m_bActive = false;
		delete this;
	}
	//! Called when pick mode cancelled.
	virtual void OnCancelPick()
	{
		m_bActive = false;
		delete this;
	}
	//! Return true if specified object is pickable.
	virtual bool OnPickFilter( CBaseObject *filterObject )
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool ChildIsValid(CBaseObject *pParent, CBaseObject *pChild, int nDir=3)
	{
		if (!pParent)
			return false;
		if (!pChild)
			return false;
		if (pParent==pChild)
			return false;
		CBaseObject *pObj;
		if (nDir & 1)
		{
			if (pObj=pChild->GetParent())
			{
				if (!ChildIsValid(pParent, pObj, 1))
				{
					return false;
				}
			}
		}
		if (nDir & 2)
		{
			for (int i=0;i<pChild->GetChildCount();i++)
			{
				if (pObj=pChild->GetChild(i))
				{
					if (!ChildIsValid(pParent, pObj, 2))
					{
						return false;
					}
				}
			}
		}
		return true;
	}

	static bool IsActive() { return m_bActive; }
private:
	static bool m_bActive;
};
bool CAttachToParentPickCallback::m_bActive = false;

//////////////////////////////////////////////////////////////////////////
void CSelectionGroup::PickAndAttach()
{
	CAttachToParentPickCallback *pCallback = new CAttachToParentPickCallback;
	GetIEditor()->PickObject( pCallback,0,"Attach Selection To Parent" );
}

//////////////////////////////////////////////////////////////////////////
void CSelectionGroup::SendEvent( ObjectEvent event )
{
	for (int i = 0; i < m_objects.size(); i++)
	{
		CBaseObject *obj = m_objects[i];
		obj->OnEvent( event );
	}
}
