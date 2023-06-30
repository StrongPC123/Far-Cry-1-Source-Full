////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   axisgizmo.cpp
//  Version:     v1.00
//  Created:     2/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AxisGizmo.h"
#include "DisplayContext.h"

#include "..\Viewport.h"
#include "..\DisplaySettings.h"
#include "ObjectManager.h"

#include "GizmoManager.h"
#include "Settings.h"
#include "Cry_Geo.h"

//////////////////////////////////////////////////////////////////////////
// CAxisGizmo implementation.
//////////////////////////////////////////////////////////////////////////
int CAxisGizmo::m_axisGizmoCount = 0;

#define PLANE_SCALE (0.3f)
#define HIT_RADIUS (4)

//////////////////////////////////////////////////////////////////////////
CAxisGizmo::CAxisGizmo( CBaseObject *object )
{
	assert( object != 0 );
	m_object = object;

	// Set selectable flag.
	SetFlags( EGIZMO_SELECTABLE );

	m_axisGizmoCount++;
	m_object->AddEventListener( functor(*this,&CAxisGizmo::OnObjectEvent) );
}

//////////////////////////////////////////////////////////////////////////
CAxisGizmo::~CAxisGizmo()
{
	m_object->RemoveEventListener( functor(*this,&CAxisGizmo::OnObjectEvent) );
	m_axisGizmoCount--;
}

//////////////////////////////////////////////////////////////////////////
void CAxisGizmo::OnObjectEvent( CBaseObject *object,int event )
{
	if (event == CBaseObject::ON_DELETE || event == CBaseObject::ON_UNSELECT)
	{
		// This gizmo must be deleted as well.
		GetGizmoManager()->RemoveGizmo(this);
		return;
	}
}

//////////////////////////////////////////////////////////////////////////
void CAxisGizmo::Display( DisplayContext &dc )
{
	bool bNotSelected = !m_object->CheckFlags(OBJFLAG_VISIBLE) || !m_object->IsSelected();
	if (bNotSelected)
	{
		// This gizmo must be deleted.
		DeleteThis();
		return;
	}
	DrawAxis( dc );
}

//////////////////////////////////////////////////////////////////////////
void CAxisGizmo::GetWorldBounds( BBox &bbox )
{
	m_object->GetBoundBox( bbox );
}

//////////////////////////////////////////////////////////////////////////
void CAxisGizmo::DrawAxis( DisplayContext &dc )
{
	float fScreenScale = dc.view->GetScreenScaleFactor(m_object->GetWorldPos());
	float size = gSettings.gizmo.axisGizmoSize * fScreenScale;

	int prevRState = dc.GetState();

	if (!(dc.flags & DISPLAY_2D))
  	prevRState = dc.SetStateFlag(GS_NODEPTHTEST);
	Vec3 x(size,0,0);
	Vec3 y(0,size,0);
	Vec3 z(0,0,size);

	Matrix44 tm;

	RefCoordSys refCoordSys = GetIEditor()->GetReferenceCoordSys();
	if (refCoordSys == COORDS_LOCAL)
	{
		tm = m_object->GetWorldTM();
		tm.NoScale();
	}
	else
	{
		tm.SetIdentity();
		if (dc.flags & DISPLAY_2D)
		{
			tm = dc.view->GetViewTM();
		}
		tm.SetTranslationOLD( m_object->GetWorldPos() );
	}

	dc.PushMatrix(tm);

	bool bNeedX = true;
	bool bNeedY = true;
	bool bNeedZ = true;

	if (dc.flags & DISPLAY_2D)
	{
		if (dc.view->GetType() == ET_ViewportXY)
			bNeedZ = false;
		else if (dc.view->GetType() == ET_ViewportXZ)
			bNeedY = false;
		else if (dc.view->GetType() == ET_ViewportYZ)
			bNeedX = false;

		if (refCoordSys == COORDS_VIEW)
		{
			bNeedX = true;
			bNeedY = true;
			bNeedZ = false;
		}
	}

	Vec3 colSelected(1,1,0);
	Vec3 axisColor(1,1,1);

	if (dc.flags & DISPLAY_2D)
	{
		//axisColor = Vec3(0,0,0);
		//colSelected = Vec3(0.8f,0.8f,0);
	}

	dc.SetColor(axisColor);
	if (bNeedX && gSettings.gizmo.axisGizmoText)
		dc.DrawTextLabel( tm.TransformPointOLD(x),1.0f,"X" );
	if (bNeedY && gSettings.gizmo.axisGizmoText)
		dc.DrawTextLabel( tm.TransformPointOLD(y),1.0f,"Y" );
	if (bNeedZ && gSettings.gizmo.axisGizmoText)
		dc.DrawTextLabel( tm.TransformPointOLD(z),1.0f,"Z" );

	int axis = GetIEditor()->GetAxisConstrains();
	if (m_highlightAxis)
		axis = m_highlightAxis;

	float linew[3];
	linew[0] = linew[1] = linew[2] = 1;
	Vec3 colX(1,0,0),colY(0,1,0),colZ(0,0,1);
	Vec3 colXArrow=colX,colYArrow=colY,colZArrow=colZ;
	if (axis)
	{
		float col[4] = { 1,0,0,1 };
		if (axis == AXIS_X || axis == AXIS_XY || axis == AXIS_XZ)
		{
			colX = colSelected;
			dc.SetColor(colSelected);
			if (bNeedX && gSettings.gizmo.axisGizmoText)
				dc.DrawTextLabel( tm.TransformPointOLD(x),1.0f,"X" );
			linew[0] = 2;
		}
		if (axis == AXIS_Y || axis == AXIS_XY || axis == AXIS_YZ)
		{
			colY = colSelected;
			dc.SetColor(colSelected);
			if (bNeedY && gSettings.gizmo.axisGizmoText)
				dc.DrawTextLabel( tm.TransformPointOLD(y),1.0f,"Y" );
			linew[1] = 2;
		}
		if (axis == AXIS_Z || axis == AXIS_XZ || axis == AXIS_YZ)
		{
			colZ = colSelected;
			dc.SetColor(colSelected);
			if (bNeedZ && gSettings.gizmo.axisGizmoText)
				dc.DrawTextLabel( tm.TransformPointOLD(z),1.0f,"Z" );
			linew[2] = 2;
		}
	}

	x = x * 0.8f;
	y = y * 0.8f;
	z = z * 0.8f;
	float fArrowScale = fScreenScale * 0.07f;
	
	if (bNeedX)
	{
		dc.SetColor( colX );
		dc.SetLineWidth( linew[0] );
		dc.DrawLine( Vec3(0,0,0),x );
		dc.SetColor( colXArrow );
		dc.DrawArrow( x-x*0.1f,x,fArrowScale );
	}
	if (bNeedY)
	{
		dc.SetColor( colY );
		dc.SetLineWidth( linew[1] );
		dc.DrawLine( Vec3(0,0,0),y );
		dc.SetColor( colYArrow );
		dc.DrawArrow( y-y*0.1f,y,fArrowScale );
	}
	if (bNeedZ)
	{
		dc.SetColor( colZ );
		dc.SetLineWidth( linew[2] );
		dc.DrawLine( Vec3(0,0,0),z );
		dc.SetColor( colZArrow );
		dc.DrawArrow( z-z*0.1f,z,fArrowScale );
	}

	dc.SetLineWidth(1);

	//////////////////////////////////////////////////////////////////////////
	// Draw axis planes.
	//////////////////////////////////////////////////////////////////////////
	Vec3 colXY[2];
	Vec3 colXZ[2];
	Vec3 colYZ[2];

	colX = Vec3(1,0,0);
	colY = Vec3(0,1,0);
	colZ = Vec3(0,0,1);
	colXY[0] = colX;
	colXY[1] = colY;
	colXZ[0] = colX;
	colXZ[1] = colZ;
	colYZ[0] = colY;
	colYZ[1] = colZ;
	
	linew[0] = linew[1] = linew[2] = 1;
	if (axis)
	{
		if (axis == AXIS_XY)
		{
			colXY[0] = colSelected;
			colXY[1] = colSelected;
			linew[0] = 2;
		}
		else if (axis == AXIS_XZ)
		{
			colXZ[0] = colSelected;
			colXZ[1] = colSelected;
			linew[1] = 2;
		}
		else if (axis == AXIS_YZ)
		{
			colYZ[0] = colSelected;
			colYZ[1] = colSelected;
			linew[2] = 2;
		}
	}

	dc.SetColor( RGB(255,255,0),0.5f );
	//dc.DrawQuad( org,org+z*0.5f,org+z*0.5f+x*0.5f,org+x*0.5f );
	size *= PLANE_SCALE;
	Vec3 p1(size,size,0);
	Vec3 p2(size,0,size);
	Vec3 p3(0,size,size);
	
	float colAlpha = 1.0f;
	x *= PLANE_SCALE; y *= PLANE_SCALE; z *= PLANE_SCALE;

	// XY
	if (bNeedX && bNeedY)
	{
		dc.SetLineWidth( linew[0] );
		dc.SetColor( colXY[0],colAlpha );
		dc.DrawLine( p1,p1-x );
		dc.SetColor( colXY[1],colAlpha );
		dc.DrawLine( p1,p1-y );
	}

	// XZ
	if (bNeedX && bNeedZ)
	{
		dc.SetLineWidth( linew[1] );
		dc.SetColor( colXZ[0],colAlpha );
		dc.DrawLine( p2,p2-x );
		dc.SetColor( colXZ[1],colAlpha );
		dc.DrawLine( p2,p2-z );
	}
	
	// YZ
	if (bNeedY && bNeedZ)
	{
		dc.SetLineWidth( linew[2] );
		dc.SetColor( colYZ[0],colAlpha );
		dc.DrawLine( p3,p3-y );
		dc.SetColor( colYZ[1],colAlpha );
		dc.DrawLine( p3,p3-z );
	}
	
	dc.SetLineWidth(1);

	colAlpha = 0.25f;

	if (axis == AXIS_XY && bNeedX && bNeedY)
	{
		dc.renderer->SetCullMode( R_CULL_DISABLE );
		dc.SetColor( colSelected,colAlpha );
		dc.DrawQuad( p1,p1-x,p1-x-y,p1-y );
		dc.renderer->SetCullMode();
	}
	else if (axis == AXIS_XZ && bNeedX && bNeedZ)
	{
		dc.renderer->SetCullMode( R_CULL_DISABLE );
		dc.SetColor( colSelected,colAlpha );
		dc.DrawQuad( p2,p2-x,p2-x-z,p2-z );
		dc.renderer->SetCullMode();
	}
	else if (axis == AXIS_YZ && bNeedY && bNeedZ)
	{
		dc.renderer->SetCullMode( R_CULL_DISABLE );
		dc.SetColor( colSelected,colAlpha );
		dc.DrawQuad( p3,p3-y,p3-y-z,p3-z );
		dc.renderer->SetCullMode();
	}

	if (!(dc.flags & DISPLAY_2D))
		dc.SetState( prevRState );

	dc.PopMatrix();
}

//////////////////////////////////////////////////////////////////////////
bool CAxisGizmo::HitTest( HitContext &hc )
{
	if (hc.distanceTollerance != 0)
		return 0;

	Vec3 org = m_object->GetWorldPos();

	float fScreenScale = hc.view->GetScreenScaleFactor(org);
	float size = gSettings.gizmo.axisGizmoSize * fScreenScale;

	Vec3 x(size,0,0);
	Vec3 y(0,size,0);
	Vec3 z(0,0,size);

	float hitDist = 0.01f * fScreenScale;

	//////////////////////////////////////////////////////////////////////////
	// Calculate ray in local space of axis.
	//////////////////////////////////////////////////////////////////////////
	Matrix44 tm;
	RefCoordSys refCoordSys = GetIEditor()->GetReferenceCoordSys();
	if (refCoordSys == COORDS_LOCAL)
	{
		tm = m_object->GetWorldTM();
		tm.NoScale();
	}
	else
	{
		tm.SetIdentity();
		//if (hc.view->IS2.flags & DISPLAY_2D)
		{
			tm = hc.view->GetViewTM();
		}
		tm.SetTranslationOLD( m_object->GetWorldPos() );
	}

	Vec3 pos = tm.GetTranslationOLD();

	Vec3 intPoint;
	Ray ray(hc.raySrc,hc.rayDir);
	Sphere sphere( pos,size );
	if (!Intersect::Ray_SphereFirst( ray,sphere,intPoint ))
	{
		m_highlightAxis = 0;
		return false;
	}

	x = tm.TransformVectorOLD(x);
	y = tm.TransformVectorOLD(y);
	z = tm.TransformVectorOLD(z);

	size *= PLANE_SCALE;
	Vec3 p1(size,size,0);
	Vec3 p2(size,0,size);
	Vec3 p3(0,size,size);

	p1 = tm.TransformPointOLD(p1);
	p2 = tm.TransformPointOLD(p2);
	p3 = tm.TransformPointOLD(p3);

	Vec3 planeX = x*PLANE_SCALE;
	Vec3 planeY = y*PLANE_SCALE;
	Vec3 planeZ = z*PLANE_SCALE;

	int hitRadius = HIT_RADIUS;
	int axis = 0;
	if (hc.view->HitTestLine( pos,pos+x,hc.point2d,hitRadius ))
		axis = AXIS_X;
	else if (hc.view->HitTestLine( pos,pos+y,hc.point2d,hitRadius ))
		axis = AXIS_Y;
	else if (hc.view->HitTestLine( pos,pos+z,hc.point2d,hitRadius ))
		axis = AXIS_Z;
	else if (hc.view->HitTestLine( p1,p1-planeX,hc.point2d,hitRadius ))
		axis = AXIS_XY;
	else if (hc.view->HitTestLine( p1,p1-planeY,hc.point2d,hitRadius ))
		axis = AXIS_XY;
	else if (hc.view->HitTestLine( p2,p2-planeX,hc.point2d,hitRadius ))
		axis = AXIS_XZ;
	else if (hc.view->HitTestLine( p2,p2-planeZ,hc.point2d,hitRadius ))
		axis = AXIS_XZ;
	else if (hc.view->HitTestLine( p3,p3-planeY,hc.point2d,hitRadius ))
		axis = AXIS_YZ;
	else if (hc.view->HitTestLine( p3,p3-planeZ,hc.point2d,hitRadius ))
		axis = AXIS_YZ;

	if (axis != 0)
	{
		hc.axis = axis;
		hc.object = m_object;
		hc.dist = 0;
	}

	m_highlightAxis = axis;
	
	return axis != 0;
}
