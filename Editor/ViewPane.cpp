// ViewPane.cpp : implementation file
//

#include "stdafx.h"
#include "ViewPane.h"
#include "ViewManager.h"

#include "LayoutWnd.h"
#include "Viewport.h"
#include "LayoutConfigDialog.h"

#include "TopRendererWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_MAXIMIZED 50000
#define ID_LAYOUT_CONFIG 50001
#define FIRST_ID_VIEWPORT 50100

/////////////////////////////////////////////////////////////////////////////
// CViewPane

IMPLEMENT_DYNCREATE(CViewPane, CView)

BEGIN_MESSAGE_MAP(CViewPane, CView)
	//{{AFX_MSG_MAP(CViewPane)
	ON_WM_CREATE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CViewPane::CViewPane()
{
	m_viewportType = ET_ViewportUnknown;

	m_viewport = 0;
	m_active = 0;
	m_nBorder = 1;

	m_titleHeight = 16;
	m_bFullscreen = false;

	m_id = -1;
}

//////////////////////////////////////////////////////////////////////////
CViewPane::~CViewPane()
{
}

//////////////////////////////////////////////////////////////////////////
void CViewPane::OnDestroy() 
{
	DetachViewport();

	CView::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CViewPane drawing

void CViewPane::OnDraw(CDC* pDC)
{
	////////////////////////////////////////////////////////////////////////
	// Paint the window with a gray brush and draw the view controls and
	// the view's caption. Also draw the context menu button
	////////////////////////////////////////////////////////////////////////

	//CPaintDC dc(this);
	CDC &dc = *pDC;

	CDC cBmpDC;
    CRect rect, rectScreen;
	CBrush cFillBrush;
	CString cWindowText;
	CFont cFont;
	CBitmap cViewTools;
	CBrush cGreenBrush(0x0022FF22);
	CBrush cWhiteBrush(0x00FFFFFF);

	//bool active = GetFocus() == m_viewport;
	bool active = m_active;
	
	// Create a font and select it into the DC. make the font bold if this is the
	// active view
	VERIFY(cFont.CreateFont(-::MulDiv(8, GetDeviceCaps(dc.m_hDC, LOGPIXELSY), 72), 0, 0, 0, 
		(active) ? FW_BOLD : FW_DONTCARE, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH, "Tahoma"));
	dc.SelectObject(cFont);

	// Get the rect of the client window
	GetClientRect(&rect);

	CRect vpRect( m_nBorder,m_nBorder+m_titleHeight-1,rect.right-m_nBorder-1,rect.bottom-m_nBorder-m_titleHeight );

	if (!m_viewport)
		dc.FillRect(&rect, &cWhiteBrush);

	bool bHighlightTitle = false;
	if (m_viewport)
	{
		if (m_viewport->IsAVIRecording() || GetIEditor()->GetAnimation()->IsRecording())
			bHighlightTitle = true;
	}

	COLORREF cTextColor = dc.GetTextColor();
	
	// Create a Title filled rect.
	rect.bottom = m_titleHeight;
	if (!bHighlightTitle)
	{
		cFillBrush.m_hObject = ::GetSysColorBrush(COLOR_BTNFACE);
	}
	else
	{
		cFillBrush.CreateSolidBrush(RGB(255,0,0));
		cTextColor = RGB(255,255,255);
	}
	dc.FillRect(&rect, &cFillBrush);


	// Get a client rect and prepare writing the view's name	
	GetClientRect(&rect);
	GetWindowText(cWindowText);
	dc.SetBkMode(TRANSPARENT);

	CString sTitleText = cWindowText;
	sTitleText.Format( "%s (%dx%d)",(const char*)cWindowText,(int)vpRect.Width(),(int)vpRect.Height() );

	COLORREF cPrevTextColor = dc.SetTextColor( cTextColor );
	// Write the view's name
	rect.top += 1;
	dc.DrawText(sTitleText, rect, DT_CENTER);
	dc.SetTextColor( cPrevTextColor );

	// Draw the button for the view's menu
	GetClientRect(&rect);
	rect.left = rect.right - 12;
	rect.right -= 3;
	rect.top = 3;
	rect.bottom = 12;
	dc.DrawFrameControl(&rect, DFC_BUTTON, ((active) ? DFCS_BUTTONPUSH : DFCS_BUTTONPUSH) | DFCS_FLAT);

	// Draw selection.
	GetClientRect(&rect);
	CPen pen;
	if (active)
		pen.CreatePen(PS_SOLID, 1, RGB(255,0,0));
	else
		pen.CreatePen(PS_SOLID, 1, RGB(255,255,255));

	dc.SelectObject( pen );
	dc.MoveTo( 0,0 );
	dc.LineTo( rect.right-1,0 );
	dc.LineTo( rect.right-1,rect.bottom-1 );
	dc.LineTo( 0,rect.bottom-1 );
	dc.LineTo( 0,0 );
}

/////////////////////////////////////////////////////////////////////////////
// CViewPane diagnostics

#ifdef _DEBUG
void CViewPane::AssertValid() const
{
	//CView::AssertValid();
}

//////////////////////////////////////////////////////////////////////////
void CViewPane::Dump(CDumpContext& dc) const
{
	//CView::Dump(dc);
}

#endif //_DEBUG

//////////////////////////////////////////////////////////////////////////
int CViewPane::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	CLogFile::WriteLine("Creating view...");
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	//m_viewport.Create( this,"EditWnd",m_editSink );

	// Create the standard toolbar
	//m_toolBar.CreateEx(this, TBSTYLE_FLAT,WS_CHILD|WS_VISIBLE|CBRS_ALIGN_TOP);
	//m_toolBar.LoadToolBar(IDR_VIEW);
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CViewPane::AssignViewport( EViewportType type )
{
	m_viewportType = type;

	CViewport *viewport = GetIEditor()->GetViewManager()->CreateView( m_viewportType,this );
	if (viewport != m_viewport && m_viewport)
	{
		DetachViewport();
	}
	m_viewport = viewport;
	if (m_viewport)
	{
		SetViewport( m_viewport );
	}
}

//////////////////////////////////////////////////////////////////////////
void CViewPane::SetViewport( CViewport *pViewport )
{
	if (pViewport)
	{
		m_viewport = pViewport;
		m_viewportType = pViewport->GetType();

		m_viewport->ModifyStyle( WS_POPUP,WS_CHILD,0 );
		m_viewport->SetParent( this );
		m_viewport->ShowWindow( SW_SHOW );

		CRect rc;
		GetClientRect( rc );
		m_viewport->MoveWindow( m_nBorder,m_nBorder+m_titleHeight-1,rc.right-m_nBorder-1,rc.bottom-m_nBorder-m_titleHeight );
		SetWindowText( m_viewport->GetName() );
		m_viewport->Invalidate(FALSE);
		Invalidate(FALSE);
	}
	else
	{
		m_viewport = 0;
		m_viewportType = ET_ViewportUnknown;
	}
}

//////////////////////////////////////////////////////////////////////////
void CViewPane::SwapViewports( CViewPane *pView )
{
	CViewport *pViewport = pView->GetViewport();
	CViewport *pViewportOld = m_viewport;

	SetViewport( pViewport );
	pView->SetViewport(pViewportOld);
}

//////////////////////////////////////////////////////////////////////////
void CViewPane::DetachViewport()
{
	if (m_viewport && m_viewport->m_hWnd != 0)
	{
		m_viewport->ShowWindow( SW_HIDE );
		m_viewport->SetParent( 0 );
		m_viewport->ModifyStyle( WS_CHILD,WS_POPUP,0 );
		GetIEditor()->GetViewManager()->ReleaseView(m_viewport);
		m_viewport = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CViewPane::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	int toolHeight = 0;
	
	/*
	// TODO: Add your message handler code here
	if (m_toolBar)
	{
		CRect rc;
		m_toolBar.MoveWindow( 0,0,cx-1,20 );
		m_toolBar.GetWindowRect( rc );
		toolHeight = rc.bottom - rc.top;
	}
	*/
	int nBorder = 2;

	if (m_viewport)
		m_viewport->MoveWindow( m_nBorder,m_nBorder+m_titleHeight-1,cx-m_nBorder-1,cy-m_nBorder-m_titleHeight );
}

//////////////////////////////////////////////////////////////////////////
BOOL CViewPane::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	// Do nothing.
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
void CViewPane::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	m_active = bActivate;
	if (bActivate)
	{
		if (m_viewport)
		{
			m_viewport->SetActive(true);
			m_viewport->SetFocus();
		}
	}
	else
	{
		if (m_viewport)
			m_viewport->SetActive(false);
	}

	RedrawWindow();
}

//////////////////////////////////////////////////////////////////////////
void CViewPane::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CView::OnRButtonDown(nFlags, point);

	////////////////////////////////////////////////////////////////////////
	// Process clicks on the view buttons and the menu button
	////////////////////////////////////////////////////////////////////////
	RECT rcClient;
	// Only continue when we have a viewport.

	// Call the event handler
	GetClientRect(&rcClient);
	ClientToScreen( &point );
	
	// Create pop up menu.
	CMenu m;
	CMenu viewsMenu;
	m.CreatePopupMenu();
	viewsMenu.CreatePopupMenu();

	if (m_viewport)
		m_viewport->OnTitleMenu( m );
	if (m.GetMenuItemCount() > 0)
	{
		m.AppendMenu( MF_SEPARATOR,0,"" );
	}
	m.AppendMenu( MF_STRING|((IsFullscreen())?MF_CHECKED:MF_UNCHECKED),ID_MAXIMIZED,"Maximized" );
	m.AppendMenu( MF_POPUP,(UINT_PTR)viewsMenu.GetSafeHmenu(),"View" );
	m.AppendMenu( MF_STRING|((IsFullscreen())?MF_CHECKED:MF_UNCHECKED),ID_LAYOUT_CONFIG,"Configure Layout..." );

	std::vector<CViewportDesc*> vdesc;
	GetIEditor()->GetViewManager()->GetViewportDescriptions( vdesc );
	for (int i = 0; i < vdesc.size(); i++)
	{
		int flags = MF_STRING;
		if (vdesc[i]->type == m_viewportType)
			flags |= MF_CHECKED;
		else
			flags |= MF_UNCHECKED;
		if (m_viewportType == ET_ViewportCamera || vdesc[i]->type == ET_ViewportCamera)
			flags |= MF_GRAYED;
		viewsMenu.AppendMenu( flags, FIRST_ID_VIEWPORT+i,vdesc[i]->name );
	}

	/*
	int ViewId = 101;
	// Append other viewes.
	std::vector<CViewport*> views;
	CViewManager::Instance()->GetViews( views );
	for (int i = 0; i < views.size(); i++)
	{
		m.AppendMenu( MF_STRING|((IsFullscreen())?MF_CHECKED:MF_UNCHECKED),ViewId+i,views[i]->GetViewName() );
	}
	*/

	int id = m.TrackPopupMenu( TPM_RETURNCMD|TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this );
	// Ids above 100 reserved for view pane commands.
	if (id < ID_MAXIMIZED)
	{
		if (m_viewport)
			m_viewport->OnTitleMenuCommand( id );
	} else if (id >= FIRST_ID_VIEWPORT)
	{
		// Viewport type specified.
		id = id - FIRST_ID_VIEWPORT;
		if (vdesc[id]->type != m_viewportType)
		{
			CLayoutWnd *wnd = GetIEditor()->GetViewManager()->GetLayout();
			if (wnd)
			{
				DetachViewport();
				wnd->AssignViewport( this,vdesc[id]->type );
			}
		}
	} else {
		CLayoutWnd *layout = GetIEditor()->GetViewManager()->GetLayout();
		switch (id)
		{
		case ID_MAXIMIZED:
			if (m_viewport && layout)
			{
        layout->MaximizeViewport( GetId() );
				//SetFullscreenViewport(true);
			}
			break;

		case ID_LAYOUT_CONFIG:
			{
				if (layout)
				{
					CLayoutConfigDialog dlg;
					dlg.SetLayout( layout->GetLayout() );
					if (dlg.DoModal() == IDOK)
					{
						// Will kill this Pane. so must be last line in this function.
						layout->CreateLayout( dlg.GetLayout() );
					}
				}
			}
			break;
		}
	}
}

void CViewPane::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	////////////////////////////////////////////////////////////////////////
	// Switch in and out of fullscreen mode for a edit view
	////////////////////////////////////////////////////////////////////////
	CLayoutWnd *wnd = GetIEditor()->GetViewManager()->GetLayout();
	if (wnd)
		wnd->MaximizeViewport( GetId() );
}

BOOL CViewPane::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CViewPane::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (m_viewport)
		m_viewport->UpdateContent( 0xFFFFFFFF );
}

void CViewPane::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	// Forward SetFocus call to child viewport.
	if (m_viewport)
		m_viewport->SetFocus();
}

BOOL CViewPane::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// Extend the framework's command route from the view to
   // the application-specific CMyShape that is currently selected
   // in the view. m_pActiveShape is NULL if no shape object
   // is currently selected in the view.
   if ((m_viewport != NULL)
      && m_viewport->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CViewPane::SetFullscren( bool f )
{
	m_bFullscreen = f;
}

//////////////////////////////////////////////////////////////////////////
void CViewPane::SetFullscreenViewport( bool b )
{
	if (!m_viewport)
		return;

	if (b)
	{
		m_viewport->SetParent( 0 );
		m_viewport->ModifyStyle( WS_CHILD,WS_POPUP,0 );
		
		GetIEditor()->GetRenderer()->ChangeResolution( 800,600,32,80,true );

	}
	else
	{
		m_viewport->SetParent( this );
		m_viewport->ModifyStyle( WS_POPUP,WS_CHILD,0 );
		GetIEditor()->GetRenderer()->ChangeResolution( 800,600,32,80,false );
	}
}
