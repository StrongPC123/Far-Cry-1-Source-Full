/////////////////////////////////////////////////////////////////////////////
// DropListBox.cpp : implementation file
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

#include "DropListBox.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CDropListBox

CDropListBox::CDropListBox()
{
	m_nLastTopIdx = 0;
}

CDropListBox::~CDropListBox()
{
}


BEGIN_MESSAGE_MAP(CDropListBox, CListBox)
	//{{AFX_MSG_MAP(CDropListBox)
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_VRC_SETCAPTURE, OnSetCapture )
	ON_MESSAGE( WM_VRC_RELEASECAPTURE, OnReleaseCapture )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDropListBox message handlers


int CDropListBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here

	//
	// Because this window doesn't have an owner, there will appear 
	// a 'blank' button on the taskbar. The following are to hide 
	// that 'blank' button on the taskbar
	ShowWindow( SW_HIDE );
	ModifyStyleEx( 0, WS_EX_TOOLWINDOW );// |WS_VSCROLL );//| WS_EX_NOACTIVATE ); // WS_EX_CONTROLPARENT
	ShowWindow( SW_SHOW );
	SetWindowPos( &wndTopMost, lpCreateStruct->x, lpCreateStruct->y, lpCreateStruct->cx, lpCreateStruct->cy, SWP_SHOWWINDOW );

	SetFont( GetOwner()->GetFont() ); 
	return 0;
}


LRESULT CDropListBox::OnSetCapture( WPARAM wParam, LPARAM lParam )
{
	SetCapture();
	return FALSE;
}

LRESULT CDropListBox::OnReleaseCapture( WPARAM wParam, LPARAM lParam )
{
	ReleaseCapture();
	return FALSE;
}



void CDropListBox::OnMouseMove(UINT nFlags, CPoint point) 
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

	//
	// Set selection item under mouse
	int nPos = point.y / GetItemHeight(0) + GetTopIndex();
	if (GetCurSel() != nPos)
	{
		SetCurSel( nPos );
	}

	//
	// Check if we have autoscrolled
	if( m_nLastTopIdx != GetTopIndex() )
	{
		int nDiff = m_nLastTopIdx - GetTopIndex();
		m_nLastTopIdx = GetTopIndex();

		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		if( m_pScroll->GetScrollInfo( &info, SIF_ALL|SIF_DISABLENOSCROLL ) )
		{
			info.nPos = m_nLastTopIdx;
			m_pScroll->SetScrollInfo( &info );
		}
	}


//	OutputDebugString( "DropListBox MouseMove\n" );

	CListBox::OnMouseMove(nFlags, point);

}


void CDropListBox::OnLButtonUp(UINT nFlags, CPoint point) 
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

	int nPos = GetCurSel();

	//
	// Send current selection to comboedit
	if( nPos != -1 )
		GetOwner()->PostMessage( WM_SELECTED_ITEM, (WPARAM)nPos, 0 );
	//	CString str;
	//	str.Format( "DropListWnd: Selected item: %d\n", nPos );
	//	OutputDebugString( str );

	//
	// Destroy dropdown
	ReleaseCapture();
	GetOwner()->PostMessage( WM_DESTROY_DROPLIST );
}

void CDropListBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	//
	// Send input to parent
/*	CRect rc;
	GetClientRect( &rc );
	CPoint pt = point;
	ClientToScreen( &pt );
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dx = pt.x;
	input.mi.dy = pt.y;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	input.mi.time = 0;
	SendInput( 1, &input, sizeof(INPUT) );
*/
	//
	// Return so that the listbox can be destroyed
//	CListBox::OnLButtonDown(nFlags, point);
}




int CDropListBox::GetBottomIndex()
{
	int nTop = GetTopIndex();
	CRect rc;
	GetClientRect( &rc );
	int nVisCount = rc.Height() / GetItemHeight(0);
	return nTop + nVisCount;
}

void CDropListBox::SetTopIdx(int nPos, BOOL bUpdateScrollbar)
{
	m_nLastTopIdx = nPos;
	SetTopIndex( nPos );
	if( bUpdateScrollbar )
	{
		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		if( m_pScroll->GetScrollInfo( &info, SIF_ALL|SIF_DISABLENOSCROLL ) )
		{
			info.nPos = m_nLastTopIdx;
			m_pScroll->SetScrollInfo( &info );
		}
	}
}

void CDropListBox::GetTextSize(LPCTSTR lpszText, int nCount, CSize &size)
{
	CClientDC dc(this);
	int nSave = dc.SaveDC();
	dc.SelectObject( GetOwner()->GetFont() );
	size = dc.GetTextExtent( lpszText, nCount );
	dc.RestoreDC(nSave);
}
int CDropListBox::SetCurSel(int nSelect)
{
	int nr = CListBox::SetCurSel( nSelect );
	if( nr != -1 )
	{
		//
		// Set scrollbar
		int nTopIdx = GetTopIndex();

		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		if( m_pScroll->GetScrollInfo( &info, SIF_ALL|SIF_DISABLENOSCROLL ) )
		{
			info.nPos = nTopIdx;
			m_pScroll->SetScrollInfo( &info );
		}
	}
	return nr;
}