// SizeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SizeDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSizeDialog dialog


CSizeDialog::CSizeDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSizeDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSizeDialog)
	m_txtTerrainSize = 0;
	//}}AFX_DATA_INIT
}


void CSizeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSizeDialog)
	DDX_Text(pDX, IDC_TERRAIN_SIZE, m_txtTerrainSize);
	DDV_MinMaxUInt(pDX, m_txtTerrainSize, 256, 4096);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSizeDialog, CDialog)
	//{{AFX_MSG_MAP(CSizeDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSizeDialog message handlers
