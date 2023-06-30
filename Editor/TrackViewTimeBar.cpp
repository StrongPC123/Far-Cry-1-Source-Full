// TrackViewTimeBar.cpp : implementation file
//

#include "stdafx.h"
#include "TrackViewTimeBar.h"

#include "TrackViewKeyList.h"
#include "AnimationContext.h"

// CTrackViewTimeBar

#define LEFT_OFFSET 30

IMPLEMENT_DYNAMIC(CTrackViewTimeBar, CWnd)
CTrackViewTimeBar::CTrackViewTimeBar()
{
	m_ticksStep = 0.1f;
	m_draggingTime = false;
}

CTrackViewTimeBar::~CTrackViewTimeBar()
{
}


BEGIN_MESSAGE_MAP(CTrackViewTimeBar, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
BOOL CTrackViewTimeBar::Create( DWORD dwStyle,const RECT &rect,CWnd *pParentWnd,UINT nID )
{
	m_bkgBrush.CreateSolidBrush(RGB(0xE0,0xE0,0xE0));
	WNDCLASS wndcls;
	ZeroStruct(wndcls);
	//you can specify your own window procedure
	wndcls.lpfnWndProc = ::DefWindowProc; 
	wndcls.hInstance = AfxGetInstanceHandle();
	wndcls.hbrBackground = (HBRUSH)m_bkgBrush.GetSafeHandle();
	// Specify your own class name for using FindWindow later
	wndcls.lpszClassName = _T("TrackViewTimeline");
	AfxRegisterClass(&wndcls);

	return CWnd::Create( _T("TrackViewTimeline"),NULL,dwStyle,rect,pParentWnd,nID );
}

// CTrackViewTimeBar message handlers
void CTrackViewTimeBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages

	//CBrush brush(GetSysColor(COLOR_3DFACE));
	CRect rc;
	GetClientRect(rc);
	//::FillRect( dc,rc,(HBRUSH)GetStockObject(GRAY_BRUSH) );
	dc.Draw3dRect( rc,GetSysColor(COLOR_3DHILIGHT),GetSysColor(COLOR_3DDKSHADOW) );

	CPen *prevPen;
	CPen ltgray(PS_SOLID,1,RGB(90,90,90));
	CPen black(PS_SOLID,1,RGB(0,0,0));
	CPen redpen(PS_SOLID,1,RGB(255,0,0));
	prevPen = dc.SelectObject(&ltgray);
	// Draw time ticks every tick step seconds.
	Range timeRange = m_timeRange;
	CString str;

	dc.SelectObject(redpen);
	int x = TimeToItem(m_currTime);
	dc.SelectObject( GetStockObject(NULL_BRUSH) );
	dc.Rectangle( x-3,rc.top,x+4,rc.bottom );

	dc.SetTextColor( RGB(0,0,0) );
	dc.SetBkMode( TRANSPARENT );
	dc.SelectObject( GetStockObject(DEFAULT_GUI_FONT) );

	int bigTickStep = 100*m_ticksStep;
	for (float t = ceilf(timeRange.start); t <= timeRange.end; t += m_ticksStep)
	{
		int x = TimeToItem(t);
		dc.MoveTo(x,rc.bottom-2);
		int tm = 10*t;
		if (tm%bigTickStep == 0)
		{
			dc.SelectObject(black);
			dc.LineTo(x,rc.bottom-14);
			str.Format( "%d",(int)t );
			dc.TextOut( x+2,rc.top,str );
			dc.SelectObject(ltgray);
		}
		else
			dc.LineTo(x,rc.bottom-6);
	}

	dc.SelectObject(redpen);
	dc.MoveTo( x,rc.top ); dc.LineTo( x,rc.bottom );
	dc.SelectObject( GetStockObject(NULL_BRUSH) );
//	dc.Rectangle( x-3,rc.top,x+4,rc.bottom );

	dc.SelectObject(prevPen);
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewTimeBar::SetTimeRange( float start,float end )
{
	m_timeRange.Set( start,end );
	Invalidate(TRUE);
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewTimeBar::SetTimeScale( float timeScale )
{
	m_timeScale = timeScale;

	Invalidate(TRUE);
}

//////////////////////////////////////////////////////////////////////////
int CTrackViewTimeBar::TimeToItem( float time )
{
	if (time < m_timeRange.start || time > m_timeRange.end)
		return -1;

	int x = LEFT_OFFSET + time*m_timeScale;
	/*
	int left = LEFT_OFFSET+m_scrollOffset.x;
	if (x < left || x > left+m_rcClient.Width())
		x = -1;
		*/
	return x;
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewTimeBar::SetCurrTime( float currTime )
{
	m_currTime = currTime;
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewTimeBar::ChangeTime( float time )
{
	time = __max(time,m_timeRange.start);
	time = __min(time,m_timeRange.end);
	m_currTime = time;
	Invalidate(TRUE);
	NMHDR hdr;
	hdr.code = TVN_CURRTIMECHANGE;
	hdr.hwndFrom = GetSafeHwnd();
	hdr.idFrom = GetDlgCtrlID();
	GetParent()->SendMessage( WM_NOTIFY,(WPARAM)hdr.idFrom,(LPARAM)&hdr );

	GetIEditor()->GetAnimation()->SetTime( m_currTime );
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewTimeBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	//int currX = TimeToItem(m_currTime);
	//if (point.x > currX-3 && point.x < currX+3)
	{
		ChangeTime( TimeFromPoint(point) );
		SetCapture();
		m_draggingTime = true;
	}

	CWnd::OnLButtonDown(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewTimeBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_draggingTime = false;
	ReleaseCapture();

	CWnd::OnLButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewTimeBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_draggingTime)
	{
		ChangeTime( TimeFromPoint(point) );
	}

	CWnd::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
float CTrackViewTimeBar::TimeFromPoint( CPoint point )
{
	int x = point.x - LEFT_OFFSET;
	float time = (float)x / m_timeScale;
	// Snap to 0.1s.
	time = ceilf(time*10-0.5f)/10;
	return time;
}