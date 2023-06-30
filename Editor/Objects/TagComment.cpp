////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tagcomment.cpp
//  Version:     v1.00
//  Created:     6/5/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Special tag point for comment.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TagComment.h"

#include "..\Viewport.h"
#include "Settings.h"

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CTagComment,CBaseObject)

#define TAGCOMMENT_RADIUS 0.2f

//////////////////////////////////////////////////////////////////////////
float CTagComment::m_helperScale = 1;

//////////////////////////////////////////////////////////////////////////
CTagComment::CTagComment()
{
	SetColor( RGB(255,160,0) );

	AddVariable( mv_comment,"Comment" );
	AddVariable( mv_fixed,"Fixed" );
}

//////////////////////////////////////////////////////////////////////////
float CTagComment::GetRadius()
{
	return TAGCOMMENT_RADIUS*m_helperScale*gSettings.gizmo.helpersScale;
}

//////////////////////////////////////////////////////////////////////////
void CTagComment::SetScale( const Vec3d &scale )
{
	// Ignore scale.
}

//////////////////////////////////////////////////////////////////////////
int CTagComment::MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
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
				pos.z = GetIEditor()->GetTerrainElevation(pos.x,pos.y) + GetRadius()*2;
			}
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
void CTagComment::Display( DisplayContext &dc )
{
	const Matrix44 &wtm = GetWorldTM();

	Vec3 wp = wtm.GetTranslationOLD();

	if (IsFrozen())
		dc.SetFreezeColor();
	else
	{
		if (!mv_fixed)
			dc.SetColor( GetColor(),0.8f );
		else
			// Fixed color.
			dc.SetColor( RGB(0,255,0),0.8f );
	}

	float fHelperScale = 1*m_helperScale*gSettings.gizmo.helpersScale;
	Vec3 x = wtm.TransformVectorOLD( Vec3(0,-fHelperScale,0) );
	dc.DrawArrow( wp,wp+x*2,fHelperScale );

	dc.DrawBall( wp,GetRadius() );

	if (IsSelected())
	{
		dc.SetSelectedColor(0.6f);
		dc.DrawBall( wp,GetRadius()+0.02f );
	}
	
	//if (!IsSelected())
//		dc.SetColor( RGB(255,255,255) );
	const char *szText = (CString)mv_comment;
	dc.DrawTextLabel( wp+Vec3(0,0,GetRadius()),1.2f,szText );

	DrawDefault( dc );
}

//////////////////////////////////////////////////////////////////////////
bool CTagComment::HitTest( HitContext &hc )
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
void CTagComment::GetBoundBox( BBox &box )
{
	Vec3 pos = GetWorldPos();
	float r = GetRadius();
	box.min = pos - Vec3(r,r,r);
	box.max = pos + Vec3(r,r,r);
}

//////////////////////////////////////////////////////////////////////////
void CTagComment::GetLocalBounds( BBox &box )
{
	float r = GetRadius();
	box.min = -Vec3(r,r,r);
	box.max = Vec3(r,r,r);
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CTagComment::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	// Dont export this. Only relevant for editor.
	return 0;
}