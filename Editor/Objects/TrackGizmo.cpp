////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackgizmo.cpp
//  Version:     v1.00
//  Created:     2/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TrackGizmo.h"
#include "DisplayContext.h"

#include "..\Viewport.h"
#include "..\DisplaySettings.h"
#include "ObjectManager.h"

#include <IMovieSystem.h>

//////////////////////////////////////////////////////////////////////////
// CTrackGizmo implementation.
//////////////////////////////////////////////////////////////////////////
#define AXIS_SIZE 0.1f

namespace {
	int s_highlightAxis = 0;
}

//////////////////////////////////////////////////////////////////////////
CTrackGizmo::CTrackGizmo()
{
	m_animNode = 0;

	m_bbox.min = Vec3(-10000,-10000,-10000);
	m_bbox.max = Vec3(10000,10000,10000);
	m_keysSelected = false;
}

//////////////////////////////////////////////////////////////////////////
CTrackGizmo::~CTrackGizmo()
{
}

//////////////////////////////////////////////////////////////////////////
void CTrackGizmo::Display( DisplayContext &dc )
{
	if (!(dc.flags & DISPLAY_TRACKS))
		return;

	if (!m_animNode)
		return;

	uint hideMask = GetIEditor()->GetDisplaySettings()->GetObjectHideMask();

	CAnimationContext *ac = GetIEditor()->GetAnimation();

	// Should have animation sequence.
	if (!ac->GetSequence())
		return;

	IAnimBlock *ablock = m_animNode->GetAnimBlock();
	if (!ablock)
		return;

	m_keysSelected = false;

	// Must have non empty position track.
	IAnimTrack *track = ablock->GetTrack(APARAM_POS);
	if (!track)
		return;

	int nkeys = track->GetNumKeys();
	if (nkeys < 2)
		return;

	Range range = ac->GetTimeRange();
	range.start = __min(range.end,track->GetKeyTime(0));
	range.end = __min(range.end,track->GetKeyTime(nkeys-1));
	//float step = range.Length() / 100.0f;
	//step = min(step,0.01f);
	float step = 0.1f;

	bool bTicks = (dc.flags & DISPLAY_TRACKTICKS) == DISPLAY_TRACKTICKS;

	// Get Spline color.
	Vec3 splineCol(0.5f,0.3f,1);
	Vec3 timeCol(0,1,0);

	m_bbox.Reset();
	
	float zOffset = 0.01f;
	Vec3 p0,p1;
	Vec3 tick(0,0,0.05f);
	track->GetValue( range.start,p0 );
	p0.z += zOffset;

	// Update bounding box.
	m_bbox.Add( p0 );

	const Matrix44 &wtm = GetMatrix();
	p0 = wtm.TransformPointOLD(p0);

	for (float t = range.start+step; t < range.end; t += step)
	{
		track->GetValue( t,p1 );
		// Update bounding box.
		m_bbox.Add( p1 );

		p1 = wtm.TransformPointOLD(p1);

		p1.z += zOffset;
		// Get Spline color.
		//dc.SetColor( splineCol.x,splineCol.y,splineCol.z,1 );
		if (bTicks)
			dc.DrawLine( p0-tick,p0+tick,timeCol,timeCol );
		dc.DrawLine( p0,p1,splineCol,splineCol );
		p0 = p1;
	}

	// Get Key color.
	dc.SetColor( 1,0,0,1 );
	float sz = 0.2f;
	for (int i = 0; i < nkeys; i++)
	{
		float t = track->GetKeyTime(i);
		track->GetValue( t,p0 );
		p0 = wtm.TransformPointOLD(p0);
		p0.z += zOffset;

		//float sz = 0.01f * dc.view->GetScreenScaleFactor(p0);
		float sz = 0.005f * dc.view->GetScreenScaleFactor(p0);

		// Draw quad.
		//dc.DrawBall( p0,sz );
		dc.DrawWireBox( p0-Vec3(sz,sz,sz),p0+Vec3(sz,sz,sz) );

		if (track->GetKeyFlags(i) & AKEY_SELECTED)
		{
			m_keysSelected = true;
			DrawAxis( dc,p0 );
			dc.SetColor( 1,0,0,1 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTrackGizmo::SetAnimNode( IAnimNode *node )
{
	m_animNode = node;
}

//////////////////////////////////////////////////////////////////////////
void CTrackGizmo::GetWorldBounds( BBox &bbox )
{
	bbox = m_bbox;
}

//////////////////////////////////////////////////////////////////////////
void CTrackGizmo::DrawAxis( DisplayContext &dc,const Vec3 &org )
{
	float size = AXIS_SIZE;
	
	int prevRState = dc.SetStateFlag(GS_NODEPTHTEST);

	Vec3 x(size,0,0);
	Vec3 y(0,size,0);
	Vec3 z(0,0,size);

	float fScreenScale = dc.view->GetScreenScaleFactor(org);
	x = x * fScreenScale;
	y = y * fScreenScale;
	z = z * fScreenScale;
	
	float col[4] = { 1,1,1,1 };
	float hcol[4] = { 1,0,0,1 };
	dc.renderer->DrawLabelEx( org+x,1.2f,col,true,true,"X" );
	dc.renderer->DrawLabelEx( org+y,1.2f,col,true,true,"Y" );
	dc.renderer->DrawLabelEx( org+z,1.2f,col,true,true,"Z" );

	Vec3 colX(1,0,0),colY(0,1,0),colZ(0,0,1);
	if (s_highlightAxis)
	{
		float col[4] = { 1,0,0,1 };
		if (s_highlightAxis == 1)
		{
			colX(1,1,0);
			dc.renderer->DrawLabelEx( org+x,1.2f,col,true,true,"X" );
		}
		if (s_highlightAxis == 2)
		{
			colY(1,1,0);
			dc.renderer->DrawLabelEx( org+y,1.2f,col,true,true,"Y" );
		}
		if (s_highlightAxis == 3)
		{
			colZ(1,1,0);
			dc.renderer->DrawLabelEx( org+z,1.2f,col,true,true,"Z" );
		}
	}

	x = x * 0.8f;
	y = y * 0.8f;
	z = z * 0.8f;
	float fArrowScale = fScreenScale * 0.07f;
	dc.SetColor( colX );
	dc.DrawArrow( org,org+x,fArrowScale );
	dc.SetColor( colY );
	dc.DrawArrow( org,org+y,fArrowScale );
	dc.SetColor( colZ );
	dc.DrawArrow( org,org+z,fArrowScale );

	dc.SetState( prevRState );
}

//////////////////////////////////////////////////////////////////////////
bool CTrackGizmo::HitTest( HitContext &hc )
{
	return false;
}