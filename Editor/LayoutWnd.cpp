// LayoutWnd.cpp : implementation file
//

#include "stdafx.h"
#include "LayoutWnd.h"
#include "ViewPane.h"
#include "ViewManager.h"
#include "Viewport.h"
#include "CryEditDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLayoutSplitter,CSplitterWnd)
IMPLEMENT_DYNCREATE(CLayoutWnd,CWnd)

/////////////////////////////////////////////////////////////////////////////
// CLayoutSplitter
//////////////////////////////////////////////////////////////////////////
CLayoutSplitter::CLayoutSplitter()
{
	m_cxSplitter = m_cySplitter = 3 + 1 + 1;
	m_cxBorderShare = m_cyBorderShare = 0;
	m_cxSplitterGap = m_cySplitterGap = 3 + 1 + 1;
	m_cxBorder = m_cyBorder = 1;
}

CLayoutSplitter::~CLayoutSplitter()
{
}

BEGIN_MESSAGE_MAP(CLayoutSplitter, CSplitterWnd)
	//{{AFX_MSG_MAP(CLayoutSplitter)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
void CLayoutSplitter::OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rectArg)
{
	// Let CSplitterWnd handle everything but the border-drawing
	if((nType != splitBorder) || (pDC == NULL))
	{
		CSplitterWnd::OnDrawSplitter(pDC, nType, rectArg);
		return;
	}

	ASSERT_VALID(pDC);

	// Draw border
	pDC->Draw3dRect(rectArg, GetSysColor(COLOR_BTNSHADOW), GetSysColor(COLOR_BTNHIGHLIGHT));
}

//////////////////////////////////////////////////////////////////////////
void CLayoutSplitter::OnSize(UINT nType, int cx, int cy) 
{
	CSplitterWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here

	if (m_hWnd && m_pRowInfo && m_pColInfo)
	{
		CRect rc;
		GetClientRect( rc );

		int i;
		int rows = GetRowCount();
		int cols = GetColumnCount();
		for (i = 0; i < rows; i++)
		{
			float idealSize = 0;
			CViewPane *viewPane = (CViewPane*)GetPane(i,0);
			if (viewPane)
			{
				CViewport *viewport = viewPane->GetViewport();
				if (viewport)
					idealSize = viewport->GetIdealSize().cy;
			}
			if (idealSize == 0)
				SetRowInfo( i,rc.bottom/rows,10 );
			else
				SetRowInfo( i,idealSize,10 );
		}
		for (i = 0; i < cols; i++)
		{
			float idealSize = 0;
			CViewPane *viewPane = (CViewPane*)GetPane(0,i);
			if (viewPane)
			{
				CViewport *viewport = viewPane->GetViewport();
				if (viewport)
					idealSize = viewport->GetIdealSize().cx;
			}
			if (idealSize == 0)
				SetColumnInfo( i,rc.right/cols,10 );
			else
			{
				SetColumnInfo( i,idealSize,10 );
			}
		}

		RecalcLayout();
	}
}

//////////////////////////////////////////////////////////////////////////
void CLayoutSplitter::CreateLayoutView( int row,int col,int id,EViewportType viewType,CCreateContext* pContext )
{
	assert( row >= 0 && row < 3 );
	assert( col >= 0 && col < 3 );
	CreateView( row,col,RUNTIME_CLASS(CViewPane),CSize(100,100),pContext );
	CViewPane *viewPane = (CViewPane*)GetPane(row,col);
	if (viewPane)
	{
		viewPane->SetId(id);
	}
}

//////////////////////////////////////////////////////////////////////////
// CLayoutWnd
//////////////////////////////////////////////////////////////////////////
CLayoutWnd::CLayoutWnd()
{
	m_splitWnd = 0;
	m_splitWnd2 = 0;

	m_bMaximized = false;
	m_maximizedView = 0;
	m_layout = (EViewLayout)-1;

	m_maximizedViewId = 0;

	for (int i = 0; i < sizeof(m_viewType)/sizeof(m_viewType[0]); i++)
		m_viewType[i] = ET_ViewportUnknown;
}

//////////////////////////////////////////////////////////////////////////
CLayoutWnd::~CLayoutWnd()
{
	if (m_splitWnd)
		delete m_splitWnd;
	m_splitWnd = 0;

	if (m_splitWnd2)
		delete m_splitWnd2;
	m_splitWnd2 = 0;
}

BEGIN_MESSAGE_MAP(CLayoutWnd, CWnd)
	//{{AFX_MSG_MAP(CLayoutWnd)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
void CLayoutWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	if (m_hWnd)
	{
		CRect rc;
		GetClientRect( rc );
		
		// Move primary split window.
		// Secondary split window will be resized within primary.
		if (m_splitWnd)
			m_splitWnd->MoveWindow(rc);


		// Resize maximized view.
		if (m_maximizedView)
			m_maximizedView->MoveWindow(rc);
	}
}

//////////////////////////////////////////////////////////////////////////
void CLayoutWnd::UnbindViewports()
{
	// First unbind all views.
	for (int i = 0; i < MAX_VIEWPORTS; i++)
	{
		CViewPane *pViewPane = GetViewPane(i);
		if (pViewPane)
		{
			pViewPane->AssignViewport( ET_ViewportUnknown );
		}
	}

	if (m_maximizedView)
		m_maximizedView->AssignViewport( ET_ViewportUnknown );
}

//////////////////////////////////////////////////////////////////////////
void CLayoutWnd::BindViewports()
{
	// First unbind all views.
	UnbindViewports();

	for (int i = 0; i < MAX_VIEWPORTS; i++)
	{
		CViewPane *pViewPane = GetViewPane(i);
		if (pViewPane)
		{
			BindViewport( pViewPane,m_viewType[pViewPane->GetId()] );
		}
	}

	if (m_splitWnd)
		m_splitWnd->SetActivePane(0,0);

	//////////////////////////////////////////////////////////////////////////
	if (GetIEditor()->GetViewManager()->GetViewCount() == 1)
	{
		//MaximizeViewport( GetIEditor()->GetViewManager()->GetView(0)->GetType() );
		//return;
	}

	Invalidate(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void CLayoutWnd::BindViewport( CViewPane *vp,EViewportType type )
{
	assert( vp );
	vp->AssignViewport( type );
	vp->SetFullscren( false );
	vp->ShowWindow( SW_SHOW );
	m_viewType[vp->GetId()] = type;
}

//////////////////////////////////////////////////////////////////////////
void CLayoutWnd::BindViewport( CViewPane *vp,CViewport *pViewport )
{
	//vp->AssignViewport( pViewport );
	//vp->ShowWindow( SW_SHOW );
	//m_viewType[vp->GetId()] = pViewport->GetType();
}

//////////////////////////////////////////////////////////////////////////
void CLayoutWnd::MaximizeViewport( int paneId )
{
	// Ignore with full screen layout.
	if (m_layout == ET_Layout0 && m_bMaximized)
		return;

	EViewportType type = m_viewType[paneId];

	CRect rc;
	GetClientRect( rc );
	if (!m_bMaximized)
	{
		UnbindViewports();
		m_maximizedViewId = paneId;
		m_bMaximized = true;

		if (m_maximizedView)
		{
			if (m_splitWnd)
				m_splitWnd->ShowWindow( SW_HIDE );
			if (m_splitWnd2)
				m_splitWnd2->ShowWindow(SW_HIDE);

			m_maximizedView->ShowWindow( SW_SHOW );
			BindViewport( m_maximizedView,type );

			m_maximizedView->SetFocus();

			((CFrameWnd*)AfxGetMainWnd())->SetActiveView( m_maximizedView );
		}
	}
	else
	{
		m_bMaximized = false;
		m_maximizedViewId = 0;

		UnbindViewports();

		if (m_maximizedView)
			m_maximizedView->ShowWindow(SW_HIDE);

		if (m_splitWnd)
			m_splitWnd->ShowWindow(SW_SHOW);
		if (m_splitWnd2)
			m_splitWnd2->ShowWindow(SW_SHOW);

		BindViewports();

		if (m_splitWnd)
			m_splitWnd->SetActivePane(0,0);
	}

	RedrawWindow();
}

//////////////////////////////////////////////////////////////////////////
void CLayoutWnd::CreateSubSplitView( int row,int col,EViewLayout splitType,CCreateContext* pContext )
{
	assert( row >= 0 && row < 3 );
	assert( col >= 0 && col < 3 );
//	m_viewType[row][col] = -1;

	//m_secondSplitWnd = new CLayoutWnd;
	//m_secondSplitWnd->CreateLayout( this,splitType,pCtx,IdFromRowCol(0,0) );
}

void CLayoutWnd::CreateLayoutView( CLayoutSplitter *wndSplitter,int row,int col,int id,EViewportType viewType,CCreateContext* pContext )
{
	wndSplitter->CreateLayoutView( row,col,id,viewType,pContext );
	m_viewType[id] = viewType;
}

//////////////////////////////////////////////////////////////////////////
void CLayoutWnd::CreateLayout( EViewLayout layout,bool bBindViewports,EViewportType defaultView )
{
	UnbindViewports();

	m_layout = layout;
	m_bMaximized = false;

	CCreateContext ctx;
	ZeroStruct(ctx);
	ctx.m_pNewViewClass = RUNTIME_CLASS(CViewPane);
	ctx.m_pCurrentDoc = GetIEditor()->GetDocument();
	ctx.m_pCurrentFrame = (CFrameWnd*)AfxGetMainWnd();
	CCreateContext *pCtx = &ctx;

	if (m_splitWnd)
	{
		m_splitWnd->ShowWindow( SW_HIDE );
		m_splitWnd->DestroyWindow();
		delete m_splitWnd;
		m_splitWnd = 0;
	}

	if (m_splitWnd2)
	{
		delete m_splitWnd2;
		m_splitWnd2 = 0;
	}

	if (m_maximizedView)
		m_maximizedView->ShowWindow( SW_HIDE );

	CRect rc;
	GetClientRect(rc);

	if (!m_maximizedView)
	{
		m_maximizedView = new CViewPane;
		m_maximizedView->SetId(0);
		m_maximizedView->Create( 0,0,WS_CHILD|WS_VISIBLE,rc,this,0,pCtx );
		m_maximizedView->ShowWindow( SW_HIDE );
		m_maximizedView->SetFullscren(true);
	}

	switch (layout) {
	case ET_Layout0:
		m_viewType[0] = defaultView;
		MaximizeViewport(0);
		break;

	case ET_Layout1:
		m_splitWnd = new CLayoutSplitter;
		m_splitWnd->CreateStatic( this,1,2 );
		CreateLayoutView( m_splitWnd,0,0,2,ET_ViewportMap,pCtx );
		CreateLayoutView( m_splitWnd,0,1,1,defaultView,pCtx );
		break;
	case ET_Layout2:
		m_splitWnd = new CLayoutSplitter;
		m_splitWnd->CreateStatic( this,2,1 );
		CreateLayoutView( m_splitWnd,0,0,2,ET_ViewportMap,pCtx );
		CreateLayoutView( m_splitWnd,1,0,1,defaultView,pCtx );
		break;

	case ET_Layout3:
		m_splitWnd = new CLayoutSplitter;
		m_splitWnd->CreateStatic( this,1,2 );
		CreateLayoutView( m_splitWnd,0,1,1,defaultView,pCtx );

		m_splitWnd2 = new CLayoutSplitter;
		m_splitWnd2->CreateStatic( m_splitWnd,2,1,WS_CHILD|WS_VISIBLE,m_splitWnd->IdFromRowCol(0,0) );
		CreateLayoutView( m_splitWnd2,0,0,2,ET_ViewportXY,pCtx );
		CreateLayoutView( m_splitWnd2,1,0,3,ET_ViewportXZ,pCtx );
		break;

	case ET_Layout4:
		m_splitWnd = new CLayoutSplitter;
		m_splitWnd->CreateStatic( this,1,2 );
		CreateLayoutView( m_splitWnd,0,0,1,defaultView,pCtx );

		m_splitWnd2 = new CLayoutSplitter;
		m_splitWnd2->CreateStatic( m_splitWnd,2,1,WS_CHILD|WS_VISIBLE,m_splitWnd->IdFromRowCol(0,1) );
		CreateLayoutView( m_splitWnd2,0,0,2,ET_ViewportXY,pCtx );
		CreateLayoutView( m_splitWnd2,1,0,3,ET_ViewportXZ,pCtx );
		break;

	case ET_Layout5:
		m_splitWnd = new CLayoutSplitter;
		m_splitWnd->CreateStatic( this,2,1 );
		CreateLayoutView( m_splitWnd,1,0,1,defaultView,pCtx );

		m_splitWnd2 = new CLayoutSplitter;
		m_splitWnd2->CreateStatic( m_splitWnd,1,2,WS_CHILD|WS_VISIBLE,m_splitWnd->IdFromRowCol(0,0) );
		CreateLayoutView( m_splitWnd2,0,0,2,ET_ViewportXY,pCtx );
		CreateLayoutView( m_splitWnd2,0,1,3,ET_ViewportXZ,pCtx );
		break;

	case ET_Layout6:
		m_splitWnd = new CLayoutSplitter;
		m_splitWnd->CreateStatic( this,2,1 );
		CreateLayoutView( m_splitWnd,0,0,1,defaultView,pCtx );

		m_splitWnd2 = new CLayoutSplitter;
		m_splitWnd2->CreateStatic( m_splitWnd,1,2,WS_CHILD|WS_VISIBLE,m_splitWnd->IdFromRowCol(1,0) );
		CreateLayoutView( m_splitWnd2,0,0,2,ET_ViewportXY,pCtx );
		CreateLayoutView( m_splitWnd2,0,1,3,ET_ViewportXZ,pCtx );
		break;

	case ET_Layout7:
		m_splitWnd = new CLayoutSplitter;
		m_splitWnd->CreateStatic( this,2,2 );
		CreateLayoutView( m_splitWnd,0,0,2,ET_ViewportXZ,pCtx );
		CreateLayoutView( m_splitWnd,0,1,3,ET_ViewportYZ,pCtx );
		CreateLayoutView( m_splitWnd,1,0,4,ET_ViewportXY,pCtx );
		CreateLayoutView( m_splitWnd,1,1,1,defaultView,pCtx );
		break;

	case ET_Layout8:
		m_splitWnd = new CLayoutSplitter;
		m_splitWnd->CreateStatic( this,2,1 );
		CreateLayoutView( m_splitWnd,1,0,1,defaultView,pCtx );

		m_splitWnd2 = new CLayoutSplitter;
		m_splitWnd2->CreateStatic( m_splitWnd,1,3,WS_CHILD|WS_VISIBLE,m_splitWnd->IdFromRowCol(0,0) );
		CreateLayoutView( m_splitWnd2,0,0,2,ET_ViewportXY,pCtx );
		CreateLayoutView( m_splitWnd2,0,1,3,ET_ViewportXZ,pCtx );
		CreateLayoutView( m_splitWnd2,0,2,4,ET_ViewportYZ,pCtx );
		break;

	default:
		CLogFile::FormatLine( "Trying to Create Unknown Layout %d",(int)layout );
		AfxMessageBox( _T("Trying to Create Unknown Layout"),MB_OK|MB_ICONERROR );
		break;
	};

	if (m_splitWnd)
	{
		m_splitWnd->MoveWindow(rc);
		m_splitWnd->SetActivePane(0,0);
	}

	if (bBindViewports && !m_bMaximized)
		BindViewports();
}

//////////////////////////////////////////////////////////////////////////
void CLayoutWnd::SaveConfig()
{
	CWinApp *pApp = AfxGetApp();
	assert( pApp );
	pApp->WriteProfileInt( "Layout","Layout",(int)m_layout );
	pApp->WriteProfileInt( "Layout","Maximized",(int)m_maximizedViewId );

	CString str;
	int v[MAX_VIEWPORTS];
	for (int i = 0; i < MAX_VIEWPORTS; i++) v[i] = m_viewType[i];
	str.Format( "%d,%d,%d,%d,%d,%d,%d,%d,%d",v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],v[8] );
	pApp->WriteProfileString( "Layout","Views",str );
}

//////////////////////////////////////////////////////////////////////////
bool CLayoutWnd::LoadConfig()
{
	CWinApp *pApp = AfxGetApp();
	assert( pApp );
	int layout = pApp->GetProfileInt( "Layout","Layout",-1 );
	int maximizedView = pApp->GetProfileInt( "Layout","Maximized",0 );
	if (layout < 0)
		return false;

	CreateLayout( (EViewLayout)layout,false );

	bool bRebindViewports = false;
	if (m_splitWnd)
	{
		CString str = pApp->GetProfileString( "Layout","Views" );
		if (!str.IsEmpty())
		{
			int v[9];
			if (sscanf( str,"%d,%d,%d,%d,%d,%d,%d,%d,%d",&v[0],&v[1],&v[2],&v[3],&v[4],&v[5],&v[6],&v[7],&v[8] ) == 9)
			{
				bRebindViewports = true;
				for (int i = 0; i < MAX_VIEWPORTS; i++)
					m_viewType[i] = (EViewportType)v[i];
			}
		}
	}

	if (maximizedView > 0 && bRebindViewports)
	{
		MaximizeViewport( maximizedView );
	}
	else
	{
		if (bRebindViewports)
			BindViewports();
		else
			CreateLayout( (EViewLayout)layout );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CLayoutWnd::AssignViewport( CViewPane *vp,EViewportType type )
{
	assert( vp );

	//UnbindViewports();

	if (vp == m_maximizedView)
	{
		BindViewport( m_maximizedView,type );
		return;
	}

	if (type == ET_ViewportCamera)
	{
		// Cannot simply assign new viewport to camera.
		// must swap with existing viewport that holds perspective view.
	}

	BindViewport( vp,type );
	
	//BindViewports();
}

//////////////////////////////////////////////////////////////////////////
CViewPane* CLayoutWnd::GetViewPane( int id )
{
	if (m_splitWnd)
	{
		for (int row = 0; row < m_splitWnd->GetRowCount(); row++)
			for (int col = 0; col < m_splitWnd->GetColumnCount(); col++)
			{
				CWnd *pWnd = m_splitWnd->GetPane(row,col);
				if (pWnd->IsKindOf(RUNTIME_CLASS(CViewPane)))
				{
					CViewPane *pane = (CViewPane*)pWnd;
					if (pane && pane->GetId() == id)
						return pane;
				}
			}
	}
	if (m_splitWnd2)
	{
		for (int row = 0; row < m_splitWnd2->GetRowCount(); row++)
			for (int col = 0; col < m_splitWnd2->GetColumnCount(); col++)
			{
				CWnd *pWnd = m_splitWnd2->GetPane(row,col);
				if (pWnd->IsKindOf(RUNTIME_CLASS(CViewPane)))
				{
					CViewPane *pane = (CViewPane*)pWnd;
					if (pane && pane->GetId() == id)
						return pane;
				}
			}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
CViewPane* CLayoutWnd::FindViewByType( EViewportType type )
{
	for (int i = 1; i < MAX_VIEWPORTS; i++)
	{
		if (m_viewType[i] == type)
		{
			return GetViewPane(i);
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void CLayoutWnd::Cycle2DViewport()
{
	// Cycle between 3 2D viewports.
	CViewPane *vp = NULL;
	vp = FindViewByType( ET_ViewportXY );
	if (vp)
	{
		AssignViewport( vp,ET_ViewportXZ );
		return;
	}
	vp = FindViewByType( ET_ViewportXZ );
	if (vp)
	{
		AssignViewport( vp,ET_ViewportYZ );
		return;
	}
	vp = FindViewByType( ET_ViewportYZ );
	if (vp)
	{
		AssignViewport( vp,ET_ViewportXY );
		return;
	}
}

//////////////////////////////////////////////////////////////////////////
void CLayoutWnd::OnDestroy()
{
	CWnd::OnDestroy();

	// Also destroy all viewports.
	for (int i = 0; i < GetIEditor()->GetViewManager()->GetViewCount(); i++)
	{
		GetIEditor()->GetViewManager()->GetView(i)->DestroyWindow();
	}

	// TODO: Add your message handler code here
}
