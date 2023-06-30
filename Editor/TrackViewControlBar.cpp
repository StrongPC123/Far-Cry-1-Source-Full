// TrackViewControlBar.cpp : implementation file
//

#include "stdafx.h"
#include "TrackViewControlBar.h"
#include "IMovieSystem.h"

// CTrackViewControlBar

IMPLEMENT_DYNAMIC(CTrackViewControlBar, CSizingControlBarG)
CTrackViewControlBar::CTrackViewControlBar()
{
}

CTrackViewControlBar::~CTrackViewControlBar()
{
}


BEGIN_MESSAGE_MAP(CTrackViewControlBar, CSizingControlBarG)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CTrackViewControlBar message handlers


void CTrackViewControlBar::OnSize(UINT nType, int cx, int cy)
{
	CSizingControlBarG::OnSize(nType, cx, cy);

	if (m_dlgTrackView.m_hWnd)
	{
		CRect rc;
		GetClientRect( rc );
		m_dlgTrackView.SetWindowPos( NULL,rc.left,rc.top,rc.right,rc.bottom,0 );
	}
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewControlBar::OnSetFocus(CWnd* pOldWnd)
{
	//CSizingControlBarG::OnSetFocus(pOldWnd);

	if (AfxGetMainWnd())
		AfxGetMainWnd()->SetFocus();
}

//////////////////////////////////////////////////////////////////////////
int CTrackViewControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	lpCreateStruct->style |= WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
	if (CSizingControlBarG::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_dlgTrackView.Create( CTrackViewDialog::IDD,this );

	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewControlBar::Update()
{
	m_dlgTrackView.Update();
}