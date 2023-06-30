// VolumePanel.cpp : implementation file
//

#include "stdafx.h"
#include "VolumePanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVolumePanel dialog


CVolumePanel::CVolumePanel(CWnd* pParent /*=NULL*/)
	: CObjectPanel(pParent)
{
	//{{AFX_DATA_INIT(CVolumePanel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CVolumePanel::DoDataExchange(CDataExchange* pDX)
{
	CObjectPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVolumePanel)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVolumePanel, CObjectPanel)
	//{{AFX_MSG_MAP(CVolumePanel)
	ON_EN_UPDATE(IDC_LENGTH,OnUpdate)
	ON_EN_UPDATE(IDC_WIDTH,OnUpdate)
	ON_EN_UPDATE(IDC_HEIGHT,OnUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVolumePanel message handlers

BOOL CVolumePanel::OnInitDialog() 
{
	CObjectPanel::OnInitDialog();
	
	m_size[0].Create( this,IDC_LENGTH );
	m_size[1].Create( this,IDC_WIDTH );
	m_size[2].Create( this,IDC_HEIGHT );
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVolumePanel::SetSize( Vec3 &size )
{
	m_size[0].SetValue( size.x );
	m_size[1].SetValue( size.y );
	m_size[2].SetValue( size.z );
}
	
Vec3 CVolumePanel::GetSize()
{
	return Vec3(m_size[0].GetValue(),m_size[1].GetValue(),m_size[2].GetValue() );
}