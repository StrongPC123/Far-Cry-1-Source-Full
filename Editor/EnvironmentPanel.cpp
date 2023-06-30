// EnvironmentPanel.cpp : implementation file
//

#include "stdafx.h"
#include "EnvironmentPanel.h"
#include "GameEngine.h"

#include "CryEditDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define HEIGHT_OFFSET 4

/////////////////////////////////////////////////////////////////////////////
// CEnvironmentPanel dialog


CEnvironmentPanel::CEnvironmentPanel(CWnd* pParent /*=NULL*/)
	: CDialog(CEnvironmentPanel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEnvironmentPanel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	Create( IDD,pParent );
}

CEnvironmentPanel::~CEnvironmentPanel()
{
}

void CEnvironmentPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEnvironmentPanel)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_APPLY, m_applyBtn);
}


BEGIN_MESSAGE_MAP(CEnvironmentPanel, CDialog)
	//{{AFX_MSG_MAP(CEnvironmentPanel)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_APPLY, OnBnClickedApply)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEnvironmentPanel message handlers

void CEnvironmentPanel::OnDestroy() 
{
	CDialog::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////
BOOL CEnvironmentPanel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// If properties window is already created.
	m_node = GetIEditor()->GetDocument()->GetEnvironmentTemplate();

	if (m_node)
	{	
		CRect rc;
		m_wndProps.Create( WS_VISIBLE|WS_CHILD|WS_BORDER,rc,this );
		m_wndProps.CreateItems( m_node );
		m_wndProps.SetUpdateCallback( functor(*this,&CEnvironmentPanel::OnPropertyChanged) );
		m_wndProps.ExpandAll();

		// Resize to fit properties.
		GetClientRect( rc );
		int h = m_wndProps.GetVisibleHeight();
		SetWindowPos( NULL,0,0,rc.right,h+HEIGHT_OFFSET*2 + 34,SWP_NOMOVE );
		
		GetClientRect( rc );
		CRect rcb;
		m_applyBtn.GetWindowRect( rcb );
		ScreenToClient( rcb );
		m_applyBtn.SetWindowPos( NULL,rcb.left,rc.bottom-28,0,0,SWP_NOSIZE );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CEnvironmentPanel::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if (m_wndProps.m_hWnd)
	{
		int h = m_wndProps.GetVisibleHeight();
		CRect rc( 2,HEIGHT_OFFSET,cx-2,h+7 );
		m_wndProps.MoveWindow( rc,TRUE );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEnvironmentPanel::OnPropertyChanged( XmlNodeRef node )
{
	//GetIEditor()->GetDocument()->ReloadEngineEnvironmentSettings();
}

void CEnvironmentPanel::OnBnClickedApply()
{

	GetIEditor()->GetGameEngine()->ReloadEnvironment();
}
