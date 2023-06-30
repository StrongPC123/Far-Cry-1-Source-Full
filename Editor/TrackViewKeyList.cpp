// TrackViewKeyList.cpp : implementation file
//

#include "stdafx.h"
#include "TrackViewKeyList.h"
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

// CTrackViewKeyList

IMPLEMENT_DYNAMIC(CTrackViewKeyList, CTrackViewKeys)
CTrackViewKeyList::CTrackViewKeyList()
{
	m_leftOffset = 30;

	m_itemWidth = 1000;
	m_itemHeight = 16;
}

CTrackViewKeyList::~CTrackViewKeyList()
{
}

BEGIN_MESSAGE_MAP(CTrackViewKeyList, CTrackViewKeys)
END_MESSAGE_MAP()

// CTrackViewKeyList message handlers

//////////////////////////////////////////////////////////////////////////
void CTrackViewKeyList::DrawTrack( int item,CDC *dc,CRect &rcItem )
{
	CPen pen(PS_SOLID,1,RGB(120,120,120));
	CPen *prevPen = dc->SelectObject( &pen );
	dc->MoveTo( rcItem.left,rcItem.bottom );
	dc->LineTo( rcItem.right,rcItem.bottom );
	dc->SelectObject( prevPen );

	IAnimTrack *track = GetTrack(item);
	if (!track)
		return;

	//dc->Draw3dRect( rcItem,GetSysColor(COLOR_3DHILIGHT),GetSysColor(COLOR_3DDKSHADOW) );
	//int minx = m_leftOffset;
	//int maxx = min( TimeToClient(

	CRect rcInner = rcItem;
	//rcInner.DeflateRect(m_leftOffset,0,m_leftOffset,0);
	rcInner.left = max( rcItem.left,m_leftOffset - m_scrollOffset.x );
	rcInner.right = min( rcItem.right,(m_scrollMax + m_scrollMin) - m_scrollOffset.x + m_leftOffset*3 );

	CRect rcInnerDraw( rcInner.left-6,rcInner.top,rcInner.right+6,rcInner.bottom );
	if (m_selected == item)
	{
		CRect rc = rcInnerDraw;
		rc.DeflateRect(1,1,1,1);
		dc->FillRect( rc,&m_selectedBrush );
	}
	dc->Draw3dRect( rcInnerDraw,GetSysColor(COLOR_3DDKSHADOW),GetSysColor(COLOR_3DHILIGHT) );
	
	// Left outside
	CRect rcOutside = rcItem;
	rcOutside.right = rcInnerDraw.left-1;
	rcOutside.DeflateRect(1,1,1,1);
	dc->SelectObject( m_bkgrBrushEmpty );
	dc->Rectangle( rcOutside );

	// Right outside.
	rcOutside = rcItem;
	rcOutside.left = rcInnerDraw.right+1;
	rcOutside.DeflateRect(1,1,1,1);
	dc->Rectangle( rcOutside );

	// Get time range of update rectangle.
	Range timeRange = GetTimeRange(rcItem);
	// Draw tick marks in time range.
	DrawTicks( dc,rcInner,timeRange );
	// Draw keys in time range.
	DrawKeys( track,dc,rcInner,timeRange );
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewKeyList::DrawKeys( IAnimTrack *track,CDC *dc,CRect &rc,Range &timeRange )
{
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

	int prevKeyPixel = -10000;
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

		if (abs(x-prevKeyPixel) < 2)
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

		prevKeyPixel = x;
		time0 = time;
	}
	dc->SelectObject( prevFont );
}

//////////////////////////////////////////////////////////////////////////
int CTrackViewKeyList::KeyFromPoint( CPoint point )
{
	int item = ItemFromPoint(point);
	if (item < 0)
		return -1;

	float t1 = TimeFromPointUnsnapped( CPoint(point.x-4,point.y) );
	float t2 = TimeFromPointUnsnapped( CPoint(point.x+4,point.y) );

	IAnimTrack *track = GetTrack(item);
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

//////////////////////////////////////////////////////////////////////////
void CTrackViewKeyList::SelectKeys( const CRect &rc )
{
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
					RecordTrackUndo( GetItem(i) );
					break;
					m_bAnySelected = true;
				}
			}

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
}

//////////////////////////////////////////////////////////////////////////
int CTrackViewKeyList::GetItemRect( int item,CRect &rect )
{
	if (item < 0 || item >= GetCount())
		return -1;
	int x = 0;
	int y = item*m_itemHeight - m_scrollOffset.y;
	rect.SetRect( x,y,x+m_rcClient.Width(),y+m_itemHeight );
	return 0;
}