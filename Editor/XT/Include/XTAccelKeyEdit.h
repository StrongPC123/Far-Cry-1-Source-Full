// XTAccelKeyEdit.h : interface for the CXTAccelKeyEdit class.
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//
//////////////////////////////////////////////////////////////////////

#if !defined(__XTACCELKEYEDIT_H__)
#define __XTACCELKEYEDIT_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTAccelKeyEdit is derived from CEdit.  It is used by the "configure
//			shortcuts page" to assign accelerator keyboard shortcuts.  This class
//			will manage the customization of keyboard shortcuts and assignment of
//			commands to those shortcuts.
class CXTAccelKeyEdit : public CEdit
{
public:

    // Summary: Constructs a CXTAccelKeyEdit object.
    CXTAccelKeyEdit();

	// Summary: Destroys a CXTAccelKeyEdit object, handles cleanup and de-allocation.
    virtual ~CXTAccelKeyEdit();

private:

	BOOL m_bShift;
	BOOL m_bCtrl;
	BOOL m_bAlt;
	WORD m_nVirtualKeyCode;
	BOOL m_bKeyDefined;

public:

	// Summary: This member function is called to reset the edit control.
	void Reset();

	// Input:	nVirtualKeyCode - Reference to a WORD object that will receive the
	//			virtual key code that represents the virtual keys the user has selected.
	//			bCtrl - Reference to a BOOL object that will be TRUE if the VK_CONTROL key
	//			was pressed, otherwise set to FALSE.
	//			bAlt - Reference to a BOOL object that will be TRUE if the VK_MENU key
	//			was pressed, otherwise set to FALSE.
	//			bShift - Reference to a BOOL object that will be TRUE if the VK_SHIFT key
	//			was pressed, otherwise set to FALSE.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary: This member function is called to determine which virtual keys
	//			have been selected.
	BOOL GetKeyInfo(WORD& nVirtualKeyCode, BOOL& bCtrl, BOOL& bAlt, BOOL& bShift);

	// Returns: TRUE if a virtual key was defined other than VK_SHIFT, VK_CONTROL or VK_MENU,
	//			otherwise returns FALSE.
	// Summary: This member function checks to see if a virtual key, other than VK_SHIFT,
	//			VK_CONTROL or VK_MENU, was defined. 
	BOOL IsKeyDefined();

protected:

	// Summary: This member function is called to update the text that is displayed
	//			in the edit field.
	void UpdateKeyString();

	// Ignore:
	//{{AFX_VIRTUAL(CXTAccelKeyEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:

	// Ignore:
	//{{AFX_MSG(CXTAccelKeyEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


//////////////////////////////////////////////////////////////////////

AFX_INLINE BOOL CXTAccelKeyEdit::IsKeyDefined() {
	return m_bKeyDefined;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__XTACCELKEYEDIT_H__)
