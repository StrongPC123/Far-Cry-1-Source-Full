// ConsoleDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ConsoleDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConsoleDialog dialog


CConsoleDialog::CConsoleDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CConsoleDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConsoleDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CConsoleDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConsoleDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConsoleDialog, CDialog)
	//{{AFX_MSG_MAP(CConsoleDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConsoleDialog message handlers

void CConsoleDialog::OnOK() 
{
}
