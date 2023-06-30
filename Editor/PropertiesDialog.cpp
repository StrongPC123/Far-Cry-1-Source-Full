// PropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "PropertiesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertiesDialog dialog


CPropertiesDialog::CPropertiesDialog( const CString &title,XmlNodeRef &node,CWnd* pParent /*=NULL*/)
	: CXTResizeDialog(CPropertiesDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPropertiesDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_title = title;
	m_node = node;
}


void CPropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropertiesDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropertiesDialog, CXTResizeDialog)
	//{{AFX_MSG_MAP(CPropertiesDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertiesDialog message handlers

BOOL CPropertiesDialog::OnInitDialog() 
{
	CXTResizeDialog::OnInitDialog();

	SetWindowText( m_title );

	CRect rc;
	GetClientRect( rc );
	m_wndProps.Create( WS_CHILD|WS_VISIBLE|WS_BORDER,rc,this );
	m_wndProps.MoveWindow( rc.left+4,rc.top+4,rc.right-8,rc.bottom-24,true );
	m_wndProps.SetUpdateCallback( functor(*this, &CPropertiesDialog::OnPropertyChange) );
	if (m_node)
	{	
		m_wndProps.CreateItems( m_node );
	}

	AutoLoadPlacement( "Dialogs\\PropertyDlg" );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPropertiesDialog::OnDestroy() 
{
	CXTResizeDialog::OnDestroy();
}

void CPropertiesDialog::OnSize(UINT nType, int cx, int cy) 
{
	CXTResizeDialog::OnSize(nType, cx, cy);
	
	CRect rc;
	GetClientRect( rc );
	//GetDlgItem(IDOK)->SetWindowPos( NULL,
	//rc.bottom -= 8;

	if (m_wndProps.m_hWnd)
		m_wndProps.MoveWindow( rc.left+4,rc.top+4,rc.right-8,rc.bottom-24,true );
}

void CPropertiesDialog::OnPropertyChange( IVariable *pVar )
{
	if (m_varCallback)
		m_varCallback( pVar );
}

void CPropertiesDialog::OnCancel()
{
	DestroyWindow();
}