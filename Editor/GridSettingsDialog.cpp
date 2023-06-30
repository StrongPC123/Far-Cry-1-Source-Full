// GridSettingsDialog.cpp : implementation file
//

#include "stdafx.h"

#include "GridSettingsDialog.h"
#include "ViewManager.h"


// CGridSettingsDialog dialog

IMPLEMENT_DYNAMIC(CGridSettingsDialog, CDialog)
CGridSettingsDialog::CGridSettingsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CGridSettingsDialog::IDD, pParent)
{
}

CGridSettingsDialog::~CGridSettingsDialog()
{
}

void CGridSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SNAP, m_snapToGrid);
	DDX_Control(pDX, IDC_ANGLESNAP, m_angleSnap);
}


BEGIN_MESSAGE_MAP(CGridSettingsDialog, CDialog)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
BOOL CGridSettingsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CGrid *pGrid = GetIEditor()->GetViewManager()->GetGrid();

	m_gridSize.Create( this,IDC_GRID_SIZE,CNumberCtrl::LEFTALIGN );
	m_gridScale.Create( this,IDC_GRID_SCALE,CNumberCtrl::LEFTALIGN );
	
	m_angleSnapScale.Create( this,IDC_ANGLESNAP_SIZE,CNumberCtrl::LEFTALIGN );
	m_angleSnapScale.SetInteger(true);

	m_gridSize.SetInteger(true);
	m_gridSize.SetRange( 0,1024 );
	m_gridSize.SetValue( pGrid->size );
	m_gridScale.SetValue( pGrid->scale );
	m_snapToGrid.SetCheck( (pGrid->IsEnabled())?BST_CHECKED:BST_UNCHECKED );
	
	m_angleSnap.SetCheck( (pGrid->IsAngleSnapEnabled())?BST_CHECKED:BST_UNCHECKED );
	m_angleSnapScale.SetValue( pGrid->GetAngleSnap() );


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CGridSettingsDialog::OnOK()
{
	CGrid *pGrid = GetIEditor()->GetViewManager()->GetGrid();

	pGrid->Enable( m_snapToGrid.GetCheck() == BST_CHECKED );
	pGrid->size = m_gridSize.GetValue();
	pGrid->scale = m_gridScale.GetValue();
	pGrid->bAngleSnapEnabled = m_snapToGrid.GetCheck() == BST_CHECKED;
	pGrid->angleSnap = m_angleSnapScale.GetValue();
	
	CDialog::OnOK();
}