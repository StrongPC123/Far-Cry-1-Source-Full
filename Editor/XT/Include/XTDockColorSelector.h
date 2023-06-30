// XTDockColorSelector.h : header file
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

#if !defined(__XTDOCKCOLORSELECTOR_H__)
#define __XTDOCKCOLORSELECTOR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTDockColorSelector is a CXTDockWindow derived class.  It is used to create
//			sizing / docking windows with color selector as a child.
class _XT_EXT_CLASS CXTDockColorSelector : public CXTDockWindow  
{
	DECLARE_DYNAMIC(CXTDockColorSelector)
		
public:
    // Summary: Constructs a CXTColorPopup object.
	CXTDockColorSelector();

    // Summary: Destroys a CXTColorPopup object, handles cleanup and de-allocation.
	virtual ~CXTDockColorSelector();

	// Input:	pParentWnd - Pointer to the window that is the control bar’s parent.
	//			nID - The control bar’s window ID.
	//			lpszCaption - Points to a null-terminated character string that 
    //			represents the control bar name. Used as text for 
    //			the caption.
	// Returns: Nonzero if successful, otherwise returns zero.
    // Summary:	This member function creates a CXTDockColorSelector.
    BOOL Create(CWnd* pParentWnd,UINT nID,LPCTSTR lpszCaption=NULL); 

    // Summary: Call this member function to get Color Selector control.
	CXTColorSelectorCtrl* GetColorSelector();

protected:

	// Ignore:
	//{{AFX_MSG(CDockWinEx1)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonClose();
	afx_msg void OnUpdateButtonClose(CCmdUI* pCmdUI);
	afx_msg LRESULT OnSelEndOK(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnQueryVisualize(WPARAM, LPARAM);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
		
private:
	
	CXTColorSelectorCtrl m_wndColors;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE 	CXTColorSelectorCtrl*  CXTDockColorSelector::GetColorSelector() {
	return &m_wndColors;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //__XTDOCKCOLORSELECTOR_H__
