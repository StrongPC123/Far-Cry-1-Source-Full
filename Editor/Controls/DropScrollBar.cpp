/////////////////////////////////////////////////////////////////////////////
// DropScrollBar.cpp : implementation file
// 
// CAdvComboBox Control
// Version: 2.1
// Date: September 2002
// Author: Mathias Tunared
// Email: Mathias@inorbit.com
// Copyright (c) 2002. All Rights Reserved.
//
// This code, in compiled form or as source code, may be redistributed 
// unmodified PROVIDING it is not sold for profit without the authors 
// written consent, and providing that this notice and the authors name 
// and all copyright notices remains intact.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "DropScrollBar.h"
#include "DropListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDropScrollBar

CDropScrollBar::CDropScrollBar()
:
m_pListBox( 0 )
{
}

CDropScrollBar::~CDropScrollBar()
{
}


BEGIN_MESSAGE_MAP(CDropScrollBar, CScrollBar)
	//{{AFX_MSG_MAP(CDropScrollBar)
	ON_WM_MOUSEMOVE()
	ON_WM_VSCROLL_REFLECT()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_VRC_SETCAPTURE, OnSetCapture )
	ON_MESSAGE( WM_VRC_RELEASECAPTURE, OnReleaseCapture )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDropScrollBar message handlers



void CDropScrollBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	//
	// Is mouse within listbox
	CRect rcClient;
	GetClientRect( rcClient );
	if( !rcClient.PtInRect( point ) )
	{
		ReleaseCapture();
		GetParent()->SendMessage( WM_VRC_SETCAPTURE );
	}
	
//	OutputDebugString( "DropScrollBar MouseMove\n" );
	CScrollBar::OnMouseMove(nFlags, point);
}




LRESULT CDropScrollBar::OnSetCapture( WPARAM wParam, LPARAM lParam )
{
	SetCapture();
	return FALSE;
}

LRESULT CDropScrollBar::OnReleaseCapture( WPARAM wParam, LPARAM lParam )
{
	ReleaseCapture();
	return FALSE;
}

void CDropScrollBar::VScroll(UINT nSBCode, UINT nPos) 
{
	// TODO: Add your message handler code here
	if( !m_pListBox )
		return;

	int nTop = m_pListBox->GetTopIndex();
	int nBottom = m_pListBox->GetBottomIndex();

	SCROLLINFO info;

	info.cbSize = sizeof(SCROLLINFO);
	if( !GetScrollInfo( &info, SIF_ALL|SIF_DISABLENOSCROLL ) )
		return;

	switch( nSBCode )
	{
	case SB_BOTTOM: // Scroll to bottom.
		break;

	case SB_ENDSCROLL: // End scroll.
		break;

	case SB_LINEDOWN: // Scroll one line down.
		info.nPos++;
		if( info.nPos > info.nMax )
			info.nPos = info.nMax;
		m_pListBox->SetTopIdx( info.nPos );
		break;

	case SB_LINEUP: // Scroll one line up.
		info.nPos--;
		if( info.nPos < info.nMin )
			info.nPos = info.nMin;
		m_pListBox->SetTopIdx( info.nPos );
		break;

	case SB_PAGEDOWN: // Scroll one page down.
		info.nPos += info.nPage;
		if( info.nPos > info.nMax )
			info.nPos = info.nMax;
		m_pListBox->SetTopIdx( info.nPos );
		break;

	case SB_PAGEUP: // Scroll one page up.
		info.nPos -= info.nPage;
		if( info.nPos < info.nMin )
			info.nPos = info.nMin;
		m_pListBox->SetTopIdx( info.nPos );
		break;

	case SB_THUMBPOSITION: // Scroll to the absolute position. The current position is provided in nPos.
		info.nPos = nPos;
		m_pListBox->SetTopIdx( info.nPos );
		break;

	case SB_THUMBTRACK: // Drag scroll box to specified position. The current position is provided in nPos.
		info.nPos = nPos;
		m_pListBox->SetTopIdx( info.nPos );
		break;

	case SB_TOP: // Scroll to top. 
		break;

	}
	SetScrollInfo( &info );
			
}



void CDropScrollBar::SetListBox( CDropListBox* pListBox )
{
	ASSERT( pListBox != NULL );

	m_pListBox = pListBox;
	int nTop = m_pListBox->GetTopIndex();
	int nBottom = m_pListBox->GetBottomIndex();
	
	SCROLLINFO info;

	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	info.nMax = m_pListBox->GetCount()-1;
	info.nMin = 0;
	info.nPage = nBottom - nTop;
	info.nPos = 0;
	info.nTrackPos = 0;

	SetScrollInfo( &info );


}

void CDropScrollBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rc;
	GetClientRect( &rc );
	if( !rc.PtInRect( point ) )
	{
		ReleaseCapture();
		GetParent()->SendMessage( WM_VRC_SETCAPTURE );
	}
	
	CScrollBar::OnLButtonDown(nFlags, point);
}

