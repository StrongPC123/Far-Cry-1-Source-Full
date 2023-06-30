// TestDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TestDialog.h"


// CTestDialog dialog

IMPLEMENT_DYNAMIC(CTestDialog, CDialog)
CTestDialog::CTestDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CTestDialog::IDD, pParent)
{
}

CTestDialog::~CTestDialog()
{
}

void CTestDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROPERTIES, m_props);
}


BEGIN_MESSAGE_MAP(CTestDialog, CDialog)
END_MESSAGE_MAP()


// CTestDialog message handlers

BOOL CTestDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	XmlParser parser;
	XmlNodeRef root = parser.parse( "c:\\mastercd\\test.xml" );
	m_props.CreateItems( root );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
