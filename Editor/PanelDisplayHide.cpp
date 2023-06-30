// PanelDisplatHide.cpp : implementation file
//

#include "stdafx.h"
#include "PanelDisplayHide.h"
#include "DisplaySettings.h"

#include "Objects\ObjectManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPanelDisplayHide dialog


CPanelDisplayHide::CPanelDisplayHide(CWnd* pParent /*=NULL*/)
	: CDialog(CPanelDisplayHide::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPanelDisplayHide)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	Create( IDD,pParent );
}


void CPanelDisplayHide::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPanelDisplayHide)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPanelDisplayHide, CDialog)
	//{{AFX_MSG_MAP(CPanelDisplayHide)

	ON_BN_CLICKED(IDC_HIDE_ALL, OnHideAll)
	ON_BN_CLICKED(IDC_HIDE_NONE, OnHideNone)
	ON_BN_CLICKED(IDC_HIDE_INVERT, OnHideInvert)
	ON_BN_CLICKED(IDC_HIDE_ENTITY, OnChangeHideMask)
	ON_BN_CLICKED(IDC_HIDE_PREFABS, OnChangeHideMask)
	ON_BN_CLICKED(IDC_HIDE_BUILDING, OnChangeHideMask)
	ON_BN_CLICKED(IDC_HIDE_GROUP, OnChangeHideMask)
	ON_BN_CLICKED(IDC_HIDE_PATH, OnChangeHideMask)
	ON_BN_CLICKED(IDC_HIDE_SOUND, OnChangeHideMask)
	ON_BN_CLICKED(IDC_HIDE_TAGPOINT, OnChangeHideMask)
	ON_BN_CLICKED(IDC_HIDE_VOLUME, OnChangeHideMask)
	ON_BN_CLICKED(IDC_HIDE_STATOBJ, OnChangeHideMask)
	ON_BN_CLICKED(IDC_HIDE_BRUSH, OnChangeHideMask)
	ON_BN_CLICKED(IDC_HIDE_HELPERS, OnChangeHideMask)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPanelDisplayHide message handlers

BOOL CPanelDisplayHide::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_mask = GetIEditor()->GetDisplaySettings()->GetObjectHideMask();
	
	SetCheckButtons();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayHide::SetMask()
{
	GetIEditor()->GetDisplaySettings()->SetObjectHideMask( m_mask );
	GetIEditor()->GetObjectManager()->InvalidateVisibleList();
	GetIEditor()->UpdateViews( eUpdateObjects );
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayHide::OnHideAll() 
{
	m_mask = 0xFFFFFFFF;
	SetCheckButtons();
	SetMask();
}

void CPanelDisplayHide::OnHideNone() 
{
	m_mask = 0;
	SetCheckButtons();
	SetMask();
}

void CPanelDisplayHide::OnHideInvert() 
{
	m_mask = ~m_mask;
	SetCheckButtons();
	SetMask();
}

void CPanelDisplayHide::SetCheckButtons()
{
	// Check or uncheck buttons.	
	CheckDlgButton( IDC_HIDE_ENTITY,(m_mask&OBJTYPE_ENTITY)?BST_CHECKED:BST_UNCHECKED );
	CheckDlgButton( IDC_HIDE_PREFABS,(m_mask&OBJTYPE_PREFAB)?BST_CHECKED:BST_UNCHECKED );
	CheckDlgButton( IDC_HIDE_GROUP,(m_mask&OBJTYPE_GROUP)?BST_CHECKED:BST_UNCHECKED );
	CheckDlgButton( IDC_HIDE_TAGPOINT,(m_mask&OBJTYPE_TAGPOINT)?BST_CHECKED:BST_UNCHECKED );
	CheckDlgButton( IDC_HIDE_PATH,(m_mask&OBJTYPE_SHAPE)?BST_CHECKED:BST_UNCHECKED );
	CheckDlgButton( IDC_HIDE_VOLUME,(m_mask&OBJTYPE_VOLUME)?BST_CHECKED:BST_UNCHECKED );
	CheckDlgButton( IDC_HIDE_BRUSH,(m_mask&OBJTYPE_BRUSH)?BST_CHECKED:BST_UNCHECKED );
	CheckDlgButton( IDC_HIDE_AIPOINT,(m_mask&OBJTYPE_AIPOINT)?BST_CHECKED:BST_UNCHECKED );
}

void CPanelDisplayHide::OnChangeHideMask() 
{
	// TODO: Add your control notification handler code here
	m_mask = 0;
	
	// Check or uncheck buttons.	
	m_mask |= IsDlgButtonChecked( IDC_HIDE_ENTITY ) ? OBJTYPE_ENTITY: 0;
	m_mask |= IsDlgButtonChecked( IDC_HIDE_PREFABS ) ? OBJTYPE_PREFAB: 0;
	m_mask |= IsDlgButtonChecked( IDC_HIDE_GROUP ) ? OBJTYPE_GROUP: 0;
	m_mask |= IsDlgButtonChecked( IDC_HIDE_TAGPOINT ) ? OBJTYPE_TAGPOINT: 0;
	m_mask |= IsDlgButtonChecked( IDC_HIDE_AIPOINT ) ? OBJTYPE_AIPOINT: 0;
	m_mask |= IsDlgButtonChecked( IDC_HIDE_PATH )  ? OBJTYPE_SHAPE: 0;
	m_mask |= IsDlgButtonChecked( IDC_HIDE_VOLUME ) ? OBJTYPE_VOLUME: 0;
	m_mask |= IsDlgButtonChecked( IDC_HIDE_BRUSH ) ? OBJTYPE_BRUSH: 0;

	SetCheckButtons();

	SetMask();
}