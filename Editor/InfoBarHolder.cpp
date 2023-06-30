// InfoBarHolder.cpp : implementation file
//

#include "stdafx.h"
#include "InfoBarHolder.h"
#include "InfoBar.h"
#include "InfoProgressBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInfoBarHolder dialog


CInfoBarHolder::CInfoBarHolder()
{
	//{{AFX_DATA_INIT(CInfoBarHolder)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_infoBar = 0;
	m_bInProgressBarMode = false;
	m_progressBar = 0;
}

CInfoBarHolder::~CInfoBarHolder()   // standard destructor
{
	delete m_infoBar;
	delete m_progressBar;
}

void CInfoBarHolder::DoDataExchange(CDataExchange* pDX)
{
	CXTDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfoBarHolder)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInfoBarHolder, CXTDialogBar)
	//{{AFX_MSG_MAP(CInfoBarHolder)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfoBarHolder message handlers
BOOL CInfoBarHolder::Create( CWnd* pParentWnd, UINT nStyle, UINT nID )
{
	CXTDialogBar::Create( pParentWnd,IDD,nStyle,nID );
	
	m_infoBar = new CInfoBar;
	m_infoBar->Create( CInfoBar::IDD,this ); 
	m_infoBar->ShowWindow( SW_SHOW );

	m_progressBar = new CInfoProgressBar;
	m_progressBar->Create( CInfoProgressBar::IDD,this ); 
	m_progressBar->ShowWindow( SW_HIDE );
	
	return TRUE;
}

//void CInfoBarHolder::OnIdleUpdateCmdUI()
void CInfoBarHolder::IdleUpdate()
{
	if (m_infoBar && !m_bInProgressBarMode)
		m_infoBar->IdleUpdate();
}

void CInfoBarHolder::EnableProgressBar( bool bEnable )
{
	m_bInProgressBarMode = bEnable;
	if (bEnable)
	{
		m_infoBar->ShowWindow( SW_HIDE );
		m_progressBar->ShowWindow( SW_SHOW );
	}
	else
	{
		m_infoBar->ShowWindow( SW_SHOW );
		m_progressBar->ShowWindow( SW_HIDE );
	}
}