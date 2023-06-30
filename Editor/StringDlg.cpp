// StringDlg.cpp : implementation file
//

#include "stdafx.h"
#include "StringDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStringDlg dialog


CStringDlg::CStringDlg( const char *title,CWnd* pParent /*=NULL*/)
	: CDialog(CStringDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStringDlg)
	m_strString = _T("");
	//}}AFX_DATA_INIT

	if (title)
		m_title = title;
}


void CStringDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStringDlg)
	DDX_Text(pDX, IDC_STRING, m_strString);
	DDV_MaxChars(pDX, m_strString, 256);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStringDlg, CDialog)
	//{{AFX_MSG_MAP(CStringDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStringDlg message handlers

BOOL CStringDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (!m_title.IsEmpty())
		SetWindowText( m_title );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CStringGroupDlg::CStringGroupDlg( const char *title,CWnd* pParent /*=NULL*/)
	: CDialog(CStringGroupDlg::IDD, pParent)
{
	m_strString = _T("");
	m_strGroup = _T("");

	if (title)
		m_title = title;
}


void CStringGroupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STRING, m_strString);
	DDV_MaxChars(pDX, m_strString, 256);
	DDX_Text(pDX, IDC_GROUP, m_strGroup);
	DDV_MaxChars(pDX, m_strGroup, 256);
}

BEGIN_MESSAGE_MAP(CStringGroupDlg, CDialog)
	//{{AFX_MSG_MAP(CStringGroupDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStringGroupDlg message handlers

BOOL CStringGroupDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (!m_title.IsEmpty())
		SetWindowText( m_title );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
