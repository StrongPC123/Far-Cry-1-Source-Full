////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   TagPoint.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CTagPoint implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TagPoint.h"
#include <IAgent.h>

#include "..\Viewport.h"
#include "Settings.h"

#include <IAISystem.h>
#include <IGame.h>
#include <IMarkers.h>

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CTagPoint,CBaseObject)
IMPLEMENT_DYNCREATE(CRespawnPoint,CTagPoint)

#define TAGPOINT_RADIUS 0.5f

//////////////////////////////////////////////////////////////////////////
float CTagPoint::m_helperScale = 1;

//////////////////////////////////////////////////////////////////////////
CTagPoint::CTagPoint()
{
	m_ITag = 0;
	m_aiTag = 0;
}

//////////////////////////////////////////////////////////////////////////
void CTagPoint::Done()
{
	if (m_ITag)
	{
		GetIEditor()->GetGame()->GetTagPointManager()->RemoveTagPoint( m_ITag );
		m_ITag = 0;
	}
	if (m_aiTag)
	{
		m_ie->GetSystem()->GetAISystem()->RemoveObject(m_aiTag);
		m_aiTag = 0;
	}
	CBaseObject::Done();
}

//////////////////////////////////////////////////////////////////////////
bool CTagPoint::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	m_ie = ie;
	
	SetColor( RGB(0,0,255) );
	bool res = CBaseObject::Init( ie,prev,file );
	
	if (IsCreateGameObjects())
	{
		// Create Tag point in game.
		if (!GetName().IsEmpty())
			CreateITagPoint();
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
float CTagPoint::GetRadius()
{
	return TAGPOINT_RADIUS * m_helperScale * gSettings.gizmo.helpersScale;
}

//////////////////////////////////////////////////////////////////////////
void CTagPoint::SetName( const CString &name )
{
	bool bModified = strcmp(name,GetName()) != 0;
	CBaseObject::SetName( name );

	if (bModified)
	{
		CreateITagPoint();
		//if (m_aiTag)
			//m_aiTag->SetName(name);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTagPoint::CreateITagPoint()
{
	if (IsCreateGameObjects())
	{
		IGame *pGame = GetIEditor()->GetGame();
		if (m_ITag)
		{
			pGame->GetTagPointManager()->RemoveTagPoint( m_ITag );
			m_ITag = 0;
		}
		if (m_aiTag)
		{
			m_ie->GetSystem()->GetAISystem()->RemoveObject(m_aiTag);
			m_aiTag = 0;
		}

		const Matrix44 &tm = GetWorldTM();
		// Create Tag point in game.
		if (!GetName().IsEmpty())
		{

			// Export world coordinates.
			//M2Q_CHANGED_BY_IVO
			//Quat q(tm);
			//Quat q = CovertMatToQuat<float>( GetTransposed44(tm) );
			Quat q = Quat( GetTransposed44(tm) );

			Vec3 angles = RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(q)));
			m_ITag = pGame->GetTagPointManager()->CreateTagPoint( (const char*)GetName(),GetWorldPos(),angles );
		}

			//if (m_aiTag)
		m_aiTag = m_ie->GetSystem()->GetAISystem()->CreateAIObject( AIOBJECT_WAYPOINT,0 );
		m_aiTag->SetName( GetName() );
		m_aiTag->SetPos( tm.GetTranslationOLD() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CTagPoint::InvalidateTM()
{
	CBaseObject::InvalidateTM();

	if (m_ITag || m_aiTag)
	{
		const Matrix44 &tm = GetWorldTM();

		//CHANGED_BY_IVO
		//Quat q(tm);
		//Quat q = CovertMatToQuat<float>( GetTransposed44(tm) );
		Quat q = Quat( GetTransposed44(tm) );

		Vec3 angles = RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(q)));
		if (m_ITag)
		{
			m_ITag->SetPos(tm.GetTranslationOLD());
			m_ITag->SetAngles(angles);
		}
		if (m_aiTag)
		{
			m_aiTag->SetPos(tm.GetTranslationOLD());
			m_aiTag->SetAngles(angles);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTagPoint::SetScale( const Vec3d &scale )
{
	// Ignore scale.
}

//////////////////////////////////////////////////////////////////////////
void CTagPoint::BeginEditParams( IEditor *ie,int flags )
{
	m_ie = ie;
	CBaseObject::BeginEditParams( ie,flags );
}

//////////////////////////////////////////////////////////////////////////
void CTagPoint::EndEditParams( IEditor *ie )
{
	CBaseObject::EndEditParams( ie );
}

//////////////////////////////////////////////////////////////////////////
int CTagPoint::MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
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
void CTagPoint::Display( DisplayContext &dc )
{
	const Matrix44 &wtm = GetWorldTM();

	//CHANGED_BY_IVO
	//Vec3 x = wtm.TransformVector( Vec3(0,-1,0) );
	float fHelperScale = 1*m_helperScale*gSettings.gizmo.helpersScale;
	Vec3 x = wtm.TransformVectorOLD( Vec3(0,-fHelperScale,0) );
	Vec3 wp = wtm.GetTranslationOLD();
	dc.SetColor( 1,1,0 );
	dc.DrawArrow( wp,wp+x*2,fHelperScale );

	if (IsFrozen())
		dc.SetFreezeColor();
	else
	{
		if (m_ITag)
			dc.SetColor( GetColor(),0.8f );
		else
		{
			//Warning bad tag point name color.
			float t = GetTickCount() / 1000.0f;
			int r = fabs(sin(t*10.0f)*255);
			r = __min(r,255);
			dc.SetColor( RGB(r,0,0),0.2f );
		}
	}

	dc.DrawBall( wp,GetRadius() );

	if (IsSelected())
	{
		dc.SetSelectedColor(0.6f);
		dc.DrawBall( wp,GetRadius()+0.02f );
	}
	DrawDefault( dc );
}

//////////////////////////////////////////////////////////////////////////
bool CTagPoint::HitTest( HitContext &hc )
{
	Vec3 origin = GetWorldPos();
	float radius = GetRadius();

	Vec3 w = origin - hc.raySrc;
	w = hc.rayDir.Cross( w );
	float d = w.GetLengthSquared();

	if (d < radius*radius + hc.distanceTollerance)
	{
		Vec3 i0;
		if (Intersect::Ray_SphereFirst(Ray(hc.raySrc,hc.rayDir),Sphere(origin,radius),i0))
		{
			hc.dist = GetDistance(hc.raySrc,i0);
			return true;
		}
		hc.dist = GetDistance(hc.raySrc,origin);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CTagPoint::GetBoundBox( BBox &box )
{
	Vec3 pos = GetWorldPos();
	float r = GetRadius();
	box.min = pos - Vec3(r,r,r);
	box.max = pos + Vec3(r,r,r);
}

//////////////////////////////////////////////////////////////////////////
void CTagPoint::GetLocalBounds( BBox &box )
{
	float r = GetRadius();
	box.min = -Vec3(r,r,r);
	box.max = Vec3(r,r,r);
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CTagPoint::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	XmlNodeRef objNode = CBaseObject::Export( levelPath,xmlNode );

	// Export position in world space.
	Matrix44 wtm = GetWorldTM();
	wtm.NoScale();

	//CHANGED_BY_IVO
	//Quat q(wtm);
	//Quat q = CovertMatToQuat<float>( GetTransposed44(wtm) );
	Quat q = Quat( GetTransposed44(wtm) );

	Vec3 worldPos = wtm.GetTranslationOLD();
	Vec3 worldAngles = Ang3::GetAnglesXYZ(Matrix33(q)) * 180.0f/gf_PI;
		
	//if (worldPos != Vec3(0,0,0))
	if (!IsEquivalent(worldPos,Vec3(0,0,0)))
		objNode->setAttr( "Pos",worldPos );

	//if (worldAngles != Vec3(0,0,0))
	if (!IsEquivalent(worldAngles,Vec3(0,0,0)))
		objNode->setAttr( "Angles",worldAngles );
		

	return objNode;
}

//////////////////////////////////////////////////////////////////////////
// CRespawnPoint implementation.
//////////////////////////////////////////////////////////////////////////
void CRespawnPoint::CreateITagPoint()
{
	if (IsCreateGameObjects())
	{
		IGame *pGame = GetIEditor()->GetGame();
		if (m_ITag)
		{
			pGame->GetTagPointManager()->RemoveRespawnPoint( m_ITag );
			pGame->GetTagPointManager()->RemoveTagPoint( m_ITag );
		}
		// Create Tag point in game.
		m_ITag = pGame->GetTagPointManager()->CreateTagPoint( (const char*)GetName(),GetWorldPos(),GetAngles() );
		if (m_ITag)
			pGame->GetTagPointManager()->AddRespawnPoint( m_ITag );
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CRespawnPoint::Done()
{
	if (m_ITag)
	{
		IGame *pGame = GetIEditor()->GetGame();
		pGame->GetTagPointManager()->RemoveRespawnPoint( m_ITag );
	}
	CTagPoint::Done();
}