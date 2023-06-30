////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   inplacecombobox.cpp
//  Version:     v1.00
//  Created:     5/6/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History: Based on Stefan Belopotocan code.
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InPlaceComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEFAULT_IPLISTBOX_HEIGHT		16 * 8

#define WM_USER_ON_SELECTION_CANCEL	(WM_USER + 10)
#define WM_USER_ON_SELECTION_OK			(WM_USER + 11)
#define WM_USER_ON_NEW_SELECTION		(WM_USER + 12)
#define WM_USER_ON_EDITCHANGE				(WM_USER + 13)

/////////////////////////////////////////////////////////////////////////////
// CInPlaceCBEdit

BOOL CInPlaceCBEdit::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN)
	{
		CWnd* pOwner = GetOwner();
		WPARAM nChar = pMsg->wParam;

		switch(nChar)
		{
			case VK_ESCAPE:
			case VK_RETURN:
			case VK_TAB:
				::PeekMessage(pMsg, NULL, NULL, NULL, PM_REMOVE);
				pOwner->SendMessage(WM_USER_ON_EDITCHANGE, nChar);
				pOwner->GetParent()->SetFocus();
				return TRUE;
			case VK_UP:
			case VK_DOWN:
				pOwner->SendMessage(WM_USER_ON_NEW_SELECTION, nChar);
				return TRUE;
			default:
				;
		}
	}
	
	return CEdit::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CInPlaceCBEdit, CEdit)
	//{{AFX_MSG_MAP(CInPlaceCBEdit)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceCBEdit message handlers

BOOL CInPlaceCBEdit::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CInPlaceCBListBox

CInPlaceCBListBox::CInPlaceCBListBox()
{
	m_pScrollBar = 0;
	m_nLastTopIdx = 0;
}

CInPlaceCBListBox::~CInPlaceCBListBox()
{
}

BEGIN_MESSAGE_MAP(CInPlaceCBListBox, CListBox)
	//{{AFX_MSG_MAP(CInPlaceCBListBox)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceCBListBox message handlers

void CInPlaceCBListBox::ProcessSelected(bool bProcess)
{
	//ReleaseCapture();

	CWnd* pOwner = GetOwner();

	if(bProcess)
	{
		int nSelectedItem = GetCurSel();
		pOwner->SendMessage(WM_USER_ON_SELECTION_OK, nSelectedItem, GetItemData(nSelectedItem));
	}
	else
		pOwner->SendMessage(WM_USER_ON_SELECTION_CANCEL);
}

void CInPlaceCBListBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CListBox::OnLButtonUp(nFlags, point);

	CRect rect;
	GetClientRect(rect);

	if(!rect.PtInRect(point))
		ProcessSelected(false);
}

void CInPlaceCBListBox::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CListBox::OnLButtonUp(nFlags, point);

	CRect rect;
	GetClientRect(rect);

	if(rect.PtInRect(point))
		ProcessSelected();
	//else
	//	ReleaseCapture();
}

void CInPlaceCBListBox::OnRButtonDown(UINT nFlags, CPoint point)
{
	CListBox::OnRButtonDown(nFlags, point);

	ProcessSelected(false);
}

BOOL CInPlaceCBListBox::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN)
	{
		switch(pMsg->wParam)
		{
			case VK_RETURN:
				ProcessSelected();
				return TRUE;
			case VK_ESCAPE:
			case VK_TAB:
				ProcessSelected(false);
				return TRUE;
			default:
				;
		}
	}
	if(pMsg->message == WM_SYSKEYDOWN)
	{
		ProcessSelected(false);
		return FALSE;
	}

	return CListBox::PreTranslateMessage(pMsg);
}

int CInPlaceCBListBox::GetBottomIndex()
{
	int nTop = GetTopIndex();
	CRect rc;
	GetClientRect( &rc );
	int nVisCount = rc.Height() / GetItemHeight(0);
	return nTop + nVisCount;
}

void CInPlaceCBListBox::SetTopIdx(int nPos, BOOL bUpdateScrollbar)
{
	m_nLastTopIdx = nPos;
	SetTopIndex( nPos );
	if( bUpdateScrollbar )
	{
		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		if( m_pScrollBar->GetScrollInfo( &info, SIF_ALL|SIF_DISABLENOSCROLL ) )
		{
			info.nPos = m_nLastTopIdx;
			m_pScrollBar->SetScrollInfo( &info );
		}
	}
}



//////////////////////////////////////////////////////////////////////////
// CInPlaceCBScrollBar
/////////////////////////////////////////////////////////////////////////////
CInPlaceCBScrollBar::CInPlaceCBScrollBar()
{
	m_pListBox = 0;
}

CInPlaceCBScrollBar::~CInPlaceCBScrollBar()
{
}

BEGIN_MESSAGE_MAP(CInPlaceCBScrollBar, CScrollBar)
	//{{AFX_MSG_MAP(CInPlaceCBScrollBar)
	ON_WM_MOUSEMOVE()
	ON_WM_VSCROLL_REFLECT()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceCBScrollBar message handlers

void CInPlaceCBScrollBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	//
	// Is mouse within listbox
	CRect rcClient;
	GetClientRect( rcClient );
	if( !rcClient.PtInRect( point ) )
	{
		//ReleaseCapture();
		//GetParent()->SendMessage( WM_VRC_SETCAPTURE );
	}

	//	OutputDebugString( "DropScrollBar MouseMove\n" );
	CScrollBar::OnMouseMove(nFlags, point);
}

void CInPlaceCBScrollBar::VScroll(UINT nSBCode, UINT nPos) 
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

//////////////////////////////////////////////////////////////////////////
void CInPlaceCBScrollBar::SetListBox( CInPlaceCBListBox* pListBox )
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

void CInPlaceCBScrollBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rc;
	GetClientRect( &rc );
	if( !rc.PtInRect( point ) )
	{
		GetOwner()->SendMessage(WM_USER_ON_SELECTION_CANCEL);
	}

	CScrollBar::OnLButtonDown(nFlags, point);
}


//////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CInPlaceComboBox
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int CInPlaceComboBox::m_nButtonDx = ::GetSystemMetrics(SM_CXHSCROLL);

IMPLEMENT_DYNAMIC(CInPlaceComboBox, CWnd)

CInPlaceComboBox::CInPlaceComboBox()
{
	m_bReadOnly = false;
	m_nCurrentSelection = -1;
}

BEGIN_MESSAGE_MAP(CInPlaceComboBox, CWnd)
	//{{AFX_MSG_MAP(CInPlaceComboBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE(WM_USER_ON_SELECTION_OK, OnSelectionOk)
	ON_MESSAGE(WM_USER_ON_SELECTION_CANCEL, OnSelectionCancel)
	ON_MESSAGE(WM_USER_ON_NEW_SELECTION, OnNewSelection)
	ON_MESSAGE(WM_USER_ON_EDITCHANGE, OnEditChange)
	
	ON_MESSAGE(WM_SELECTED_ITEM, OnSelectionOk)
	ON_MESSAGE(WM_DESTROY_DROPLIST, OnSelectionCancel)
	//}}AFX_MSG_MAP
	ON_WM_MOVE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceComboBox message handlers

int CInPlaceComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if(CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rect;
	GetClientRect(rect);
	rect.right -= m_nButtonDx;

	CWnd* pParent = GetParent();
	ASSERT(pParent != NULL);

	CFont* pFont = pParent->GetFont();

	int flags = 0;
	if (m_bReadOnly)
		flags |= ES_READONLY;

	m_wndEdit.Create(WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|flags, rect, this, 2);
	m_wndEdit.SetOwner(this);
	m_wndEdit.SetFont(pFont);

	//m_minListWidth = 300;

	rect.right += m_nButtonDx - 1;
	rect.top = rect.bottom + 2;
	rect.bottom += 100;

	/*
	CString myClassName = AfxRegisterWndClass(
		CS_VREDRAW | CS_HREDRAW,::LoadCursor(NULL, IDC_ARROW),(HBRUSH)::GetStockObject(WHITE_BRUSH),NULL );

	m_wndDropDown.CreateEx( 0,myClassName,0,WS_POPUP|WS_BORDER,rect,GetDesktopWindow(),0 );
	m_wndDropDown.ModifyStyleEx( 0,WS_EX_TOOLWINDOW|WS_EX_TOPMOST );
	
	rect.right -= m_nButtonDx;
	//int nListStyle = WS_VISIBLE|WS_CHILD|LBS_DISABLENOSCROLL|LBS_HASSTRINGS|LBS_NOTIFY;
	int nListStyle = WS_VISIBLE|WS_CHILD|LBS_HASSTRINGS|LBS_NOTIFY;
	m_wndList.Create( nListStyle, rect, &m_wndDropDown, 0);
	m_wndList.SetOwner(this);
	m_wndList.SetFont(pFont);

	rect.right += m_nButtonDx;
	m_scrollBar.Create( SBS_VERT|SBS_RIGHTALIGN|WS_CHILD,rect,&m_wndDropDown,1 );
	m_scrollBar.ShowWindow(SW_SHOW);

	m_wndList.SetScrollBar(&m_scrollBar);
	m_scrollBar.SetListBox(&m_wndList);
	*/
	m_wndDropDown.Create( 0,0,WS_BORDER|WS_CHILD|LBS_DISABLENOSCROLL|LBS_HASSTRINGS|LBS_NOTIFY,rect,GetDesktopWindow(),0 );
	m_wndDropDown.SetFont(pFont);
	m_wndDropDown.GetListBox().SetFont(pFont);
	m_wndDropDown.GetListBox().SetOwner(this);
	m_wndDropDown.SetOwner(this);
	
	return 0;
}

void CInPlaceComboBox::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
		
	m_wndEdit.SetWindowPos(NULL, 0, 0, cx - m_nButtonDx, cy, SWP_NOZORDER|SWP_NOMOVE);
}

void CInPlaceComboBox::MoveControl(CRect& rect)
{
	CRect prevRect;
	GetClientRect(prevRect);

	CWnd* pParent = GetParent();

	ClientToScreen(prevRect);
	pParent->ScreenToClient(prevRect);
	pParent->InvalidateRect(prevRect);

	MoveWindow(rect, FALSE);
}

void CInPlaceComboBox::GetDropDownRect( CRect &rect )
{
	GetWindowRect(rect);
	rect.top = rect.bottom;
	rect.bottom += 100;
}

void CInPlaceComboBox::ResetListBoxHeight()
{
	/*
	CRect rect;

	GetClientRect(rect);
	rect.right -= 1;

	int nItems = m_wndList.GetCount();
	int nListBoxHeight = nItems > 0 ? nItems * m_nButtonDx : DEFAULT_IPLISTBOX_HEIGHT;

	if(nListBoxHeight > DEFAULT_IPLISTBOX_HEIGHT)
		nListBoxHeight = DEFAULT_IPLISTBOX_HEIGHT;
	*/
}

void CInPlaceComboBox::OnPaint() 
{
	CPaintDC dc(this);
	
	// Nakresli tlaèítko
	CRect rect;

	GetClientRect(rect);
	rect.left = rect.right - m_nButtonDx;

#if 1
	dc.DrawFrameControl(rect, DFC_SCROLL, m_wndDropDown.IsWindowVisible() ? 
		DFCS_SCROLLDOWN|DFCS_PUSHED : DFCS_SCROLLDOWN);
#else
	dc.DrawFrameControl(rect, DFC_SCROLL, m_wndDropDown.IsWindowVisible() ? 
		DFCS_SCROLLDOWN|DFCS_PUSHED|DFCS_FLAT : DFCS_SCROLLDOWN|DFCS_FLAT);
#endif
}

BOOL CInPlaceComboBox::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;	
}

void CInPlaceComboBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDown(nFlags, point);

	CRect rect;
	GetClientRect(rect);

	CRect rectButton(rect);
	rectButton.left = rectButton.right - m_nButtonDx;

	if(rectButton.PtInRect(point))
	{
		int nDoAction = m_wndDropDown.IsWindowVisible() ? SW_HIDE : SW_SHOW;

		if (nDoAction == SW_SHOW)
		{
			ResetListBoxHeight();

			CRect rc;
			GetDropDownRect(rc);
			if (rc.Width() < m_minListWidth+10)
			{
				//m_wndDropDown.SetWindowPos( &wndTopMost, rc.left,rc.top,rc.Width(),rc.Height(), SWP_SHOWWINDOW );
				//m_wndDropDown.GetClientRect(rc);
				//m_wndList.MoveWindow(rc);
			}
			//m_wndDropDown.SetWindowPos( &wndTopMost, rc.left,rc.top,rc.Width(),rc.Height(), SWP_SHOWWINDOW );
			//m_wndDropDown.MoveWindow( rc );
			//m_wndDropDown.GetClientRect(rc);
			//m_wndList.MoveWindow(rc);

			CRect rect;
			GetDropDownRect(rect);
			m_wndDropDown.SetWindowPos( &wndTopMost, rect.left,rect.top,rect.Width(),rect.Height(), SWP_SHOWWINDOW );
		}

		m_wndDropDown.ShowWindow(nDoAction);
		InvalidateRect(rectButton, FALSE);

		if(nDoAction == SW_SHOW)
		{
			m_wndDropDown.GetListBox().SetFocus();
			m_wndDropDown.SetCapture();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CInPlaceComboBox::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	m_wndEdit.SetFocus();
}

void CInPlaceComboBox::HideListBox()
{
	if (GetCapture())
		ReleaseCapture();
	m_wndDropDown.ShowWindow(SW_HIDE);

	CRect rectButton;

	GetClientRect(rectButton);
	rectButton.left = rectButton.right - m_nButtonDx;

	InvalidateRect(rectButton, FALSE);

	m_wndEdit.SetFocus();
}

//////////////////////////////////////////////////////////////////////////
LRESULT CInPlaceComboBox::OnSelectionOk(WPARAM wParam, LPARAM /*lParam*/)
{
	HideListBox();

	SetCurSelToEdit(m_nCurrentSelection = int(wParam));

	if (m_updateCallback)
		m_updateCallback();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
LRESULT CInPlaceComboBox::OnSelectionCancel(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	HideListBox();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
LRESULT CInPlaceComboBox::OnEditChange(WPARAM wParam, LPARAM lParam)
{
	HideListBox();
	if (m_updateCallback)
		m_updateCallback();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
LRESULT CInPlaceComboBox::OnNewSelection(WPARAM wParam, LPARAM /*lParam*/)
{
	/*
	int nItems = m_wndList.GetCount();

	if(nItems > 0)
	{
		if(wParam == VK_UP)
		{
			if(m_nCurrentSelection > 0)
				SetCurSel(m_nCurrentSelection - 1);
		}
		else
		{
			if(m_nCurrentSelection < nItems - 1)
				SetCurSel(m_nCurrentSelection + 1);
		}
	}
	*/

	return TRUE;
}

void CInPlaceComboBox::SetCurSelToEdit(int nSelect)
{
	CString strText;

	if(nSelect != -1)
		m_wndDropDown.GetListBox().GetText(nSelect, strText);
		
	m_wndEdit.SetWindowText(strText);
	m_wndEdit.SetSel(0, -1); 
}

int CInPlaceComboBox::GetCount() const
{
	return m_wndDropDown.GetListBox().GetCount();
}

int CInPlaceComboBox::SetCurSel(int nSelect, bool bSendSetData)
{
	if(nSelect >= m_wndDropDown.GetListBox().GetCount())
		return CB_ERR;


	int nRet = m_wndDropDown.GetListBox().SetCurSel(nSelect);

	if(nRet != -1)
	{
		SetCurSelToEdit(nSelect);
		m_nCurrentSelection = nSelect;

		if(bSendSetData)
		{
			if (m_updateCallback)
				m_updateCallback();
		}
	}

	return nRet;
}

//////////////////////////////////////////////////////////////////////////
void CInPlaceComboBox::SelectString( LPCTSTR pStrText )
{
	m_wndEdit.SetWindowText(pStrText);
	int sel = m_wndDropDown.GetListBox().FindString( -1,pStrText );
	if (sel != LB_ERR)
	{
		SetCurSel(sel,false);
	}
}

//////////////////////////////////////////////////////////////////////////
CString CInPlaceComboBox::GetSelectedString()
{
	CString str;
	m_wndEdit.GetWindowText( str );
	return str;
}

CString CInPlaceComboBox::GetTextData() const
{
	CString strText;

	if(m_nCurrentSelection != -1)
		m_wndDropDown.GetListBox().GetText(m_nCurrentSelection, strText);

	return strText;
}

int CInPlaceComboBox::AddString(LPCTSTR pStrText, DWORD nData)
{
	int nIndex = m_wndDropDown.GetListBox().AddString(pStrText);

	CDC *dc = GetDC();
	CSize size = dc->GetTextExtent( pStrText );
	ReleaseDC(dc);

	if (size.cx > m_minListWidth)
		m_minListWidth = size.cx;

	return m_wndDropDown.GetListBox().SetItemData(nIndex, nData);
}

//////////////////////////////////////////////////////////////////////////
void CInPlaceComboBox::ResetContent()
{
	m_wndDropDown.GetListBox().ResetContent();

	m_nCurrentSelection = -1;
}

//////////////////////////////////////////////////////////////////////////
void CInPlaceComboBox::OnMove(int x, int y)
{
	CWnd::OnMove(x, y);

	if (m_wndDropDown.m_hWnd)
	{
		CRect rect;
		GetDropDownRect(rect);

		m_wndDropDown.MoveWindow( rect );
		/*
		m_wndDropDown.GetClientRect(rect);

		rect.right -= m_nButtonDx;
		m_wndList.MoveWindow(rect);
		
		rect.left = rect.right+1;
		rect.right += m_nButtonDx;
		m_scrollBar.MoveWindow(rect);
		*/
	}
}
