// ToolButton.cpp : implementation file
//

#include "stdafx.h"
#include "PickObjectButton.h"

/////////////////////////////////////////////////////////////////////////////
// CPickObjectButton
IMPLEMENT_DYNAMIC(CPickObjectButton,CColorCheckBox)

//////////////////////////////////////////////////////////////////////////
CPickObjectButton::CPickObjectButton()
{
	m_targetClass = 0;
	m_bMultipick = false;
}

CPickObjectButton::~CPickObjectButton()
{
}

BEGIN_MESSAGE_MAP(CPickObjectButton, CButton)
	//{{AFX_MSG_MAP(CPickObjectButton)
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPickObjectButton::OnClicked()
{
	if (GetCheck() == 1)
	{
		SetCheck(0);
		GetIEditor()->CancelPick();
		return;
	}

	SetCheck(1);
	GetIEditor()->PickObject( this,m_targetClass,m_statusText,m_bMultipick );
}

//////////////////////////////////////////////////////////////////////////
void CPickObjectButton::OnPick( CBaseObject *picked )
{
	if (!m_bMultipick)
		SetCheck(0);
	if (m_pickCallback)
		m_pickCallback->OnPick( picked );
}

//////////////////////////////////////////////////////////////////////////
void CPickObjectButton::OnCancelPick()
{
	SetCheck(0);
	if (m_pickCallback)
		m_pickCallback->OnCancelPick();
}

//////////////////////////////////////////////////////////////////////////
void CPickObjectButton::SetPickCallback( IPickObjectCallback *callback,const CString &statusText,CRuntimeClass *targetClass,bool bMultiPick )
{
	m_statusText = statusText;
	m_pickCallback = callback;
	m_targetClass = targetClass;
	m_bMultipick = bMultiPick;
}

//////////////////////////////////////////////////////////////////////////
bool CPickObjectButton::OnPickFilter( CBaseObject *filterObject )
{
	if (m_pickCallback)
		return m_pickCallback->OnPickFilter( filterObject );
	return true;
}