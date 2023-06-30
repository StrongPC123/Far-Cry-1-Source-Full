// SelectionCombo.cpp : implementation file
//

#include "stdafx.h"
#include "SelectionCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectionCombo

CSelectionCombo::CSelectionCombo()
{
}

CSelectionCombo::~CSelectionCombo()
{
}


BEGIN_MESSAGE_MAP(CSelectionCombo, CXTFlatComboBox)
	//{{AFX_MSG_MAP(CSelectionCombo)
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectionCombo message handlers

BOOL CSelectionCombo::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	return CXTFlatComboBox::Create( dwStyle,rect,pParentWnd,nID );
}

UINT CSelectionCombo::OnGetDlgCode() 
{
	return DLGC_WANTMESSAGE;
}

BOOL CSelectionCombo::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			NMCBEENDEDIT endEdit;
			endEdit.hdr.code = CBEN_ENDEDIT;
			endEdit.hdr.hwndFrom = m_hWnd;
			endEdit.hdr.idFrom = GetDlgCtrlID();
			endEdit.fChanged = true;
			endEdit.iNewSelection = CB_ERR;
			endEdit.iWhy = CBENF_RETURN;

			CString text;
			GetWindowText( text );
			strcpy( endEdit.szText,text );

			GetParent()->SendMessage( WM_NOTIFY,(WPARAM)GetDlgCtrlID(),(LPARAM)(&endEdit) );
			return TRUE;
		}
		if (pMsg->wParam == VK_ESCAPE)
		{
			SetWindowText( "" );
			return TRUE;
		}
	}

	if (pMsg->message == WM_KILLFOCUS)
	{
		NMCBEENDEDIT endEdit;
		endEdit.hdr.code = CBEN_ENDEDIT;
		endEdit.hdr.hwndFrom = m_hWnd;
		endEdit.hdr.idFrom = GetDlgCtrlID();
		endEdit.fChanged = true;
		endEdit.iNewSelection = CB_ERR;
		endEdit.iWhy = CBENF_KILLFOCUS;

		CString text;
		GetWindowText( text );
		strcpy( endEdit.szText,text );

		GetParent()->SendMessage( WM_NOTIFY,(WPARAM)GetDlgCtrlID(),(LPARAM)(&endEdit) );
		return TRUE;
	}
	
	return CXTFlatComboBox::PreTranslateMessage(pMsg);
}
