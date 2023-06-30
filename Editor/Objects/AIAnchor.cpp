////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   aianchor.cpp
//  Version:     v1.00
//  Created:     9/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AIAnchor.h"

#include "..\AI\AIManager.h"
#include "..\Viewport.h"
#include "Settings.h"

#include <IAISystem.h>
#include <IAgent.h>

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CAIAnchor,CBaseObject)

//////////////////////////////////////////////////////////////////////////
float CAIAnchor::m_helperScale = 1;

//////////////////////////////////////////////////////////////////////////
CAIAnchor::CAIAnchor()
{
	m_aiObject = 0;

	AddVariable( mv_action,"Action",functor(*this,&CAIAnchor::OnActionChange),IVariable::DT_AI_ANCHOR );
}

//////////////////////////////////////////////////////////////////////////
void CAIAnchor::Done()
{
	if (m_aiObject)
	{
		GetIEditor()->GetSystem()->GetAISystem()->RemoveObject(m_aiObject);
		m_aiObject = 0;
	}
	CBaseObject::Done();
}

//////////////////////////////////////////////////////////////////////////
bool CAIAnchor::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	SetColor( RGB(0,255,0) );
	bool res = CBaseObject::Init( ie,prev,file );

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool CAIAnchor::CreateGameObject()
{
	CString action = mv_action;
	int aiObjectType = GetIEditor()->GetAI()->AnchorActionToId(action);

	if (aiObjectType < 0)
		aiObjectType = 0;

	if (aiObjectType >= 0)
	{
		if (m_aiObject)
		{
			GetIEditor()->GetSystem()->GetAISystem()->RemoveObject(m_aiObject);
			m_aiObject = 0;
		}

		m_aiObject = GetIEditor()->GetSystem()->GetAISystem()->CreateAIObject( aiObjectType,0 );
		if (m_aiObject)
		{
			m_aiObject->SetName( GetName() );
			m_aiObject->SetRadius( GetArea() );
			m_aiObject->SetPos( GetWorldPos() );
			m_aiObject->SetAngles( GetWorldAngles() );
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
float CAIAnchor::GetRadius()
{
	return 0.5f*m_helperScale*gSettings.gizmo.helpersScale;
}

//////////////////////////////////////////////////////////////////////////
void CAIAnchor::SetName( const CString &name )
{
	CBaseObject::SetName( name );
	if (m_aiObject)
		m_aiObject->SetName(name);
}

//////////////////////////////////////////////////////////////////////////
void CAIAnchor::InvalidateTM()
{
	CBaseObject::InvalidateTM();

	if (m_aiObject)
	{
		m_aiObject->SetPos( GetWorldPos() );
		m_aiObject->SetAngles( GetWorldAngles() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAIAnchor::SetScale( const Vec3d &scale )
{
	// Ignore scale.
}

//////////////////////////////////////////////////////////////////////////
void CAIAnchor::BeginEditParams( IEditor *ie,int flags )
{
	CBaseObject::BeginEditParams( ie,flags );
}

//////////////////////////////////////////////////////////////////////////
void CAIAnchor::EndEditParams( IEditor *ie )
{
	CBaseObject::EndEditParams( ie );
}

//////////////////////////////////////////////////////////////////////////
int CAIAnchor::MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	if (event == eMouseMove || event == eMouseLDown)
	{
		Vec3 pos;
		if (GetIEditor()->GetAxisConstrains() != AXIS_TERRAIN)
		{
			pos = view->MapViewToCP(point);
		}
		else
		{
			// Snap to terrain.
			bool hitTerrain;
			pos = view->ViewToWorld( point,&hitTerrain );
			if (hitTerrain)
			{
				pos.z = GetIEditor()->GetTerrainElevation(pos.x,pos.y) + 1.0f;
			}
			pos = view->SnapToGrid(pos);
		}

		pos = view->SnapToGrid(pos);
		SetPos( pos );
		if (event == eMouseLDown)
			return MOUSECREATE_OK;
		return MOUSECREATE_CONTINUE;
	}
	return CBaseObject::MouseCreateCallback( view,event,point,flags );
}

//////////////////////////////////////////////////////////////////////////
void CAIAnchor::Display( DisplayContext &dc )
{
	const Matrix44 &wtm = GetWorldTM();

	//CHANGED_BY_IVO
	Vec3 x = wtm.TransformVectorOLD( Vec3(0,-1,0) );

	Vec3 wp = wtm.GetTranslationOLD();

	if (IsFrozen())
		dc.SetFreezeColor();
	else
		dc.SetColor( GetColor() );

	//dc.DrawArrow( wp,wp+x*0.5,0.5 );

	//dc.DrawBall( wp,GetRadius() );

	Matrix44 tm(wtm);
	dc.RenderObject( STATOBJECT_ANCHOR,tm );

	if (IsSelected())
	{
		dc.SetSelectedColor();
		//dc.DrawBall( wp,GetRadius()+0.02f );

		dc.PushMatrix(wtm);
		float r = GetRadius();
		dc.DrawWireBox( -Vec3(r,r,r),Vec3(r,r,r) );
		dc.PopMatrix();
	}
	DrawDefault( dc );
}

//////////////////////////////////////////////////////////////////////////
bool CAIAnchor::HitTest( HitContext &hc )
{
	Vec3 origin = GetWorldPos();
	float radius = GetRadius();

	Vec3 w = origin - hc.raySrc;
	w = hc.rayDir.Cross( w );
	float d = w.Length();

	if (d < radius + hc.distanceTollerance)
	{
		hc.dist = GetDistance(hc.raySrc,origin);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CAIAnchor::GetBoundBox( BBox &box )
{
	Vec3 pos = GetWorldPos();
	float r = GetRadius();
	box.min = pos - Vec3(r,r,r);
	box.max = pos + Vec3(r,r,r);
}

//////////////////////////////////////////////////////////////////////////
void CAIAnchor::GetLocalBounds( BBox &box )
{
	float r = GetRadius();
	box.min = -Vec3(r,r,r);
	box.max = Vec3(r,r,r);
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CAIAnchor::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	XmlNodeRef objNode = CBaseObject::Export( levelPath,xmlNode );

	// Export position in world space.
	Matrix44 wtm = GetWorldTM();
	wtm.NoScale();

	//QUAT_CHANGED_BY_IVO
	//Quat q(wtm);
	//Quat q = CovertMatToQuat<float>( GetTransposed44(wtm) );
	Quat q = Quat( GetTransposed44(wtm) );

	Vec3 worldPos = wtm.GetTranslationOLD();
	Vec3 worldAngles = Ang3::GetAnglesXYZ(Matrix33(q)) * 180.0f/gf_PI;

	CString actionName = mv_action;
	int aiObjectType = GetIEditor()->GetAI()->AnchorActionToId( actionName );

	objNode->setAttr( "AnchorId",aiObjectType );
	objNode->setAttr( "Area",GetArea() );
		
	if (!IsEquivalent(worldPos,Vec3(0,0,0),0))
		objNode->setAttr( "Pos",worldPos );
		
	if (!IsEquivalent(worldAngles,Vec3(0,0,0),0))
		objNode->setAttr( "Angles",worldAngles );

	return objNode;
}

//////////////////////////////////////////////////////////////////////////
void CAIAnchor::OnActionChange( IVariable *var )
{
	if (m_aiObject)
		CreateGameObject();
}

//////////////////////////////////////////////////////////////////////////
void CAIAnchor::OnUIUpdate()
{
	CBaseObject::OnUIUpdate();

	if (m_aiObject)
		m_aiObject->SetRadius(GetArea());
}