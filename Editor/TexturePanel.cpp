// TexturePanel.cpp : implementation file
//

#include "stdafx.h"
#include "cryedit.h"
#include "TexturePanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTexturePanel dialog


CTexturePanel::CTexturePanel( class CTextureTool *tool,CWnd* pParent /* = NULL */)
	: CDialog(CTexturePanel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTexturePanel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_tool = tool;
		
	Create( IDD,pParent );
}


void CTexturePanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTexturePanel)
	DDX_Control(pDX, IDC_APPLY, m_apply);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTexturePanel, CDialog)
	//{{AFX_MSG_MAP(CTexturePanel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTexturePanel message handlers

BOOL CTexturePanel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_shift[0].Create( this,IDC_SHIFTX );
	m_shift[1].Create( this,IDC_SHIFTY );

	m_scale[0].Create( this,IDC_SCALEX );
	m_scale[1].Create( this,IDC_SCALEY );

	m_rotate.Create( this,IDC_ROTATE );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
