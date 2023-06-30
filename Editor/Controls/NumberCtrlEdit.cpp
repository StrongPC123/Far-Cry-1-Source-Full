////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   numberctrledit.cpp
//  Version:     v1.00
//  Created:     26/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NumberCtrlEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNumberCtrlEdit

void CNumberCtrlEdit::SetText(const CString& strText)
{
	m_strInitText = strText;

	SetWindowText(strText);
}

BOOL CNumberCtrlEdit::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		switch(pMsg->wParam)
		{
			//case VK_ESCAPE:
			case VK_RETURN:
			//case VK_TAB:
				//::PeekMessage(pMsg, NULL, NULL, NULL, PM_REMOVE);
				// Call update callback.
				if (m_onUpdate)
					m_onUpdate();
				//return TRUE;
			default:
				;
		}
	}
	
	return CEdit::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CNumberCtrlEdit, CEdit)
	//{{AFX_MSG_MAP(CNumberCtrlEdit)
	ON_WM_CREATE()
	ON_WM_KILLFOCUS()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumberCtrlEdit message handlers

int CNumberCtrlEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CEdit::OnCreate(lpCreateStruct) == -1) 
		return -1;

	CFont* pFont = GetParent()->GetFont();
	SetFont(pFont);

	SetWindowText(m_strInitText);

	return 0;
}

void CNumberCtrlEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);
	// Call update callback.
	if (m_onUpdate)
		m_onUpdate();
}

BOOL CNumberCtrlEdit::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CNumberCtrlEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);

	SetSel(0,-1);
}

//////////////////////////////////////////////////////////////////////////
void CNumberCtrlEdit::OnChar( UINT nChar,UINT nRepCnt,UINT nFlags )
{
	if ((nChar >= '0' && nChar <= '9') || nChar == '-' || nChar == '.' || nChar == VK_BACK)
		CEdit::OnChar(nChar,nRepCnt,nFlags);
}
