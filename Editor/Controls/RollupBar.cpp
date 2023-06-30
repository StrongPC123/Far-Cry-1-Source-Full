// RollupBar.cpp: implementation of the CRollupBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RollupBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRollupBar::CRollupBar()
{
	CLogFile::WriteLine("RollUp bar created");
}

CRollupBar::~CRollupBar()
{
	CLogFile::WriteLine("RollUp bar destroied");
}

BEGIN_MESSAGE_MAP(CRollupBar, CWnd)
    //{{AFX_MSG_MAP(CWnd)
    ON_WM_CREATE()
		ON_NOTIFY( TCN_SELCHANGE, IDC_ROLLUPTAB, OnTabSelect )
		ON_WM_SIZE()
		ON_WM_CTLCOLOR()
		ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CRollupBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	////////////////////////////////////////////////////////////////////////
	// 
	////////////////////////////////////////////////////////////////////////

	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//m_cImageList.Create(IDB_TREE_VIEW, 16, 1, RGB (255, 0, 255));
	//m_tabImageList.Create(IDB_TABPANEL, 22, 1, RGB (0,255,255));
	CMFCUtils::LoadTrueColorImageList( m_tabImageList,IDB_TABPANEL,22,TOOLBAR_TRANSPARENT_COLOR );

	CRect rc;
	m_tab.Create( TCS_HOTTRACK|TCS_TABS|TCS_FOCUSNEVER|TCS_SINGLELINE| WS_CHILD|WS_VISIBLE,rc,this,IDC_ROLLUPTAB );
	m_tab.ModifyStyle( WS_BORDER,0,0 );
	m_tab.ModifyStyleEx( WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_WINDOWEDGE,0,0 );
	m_tab.SetImageList( &m_tabImageList );
	m_tab.InsertItem( 0,NULL,0 );
	m_tab.InsertItem( 1,NULL,1 );
	m_tab.InsertItem( 2,NULL,2 );
	m_tab.InsertItem( 3,NULL,3 );
	m_tab.SetFont( CFont::FromHandle( (HFONT)::GetStockObject(DEFAULT_GUI_FONT)) );

	m_selectedCtrl = 0;

	return 0;
}

void CRollupBar::OnSize(UINT nType, int cx, int cy)
{
	////////////////////////////////////////////////////////////////////////
	// Resize
	////////////////////////////////////////////////////////////////////////

	RECT rcRollUp;

	CWnd::OnSize(nType, cx, cy);

	// Get the size of the client window
	GetClientRect(&rcRollUp);

	m_tab.MoveWindow(	rcRollUp.left, rcRollUp.top,
										rcRollUp.right,rcRollUp.bottom );


	CRect rc;
	for (int i = 0; i < m_controls.size(); i++)
	{
		CRect irc;
		m_tab.GetItemRect( 0,irc );
		m_tab.GetClientRect( rc );
		if (m_controls[i]) 
		{
			rc.left += 1;
			rc.right -= 2;
			rc.top += irc.bottom-irc.top+8;
			rc.bottom -= 2;
			m_controls[i]->MoveWindow( rc );
		}
	}
	
		/*
		// Set the position of the listbox
		m_pwndRollUpCtrl->SetWindowPos(NULL, rcRollUp.left + 3, rcRollUp.top + 3 + h, rcRollUp.right - 6, 
		rcRollUp.bottom - 6 - m_infoSize.cy - infoOfs - h, SWP_NOZORDER);
	*/
}

void CRollupBar::OnTabSelect(NMHDR* pNMHDR, LRESULT* pResult)
{
	int sel = m_tab.GetCurSel();
	Select( sel );
}

void CRollupBar::Select( int num )
{
	m_selectedCtrl = num;
	for (int i = 0; i < m_controls.size(); i++)
	{
		if (i == num)
		{
			m_controls[i]->ShowWindow( SW_SHOW );
		}
		else
		{
			m_controls[i]->ShowWindow( SW_HIDE );
		}
	}
}

void CRollupBar::SetRollUpCtrl( int i,CRollupCtrl *pCtrl )
{
	if (i >= m_controls.size())
	{
		m_controls.resize( i+1 );
	}
	pCtrl->SetParent( &m_tab );
	m_controls[i] = pCtrl;
	if (i != m_tab.GetCurSel())
	{
		m_controls[i]->ShowWindow( SW_HIDE );
	}
}

CRollupCtrl* CRollupBar::GetCurrentCtrl()
{
	ASSERT( m_selectedCtrl < m_controls.size() );
	return m_controls[m_selectedCtrl];
}