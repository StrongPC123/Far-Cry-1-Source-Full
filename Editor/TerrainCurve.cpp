// TerrainCurve.cpp : implementation file
//

#include "stdafx.h"
#include "TerrainCurve.h"
#include "CryEditDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTerrainCurve dialog


CTerrainCurve::CTerrainCurve(CWnd* pParent /*=NULL*/)
	: CDialog(CTerrainCurve::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTerrainCurve)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTerrainCurve::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTerrainCurve)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTerrainCurve, CDialog)
	//{{AFX_MSG_MAP(CTerrainCurve)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTerrainCurve message handlers

BOOL CTerrainCurve::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Create the curve editing window
	m_wndCurve.SetCurveObject(&GetIEditor()->GetDocument()->m_cTerrainCurve);
	m_wndCurve.Create("CurveWnd", CRect(10, 10, 256, 256), this, IDC_CURVE_WND, false);	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTerrainCurve::OnDestroy() 
{
	CDialog::OnDestroy();

	// Prevent the editing window from destroying the curve object
	m_wndCurve.SetCurveObject(NULL);
}
