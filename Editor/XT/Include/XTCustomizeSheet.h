// XTCustomizeSheet.h interface for the CXTCustomizeSheet class.
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

#if !defined(__XTCUSTOMIZESHEET_H__)
#define __XTCUSTOMIZESHEET_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTCustomizeSheet is a multiple inheritance class derived from CXTDialogState
//			and CXTResizePropertySheet. It is a property sheet that is displayed during
//			toolbar customization.
class _XT_EXT_CLASS CXTCustomizeSheet : CXTDialogState, public CXTResizePropertySheet
{
	DECLARE_DYNAMIC(CXTCustomizeSheet)

	CMap<HWND, HWND, bool, bool> m_map; // A map to store active custom bars that get mouse input.

public:

	// BULLETED LIST:

	// Input:	pFrameWnd - Pointer to the frame window on which to execute customizations.
	//			dwCustStyle - Specifies which pages to include when the customize dialog is
    //			displayed.  It can be one or more of the following:
	//			[ul]
    //			[li]<b>CUST_XT_TOOLBARS</b> Displays the Toolbars tab in the toolbar customize property sheet.[/li]
    //			[li]<b>CUST_XT_COMMANDS</b> Displays the Commands tab in the toolbar customize property sheet.[/li]
    //			[li]<b>CUST_XT_KEYBOARD</b> Displays the Keyboard tab in the toolbar customize property sheet.[/li]
    //			[li]<b>CUST_XT_TOOLS</b> Displays the Tools tab in the toolbar customize property sheet.[/li]
    //			[li]<b>CUST_XT_OPTIONS</b> Displays the Options tab in the toolbar customize property sheet.[/li]
    //			[li]<b>CUST_XT_DEFAULT</b> Same as CUST_XT_TOOLBARS|CUST_XT_COMMANDS|CUST_XT_KEYBOARD|CUST_XT_OPTIONS.[/li]
    //			If no flags are defined the constructor will ASSERT.
	// Summary: Constructs a CXTCustomizeSheet object.
	CXTCustomizeSheet(CFrameWnd* pFrameWnd,DWORD dwCustStyle);

	// Summary: Destroys a CXTCustomizeSheet object, handles cleanup and de-allocation.
	virtual ~CXTCustomizeSheet();

	bool m_bAppActive;				// true if the application is currently active, or false otherwise.
	CRect m_rcPage;					// Size for each page displayed.
	CXTCustTools* m_pPage4;			// Fourth property page.
	CXTCustOptions* m_pPage5;		// Fifth property page.
	CXTCustToolBarPage* m_pPage1;   // First property page.
	CXTCustomizeContext m_context;  // Current context.
	CXTCustCommandsPage* m_pPage2;  // Second property page.
	CXTCustAccelerators* m_pPage3;  // Third property page.

	// Input:		bMode - true to enable toolbar customization.
	// Summary: This member function sets the customization mode on or off in all applicable
	//			control bars.
	void SetCustMode(bool bMode);

	// Ignore:
	//{{AFX_VIRTUAL(CXTCustomizeSheet)
	public:
	virtual BOOL OnInitDialog();
	virtual INT_PTR DoModal();
	//}}AFX_VIRTUAL

protected:

	// Ignore:
	//{{AFX_MSG(CXTCustomizeSheet)
	afx_msg void OnDestroy();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnCloseBtn();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()

private:
	
    CFrameWnd*         m_pWndParent;

	bool FilterMouse(LPARAM lParam);
	bool FilterKey(LPARAM lParam);
	void StoreInfo(CXTCustomControlBarInfo* pInfo);
	void OnBarDestroyed(CControlBar* pBar);

	friend class CXTCustomizeContext;
	friend class CXTCustToolBarPage;
	friend class CXTCustCommandsPage;
	friend class CXTCustAccelerators;
	friend class CXTCustTools;
	friend class CXTCustOptions;
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTCUSTOMIZESHEET_H__)