// TerrainHolePanel.cpp : implementation file
//

#include "stdafx.h"
#include "TerrainHolePanel.h"
#include "TerrainHoleTool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTerrainHolePanel dialog


CTerrainHolePanel::CTerrainHolePanel(CTerrainHoleTool *tool,CWnd* pParent /*=NULL*/)
	: CDialog(CTerrainHolePanel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTerrainHolePanel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	assert( tool != 0 );
	m_tool = tool;
	Create( IDD,pParent );
}


void CTerrainHolePanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTerrainHolePanel)
	DDX_Control(pDX, IDC_RADIUS, m_radius);
	DDX_Control(pDX, IDC_HOLE_REMOVE, m_removeHole);
	DDX_Control(pDX, IDC_HOLE_MAKE, m_makeHole);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTerrainHolePanel, CDialog)
	//{{AFX_MSG_MAP(CTerrainHolePanel)
	ON_BN_CLICKED(IDC_HOLE_MAKE, OnHoleMake)
	ON_BN_CLICKED(IDC_HOLE_REMOVE, OnHoleRemove)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_RADIUS, OnReleasedcaptureRadius)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTerrainHolePanel message handlers

void CTerrainHolePanel::OnHoleMake() 
{
	SetMakeHole(true);
	m_tool->SetMakeHole(true);
}

void CTerrainHolePanel::OnHoleRemove() 
{
	SetMakeHole(false);
	m_tool->SetMakeHole(false);
}

BOOL CTerrainHolePanel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	//m_makeHole.SetPushedBkColor( STD_PUSHED_COLOR );
	//m_removeHole.SetPushedBkColor( STD_PUSHED_COLOR );
	
	m_makeHole.SetCheck(1);
	m_removeHole.SetCheck(0);

	m_radius.SetRange( 1,100 );
	m_radius.SetPos( m_tool->GetBrushRadius()*2.0f );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTerrainHolePanel::OnReleasedcaptureRadius(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int radius = m_radius.GetPos();
	m_tool->SetBrushRadius(radius/2.0f);
	*pResult = 0;
}

void CTerrainHolePanel::SetMakeHole( bool bEnable )
{
	if (bEnable)
	{
		m_makeHole.SetCheck(1);
		m_removeHole.SetCheck(0);
	}
	else
	{
		m_makeHole.SetCheck(0);
		m_removeHole.SetCheck(1);
	}
}