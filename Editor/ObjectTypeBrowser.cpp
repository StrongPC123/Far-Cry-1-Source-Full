// ObjectTypeBrowser.cpp : implementation file
//

#include "stdafx.h"
#include "ObjectTypeBrowser.h"
#include "ObjectCreateTool.h"
#include "Objects\ObjectManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ObjectTypeBrowser dialog


ObjectTypeBrowser::ObjectTypeBrowser(CWnd* pParent /*=NULL*/)
	: CDialog(ObjectTypeBrowser::IDD, pParent)
{
	//{{AFX_DATA_INIT(ObjectTypeBrowser)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_createTool = 0;

	Create( IDD,pParent );

	m_listBrush.CreateSolidBrush( RGB( 0xE0,0xE0,0xE0 ) );
}


void ObjectTypeBrowser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ObjectTypeBrowser)
	DDX_Control(pDX, IDC_LIST2, m_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ObjectTypeBrowser, CDialog)
	//{{AFX_MSG_MAP(ObjectTypeBrowser)
	ON_LBN_SELCHANGE(IDC_LIST2, OnSelchangeList)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ObjectTypeBrowser message handlers

void ObjectTypeBrowser::SetCategory( CObjectCreateTool *createTool,const CString &category )
{
	assert( createTool != 0 );
	m_createTool = createTool;
	m_category = category;
	std::vector<CString> types;
	GetIEditor()->GetObjectManager()->GetClassTypes( category,types );

	m_lastSel = -1;

	m_list.ResetContent();
	for (int i = 0; i < types.size(); i++)
	{
		int item = m_list.AddString( types[i] );
		m_list.SetItemHeight( item,16 );
	}
	if (types.size() > 0)
	{
		m_lastSel = 0;
		m_list.SetCurSel( 0 );
		m_createTool->StartCreation( types[0] );
	}
}

void ObjectTypeBrowser::OnSelchangeList() 
{
	// TODO: Add your control notification handler code here
	int sel = m_list.GetCurSel();
	if (sel >= 0 && m_lastSel != sel)
	{
		m_lastSel = sel;
		CString type;
		m_list.GetText( sel,type );

		// Start creating object of this type.
		if (m_createTool)
			m_createTool->StartCreation( type );
	}
}

HBRUSH ObjectTypeBrowser::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if (pWnd->GetDlgCtrlID() == IDC_LIST2)
	{
		// Set the background mode for text to transparent 
    // so background will show thru.
    pDC->SetBkMode(TRANSPARENT);
		hbr = m_listBrush;
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void ObjectTypeBrowser::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if (m_list.GetSafeHwnd())
	{
		m_list.SetWindowPos( NULL,0,0,cx-8,cy-10,SWP_NOMOVE );
	}
}
