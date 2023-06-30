// StartupLogoDialog.cpp : implementation file
//

#include "stdafx.h"
#include "StartupLogoDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStartupLogoDialog dialog


CStartupLogoDialog::CStartupLogoDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CStartupLogoDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStartupLogoDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CStartupLogoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStartupLogoDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStartupLogoDialog, CDialog)
	//{{AFX_MSG_MAP(CStartupLogoDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStartupLogoDialog message handlers


void CStartupLogoDialog::SetVersion( const Version &v )
{
	if (m_hWnd != NULL)
		SetDlgItemText( IDC_VERSION,CString("v")+v.ToString() );
}

void CStartupLogoDialog::SetInfo( const CString &text )
{
	if (m_hWnd != NULL)
		SetDlgItemText( IDC_INFO_TEXT,text );
}

BOOL CStartupLogoDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText( "Starting Crytek Editor" );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
