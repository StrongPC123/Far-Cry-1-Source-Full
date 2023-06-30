// LoadingDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LoadingDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoadingDialog dialog

CLoadingDialog::CLoadingDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLoadingDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoadingDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLoadingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoadingDialog)
	DDX_Control(pDX, IDC_CONSOLE_OUTPUT, m_lstConsoleOutput);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoadingDialog, CDialog)
	//{{AFX_MSG_MAP(CLoadingDialog)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadingDialog message handlers

HBRUSH CLoadingDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	////////////////////////////////////////////////////////////////////////
	// Change the background color of the listbox to the window background
	// color. Also change the text background color
	////////////////////////////////////////////////////////////////////////

	HBRUSH hbr;

	if (nCtlColor == CTLCOLOR_LISTBOX)
	{
		// Get the background color and return this brush
		hbr = (HBRUSH) ::GetSysColorBrush(COLOR_BTNFACE);

		// Modify the text background mode
		pDC->SetBkMode(TRANSPARENT);
	}
	else
	{
		// Use default from base class
		hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}
