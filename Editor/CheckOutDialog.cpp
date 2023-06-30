// CheckOutDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CheckOutDialog.h"


// CCheckOutDialog dialog

IMPLEMENT_DYNAMIC(CCheckOutDialog, CDialog)

CCheckOutDialog::CCheckOutDialog( const CString &file,CWnd* pParent /*=NULL*/)
	: CDialog(CCheckOutDialog::IDD, pParent)
	, m_text(_T(""))
{
	m_file = file;
	m_result = CANCEL;
}

CCheckOutDialog::~CCheckOutDialog()
{
}

void CCheckOutDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TEXT, m_text);
}


BEGIN_MESSAGE_MAP(CCheckOutDialog, CDialog)
	ON_BN_CLICKED(IDC_CHECKOUT, OnBnClickedCheckout)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
// CCheckOutDialog message handlers
void CCheckOutDialog::OnBnClickedCheckout()
{
	// Check out this file.
	m_result = CHECKOUT;
	OnOK();
}

//////////////////////////////////////////////////////////////////////////
void CCheckOutDialog::OnBnClickedOk()
{
	// Overwrite this file.
	m_result = OVERWRITE;
	OnOK();
}

BOOL CCheckOutDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString title;
	GetWindowText( title );
	title += "  (";
	title += m_file;
	title += ")";
	SetWindowText( title );

	m_text.Format( _T("%s is read-only file, can be under Source Control."),(const char*)m_file );

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
