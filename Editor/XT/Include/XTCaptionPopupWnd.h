// XTCaptionPopupWnd.h : interface for the CXTCaptionPopupWnd class.
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

#if !defined(__XTCAPTIONPOPUPWND_H__)
#define __XTCAPTIONPOPUPWND_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTCaptionPopupWnd is a CWnd derived class.  It is used by the CXTCaption
//			class to display a popup child window similar to Outlook&trade;.
class _XT_EXT_CLASS CXTCaptionPopupWnd : public CWnd
{
	DECLARE_DYNAMIC(CXTCaptionPopupWnd)

public:

    // Summary: Constructs a CXTCaptionPopupWnd object.
	CXTCaptionPopupWnd();

	// Summary: Destroys a CXTCaptionPopupWnd object, handles cleanup and de-allocation.
    virtual ~CXTCaptionPopupWnd();

protected:
	
    CWnd*         m_pParentWnd;    // A CWnd pointer that represents the parent window.
    CWnd*         m_pChildWnd;     // A CWnd pointer that represents the child displayed in the popup.
    CWnd*         m_pChildParent;  // A CWnd pointer that represents the parent of the child window.
    CXTButton     m_CaptionButton; // Close button associated with the caption.
    CXTCaption    m_Caption;       // Caption that is displayed when the window is active.
    CXTIconHandle m_hIconOn;       // Handle to a normal button icon.
    CXTIconHandle m_hIconOff;      // Handle to a pressed button icon.

public:
	
    // Input:	rect - A CRect reference that represents the size of the popup window.
	//			pParentWnd - A CWnd pointer that represents the popup window.
	//			pChildWnd - A CWnd pointer that represents the child to be displayed when
	//			the window is activated.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary: This member function is called to create and display the popup
    //			window.  
	virtual BOOL Create(const CRect& rect, CWnd* pParentWnd, CWnd* pChildWnd);

	// Ignore:
	//{{AFX_VIRTUAL(CXTCaptionPopupWnd)
	//}}AFX_VIRTUAL

protected:

    // Ignore:
	//{{AFX_MSG(CXTCaptionPopupWnd)
	afx_msg void OnDestroy();
	afx_msg void OnCaptButton();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTCAPTIONPOPUPWND_H__)