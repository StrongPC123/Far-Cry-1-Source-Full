// SoundObjectPanel.cpp : implementation file
//

#include "stdafx.h"
#include "SoundObjectPanel.h"
#include "Objects\\SoundObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSoundObjectPanel dialog


CSoundObjectPanel::CSoundObjectPanel(CWnd* pParent /*=NULL*/)
	: CDialog(CSoundObjectPanel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSoundObjectPanel)
	//}}AFX_DATA_INIT

	Create( IDD,pParent );
}


void CSoundObjectPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoundObjectPanel)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSoundObjectPanel, CDialog)
	//{{AFX_MSG_MAP(CSoundObjectPanel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSoundObjectPanel message handlers

BOOL CSoundObjectPanel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_innerRadius.Create( this,IDC_INNER_RADIUS );
	m_innerRadius.SetRange( 1,10000 );
	
	m_outerRadius.Create( this,IDC_OUTER_RADIUS );
	m_outerRadius.SetRange( 1,10000 );
	
	m_volume.Create( this,IDC_VOLUME );
	m_volume.SetRange( 0,100 );
	m_volume.SetInteger(true);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}