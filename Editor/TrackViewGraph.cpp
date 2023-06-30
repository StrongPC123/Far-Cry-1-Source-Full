////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackviewgraph.cpp
//  Version:     v1.00
//  Created:     23/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrackViewGraph.h"
#include "Controls\MemDC.h"

#include "TrackViewDialog.h"
#include "AnimationContext.h"

//#include "IMovieSystem.h"

enum ETVMouseMode
{
	MOUSE_MODE_NONE = 0,
	MOUSE_MODE_SELECT = 1,
	MOUSE_MODE_MOVE,
	MOUSE_MODE_CLONE,
	MOUSE_MODE_DRAGTIME
};

#define KEY_TEXT_COLOR RGB(255,255,255)

// CTrackViewGraph

IMPLEMENT_DYNAMIC(CTrackViewGraph, CTrackViewKeys)

CTrackViewGraph::CTrackViewGraph()
{
}

CTrackViewGraph::~CTrackViewGraph()
{
}


BEGIN_MESSAGE_MAP(CTrackViewGraph, CTrackViewKeys)
END_MESSAGE_MAP()

// CTrackViewGraph message handlers

//////////////////////////////////////////////////////////////////////////
void CTrackViewGraph::DrawTrack( int item,CDC *dc,CRect &rcItem )
{
	IAnimTrack *track = GetTrack(item);
	if (!track)
		return;

	if (item == m_selected)
		DrawGraph( track,dc,rcItem );	
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewGraph::DrawKeys( IAnimTrack *track,CDC *dc,CRect &rc,Range &timeRange )
{
	/*
	CPen selPen( PS_SOLID,1,RGB(255,255,0) );
	
	char keydesc[1024];
	int numKeys = track->GetNumKeys();

	// If this track is boolean draw bars to show true value.
	if (track->GetValueType() == ATRACK_BOOL)
	{
		int x0 = TimeToClient(timeRange.start);
		float t0 = timeRange.start;
		CRect itemrc;

		CBrush *prevBrush = dc->SelectObject( &m_visibilityBrush );
		for (int i = 0; i < numKeys; i++)
		{
			float time = track->GetKeyTime(i);
			if (time < timeRange.start)
				continue;
			if (time > timeRange.end)
				break;

			int x = TimeToClient(time);
			bool val = false;
			track->GetValue( time-0.001f,val );
			if (val)
			{
				dc->Rectangle( x0,rc.top+5,x,rc.bottom-5 );
			}

			t0 = time;
			x0 = x;
		}
		int x = TimeToClient(timeRange.end);
		bool val = false;
		track->GetValue( timeRange.end-0.001f,val );
		if (val)
		{
			dc->Rectangle( x0,rc.top+5,x,rc.bottom-5 );
		}
		dc->SelectObject( &prevBrush );
	}

	CFont *prevFont = dc->SelectObject( m_descriptionFont );

	dc->SetTextColor(KEY_TEXT_COLOR);
	dc->SetBkMode(TRANSPARENT);

	float time0 = FLT_MIN;
	// Draw keys.
	for (int i = 0; i < numKeys; i++)
	{
		float time = track->GetKeyTime(i);
		
		// Get info about that key.
		const char *description = NULL;
		float duration = 0;
		track->GetKeyInfo(i,description,duration);

		if (time+duration < timeRange.start)
			continue;
		if (time > timeRange.end)
			break;
		
		int x = TimeToClient(time);
		if (duration > 0)
		{
			// Draw key duration.
			int x1 = TimeToClient(time+duration);
			if (x1 < 0)
			{
				if (x > 0)
					x1 = rc.right;
			}
			CBrush *prevBrush = dc->SelectObject( &m_visibilityBrush );
			dc->Rectangle( x,rc.top+3,x1+1,rc.bottom-3 );
			dc->SelectObject( &prevBrush );
			dc->MoveTo(x1,rc.top);
			dc->LineTo(x1,rc.bottom);
		}

		if (description)
		{
			strcpy(keydesc,"{");
			strcat(keydesc,description);
			strcat(keydesc,"}");
			// Draw key description text.
			// Find next key.
			int x1 = x + 200;
			if (x < 0)
				x1 = rc.right;
			if (i+1 < numKeys)
			{
				x1 = TimeToClient( track->GetKeyTime(i+1) ) - 10;
			}
			CRect textRect(x+5,rc.top,x1,rc.bottom );
			textRect &= rc;
			dc->DrawText( keydesc,strlen(keydesc),textRect,DT_LEFT|DT_END_ELLIPSIS|DT_VCENTER|DT_SINGLELINE  );
		}

		if (x < 0)
			continue;

		if (time == time0)
		{
			// If two keys on the same time.
			m_imageList.Draw( dc,2,CPoint(x-6,rc.top+2),ILD_TRANSPARENT );
		}
		else
		{
			if (track->GetKeyFlags(i) & AKEY_SELECTED)
			{
				CPen *prevPen = dc->SelectObject( &selPen );
				dc->MoveTo(x,m_rcClient.top);
				dc->LineTo(x,m_rcClient.bottom);
				dc->SelectObject(prevPen);
				m_imageList.Draw( dc,1,CPoint(x-6,rc.top+2),ILD_TRANSPARENT );
			}
			else
			{
				m_imageList.Draw( dc,0,CPoint(x-6,rc.top+2),ILD_TRANSPARENT );
			}
		}

		time0 = time;
	}
	dc->SelectObject( prevFont );
	*/
}

int CTrackViewGraph::KeyFromPoint( CPoint point )
{
	float t1 = TimeFromPointUnsnapped( CPoint(point.x-4,point.y) );
	float t2 = TimeFromPointUnsnapped( CPoint(point.x+4,point.y) );
	IAnimTrack *track = GetTrack(GetCurSel());
	if (!track)
		return -1;
	int numKeys = track->GetNumKeys();
	for (int i = 0; i < numKeys; i++)
	{
		float time = track->GetKeyTime(i);
		if (time >= t1 && time <= t2)
		{
			return i;
		}
	}
	return -1;
}

void CTrackViewGraph::SelectKeys( const CRect &rc )
{
	/*
	// put selection rectangle from client to item space.
	CRect rci = rc;
	rci.OffsetRect( m_scrollOffset );

	Range selTime = GetTimeRange( rci );

	CRect rcItem;
	for (int i = 0; i < GetCount(); i++)
	{
		GetItemRect(i,rcItem);
		// Decrease item rectangle a bit.
		rcItem.DeflateRect(4,4,4,4);
		// Check if item rectanle intersects with selection rectangle in y axis.
		if ((rcItem.top >= rc.top && rcItem.top <= rc.bottom) || 
				(rcItem.bottom >= rc.top && rcItem.bottom <= rc.bottom) ||
				(rc.top >= rcItem.top && rc.top <= rcItem.bottom) ||
				(rc.bottom >= rcItem.top && rc.bottom <= rcItem.bottom))
		{
			IAnimTrack *track = GetTrack(i);
			if (!track)
				continue;

			// Check which keys we intersect.
			for (int j = 0; j < track->GetNumKeys(); j++)
			{
				float time = track->GetKeyTime(j);
				if (selTime.IsInside(time))
				{
					track->SetKeyFlags( j,track->GetKeyFlags(j)|AKEY_SELECTED );
					m_bAnySelected = true;
				}
			}
		}
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
int CTrackViewGraph::GetItemRect( int item,CRect &rect )
{
	rect = m_rcClient;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
int	CTrackViewGraph::ItemFromPoint( CPoint pnt )
{
	return GetCurSel();
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewGraph::DrawGraph( IAnimTrack *track,CDC *dc,CRect &rcItem )
{
	EAnimTrackType trackType = track->GetType();
	EAnimValue valueType = track->GetValueType();
	if (valueType != AVALUE_FLOAT && valueType != AVALUE_VECTOR && valueType != AVALUE_QUAT)
	{
		return;
	}

	float fMiddle;
	fMiddle = (m_fMaxValue + m_fMinValue) / 2.0f;

	float yOffset = (rcItem.bottom + rcItem.top)/2;

	DrawGraphAxis( dc,rcItem,yOffset );

	CPen *prevPen;
	CPen pen[4];
	CPen blackPen;
	pen[0].CreatePen( PS_SOLID,1,RGB(255,0,0) );
	pen[1].CreatePen( PS_SOLID,1,RGB(0,255,0) );
	pen[2].CreatePen( PS_SOLID,1,RGB(0,0,255) );
	pen[3].CreatePen( PS_SOLID,1,RGB(0,255,255) );

	prevPen = dc->SelectObject( &pen[0] );

	if (valueType == AVALUE_VECTOR)
	{
		CPoint p0[3],p[3];
		Vec3 val;
		CRect rc = rcItem;

		float xoffset = 0;

		int x1 = rc.left;
		int x2 = rc.right;

		int step = 4;

		// Draw first track spline.
		for (int x = x1; x < x2; x += step)
		{
			float time = TimeFromPointUnsnapped(CPoint(x,0));
			track->GetValue( time,val );

			p[0].x = x;
			p[0].y = yOffset - (val.x - fMiddle)*m_fValueScale;
			p[1].x = x;
			p[1].y = yOffset - (val.y - fMiddle)*m_fValueScale;
			p[2].x = x;
			p[2].y = yOffset - (val.z - fMiddle)*m_fValueScale;

			if (x == x1) // first time.
			{
				memcpy( p0,p,sizeof(p) );
			}

			dc->SelectObject( &pen[0] );
			dc->MoveTo( p0[0] );
			dc->LineTo( p[0] );

			dc->SelectObject( &pen[1] );
			dc->MoveTo( p0[1] );
			dc->LineTo( p[1] );
			
			dc->SelectObject( &pen[2] );
			dc->MoveTo( p0[2] );
			dc->LineTo( p[2] );
			
			memcpy( p0,p,sizeof(p) );
		}

		// Draw Keys.
		int y;
		dc->SelectObject( GetStockObject(BLACK_PEN) );
		Range timeRange = GetTimeRange(rcItem);
		int numKeys = track->GetNumKeys();
		for (int i = 0; i < numKeys; i++)
		{
			float time = track->GetKeyTime(i);

			int x = TimeToClient(time);
			if (time < timeRange.start)
				continue;
			if (time > timeRange.end)
				break;

			track->GetValue( time,val );
			y = yOffset - (val.x - fMiddle)*m_fValueScale;
			dc->Rectangle( x-3,y-3,x+3,y+3 );
			
			y = yOffset - (val.y - fMiddle)*m_fValueScale;
			dc->Rectangle( x-3,y-3,x+3,y+3 );
			
			y = yOffset - (val.z - fMiddle)*m_fValueScale;
			dc->Rectangle( x-3,y-3,x+3,y+3 );
		}
	}
	dc->SelectObject( prevPen );
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewGraph::DrawGraphAxis( CDC *dc,CRect &rcItem,int y )
{
	Range timeRange = GetTimeRange(rcItem);

	dc->MoveTo( rcItem.left,y );
	dc->LineTo( rcItem.right,y );
	dc->MoveTo( rcItem.left+20,rcItem.top );
	dc->MoveTo( rcItem.left+20,rcItem.bottom );

	CRect rc = m_rcClient;
	rc.top = y - 5;
	rc.bottom = y;
	DrawTicks( dc,rc,timeRange );
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewGraph::SetCurSel( int sel )
{
	// Fit graph to view.
	CTrackViewKeys::SetCurSel(sel);

	IAnimTrack *track = GetTrack(sel);
	if (!track)
		return;

	CRect rc = m_rcClient;
	rc.SubtractRect( rc,m_rcTimeline );

	FitGraphToRect(track,rc);
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewGraph::FitGraphToRect( IAnimTrack *track,const CRect &rcItem )
{
	EAnimTrackType trackType = track->GetType();
	EAnimValue valueType = track->GetValueType();
	if (valueType != AVALUE_FLOAT && valueType != AVALUE_VECTOR && valueType != AVALUE_QUAT)
	{
		return;
	}

	if (valueType == AVALUE_VECTOR)
	{
		Vec3 min,max;
		Vec3 val;
		min=SetMaxBB();
		max=SetMinBB();

		CRect rc = rcItem;

		int x1 = rc.left;
		int x2 = rc.right;

		// Draw first track spline.
		for (int x = x1; x < x2; x++)
		{
			float time = TimeFromPointUnsnapped(CPoint(x,0));
			track->GetValue( time,val );
			min.CheckMin(val);
			max.CheckMax(val);
		}

		m_fMinValue = __min(min.x,min.y);
		m_fMinValue = __min(m_fMinValue,min.z);

		m_fMaxValue = __max(max.x,max.y);
		m_fMaxValue = __max(m_fMaxValue,max.z);

		m_fValueScale = (rc.Height() - 30)/(m_fMaxValue - m_fMinValue);
	}
}