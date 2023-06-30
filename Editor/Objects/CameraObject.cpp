////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   TagPoint.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CCameraObject implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "CameraObject.h"
#include "ObjectManager.h"
#include "..\Viewport.h"

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CCameraObject,CEntity)
IMPLEMENT_DYNCREATE(CCameraObjectTarget,CEntity)

#define CAMERA_COLOR RGB(0,255,255)
#define CAMERA_CONE_LENGTH 4
#define CAMERABOX_RADIUS 0.7f

//////////////////////////////////////////////////////////////////////////
CCameraObject::CCameraObject()
{
	mv_fov = DEG2RAD(60.0f);
	m_creationStep = 0;
	SetColor(CAMERA_COLOR);

	AddVariable( mv_fov,"FOV",functor(*this,&CCameraObject::OnFovChange),IVariable::DT_ANGLE );
}

//////////////////////////////////////////////////////////////////////////
void CCameraObject::Done()
{
	CBaseObjectPtr lookat = GetLookAt();

	CEntity::Done();

	if (lookat)
	{
		// If look at is also camera class, delete lookat target.
		if (lookat->IsKindOf(RUNTIME_CLASS(CCameraObjectTarget)))
			GetObjectManager()->DeleteObject( lookat );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CCameraObject::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	bool res = CEntity::Init( ie,prev,file );

	m_entityClass = "CameraSource";

	if (prev)
	{
		CBaseObjectPtr prevLookat = prev->GetLookAt();
		if (prevLookat)
		{
			CBaseObjectPtr lookat = GetObjectManager()->NewObject( prevLookat->GetClassDesc(),prevLookat );
			if (lookat)
			{
				lookat->SetPos( prevLookat->GetPos() + Vec3(3,0,0) );
				GetObjectManager()->ChangeObjectName( lookat,CString(GetName()) + " Target" );
				SetLookAt( lookat );
			}
		}
	}
	
	return res;
}

void CCameraObject::Serialize( CObjectArchive &ar )
{
	CEntity::Serialize( ar );
}

//////////////////////////////////////////////////////////////////////////
IAnimNode* CCameraObject::CreateAnimNode()
{
	int nodeId = GetId().Data1;
	IAnimNode *animNode = GetIEditor()->GetMovieSystem()->CreateNode( ANODE_CAMERA,nodeId );
	//IAnimNode *animNode = CEntity::CreateAnimNode();
	if (animNode) 
	{
		float fov = mv_fov;
		animNode->SetParamValue( 0,APARAM_FOV,RAD2DEG(fov) );
	}
	return animNode;
}

//////////////////////////////////////////////////////////////////////////
void CCameraObject::OnNodeAnimated()
{
	assert( m_animNode != 0 ); // Only can be called if there`s an anim node present.

	CEntity::OnNodeAnimated();
	// Get fov out of node at current time.
	float fov = RAD2DEG(mv_fov);
	if (m_animNode->GetParamValue( GetIEditor()->GetAnimation()->GetTime(),APARAM_FOV,fov ))
	{
		mv_fov = DEG2RAD(fov);
	}
}

//////////////////////////////////////////////////////////////////////////
void CCameraObject::SetName( const CString &name )
{
	CEntity::SetName(name);
	if (GetLookAt())
	{
		GetLookAt()->SetName( CString(GetName()) + " Target" );
	}
}

//////////////////////////////////////////////////////////////////////////
void CCameraObject::SetScale( const Vec3d &scale )
{
	// Ignore scale.
}

//////////////////////////////////////////////////////////////////////////
void CCameraObject::BeginEditParams( IEditor *ie,int flags )
{
	// Skip entity begin edit params.
	CBaseObject::BeginEditParams( ie,flags );
}

//////////////////////////////////////////////////////////////////////////
void CCameraObject::EndEditParams( IEditor *ie )
{
	// Skip entity end edit params.
	CBaseObject::EndEditParams( ie );
}

//////////////////////////////////////////////////////////////////////////
int CCameraObject::MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	if (event == eMouseMove || event == eMouseLDown || event == eMouseLUp)
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

		if (m_creationStep == 1)
		{
			if (GetLookAt())
			{
				GetLookAt()->SetPos( pos );
			}
		}
		else
		{
			SetPos(pos);
		}
		
		if (event == eMouseLDown && m_creationStep == 0)
		{
			m_creationStep = 1;
		}
		if (event == eMouseMove && 1 == m_creationStep && !GetLookAt())
		{
			float d = GetDistance(pos,GetPos());
			if (d*view->GetScreenScaleFactor(pos) > 1)
			{
				// Create LookAt Target.
				GetIEditor()->ResumeUndo();
				CreateTarget();
				GetIEditor()->SuspendUndo();
			}
		}
		if (eMouseLUp == event && 1 == m_creationStep)
			return MOUSECREATE_OK;

		return MOUSECREATE_CONTINUE;
	}
	return CBaseObject::MouseCreateCallback( view,event,point,flags );
}

//////////////////////////////////////////////////////////////////////////
void CCameraObject::CreateTarget()
{
	// Create another camera object for target.
	CCameraObject *camTarget = (CCameraObject*)GetObjectManager()->NewObject( "CameraTarget" );
	if (camTarget)
	{
		camTarget->SetName( CString(GetName()) + " Target" );
		camTarget->SetPos( GetWorldPos() + Vec3(3,0,0) );
		SetLookAt( camTarget );
	}
}

//////////////////////////////////////////////////////////////////////////
void CCameraObject::Display( DisplayContext &dc )
{
	const Matrix44 &wtm = GetWorldTM();
	Vec3 wp = wtm.GetTranslationOLD();

	float fScale = dc.view->GetScreenScaleFactor(wp) * 0.03f;

	if (GetLookAt())
	{
		// Look at camera.
		if (IsFrozen())
			dc.SetFreezeColor();
		else
			dc.SetColor( GetColor() );

		bool bSelected = IsSelected();

		if (bSelected || GetLookAt()->IsSelected())
		{
			dc.SetSelectedColor();
		}

		// Line from source to target.
		dc.DrawLine( wp,GetLookAt()->GetWorldPos() );

		if (bSelected)
		{
			dc.SetSelectedColor();
		}
		else if (IsFrozen())
			dc.SetFreezeColor();
		else
			dc.SetColor( GetColor() );

		dc.PushMatrix( wtm );
		
		Vec3 sz(0.2f*fScale,0.1f*fScale,0.2f*fScale);
		dc.DrawWireBox( -sz,sz );

		float dist = 1.0f;
		if (bSelected)
		{
			dist = GetDistance(GetLookAt()->GetWorldPos(),wtm.GetTranslationOLD());
			DrawCone( dc,dist );
		}
		else
		{
			DrawCone( dc,dist,fScale );
		}
		dc.PopMatrix();
	}
	else
	{
		// Free camera
		if (IsSelected())
			dc.SetSelectedColor();
		else if (IsFrozen())
			dc.SetFreezeColor();
		else
			dc.SetColor( GetColor() );
		
		dc.PushMatrix( wtm );
		
		Vec3 sz(0.2f*fScale,0.1f*fScale,0.2f*fScale);
		dc.DrawWireBox( -sz,sz );

		float dist = CAMERA_CONE_LENGTH;
		DrawCone( dc,dist,fScale );
		dc.PopMatrix();
	}

	//dc.DrawIcon( ICON_QUAD,wp,0.1f*dc.view->GetScreenScaleFactor(wp) );

	DrawDefault( dc );
}

//////////////////////////////////////////////////////////////////////////
bool CCameraObject::HitTest( HitContext &hc )
{
	Vec3 origin = GetWorldPos();
	float radius = CAMERABOX_RADIUS/2.0f;

	float fScale = hc.view->GetScreenScaleFactor(origin) * 0.03f;

	Vec3 w = origin - hc.raySrc;
	w = hc.rayDir.Cross( w );
	float d = w.Length();

	if (d < radius*fScale + hc.distanceTollerance)
	{
		hc.dist = GetDistance(hc.raySrc,origin);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CCameraObject::HitTestRect( HitContext &hc )
{
	// transform all 8 vertices into world space
	CPoint p = hc.view->WorldToView( GetWorldPos() );
	if (hc.rect.PtInRect(p))
	{
		hc.object = this;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CCameraObject::GetBoundBox( BBox &box )
{
	Vec3 pos = GetWorldPos();
	float r;
	r = 1;
	box.min = pos - Vec3(r,r,r);
	box.max = pos + Vec3(r,r,r);
	if (GetLookAt())
	{
		box.Add( GetLookAt()->GetWorldPos() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CCameraObject::GetLocalBounds( BBox &box )
{
	GetBoundBox( box );
	Matrix44 invTM( GetWorldTM() );
	invTM.Invert44();
	box.Transform( invTM );
}

//////////////////////////////////////////////////////////////////////////
float CCameraObject::GetFOV() const
{
	return mv_fov;
}

//////////////////////////////////////////////////////////////////////////
float CCameraObject::GetAspect() const
{
	return 1.0f;
}

//////////////////////////////////////////////////////////////////////////
void CCameraObject::GetConePoints( Vec3 q[4],float dist )
{
	if (dist > 1e8f)
		dist = 1e8f;
	float ta = (float)tan(0.5f*GetFOV());
	float w = dist * ta;
//	float h = w * (float).75; //  ASPECT ??
	float h = w / GetAspect();
	
	//q[0] = Vec3( w, h,-dist);
	//q[1] = Vec3(-w, h,-dist);
	//q[2] = Vec3(-w,-h,-dist);
	//q[3] = Vec3( w,-h,-dist);

	q[0] = Vec3( w,-dist, h);
	q[1] = Vec3(-w,-dist, h);
	q[2] = Vec3(-w,-dist,-h);
	q[3] = Vec3( w,-dist,-h);
}

void CCameraObject::DrawCone( DisplayContext &dc,float dist,float fScale )
{
	Vec3 q[4];
	GetConePoints(q,dist);

	q[0] *= fScale;
	q[1] *= fScale;
	q[2] *= fScale;
	q[3] *= fScale;

	Vec3 org(0,0,0);
	dc.DrawLine( org,q[0] );
	dc.DrawLine( org,q[1] );
	dc.DrawLine( org,q[2] );
	dc.DrawLine( org,q[3] );

	// Draw quad.
	dc.DrawPolyLine( q,4 );

	// Draw cross.
	//dc.DrawLine( q[0],q[2] );
	//dc.DrawLine( q[1],q[3] );
}

	/*

void CCameraObject::DrawCone( DisplayContext &dc )
{
	float dist = 10;

	Vec3 q[5], u[3];
	GetConePoints(q,dist);

	if (colid)	gw->setColor( LINE_COLOR, GetUIColor(colid));
	if (drawDiags) {
		u[0] =  q[0];	u[1] =  q[2];	
		gw->polyline( 2, u, NULL, NULL, FALSE, NULL );	
		u[0] =  q[1];	u[1] =  q[3];	
		gw->polyline( 2, u, NULL, NULL, FALSE, NULL );	
	}
	gw->polyline( 4, q, NULL, NULL, TRUE, NULL );	
	if (drawSides) {
		gw->setColor( LINE_COLOR, GetUIColor(COLOR_CAMERA_CONE));
		u[0] = Point3(0,0,0);
		for (int i=0; i<4; i++) {
			u[1] =  q[i];	
			gw->polyline( 2, u, NULL, NULL, FALSE, NULL );	
		}
	}
}
	*/

//////////////////////////////////////////////////////////////////////////
void CCameraObject::OnFovChange( IVariable *var )
{
	if (m_animNode)
	{
		float fov = mv_fov;
		m_animNode->SetParamValue( GetIEditor()->GetAnimation()->GetTime(),APARAM_FOV, RAD2DEG(fov) );
	}
}

/*
//////////////////////////////////////////////////////////////////////////
void CCameraObject::OnPropertyChange( const CString &property )
{
	CEntity::OnPropertyChange( property );

	float fov = m_fov;
	GetParams()->getAttr( "FOV",fov );
	m_fov = fov;
	if (m_animNode)
		m_animNode->SetParam( GetIEditor()->GetAnimation()->GetTime(),APARAM_FOV, fov );
}
*/


//////////////////////////////////////////////////////////////////////////
//
// CCameraObjectTarget implementation.
//
//////////////////////////////////////////////////////////////////////////
CCameraObjectTarget::CCameraObjectTarget()
{
	SetColor(CAMERA_COLOR);
}

bool CCameraObjectTarget::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	SetColor( CAMERA_COLOR );
	bool res = CEntity::Init( ie,prev,file );
	m_entityClass = "CameraTargetPoint";
	return res;
}

//////////////////////////////////////////////////////////////////////////
void CCameraObjectTarget::Display( DisplayContext &dc )
{
	Vec3 wp = GetWorldPos();

	float fScale = dc.view->GetScreenScaleFactor(wp) * 0.03f;

	if (IsSelected())
		dc.SetSelectedColor();
	else if (IsFrozen())
		dc.SetFreezeColor();
	else
		dc.SetColor( GetColor() );
		
	Vec3 sz(0.2f*fScale,0.2f*fScale,0.2f*fScale);
	dc.DrawWireBox( wp-sz,wp+sz );

	DrawDefault( dc );
}

//////////////////////////////////////////////////////////////////////////
bool CCameraObjectTarget::HitTest( HitContext &hc )
{
	Vec3 origin = GetWorldPos();
	float radius = CAMERABOX_RADIUS/2.0f;

	float fScale = hc.view->GetScreenScaleFactor(origin) * 0.03f;

	Vec3 w = origin - hc.raySrc;
	w = hc.rayDir.Cross( w );
	float d = w.Length();

	if (d < radius*fScale + hc.distanceTollerance)
	{
		hc.dist = GetDistance(hc.raySrc,origin);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CCameraObjectTarget::GetBoundBox( BBox &box )
{
	Vec3 pos = GetWorldPos();
	float r = CAMERABOX_RADIUS;
	box.min = pos - Vec3(r,r,r);
	box.max = pos + Vec3(r,r,r);
}

void CCameraObjectTarget::Serialize( CObjectArchive &ar )
{
	CEntity::Serialize( ar );
}