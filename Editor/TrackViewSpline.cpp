////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackviewspline.cpp
//  Version:     v1.00
//  Created:     7/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrackViewSpline.h"

#include "IMovieSystem.h"

// CTrackViewSpline

IMPLEMENT_DYNAMIC(CTrackViewSpline, CWnd)
CTrackViewSpline::CTrackViewSpline()
{
	m_ticksStep = 0.1f;
}

CTrackViewSpline::~CTrackViewSpline()
{
}


BEGIN_MESSAGE_MAP(CTrackViewSpline, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CTrackViewSpline message handlers


void CTrackViewSpline::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages
	CRect rc;
	GetClientRect( rc );
	
	CPoint org;
	int y = (rc.top + rc.bottom)/2;

	org.x = 10;
	org.y = y;
  
	// Draw axis.
	dc.MoveTo( org );
	dc.LineTo( org+CPoint(rc.right-rc.left-org.x,0) );
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewSpline::SetTrack( IAnimTrack *track )
{
	m_track = track;
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewSpline::SetTimeRange( float start,float end )
{
	m_timeRange.Set( start,end );
	Invalidate(TRUE);
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewSpline::SetTimeScale( float timeScale )
{
	m_timeScale = timeScale;

	Invalidate(TRUE);
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewSpline::SetCurrTime( float currTime )
{
	m_currTime = currTime;
}