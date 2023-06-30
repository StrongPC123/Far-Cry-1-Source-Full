////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Group.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CGroup implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Group.h"

#include "ObjectManager.h"

#include "..\Viewport.h"
#include "..\DisplaySettings.h"

int CGroup::s_groupGeomMergeId = 0;

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CGroup,CBaseObject)

/*
//////////////////////////////////////////////////////////////////////////
//
class CUndoGroupChildsObject : public IUndoObject
{
public:
	CUndoGroupChildsObject( CGroup *grp,const char *undoDescription )
	{
		// Stores the current state of this object.
		assert( grp != 0 );
		m_undoDescription = undoDescription;
		m_objectId = grp->GetId();
		for (int i = 0; i < grp->GetChildCount(); i++)
			m_undo.push_back( grp->GetChild(i)->GetId() );
	}
protected:
	virtual void Release() { delete this; };
	virtual int GetSize()
	{
		return sizeof(*this) + m_undo.size()*sizeof(int)+ m_redo.size()*sizeof(int) + m_undoDescription.GetLength();
	}
	virtual const char* GetDescription() { return m_undoDescription; };

	virtual void Undo( bool bUndo )
	{
		CGroup *grp = (CGroup*)GetIEditor()->GetObjectManager()->FindObject( m_objectId );
		if (!grp || grp->GetType() != OBJTYPE_GROUP)
			return;
		
		GetIEditor()->SuspendUndo();
		if (bUndo)
		{
			m_redo.clear();
			// Save current object state.
			for (int i = 0; i < grp->GetChildCount(); i++)
				m_redo.push_back( grp->GetChild(i)->GetId() );
		}
		
		grp->DetachAll();
		// Undo object state.
		for (int i = 0; i < m_undo.size(); i++)
		{
			CBaseObject *obj = GetIEditor()->GetObjectManager()->FindObject( m_undo[i] );
			if (obj)
				grp->AddChild( obj );
		}

		GetIEditor()->ResumeUndo();
	}
	virtual void Redo()
	{
		CGroup *grp = (CGroup*)GetIEditor()->GetObjectManager()->FindObject( m_objectId );
		if (!grp || grp->GetType() != OBJTYPE_GROUP)
			return;

		GetIEditor()->SuspendUndo();		

		grp->RemoveAllChilds();
		// Redo object state.
		for (int i = 0; i < m_redo.size(); i++)
		{
			CBaseObject *obj = GetIEditor()->GetObjectManager()->FindObject( m_redo[i] );
			if (obj)
				grp->AddChild( obj );
		}

		GetIEditor()->ResumeUndo();
	}

private:
	CString m_undoDescription;
	int m_objectId;
	std::vector<int> m_undo;
	std::vector<int> m_redo;
};
*/

//////////////////////////////////////////////////////////////////////////
CGroup::CGroup()
{
	m_opened = false;
	m_bAlwaysDrawBox = true;
	m_ignoreChildModify = false;
	m_bbox.min = m_bbox.max = Vec3(0,0,0);
	m_bBBoxValid = false;
	SetColor( RGB(0,255,0) ); // Green

	m_geomMergeId = ++s_groupGeomMergeId;
	AddVariable( mv_mergeStaticGeom,"MergeGeom",_T("Merge Static Geometry"),functor(*this,&CGroup::OnMergeStaticGeom) );
}

inline void RecursivelyGetAllChilds( CBaseObject *obj,std::vector<CBaseObjectPtr> &childs )
{
	for (int i = 0; i < obj->GetChildCount(); i++)
	{
		CBaseObject *c = obj->GetChild(i);
		childs.push_back(c);
		RecursivelyGetAllChilds( c,childs );
	}
}

//////////////////////////////////////////////////////////////////////////
void CGroup::Done()
{
	DeleteAllChilds();
	CBaseObject::Done();
}

//////////////////////////////////////////////////////////////////////////
void CGroup::DeleteAllChilds()
{
	std::vector<CBaseObjectPtr> childs;
	RecursivelyGetAllChilds( this,childs );
	for (int i = 0; i < childs.size(); i++)
	{
		GetObjectManager()->DeleteObject(childs[i]);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CGroup::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	m_ie = ie;
	bool res = CBaseObject::Init( ie,prev,file );
	if (prev)
	{
		InvalidateBBox();
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////
void CGroup::AttachChild( CBaseObject* child,bool bKeepPos )
{
	CBaseObject::AttachChild( child,bKeepPos );
	// Set Group pointer of all non group child objects to this group.
	RecursivelySetGroup( child,this );
	InvalidateBBox();
}

//////////////////////////////////////////////////////////////////////////
void CGroup::RemoveChild( CBaseObject *child )
{
	// Set Group pointer of all non group child objects to parent group if present.
	RecursivelySetGroup( child,GetGroup() );
	CBaseObject::RemoveChild( child );
	InvalidateBBox();
}

//////////////////////////////////////////////////////////////////////////
void CGroup::RecursivelySetGroup( CBaseObject *object,CGroup *pGroup )
{
	object->SetGroup(pGroup);
	
	if (object->GetType() != OBJTYPE_GROUP)
	{
		int numChilds = object->GetChildCount();
		for (int i = 0; i < numChilds; i++)
		{
			RecursivelySetGroup( object->GetChild(i),pGroup );
		}
	}
	else
	{
		if (pGroup)
		{
			CGroup *pChildGroup = (CGroup*)object;
			if (!pGroup->IsOpen())
				pChildGroup->Close();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CGroup::RecursivelySetFlags( CBaseObject *object,int flags )
{
	object->SetFlags( flags );

	int numChilds = object->GetChildCount();
	for (int i = 0; i < numChilds; i++)
	{
		RecursivelySetFlags( object->GetChild(i),flags );
	}
}

//////////////////////////////////////////////////////////////////////////
void CGroup::RecursivelySetLayer( CBaseObject *object,CObjectLayer *pLayer )
{
	object->SetLayer( pLayer );

	int numChilds = object->GetChildCount();
	for (int i = 0; i < numChilds; i++)
	{
		RecursivelySetLayer( object->GetChild(i),pLayer );
	}
}

//////////////////////////////////////////////////////////////////////////
void CGroup::GetBoundBox( BBox &box )
{
	if (!m_bBBoxValid)
		CalcBoundBox();
	box = m_bbox;
	box.Transform( GetWorldTM() );
}

//////////////////////////////////////////////////////////////////////////
void CGroup::GetLocalBounds( BBox &box )
{
	if (!m_bBBoxValid)
		CalcBoundBox();
	box = m_bbox;
}

//////////////////////////////////////////////////////////////////////////
bool CGroup::HitTest( HitContext &hc )
{
	bool selected = false;
	if (m_opened)
	{
		selected = HitTestChilds( hc );
	}

	if (!selected)
	{
		Vec3 p;

		Matrix44 invertWTM = GetWorldTM();
		Vec3 worldPos = invertWTM.GetTranslationOLD();
		invertWTM.Invert44();

		Vec3 xformedRaySrc = invertWTM.TransformPointOLD(hc.raySrc);
		Vec3 xformedRayDir = GetNormalized( invertWTM.TransformVectorOLD(hc.rayDir) );

		float epsilonDist = hc.view->GetScreenScaleFactor( worldPos ) * 0.01f;
		float hitDist;

		float tr = hc.distanceTollerance/2 + 1;
		BBox box;
		box.min = m_bbox.min - Vec3(tr+epsilonDist,tr+epsilonDist,tr+epsilonDist);
		box.max = m_bbox.max + Vec3(tr+epsilonDist,tr+epsilonDist,tr+epsilonDist);
		if (box.IsIntersectRay( xformedRaySrc,xformedRayDir,p ))
		{
			if (m_bbox.RayEdgeIntersection( xformedRaySrc,xformedRayDir,epsilonDist,hitDist,p ))
			{
				hc.dist = GetDistance(xformedRaySrc,p);
				return true;
			}
			else
			{
				// Check if any childs of closed group selected.
				if (!m_opened)
				{
					if (HitTestChilds( hc ))
					{
						hc.object = this;
						return true;
					}
				}
			}
		}
	}

	return selected;
}

//////////////////////////////////////////////////////////////////////////
bool CGroup::HitTestChilds( HitContext &hcOrg )
{
	float mindist = FLT_MAX;

	//uint hideMask = GetIEditor()->GetDisplaySettings()->GetObjectHideMask();

	HitContext hc;

	CBaseObject *selected = 0;
	int numChilds = GetChildCount();
	for (int i = 0; i < numChilds; i++)
	{
		CBaseObject *obj = GetChild(i);

		if (obj->IsHidden())
			continue;
		if (obj->IsFrozen())
			continue;

		hc = hcOrg;

		if (obj->IsSelected())
		{
			int axis = obj->HitTestAxis( hc );
			if (axis != 0)
			{
				hcOrg.object = obj;
				hcOrg.axis = axis;
				hcOrg.dist = hc.dist;
				return true;
			}
		}

		if (obj->HitTest(hc))
		{
			if (hc.dist < mindist)
			{
				mindist = hc.dist;
				// If collided object specified, accept it, overwise take tested object itself.
				if (hc.object)
					selected = hc.object;
				else
					selected = obj;
				hc.object = 0;
			}
		}
	}
	if (selected)
	{
		hcOrg.object = selected;
		hcOrg.dist = mindist;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CGroup::BeginEditParams( IEditor *ie,int flags )
{
	m_ie = ie;
	CBaseObject::BeginEditParams( ie,flags );
}

//////////////////////////////////////////////////////////////////////////
void CGroup::EndEditParams( IEditor *ie )
{
	CBaseObject::EndEditParams( ie );
}

void CGroup::SetPos( const Vec3d &pos )
{
	CBaseObject::SetPos(pos);
}

void CGroup::SetAngles( const Vec3d &angles )
{
	CBaseObject::SetAngles(angles);
}

void CGroup::SetScale( const Vec3d &scale )
{
	CBaseObject::SetScale(scale);
}

//////////////////////////////////////////////////////////////////////////
void CGroup::Display( DisplayContext &dc )
{
	bool hideNames = dc.flags & DISPLAY_HIDENAMES;

	DrawDefault(dc,GetColor());

	if (!IsHighlighted())
	{
		dc.PushMatrix( GetWorldTM() );

		if (IsSelected())
		{
			dc.SetSelectedColor();
			dc.DrawWireBox( m_bbox.min,m_bbox.max );
		}
		else
		{
			if (m_bAlwaysDrawBox)
			{
				if (IsFrozen())
					dc.SetFreezeColor();
				else
					dc.SetColor( GetColor() );
				dc.DrawWireBox( m_bbox.min,m_bbox.max );
			}
		}
		dc.PopMatrix();
	}

	/*
	uint hideMask = GetIEditor()->GetDisplaySettings()->GetObjectHideMask();
	CObjectManager *objMan = GetObjectManager();
	int numChilds = GetChildCount();
	for (int i = 0; i < numChilds; i++)
	{
		CBaseObject *obj = GetChild(i);
		if (!m_opened)
		{
			dc.flags |= DISPLAY_HIDENAMES;
		}
		// Check if object hidden.
		if (obj->IsHidden())
		{
			obj->UpdateVisibility(false);
			continue;
		}
		obj->Display( dc );
		obj->UpdateVisibility(true);
	}
	if (hideNames)
		dc.flags |= DISPLAY_HIDENAMES;
	else
		dc.flags &= ~DISPLAY_HIDENAMES;
	*/
}

//////////////////////////////////////////////////////////////////////////
void CGroup::Serialize( CObjectArchive &ar )
{
	CBaseObject::Serialize( ar );

	if (ar.bLoading)
	{
		// Loading.
		ar.node->getAttr( "Opened",m_opened );
		if (!ar.bUndo)
			SerializeChilds( ar );
	}
	else
	{
		ar.node->setAttr( "Opened",m_opened );
		if (!ar.bUndo)
			SerializeChilds( ar );
	}
}

//////////////////////////////////////////////////////////////////////////
void CGroup::SerializeChilds( CObjectArchive &ar )
{
	XmlNodeRef xmlNode = ar.node;
	if (ar.bLoading)
	{
		// Loading.
		DetachAll();
	
		// Loading.
		XmlNodeRef childsRoot = xmlNode->findChild( "Objects" );
		if (childsRoot)
		{
			// Load all childs from XML archive.
			ar.LoadObjects( childsRoot );
			InvalidateBBox();
		}
	}
	else
	{
		if (GetChildCount() > 0)
		{
			// Saving.
			XmlNodeRef root = xmlNode->newChild( "Objects" );
			ar.node = root;

			// Save all child objects to XML.
			int num = GetChildCount();
			for (int i = 0; i < num; i++)
			{
				CBaseObject *obj = GetChild(i);
				ar.SaveObject( obj );
			}
		}
	}
	ar.node = xmlNode;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CGroup::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	/*
	// Saving.
	XmlNodeRef root( "Objects" );
	xmlNode->addChild( root );
		
	// Save all child objects to XML.
	for (int i = 0; i < m_childs.size(); i++)
	{
		CBaseObject *obj = m_childs[i];
		
		XmlNodeRef objNode( "Object" );
		objNode->setAttr( "Type",obj->GetTypeName() );
		root->addChild( objNode );
			
		obj->Serialize( objNode,false );
	}
	*/
	
	return CBaseObject::Export( levelPath,xmlNode );
}

//////////////////////////////////////////////////////////////////////////
void CGroup::Ungroup()
{
	std::vector<CBaseObjectPtr> childs;
	for (int i = 0; i < GetChildCount(); i++)
	{
		childs.push_back(GetChild(i));
	}
	DetachAll();
	// Select ungrouped objects.
	for (int i = 0; i < childs.size(); i++)
	{
		GetObjectManager()->SelectObject( childs[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
void CGroup::Open()
{
	if (!m_opened)
	{
	}
	m_opened = true;
}

//////////////////////////////////////////////////////////////////////////
void CGroup::Close()
{
	if (m_opened)
	{
	}
	m_opened = false;
}

//////////////////////////////////////////////////////////////////////////
void CGroup::RecursivelyGetBoundBox( CBaseObject *object,BBox &box )
{
	BBox b;
	object->GetBoundBox( b );
	box.Add(b.min);
	box.Add(b.max);

	int numChilds = object->GetChildCount();
	for (int i = 0; i < numChilds; i++)
	{
		RecursivelyGetBoundBox( object->GetChild(i),box );
	}
}

//////////////////////////////////////////////////////////////////////////
void CGroup::CalcBoundBox()
{
	Matrix44 ltm = GetLocalTM();
	Matrix44 tm;
	tm.SetIdentity();
	SetWorldTM(tm);

	// Calc local bounds box..
	BBox box;
	box.Reset();

	int numChilds = GetChildCount();
	for (int i = 0; i < numChilds; i++)
	{
		RecursivelyGetBoundBox( GetChild(i),box );
	}

	if (numChilds == 0)
	{
		box.min = Vec3(-1,-1,-1);
		box.max = Vec3(1,1,1);
	}

	SetLocalTM(ltm);

	m_bbox = box;
	m_bBBoxValid = true;
}

//////////////////////////////////////////////////////////////////////////
void CGroup::OnChildModified()
{
	if (m_ignoreChildModify)
		return;
		
	InvalidateBBox();
}

//! Select objects withing specified distance from givven position.
int CGroup::SelectObjects( const BBox &box,bool bUnselect )
{
	int numSel = 0;

	BBox objBounds;
	uint hideMask = GetIEditor()->GetDisplaySettings()->GetObjectHideMask();
	int num = GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		CBaseObject *obj = GetChild(i);

		if (obj->IsHidden())
			continue;
		
		if (obj->IsFrozen())
			continue;

		if (obj->GetGroup())
			continue;
		
		obj->GetBoundBox( objBounds );
		if (box.IsIntersectBox(objBounds))
		{
			numSel++;
			if (!bUnselect)
				GetObjectManager()->SelectObject( obj );
			else
				GetObjectManager()->UnselectObject( obj );
		}
		// If its group.
		if (obj->GetRuntimeClass() == RUNTIME_CLASS(CGroup))
		{
			numSel += ((CGroup*)obj)->SelectObjects( box,bUnselect );
		}
	}
	return numSel;
}

//////////////////////////////////////////////////////////////////////////
void CGroup::OnEvent( ObjectEvent event )
{
	CBaseObject::OnEvent(event);
	
	int i;
	switch (event)
	{
		case EVENT_DBLCLICK:
			if (IsOpen())
			{
				int numChilds = GetChildCount();
				for (i = 0; i < numChilds; i++)
				{
					GetObjectManager()->SelectObject( GetChild(i) );
				}
			}
			break;
		
		default:
			{
				int numChilds = GetChildCount();
				for (int i = 0; i < numChilds; i++)
				{
					GetChild(i)->OnEvent( event );
				}
			}
			break;
	}
};

//////////////////////////////////////////////////////////////////////////
void CGroup::OnMergeStaticGeom( IVariable *pVar )
{
}

//////////////////////////////////////////////////////////////////////////
int CGroup::GetGeomMergeId() const
{
	if (mv_mergeStaticGeom)
	{
		return m_geomMergeId;
	}
	return 0;
}
