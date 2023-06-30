// LayoutConfigDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LayoutConfigDialog.h"


// CLayoutConfigDialog dialog

IMPLEMENT_DYNAMIC(CLayoutConfigDialog, CDialog)
CLayoutConfigDialog::CLayoutConfigDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLayoutConfigDialog::IDD, pParent)
{
	m_layout = ET_Layout1;
}

CLayoutConfigDialog::~CLayoutConfigDialog()
{
}

void CLayoutConfigDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LAYOUTS, m_layouts);
}


BEGIN_MESSAGE_MAP(CLayoutConfigDialog, CDialog)
END_MESSAGE_MAP()


// CLayoutConfigDialog message handlers

BOOL CLayoutConfigDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_imageList.Create( MAKEINTRESOURCE(IDB_LAYOUTS),32,1,RGB(0,0,0) );
	m_layouts.SetImageList( &m_imageList,LVSIL_NORMAL  );

	m_layouts.SetBkColor( GetSysColor(COLOR_BTNFACE) );
	m_layouts.SetExtendedStyle(LVS_EX_BORDERSELECT);
	m_layouts.SetIconSpacing(CSize(42,36));

	m_layouts.InsertItem( 0,NULL,0 );
	m_layouts.InsertItem( 1,NULL,1 );
	m_layouts.InsertItem( 2,NULL,2 );
	m_layouts.InsertItem( 3,NULL,3 );
	m_layouts.InsertItem( 4,NULL,4 );
	m_layouts.InsertItem( 5,NULL,5 );
	m_layouts.InsertItem( 6,NULL,6 );
	m_layouts.InsertItem( 7,NULL,7 );
	m_layouts.InsertItem( 8,NULL,8 );

	CRect rc;
	m_layouts.GetClientRect(rc);
	rc.top += 8;
	m_layouts.SetWorkAreas( 1,&rc );

	m_layouts.Arrange(LVA_DEFAULT);

	int sel = m_layout;
	m_layouts.SetItemState( sel,LVIS_SELECTED,LVIS_SELECTED );
	m_layouts.SetItemState( sel,LVIS_FOCUSED,LVIS_FOCUSED );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CLayoutConfigDialog::OnOK()
{
	m_layout = (EViewLayout)m_layouts.GetSelectionMark();
	CDialog::OnOK();
}