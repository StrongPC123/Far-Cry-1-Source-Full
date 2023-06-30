// ColorCheckBox.cpp : implementation file
//

#include "stdafx.h"
#include "ColorCheckBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorCheckBox
IMPLEMENT_DYNCREATE( CColorCheckBox,CButton )

CColorCheckBox::CColorCheckBox()
{
	m_nChecked = 0;
}

CColorCheckBox::~CColorCheckBox()
{
}

//BEGIN_MESSAGE_MAP(CColorCheckBox, CColoredPushButton)
//	//{{AFX_MSG_MAP(CColorCheckBox)
//		// NOTE - the ClassWizard will add and remove mapping macros here.
//	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorCheckBox message handlers

void CColorCheckBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if (m_nChecked == 1)
	{
		lpDrawItemStruct->itemState |= ODS_SELECTED;
	}
	CColoredPushButton::DrawItem( lpDrawItemStruct );
	
}

//////////////////////////////////////////////////////////////////////////
void CColorCheckBox::SetCheck(int nCheck)
{
	if (m_nChecked != nCheck)
	{
		m_nChecked = nCheck;
		if(::IsWindow(m_hWnd))
			Invalidate();
	}
};

//////////////////////////////////////////////////////////////////////////
void CColorCheckBox::PreSubclassWindow() 
{
	CColoredPushButton::PreSubclassWindow();
	SetButtonStyle( BS_PUSHBUTTON|BS_OWNERDRAW );
}
