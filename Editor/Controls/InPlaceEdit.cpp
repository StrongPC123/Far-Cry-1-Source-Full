////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   inplaceedit.cpp
//  Version:     v1.00
//  Created:     5/6/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History: Based on Stefan Belopotocan code.
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InPlaceEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit
CInPlaceEdit::CInPlaceEdit( const CString& srtInitText,OnChange onchange )
{
	m_strInitText = srtInitText;
	m_onChange = onchange;
}

CInPlaceEdit::~CInPlaceEdit()
{
}

void CInPlaceEdit::SetText(const CString& strText)
{
	m_strInitText = strText;

	SetWindowText(strText);
	SetSel(0, -1);
}

BOOL CInPlaceEdit::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		switch(pMsg->wParam)
		{
			case VK_ESCAPE:
			case VK_RETURN:
				::PeekMessage(pMsg, NULL, NULL, NULL, PM_REMOVE);
			case VK_TAB:
				GetParent()->SetFocus();
				if (m_onChange)
					m_onChange();
				return TRUE;
			default:
				;
		}
	}
	
	return CEdit::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CInPlaceEdit, CEdit)
	//{{AFX_MSG_MAP(CInPlaceEdit)
	ON_WM_CREATE()
	ON_WM_KILLFOCUS()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit message handlers

int CInPlaceEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CEdit::OnCreate(lpCreateStruct) == -1) 
		return -1;

	CFont* pFont = GetParent()->GetFont();
	SetFont(pFont);

	SetWindowText(m_strInitText);

	return 0;
}

void CInPlaceEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);
}

BOOL CInPlaceEdit::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
