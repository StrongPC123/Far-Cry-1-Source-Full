////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   customcomboboxex.h
//  Version:     v1.00
//  Created:     4/9/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __customcomboboxex_h__
#define __customcomboboxex_h__
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CCustomComboBoxEx template control

template<class BASE_TYPE>
class CCustomComboBoxEx : public BASE_TYPE
{
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg UINT OnGetDlgCode();

	DECLARE_TEMPLATE_MESSAGE_MAP()
};

BEGIN_TEMPLATE_MESSAGE_MAP(class BASE_TYPE, CCustomComboBoxEx<BASE_TYPE>, BASE_TYPE)
	//{{AFX_MSG_MAP(CColorPushButton)
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_TEMPLATE_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
template<class BASE_TYPE>
UINT CCustomComboBoxEx<BASE_TYPE>::OnGetDlgCode() 
{
	return DLGC_WANTMESSAGE;
}

//////////////////////////////////////////////////////////////////////////
template<class BASE_TYPE>
BOOL CCustomComboBoxEx<BASE_TYPE>::PreTranslateMessage(MSG* pMsg) 
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

	return BASE_TYPE::PreTranslateMessage(pMsg);
}


#endif // __customcomboboxex_h__
