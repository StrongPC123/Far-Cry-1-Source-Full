// InfoProgressBar.cpp : implementation file
//

#include "stdafx.h"
#include "cryedit.h"
#include "InfoProgressBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInfoProgressBar dialog


CInfoProgressBar::CInfoProgressBar(CWnd* pParent /*=NULL*/)
	: CDialog(CInfoProgressBar::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInfoProgressBar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bCanceled = false;
	m_percent = 0;
}


void CInfoProgressBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfoProgressBar)
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_INFO, m_info);
	DDX_Control(pDX, IDC_CANCEL, m_cancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInfoProgressBar, CDialog)
	//{{AFX_MSG_MAP(CInfoProgressBar)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfoProgressBar message handlers

//////////////////////////////////////////////////////////////////////////
void CInfoProgressBar::BeginProgress( const CString &infoText )
{
	m_info.SetWindowText( infoText );
	m_progress.SetRange( 0,100 );
	m_progress.SetPos( 0 );
	m_percent = 0;
	m_bCanceled = false;
}

bool CInfoProgressBar::UpdateProgress( int percent )
{
	if (percent != m_percent)
		m_progress.SetPos( percent );
	m_percent = percent;
	return !m_bCanceled;
}

void CInfoProgressBar::OnCancel() 
{
	m_bCanceled = true;
}
