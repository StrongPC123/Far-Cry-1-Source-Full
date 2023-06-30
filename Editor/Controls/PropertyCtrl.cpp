////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   propertyctrl.cpp
//  Version:     v1.00
//  Created:     5/6/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Implementation of CPropertyCtrl.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PropertyCtrl.h"
#include "PropertyItem.h"
#include "MemDC.h"
#include "Clipboard.h"

#define PROPERTY_LEFT_BORDER 15
#define OFFSET_CHILD 14
#define DEFAULT_ITEM_HEIGHT 14

#define CATEGORY_LINE_COLOR RGB(140,140,140)
#define LINE_COLOR RGB(210,210,210)
#define KILLFOCUS_TIMER 1002

// CPropertyCtrl

IMPLEMENT_DYNAMIC(CPropertyCtrl, CWnd)

BEGIN_MESSAGE_MAP(CPropertyCtrl, CWnd)
	ON_WM_GETDLGCODE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_VSCROLL()
	ON_WM_KEYDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_CREATE()
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_WM_TIMER()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CPropertyCtrl::CPropertyCtrl()
{
	m_root = 0;
	m_updateFunc = 0;
	m_selChangeFunc = 0;
	m_bEnableCallback = true;
	m_bEnableSelChangeCallback = true;
	m_selected = 0;

	//CUndoProperties::m_sCurrentPropertyWindow = this;

	//m_bgBrush.Attach( (HBRUSH)GetStockObject(WHITE_BRUSH) );
	m_bgBrush.CreateSolidBrush( GetSysColor(COLOR_WINDOW) );
	m_icons.Create(IDB_PROPERTIES, 14, 1, RGB (255,255,255));
	m_leftRightCursor = AfxGetApp()->LoadCursor( IDC_LEFTRIGHT );

	m_bSplitterDrag = false;
	m_splitter = 30;

	m_scrollOffset = CPoint(0,0);
	m_bDisplayOnlyModified = false;

	m_nTimer = 0;
	m_pBoldFont = 0;

	m_nFlags = 0;
	m_nItemHeight = DEFAULT_ITEM_HEIGHT;
}

//////////////////////////////////////////////////////////////////////////
CPropertyCtrl::~CPropertyCtrl()
{
	delete m_pBoldFont;
}

//////////////////////////////////////////////////////////////////////////
UINT CPropertyCtrl::OnGetDlgCode()
{
	// Want to handle all Tab and arrow keys myself, not delegate it to dialog.
	return DLGC_WANTALLKEYS;
}

/*
//////////////////////////////////////////////////////////////////////////
CSize CPropertyCtrl::GetListSize()
{
	FRect ClientRect = GetClientRect();
	FRect R(0,0,0,4);//!!why?
	for( INT i=List.GetCount()-1; i>=0; i-- )
		R.Max.Y += List.GetItemHeight( i );
	AdjustWindowRect( R, GetWindowLongX(List,GWL_STYLE), 0 );
	CSize size;
	size.cx = ClientRect.Width();
	size.cy = R.Height();
	return size;
}
*/

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::OnItemChange( CPropertyItem *item )
{
	// Called when item, gets modified.
	if (m_updateFunc != 0 && m_bEnableCallback)
	{
		m_bEnableCallback = false;
		m_updateFunc( item->GetXmlNode() );
		m_bEnableCallback = true;
	}
	if (m_updateVarFunc != 0 && m_bEnableCallback)
	{
		m_bEnableCallback = false;
		m_updateVarFunc( item->GetVariable() );
		m_bEnableCallback = true;
	}

	//GetIEditor()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::ExpandAll()
{
	if (m_root)
		ExpandAllChilds( m_root,true );
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::ExpandAllChilds( CPropertyItem *item,bool bRecursive )
{
	for (int i = 0; i < item->GetChildCount(); i++)
	{
		if (IsCategory( item->GetChild(i) ))
		{
			Expand( item->GetChild(i),true );
			if (bRecursive)
				ExpandAllChilds( item->GetChild(i),bRecursive );
		}
	}
	CalcLayout();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::ReloadValues()
{
	bool prev = m_bEnableCallback;
	m_bEnableCallback = false;

	// Make sure No selection.
	SelectItem(0);

	if (m_root)
		m_root->ReloadValues();
	
	m_bEnableCallback = prev;
	Invalidate();
}

bool CPropertyCtrl::EnableUpdateCallback( bool bEnable )
{
	bool prev = m_bEnableCallback;
	m_bEnableCallback = bEnable;
	return prev;
};

bool CPropertyCtrl::EnableSelChangeCallback( bool bEnable )
{
	bool prev = m_bEnableSelChangeCallback;
	m_bEnableSelChangeCallback = bEnable;
	return prev;
};

//////////////////////////////////////////////////////////////////////////
// Register your unique class name that you wish to use
void CPropertyCtrl::RegisterWindowClass()
{
	WNDCLASS wndcls;

	memset(&wndcls, 0, sizeof(WNDCLASS));   // start with NULL
	// defaults

	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;

	//you can specify your own window procedure
	wndcls.lpfnWndProc = ::DefWindowProc; 
	wndcls.hInstance = AfxGetInstanceHandle();
	wndcls.hIcon = NULL;
	wndcls.hCursor = AfxGetApp()->LoadStandardCursor( IDC_ARROW );
	wndcls.hbrBackground = NULL;
	wndcls.lpszMenuName = NULL;

	// Specify your own class name for using FindWindow later
	wndcls.lpszClassName = _T("PropertyCtrl");

	// Register the new class and exit if it fails
	AfxRegisterClass(&wndcls);
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::Create( DWORD dwStyle,const CRect &rc,CWnd *pParent,UINT nID )
{
	RegisterWindowClass();
	CWnd::Create( _T("PropertyCtrl"),"",dwStyle,rc,pParent,nID );

	ModifyStyle( 0,WS_VSCROLL );
}

int CPropertyCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	Init();
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	Init();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::OnDestroy()
{
	delete m_pBoldFont;
	m_pBoldFont = NULL;

	if (m_nTimer)
	{
		KillTimer(m_nTimer);
		m_nTimer = 0;
	}
	SelectItem(0);
	CWnd::OnDestroy();
	// TODO: Add your message handler code here
}

void CPropertyCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDown(nFlags, point);

	SetFocus();

	m_mouseDownPos = point;
	
	bool bSplitter = IsOverSplitter(point);
	if (!bSplitter)
	{
		CPropertyItem *item = GetItemFromPoint(point);
		if (item)
		{
			if (nFlags & MK_CONTROL)
			{
				if (item->IsSelected())
					MultiUnselectItem(item);
				else
					MultiSelectItem(item);
			}
			else if (nFlags & MK_SHIFT)
			{
				MultiSelectRange(item);
			}
			else
			{
				// Select clicked item.
				SelectItem( item );
				Expand( m_selected,!m_selected->IsExpanded() );
				item->OnLButtonDown( nFlags,point );

				CRect itemRc;
				GetItemRect(item,itemRc);

				itemRc = GetItemValueRect(itemRc);
				if (itemRc.PtInRect(point))
				{
					// Clicked in value rectangle.
					item->SetFocus();
				}
			}
		}
	}
	else
	{
		m_bSplitterDrag = true;
		SetCapture();
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bSplitterDrag)
	{
		m_bSplitterDrag = false;
		ReleaseCapture();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDblClk(nFlags, point);

	CPropertyItem *item = GetItemFromPoint(point);
	if (item)
	{
		// Select clicked item.
		SelectItem( item );
		item->OnLButtonDblClk( nFlags,point );
	}
}

BOOL CPropertyCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_selected)
	{
		//m_selected->OnMouseWheel( nFlags,zDelta,pt );
	}

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CPropertyCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	CClipboard clipboard;

	// Popup Menu with Event selection.
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu( MF_STRING,1,_T("Copy") );
	menu.AppendMenu( MF_STRING,2,_T("Copy Recursively") );
	menu.AppendMenu( MF_STRING,3,_T("Copy All") );
	menu.AppendMenu( MF_SEPARATOR,0,_T("") );
	if (clipboard.IsEmpty())
		menu.AppendMenu( MF_STRING|MF_GRAYED,4,_T("Paste") );
	else
		menu.AppendMenu( MF_STRING,4,_T("Paste") );

	CPoint p;
	::GetCursorPos(&p);
	int res = ::TrackPopupMenuEx( menu.GetSafeHmenu(),TPM_LEFTBUTTON|TPM_RETURNCMD,p.x,p.y,GetSafeHwnd(),NULL );
	switch (res)
	{
		case 1:
			OnCopy(false);
			break;
		case 2:
			OnCopy(true);
			break;
		case 3:
			OnCopyAll();
			break;
		case 4:
			OnPaste();
			break;
	}
	//	CWnd::OnRButtonUp(nFlags, point);
}

void CPropertyCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	/*
	CPropertyItem *item = GetItemFromPoint(point);
	if (item && !item->IsSelected())
	{
		// Select clicked item.
		SelectItem( item );
		item->OnRButtonDown( nFlags,point );
	}
	CWnd::OnRButtonDown(nFlags, point);
	*/
}

void CPropertyCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if (nChar == VK_DOWN || nChar == VK_TAB)
	{
		Items items;
		GetVisibleItems( m_root,items );
		for (int i = 0; i < items.size(); i++)
		{
			if (items[i] == m_selected)
			{
				if (i < items.size()-1)
				{
					SelectItem( items[i+1] );
					break;
				}
				break;
			}
		}
	}
	if (nChar == VK_UP)
	{
		Items items;
		GetVisibleItems( m_root,items );
		for (int i = 0; i < items.size(); i++)
		{
			if (items[i] == m_selected)
			{
				if (i > 0)
				{
					SelectItem( items[i-1] );
					break;
				}
				break;
			}
		}
	}

	if (nChar == VK_LEFT || nChar == VK_RIGHT)
	{
		if (m_selected)
			Expand( m_selected,!m_selected->IsExpanded() );
	}

	if (nChar == VK_RETURN)
	{
		if (m_selected)
			m_selected->SetFocus();
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPropertyCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	// TODO: Add your message handler code here
}

void CPropertyCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	// TODO: Add your message handler code here
}

void CPropertyCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (m_offscreenBitmap.GetSafeHandle() != NULL)
		m_offscreenBitmap.DeleteObject();
	

	CRect rc;
	GetClientRect( rc );

	m_splitter = rc.Width()/2;

	CDC *dc = GetDC();
	m_offscreenBitmap.CreateCompatibleBitmap( dc,rc.Width(),rc.Height() );
	ReleaseDC(dc);

	if (m_tooltip.m_hWnd)
	{
		m_tooltip.DelTool(this,1);
		m_tooltip.AddTool( this,"",rc,1 );
	}

	CalcLayout();
}

BOOL CPropertyCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;

//	return CWnd::OnEraseBkgnd(pDC);
}

void CPropertyCtrl::OnPaint()
{
	CPaintDC PaintDC(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages

	CRect rcClient;
	GetClientRect( rcClient );

	if (m_offscreenBitmap.GetSafeHandle() == NULL)
	{
		m_offscreenBitmap.CreateCompatibleBitmap( &PaintDC,rcClient.Width(),rcClient.Height() );
	}

	CMemDC dc( PaintDC,&m_offscreenBitmap );

	dc.FillRect( &PaintDC.m_ps.rcPaint,&m_bgBrush );

	dc.SelectObject( GetFont() );

	CRect rc = rcClient;
	int y = -m_scrollOffset.y;
	Items items;
	GetVisibleItems( m_root,items );
	for (int i = 0; i < items.size(); i++)
	{
		int itemHeight = GetItemHeight( items[i] );
		rc.top = y;
		rc.bottom = y + itemHeight;
		if (rc.bottom > 0 && rc.top < rcClient.bottom)
			DrawItem( items[i],dc,rc );
		y += itemHeight;
	}
}

void CPropertyCtrl::DrawItem( CPropertyItem *item,CDC &dc,CRect &itemRect )
{
	CRect rect = itemRect;

	int nLeftBorder = rect.left + PROPERTY_LEFT_BORDER;

	bool bCtrlDisabled = IsWindowEnabled() != TRUE;
	bool bItemDisabled = item->IsDisabled() || bCtrlDisabled;
	bool bItemBold = item->IsBold();

	COLORREF crModifiedText = RGB(180,0,0);
	COLORREF crBackground = ::GetSysColor(COLOR_BTNFACE);
	//COLORREF crText = RGB(0,0,0); //::GetSysColor(nCrText);
	COLORREF crText = ::GetSysColor(COLOR_WINDOWTEXT);
	COLORREF crTextCategory = ::GetSysColor(COLOR_GRAYTEXT);
	COLORREF crTextShadow = RGB(130,130,160); //::GetSysColor(nCrText);
	COLORREF crTextDisabled = ::GetSysColor(COLOR_GRAYTEXT);

	bool bDotNetStyle = m_nFlags & F_VS_DOT_NET_STYLE;

	bool bCategory = IsCategory(item);

	if (!m_pBoldFont)
	{
		m_pBoldFont = new CFont;
		int height = -::MulDiv(8, GetDeviceCaps(dc, LOGPIXELSY),72);
		m_pBoldFont->CreateFont(height, 0, 0, 0,	FW_BOLD, FALSE, FALSE, FALSE,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
			DEFAULT_PITCH, "MS Sans Serif" );
	}

	if (bCategory)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		
		//int nCrBackground, nCrText;
		//nCrBackground = COLOR_BTNFACE;
		//nCrText = COLOR_WINDOWTEXT;
		
		CRect rc1 = rect;
		rc1.InflateRect(0,0,0,1);

		//dc.FillSolidRect(rect, RGB(220,220,220));
		//dc.FillSolidRect(rect, crBackground );
		//dc.Draw3dRect( rc1,GetSysColor(COLOR_BTNHIGHLIGHT),GetSysColor(COLOR_BTNSHADOW));

		//////////////////////////////////////////////////////////////////////////
		// Draw category solid rectangle.
		if (bDotNetStyle)
			dc.FillSolidRect( rc1,crBackground );
		else
		{
			CBrush gray( crBackground );
			CPen pen(PS_SOLID, 1, CATEGORY_LINE_COLOR );
			CBrush *pOldBrush = dc.SelectObject( &gray );
			CPen* pOldPen = dc.SelectObject(&pen);
			dc.Rectangle( rc1 );
			dc.SelectObject( pOldBrush );
			dc.SelectObject( pOldPen );
		}

		COLORREF crOldBkColor = dc.SetBkColor(crBackground);
		COLORREF crOldTextColor = dc.SetTextColor(crText);
		
		rect.left += PROPERTY_LEFT_BORDER;
		rect.left += 2;
		rect.right -= 2;
		
		dc.SetBkMode( TRANSPARENT );

		if (bItemDisabled)
		{
			crText = crTextDisabled;
		}

		//if (bDotNetStyle)
		{
			// Draw simple bold text.
			dc.SetTextColor( crTextCategory );
			CFont *pPrevFont = dc.SelectObject( m_pBoldFont );
			rect.OffsetRect(1,1);
			dc.DrawText(item->GetName(), &rect, DT_SINGLELINE);
			dc.SelectObject( pPrevFont );
		}
		/*
		else
		{
			// Draw shadowed text.
			if (!bItemDisabled)
			{
				// Text shadow part.
				dc.SetTextColor( crTextShadow );
				rect.OffsetRect( CPoint(2,2) );
				dc.DrawText(item->GetName(), &rect, DT_SINGLELINE);
			}
			dc.SetTextColor( crText );
			rect.OffsetRect( CPoint(-1,-1) );
			dc.DrawText(item->GetName(), &rect, DT_SINGLELINE);
		}
		*/
		
		dc.SetTextColor(crOldTextColor);
		dc.SetBkColor(crOldBkColor);

		if (item->IsExpandable() && !item->IsExpanded())
			// plus
			DrawSign( dc,CPoint( (itemRect.left+PROPERTY_LEFT_BORDER)/2,(itemRect.top+itemRect.bottom)/2),true );
		else
			// minus
			DrawSign( dc,CPoint( (itemRect.left+PROPERTY_LEFT_BORDER)/2,(itemRect.top+itemRect.bottom)/2),false );
	}
	else
	{	
		//////////////////////////////////////////////////////////////////////////
		// Draw normal item, (No category)
		//////////////////////////////////////////////////////////////////////////
		CPen pen(PS_SOLID, 1, LINE_COLOR );
		CPen* pOldPen = dc.SelectObject(&pen);
		
		if (bDotNetStyle)
		{
			dc.FillSolidRect( rect.left,rect.top,PROPERTY_LEFT_BORDER,rect.Height(),crBackground );
		}
		else
		{
		// Vertical separator line.
			dc.MoveTo(nLeftBorder, rect.top);
			dc.LineTo(nLeftBorder, rect.bottom);
		}
		
		rect = itemRect;
		
		// Horizontal Line.
		dc.MoveTo(rect.left, rect.bottom );
		dc.LineTo(rect.right, rect.bottom );

		rect.left += PROPERTY_LEFT_BORDER;

		nLeftBorder += m_splitter;
		
		dc.MoveTo(nLeftBorder, rect.top);
		dc.LineTo(nLeftBorder, rect.bottom);
		
		rect.left += 1;
		rect.top += 1;
		//rect.bottom -= 1;
		rect.right = nLeftBorder;
		
		int nCrBackground, nCrText;
		
		if (item->IsSelected())
		{
			nCrBackground = COLOR_HIGHLIGHT;
			nCrText = COLOR_HIGHLIGHTTEXT;
			crBackground = ::GetSysColor(nCrBackground);
		}
		else
		{
			nCrBackground = COLOR_WINDOW;
			nCrText = COLOR_WINDOWTEXT;
			crBackground = ::GetSysColor(nCrBackground);
			if (bItemBold && !bItemDisabled)
			{
				crBackground = RGB(240,240,240);
			}
		}
		crText = ::GetSysColor(nCrText);

		if (bItemDisabled)
		{
			crText = crTextDisabled;
		}
		else if (m_bDisplayOnlyModified && item->IsModified())
		{
			crText = crModifiedText;
		}

		dc.FillSolidRect(rect, crBackground);
		COLORREF crOldBkColor = dc.SetBkColor(crBackground);
		COLORREF crOldTextColor = dc.SetTextColor(crText);
		
		int textOffset = CalcOffset( item );
		rect.left += 2 + textOffset*OFFSET_CHILD;
		rect.right -= 2;
		
		/*
		CFont* pOldFont = NULL;
		CFont fontLabel;
		
		if(bTabItem)
		{
			LOGFONT logFont;
			CFont* pFont = GetFont();
			pFont->GetLogFont(&logFont);
			
			logFont.lfWeight = FW_BOLD;
			fontLabel.CreateFontIndirect(&logFont);
			
			pOldFont = dc.SelectObject(&fontLabel);
		}
		*/

		CFont *pPrevFont = 0;
		if (bItemBold && !bItemDisabled)
		{
			/*
			CRect rc = rect;
			// Text shadow part.
			dc.SetTextColor( crTextShadow );
			rc.OffsetRect( CPoint(1,1) );
			dc.DrawText(item->GetName(), &rc, DT_SINGLELINE|DT_VCENTER);
			dc.SetTextColor(crText);
			*/
			

			pPrevFont = dc.SelectObject( m_pBoldFont );
		}

    // Draw text label.
		dc.DrawText(item->GetName(), &rect, DT_SINGLELINE|DT_VCENTER);

		if (bItemBold && !bItemDisabled)
		{
			dc.SelectObject(pPrevFont);
		}
		
		dc.SelectObject(pOldPen);
		dc.SetTextColor(crOldTextColor);
		dc.SetBkColor(crOldBkColor);
		//dc.SetBkMode(TRANSPARENT);
		
		/*
		if(pOldFont != NULL)
			dc.SelectObject(pOldFont);
			*/

		if (item->IsExpandable())
		{
			if (!item->IsExpanded())
				// plus
				DrawSign( dc,CPoint( (itemRect.left+PROPERTY_LEFT_BORDER)/2,(itemRect.top+itemRect.bottom)/2),true );
			else
				// minus
				DrawSign( dc,CPoint( (itemRect.left+PROPERTY_LEFT_BORDER)/2,(itemRect.top+itemRect.bottom)/2),false );
		}
		else
		{
			// Draw Item description icon.
			if (!bDotNetStyle)
			{
				CPoint iconPnt( itemRect.left,itemRect.top+1 );
				m_icons.Draw( &dc,item->GetImage(),iconPnt,ILD_TRANSPARENT );
			}
		}
		
		if (!bCtrlDisabled)
		{
			CRect valueRect = GetItemValueRect(itemRect);

			//if (!item->IsSelected())
			{
				bool bDisplayValue = true;
				if (m_bDisplayOnlyModified && !item->IsModified())
				{
					bDisplayValue = false;
				}
				if (bDisplayValue)
				{
					//valueRect.left += 3;
					item->DrawValue(&dc, valueRect);
					//valueRect.left -= 3;
				}
			}
			item->MoveInPlaceControl( valueRect );
		}
	}
}

void CPropertyCtrl::DrawSign( CDC &dc,CPoint point,bool plus )
{
	CRect rcSign(point.x-4,point.y-4,point.x+5,point.y + 5);

	dc.FillRect(rcSign, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
	dc.FrameRect(rcSign, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));

	CPoint ptCenter(rcSign.CenterPoint());

	// minus		
	dc.MoveTo(ptCenter.x - 2, ptCenter.y);
	dc.LineTo(ptCenter.x + 3, ptCenter.y);

	if (plus)
	{
		// plus
		dc.MoveTo(ptCenter.x, ptCenter.y - 2);
		dc.LineTo(ptCenter.x, ptCenter.y + 3);
	}
}

CRect CPropertyCtrl::GetItemValueRect( const CRect &rc )
{
	CRect rect = rc;
	rect.left += PROPERTY_LEFT_BORDER;
	rect.left += m_splitter;

	rect.DeflateRect(3, 1, 0, 0);
	return rect;
}

void CPropertyCtrl::GetItemRect(  CPropertyItem *item,CRect &rect )
{
	rect.SetRect( 0,0,0,0 );

	CRect rc;
	GetClientRect( rc );
	int y = -m_scrollOffset.y;
	Items items;
	GetVisibleItems( m_root,items );
	for (int i = 0; i < items.size(); i++)
	{
		int itemHeight = GetItemHeight( items[i] );
		rc.top = y;
		rc.bottom = y + itemHeight;
		if (items[i] == item)
		{
			rect = rc;
			return;
		}
		y += itemHeight;
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::SetItemHeight( int nItemHeight )
{
	m_nItemHeight = nItemHeight;
}

//////////////////////////////////////////////////////////////////////////
int CPropertyCtrl::GetItemHeight( CPropertyItem *item ) const
{
	if (m_nFlags & F_VARIABLE_HEIGHT)
	{
		return item->GetHeight();
	}
	return m_nItemHeight;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::CreateItems( XmlNodeRef &node )
{
	SelectItem(0);

	m_root = 0;

	m_xmlRoot = node;
	m_root = new CPropertyItem( this );
	m_root->SetXmlNode( node );
	m_root->SetExpanded( true );

	CalcLayout();

	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::SetRootName( const CString &rootName )
{
	if (m_root)
		m_root->SetName( rootName );
}

//////////////////////////////////////////////////////////////////////////
CPropertyItem* CPropertyCtrl::AddVarBlock( CVarBlock *varBlock,const char *szCategory )
{
	assert( varBlock );
	if (!m_root)
		m_root = new CPropertyItem( this );

	CPropertyItem *root = m_root;

	if (szCategory && strlen(szCategory) > 0)
	{
		CPropertyItem *pCategory = new CPropertyItem( this );
		pCategory->SetName(szCategory);
		m_root->AddChild(pCategory);
		root = pCategory;
	}
	for (int i = 0; i < varBlock->GetVarsCount(); i++)
	{
		CPropertyItem *childItem = new CPropertyItem( this );
		root->AddChild(childItem);
		IVariable *var = varBlock->GetVariable(i);
		childItem->SetVariable(var);
	}
	m_root->SetExpanded( true );

	CalcLayout();
	Invalidate();

	return root;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::DeleteAllItems()
{
	SelectItem(0);
	m_root = 0;
	CalcLayout();
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::GetVisibleItems( CPropertyItem *item, std::vector<CPropertyItem*> &items )
{
	if (!item)
		return;

	if (item == m_root)
	{
		if (!m_root->GetName().IsEmpty())
		{
			items.push_back(m_root);
		}
	}

	if (item->IsExpanded())
	{
		for (int i = 0; i < item->GetChildCount(); i++)
		{
			CPropertyItem *child = item->GetChild(i);
			items.push_back( child );
			if (item->IsExpanded())
				GetVisibleItems( child,items );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
CPropertyItem* CPropertyCtrl::GetItemFromPoint( CPoint point )
{
	CRect rc;
	GetClientRect( rc );

	int y = -m_scrollOffset.y;

	Items items;
	GetVisibleItems( m_root,items );
	for (int i = 0; i < items.size(); i++)
	{
		int itemHeight = GetItemHeight( items[i] );
		rc.top = y;
		rc.bottom = y + itemHeight;
		if (rc.PtInRect(point))
		{
			return items[i];
		}
		y += itemHeight;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::SelectItem( CPropertyItem *item )
{
	if (m_selected)
	{
		m_selected->SetSelected(false);
		m_selected->DestroyInPlaceControl();
	}
	if (!m_multiSelectedItems.empty())
	{
		// Clear multiple selected items.
		for (int i = 0; i < m_multiSelectedItems.size(); i++)
		{
			m_multiSelectedItems[i]->SetSelected(false);
		}
		m_multiSelectedItems.clear();
	}
	m_selected = item;
	if (m_selected)
	{
		m_multiSelectedItems.push_back(m_selected);
		m_selected->SetSelected(true);
		CreateInPlaceControl();

		CRect rcClient;
		CRect rc;
		GetClientRect(rcClient);
		GetItemRect( m_selected,rc );
		if (rc.bottom > rcClient.bottom)
		{
			int y = m_scrollOffset.y + (rc.bottom - rcClient.bottom);
			FlatSB_SetScrollPos( GetSafeHwnd(),SB_VERT,y,TRUE );
			m_scrollOffset.y = y;
		}
		else if (rc.top < rcClient.top)
		{
			int y = m_scrollOffset.y - (rcClient.top - rc.top);
			FlatSB_SetScrollPos( GetSafeHwnd(),SB_VERT,y,TRUE );
			m_scrollOffset.y = y;
		}
		if (m_selChangeFunc!=NULL && m_bEnableSelChangeCallback)
		{
			m_selChangeFunc(m_selected->GetXmlNode());
		}
	}else
	{
		if (m_selChangeFunc!=NULL && m_bEnableSelChangeCallback)
		{
			m_selChangeFunc(NULL);
		}
	}
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::CreateInPlaceControl()
{
	if (!m_selected)
		return;

	CRect rc;
	GetItemRect(m_selected,rc);
	rc = GetItemValueRect(rc);

	if (m_selected->IsDisabled())
		return;

	m_selected->CreateInPlaceControl( this,rc );

}

//////////////////////////////////////////////////////////////////////////
int CPropertyCtrl::CalcOffset( CPropertyItem *item )
{
	if (item == m_root)
		return 0;

	int offset = 0;
	while (item && item != m_root && !IsCategory(item))
	{
		item = item->GetParent();
		offset++;
	};
	if (offset < 1)
		return 0;
	return offset-1;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove(nFlags, point);

	if (m_bSplitterDrag)
	{
		CRect rc;
		GetClientRect( rc );
		int x = point.x - PROPERTY_LEFT_BORDER;
		if (x < 30)
			x = 30;
		if (x > rc.right - 30 - PROPERTY_LEFT_BORDER)
			x = rc.right - 30 - PROPERTY_LEFT_BORDER;
		if (x != m_splitter)
		{
			m_splitter = x;
			Invalidate();
		}
	}
	else
	{
		CPropertyItem *item = GetItemFromPoint(point);
		if (nFlags & MK_LBUTTON && !(nFlags&(MK_SHIFT|MK_CONTROL)))
		{
			if (item != m_selected)
			{
				// Select clicked item.
				SelectItem( item );
			}
		}
		ProcessTooltip(item);
	}
	if (IsOverSplitter(point))
	{
		SetCursor( m_leftRightCursor );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::ProcessTooltip( CPropertyItem *item )
{
	if (m_tooltip.m_hWnd)
	{
		if (item && !IsCategory(item))
		{
			if (item != m_prevTooltipItem)
			{
				m_tooltip.UpdateTipText( item->GetTip(),this,1 );
				m_tooltip.Activate(TRUE);
			}
			m_prevTooltipItem = item;
		}
		else
		{
			m_tooltip.Activate(FALSE);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CPropertyCtrl::IsCategory( CPropertyItem *item )
{
	if (item && (item->GetParent() == m_root || item == m_root) && item->IsExpandable() && !item->IsNotCategory())
		return true;

	/*
	// 2nd rule.
	if (item && item->GetType() == ePropertyInvalid && item->IsExpandable())
		return true;
	*/

	return false;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::Expand( CPropertyItem *item,bool bExpand )
{
	assert( item );
	item->SetExpanded( bExpand );
	Invalidate();

	CalcLayout();
}

//////////////////////////////////////////////////////////////////////////
bool CPropertyCtrl::IsOverSplitter( CPoint point )
{
	if (point.x >= PROPERTY_LEFT_BORDER+m_splitter-2 && point.x <= PROPERTY_LEFT_BORDER+m_splitter+2)
	{
		CPropertyItem *item = GetItemFromPoint(point);
		if (item && IsCategory(item))
			return false;
		return true;
	}
	return false;
}

BOOL CPropertyCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default
	if (m_bSplitterDrag)
	{
		SetCursor( m_leftRightCursor );
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::CalcLayout()
{
	CRect rc;
	GetClientRect( rc );

	// Set scroll info.
	int nPage = rc.Height();

	int h = GetVisibleHeight();

	SCROLLINFO si;
	ZeroStruct(si);
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = h;
	si.nPage = nPage;
	si.nPos = m_scrollOffset.y;
	//si.nPage = max(0,m_rcClient.Width() - LEFT_OFFSET*2);
	//si.nPage = 1;
	//si.nPage = 1;
	FlatSB_SetScrollInfo( GetSafeHwnd(),SB_VERT,&si,TRUE );
	FlatSB_GetScrollInfo( GetSafeHwnd(),SB_VERT,&si );
	m_scrollOffset.y = si.nPos;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si;
	ZeroStruct(si);
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	FlatSB_GetScrollInfo( GetSafeHwnd(),SB_VERT,&si );

	// Get the minimum and maximum scroll-bar positions.
	int minpos = si.nMin;
	int maxpos = si.nMax;
	int nPage = si.nPage;

	// Get the current position of scroll box.
	int curpos = si.nPos;

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:      // Scroll to far left.
		curpos = minpos;
		break;

	case SB_RIGHT:      // Scroll to far right.
		curpos = maxpos;
		break;

	case SB_ENDSCROLL:   // End scroll.
		break;

	case SB_LINELEFT:      // Scroll left.
		if (curpos > minpos)
			curpos--;
		break;

	case SB_LINERIGHT:   // Scroll right.
		if (curpos < maxpos)
			curpos++;
		break;

	case SB_PAGELEFT:    // Scroll one page left.
		if (curpos > minpos)
			curpos = max(minpos, curpos - (int)nPage);
		break;

	case SB_PAGERIGHT:      // Scroll one page right.
		if (curpos < maxpos)
			curpos = min(maxpos, curpos + (int)nPage);
		break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		curpos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		curpos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	FlatSB_SetScrollPos( GetSafeHwnd(),SB_VERT,curpos,TRUE );

	m_scrollOffset.y = curpos;
	Invalidate();

	//CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

//////////////////////////////////////////////////////////////////////////
BOOL CPropertyCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (!m_tooltip.m_hWnd)
	{
		CRect rc;
		GetClientRect(rc);
		m_tooltip.Create( this );
		m_tooltip.SetDelayTime( 500 );
		m_tooltip.SetMaxTipWidth(600);
		m_tooltip.AddTool( this,"",rc,1 );
		m_tooltip.Activate(FALSE);
	}
	m_tooltip.RelayEvent(pMsg);

	return CWnd::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::Init()
{
	ModifyStyle( 0,WS_VSCROLL );
	ModifyStyle( WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0 );
	
	InitializeFlatSB( GetSafeHwnd() );
	FlatSB_SetScrollProp( GetSafeHwnd(),WSB_PROP_CXVSCROLL,14,FALSE );
	FlatSB_SetScrollProp( GetSafeHwnd(),WSB_PROP_VSTYLE,FSB_ENCARTA_MODE,FALSE );
	FlatSB_EnableScrollBar( GetSafeHwnd(),SB_VERT,ESB_ENABLE_BOTH );

	CRect rc;
	GetClientRect( rc );
	m_splitter = rc.Width()/2;

	if (!m_nTimer)
	{
		m_nTimer = SetTimer( KILLFOCUS_TIMER,500,NULL );
	}
}

//////////////////////////////////////////////////////////////////////////
int CPropertyCtrl::GetVisibleHeight()
{
	int y = 0;
	Items items;
	GetVisibleItems( m_root,items );
	for (int i = 0; i < items.size(); i++)
	{
		y += GetItemHeight( items[i] );
	}
	return y - 2;
}

//////////////////////////////////////////////////////////////////////////
LRESULT CPropertyCtrl::OnGetFont(WPARAM wParam, LPARAM)
{
	LRESULT res = Default();
	if (!res)
	{
		res = (LRESULT)GetStockObject(DEFAULT_GUI_FONT);
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////
BOOL CPropertyCtrl::EnableWindow( BOOL bEnable )
{
	SelectItem(0);
	return CWnd::EnableWindow( bEnable );
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == KILLFOCUS_TIMER)
	{
		if (m_selected)
		{
			// Check if need to kill focus.
			CWnd *pFocusWnd = GetFocus();
			if (pFocusWnd && pFocusWnd != this && !IsChild(pFocusWnd))
			{
				// And nothing should be captured.
				//CWnd *pFocusWndOwner = pFocusWnd->GetOwner();
				if (!GetCapture())
				{
					// Loose selection.
					//@TODO: restore
					//SelectItem(0);
				}
			}
		}
	}

	CWnd::OnTimer(nIDEvent);
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::ClearSelection()
{
	SelectItem(0);
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::DeleteItem( CPropertyItem *pItem )
{
	ClearSelection();
	assert( pItem );
	// Find this item and delete.
	CPropertyItem *pParentItem = pItem->GetParent();
	if (pParentItem)
	{
		pParentItem->RemoveChild( pItem );
		CalcLayout();
		Invalidate();
	}
}

//////////////////////////////////////////////////////////////////////////
CPropertyItem* CPropertyCtrl::FindItemByVar( IVariable *pVar )
{
	return m_root->FindItemByVar(pVar);
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::MultiSelectItem( CPropertyItem *pItem )
{
	if (!GetSelectedItem())
		SelectItem(pItem);
	pItem->SetSelected(true);
	m_multiSelectedItems.push_back(pItem);
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::MultiUnselectItem( CPropertyItem *pItem )
{
	if (pItem != GetSelectedItem())
	{
		stl::find_and_erase( m_multiSelectedItems,pItem );
		pItem->SetSelected(false);
	}
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::MultiSelectRange( CPropertyItem *pAnchorItem )
{
	if (!GetSelectedItem())
	{
		SelectItem(pAnchorItem);
		return;
	}
	if (pAnchorItem == GetSelectedItem())
		return;

	int i;
	int p1 = -1;
	int p2 = -1;
	Items items;
	GetVisibleItems( m_root,items );
	for (i = 0; i < items.size(); i++)
	{
		if (items[i] == GetSelectedItem())
			p1 = i;
		if (items[i] == pAnchorItem)
			p2 = i;
	}
	int start = min(p1,p2);
	int end = max(p1,p2);
	for (i = start; i <= end; i++)
	{
		MultiSelectItem( items[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::OnCopy( bool bRecursively )
{
	if (!m_multiSelectedItems.empty())
	{
		CClipboard clipboard;
		XmlNodeRef rootNode = new CXmlNode("PropertyCtrl");
		for (int i = 0; i < m_multiSelectedItems.size(); i++)
		{
			CPropertyItem *pItem = m_multiSelectedItems[i];
			CopyItem( rootNode,pItem,bRecursively );
		}
		clipboard.Put(rootNode);
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::OnCopyAll()
{
	if (m_root)
	{
		CClipboard clipboard;
		XmlNodeRef rootNode = new CXmlNode("PropertyCtrl");
		for (int i = 0; i < m_root->GetChildCount(); i++)
		{
			CopyItem( rootNode,m_root->GetChild(i),true );
		}
		clipboard.Put(rootNode);
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::CopyItem( XmlNodeRef rootNode,CPropertyItem *pItem,bool bRecursively )
{
	XmlNodeRef node = rootNode->newChild("PropertyItem");
	node->setAttr( "Name",pItem->GetFullName() );
	node->setAttr( "Value",pItem->GetValue() );
	if (bRecursively)
	{
		for (int i = 0; i < pItem->GetChildCount(); i++)
		{
			CopyItem( rootNode,pItem->GetChild(i),bRecursively );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyCtrl::OnPaste()
{
	CClipboard clipboard;

	CUndo undo( "Paste Properties" );

	XmlNodeRef rootNode = clipboard.Get();
	if (rootNode != NULL && rootNode->isTag("PropertyCtrl"))
	{
		for (int i = 0; i < rootNode->getChildCount(); i++)
		{
			XmlNodeRef node = rootNode->getChild(i);
			CString value;
			CString name;
			node->getAttr( "Name",name );
			node->getAttr( "Value",value );
			CPropertyItem *pItem = m_root->FindItemByFullName(name);
			if (pItem)
				pItem->SetValue(value);
		}
	}
}
