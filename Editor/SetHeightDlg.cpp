// SetHeightDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SetHeightDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetHeightDlg dialog


CSetHeightDlg::CSetHeightDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetHeightDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetHeightDlg)
		// NOTE: the ClassWizard will add member initialization here
	m_sldHeight = 0;
	//}}AFX_DATA_INIT
}


void CSetHeightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetHeightDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Slider(pDX, IDC_HEIGHT_SLIDER, m_sldHeight);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetHeightDlg, CDialog)
	//{{AFX_MSG_MAP(CSetHeightDlg)
		// NOTE: the ClassWizard will add message map macros here
		ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetHeightDlg message handlers

BOOL CSetHeightDlg::OnInitDialog() 
{
	////////////////////////////////////////////////////////////////////////
	// Set the range of the slider control
	////////////////////////////////////////////////////////////////////////

	CSliderCtrl ctrlSlider;

	CLogFile::WriteLine("Opening set height dialog...");

	CDialog::OnInitDialog();

	// Set the range for the water level slider
	VERIFY(ctrlSlider.Attach(GetDlgItem(IDC_HEIGHT_SLIDER)->m_hWnd));
	ctrlSlider.SetRange(0, 255, TRUE);
	ctrlSlider.Detach();

	// Update the static control
	UpdateData(FALSE);
	OnHScroll(0, 0, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSetHeightDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	////////////////////////////////////////////////////////////////////////
	// Update the static control with the slider value
	////////////////////////////////////////////////////////////////////////

	char szValue[32];
	CSliderCtrl ctrlSlider;

	VERIFY(ctrlSlider.Attach(GetDlgItem(IDC_HEIGHT_SLIDER)->m_hWnd));
	sprintf(szValue, "%i", ctrlSlider.GetPos());
	SetDlgItemText(IDC_HEIGHT_STATIC, szValue);
	ctrlSlider.Detach();
	
	// Don't call the base class because we might call this function
	// with a NULL pointer just to update the static control

	// CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSetHeightDlg::SetHeight( float iHeight )
{
	////////////////////////////////////////////////////////////////////////
	// Set the value of the height slider
	////////////////////////////////////////////////////////////////////////

	m_sldHeight = iHeight;

	if (m_hWnd)
		UpdateData(FALSE);
}

float CSetHeightDlg::GetHeight()
{
	////////////////////////////////////////////////////////////////////////
	// Get the value of the height slider
	////////////////////////////////////////////////////////////////////////

	if (m_hWnd)
		UpdateData(TRUE);

	return m_sldHeight;
}
