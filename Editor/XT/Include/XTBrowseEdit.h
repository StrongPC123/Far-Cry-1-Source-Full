// XTBrowseEdit.h : interface for the CXTBrowseEdit class.
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

#if !defined(__XTBROWSEEDIT_H__)
#define __XTBROWSEEDIT_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CXTBrowseBtn;

//////////////////////////////////////////////////////////////////////
// Summary: CXTBrowseEdit is a CXTEdit derived class.  This class also has a push
//			button associated with it.  The control can be used to search for directories
//			and files, or activate a popup menu.
class _XT_EXT_CLASS CXTBrowseEdit : public CXTEdit
{
	DECLARE_DYNAMIC(CXTBrowseEdit)

public:

    // Summary: Constructs a CXTBrowseEdit object.
    CXTBrowseEdit();

	// Summary: Destroys a CXTBrowseEdit object, handles cleanup and de-allocation.
    virtual ~CXTBrowseEdit();

protected:

    int           m_nGap;             // Distance between the button and edit control.
    bool          m_bBrowsing;        // true if in browse operation.
    BOOL          m_bOpenFileDialog;  // TRUE for Open File dialog, FALSE for Save as.  See SetDlgOpenFile(...).
    UINT          m_nMenu;            // Popup menu ID.
    UINT          m_nSubMenuIndx;     // Index of a popup submenu.
    CWnd*         m_pParentWnd;       // A CWnd pointer that represents the parent of the edit control.
    DWORD         m_dwFlags;          // File dialog styles.
    DWORD         m_dwBStyle;         // Search type.
	DWORD		  m_dwInitSignature;  // Used for one-time initialization.
    CString       m_strDefExt;        // Default file extension.
    CString       m_strFileName;      // Default file name.
    CString       m_strFilter;        // Default file filter.
    CString       m_strTitle;         // Directory dialog title.
    CFileDialog*  m_pFileDialog;      // Points to a valid CFileDialog object.
    CXTBrowseBtn* m_pBrowseBtn;       // Pointer to a push button.
	
public:
	
	// Returns: true if the control is displaying a File Open dialog or popup menu.
	// Summary: Call this member function to determine if the browse edit control is in
	//			the middle of a browse operation.  
	bool IsBrowsing();

	// Input:	nGap - Gap, in pixels, between the browse button and edit window.
    // Summary: Call this member function to set the gap between the edit window and the
    //			browse button.
	void SetGap(int nGap);

	// Input:	pFileDialog - Points to the CFileDialog object that will replace the 
	//			standard File Open dialog.
	// Summary: This member function will set a CFileDialog derived class object to be 
    //			the file open dialog.
    virtual void SetFileDialog(CFileDialog* pFileDialog);

	// Input:	bOpenFileDialog - Set to TRUE to construct a File Open dialog box, or 
	//			FALSE to construct a File Save as dialog box.
	// Summary: This member function will set the File Open dialog style.
	virtual void SetDlgOpenFile(BOOL bOpenFileDialog=TRUE);

	// Input:	strDefExt - Points to a NULL terminated string that represents the 
	//			default file extension to be used with the File Open dialog.
	// Summary: This member function sets the default extension for the File Open dialog.
	virtual void SetDlgDefExt(LPCTSTR strDefExt=NULL);

	// Input:	strFileName - Points to a NULL terminated string that represents the 
	//			default file name to be used with the File Open dialog.
	// Summary: This member function sets the default file name for the File Open dialog.
	virtual void SetDlgFileName(LPCTSTR strFileName=NULL);

	// Input:	dwFlags - The desired OFN_ styles for the File Open dialog.
	// Summary: This member function sets the style flags for the File Open dialog.
	virtual void SetDlgStyle(DWORD dwFlags=OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT);

	// Input:	strFilter - Points to a NULL terminated string that represents the file 
	//			filter used by the File Open dialog.
	// Summary: This member function sets the file filter for the File Open dialog.
    virtual void SetDlgFilter(LPCTSTR strFilter=NULL);

	// Input:	pParentWnd - Points to a CWnd object that represents the owner window 
	//			for this control.
    // Summary: This member function sets the owner for the File Open dialog.
	virtual void SetDlgOwner(CWnd* pParentWnd=NULL);

	// Input:	strTitle - Points to a NULL terminated string the represents the title 
	//			of the "browse for directory" dialog.
	// Summary: This member function sets the title for the directory dialog.
	virtual void SetDlgTitle(LPCTSTR strTitle=NULL);

	// BULLETED LIST:

	// Input:	dwBStyle - A DWORD value that represents the type of search to perform. It can
	//			be any one of the following styles:
	//			[ul]
	//			[li]<b>BES_XT_CHOOSEDIR</b> Display the choose folder dialog.[/li]
	//			[li]<b>BES_XT_CHOOSEFILE</b> Display the choose file dialog.[/li]
	//			[li]<b>BES_XT_POPUPMENU</b> Display a user defined context menu.[/li]
	//			[/ul]
	//			nMenu - If 'dwBStyle' contains the BES_XT_POPUPMENU flag, then 'nMenu' represents
	//			the resource ID of a popup menu. Otherwise this value is ignored.
	//			nSubMenuIndx - Index of submenu to display.
	// Summary: This member function sets the current search type for the control.
	virtual void SetBrowseStyle(DWORD dwBStyle,UINT nMenu=0,int nSubMenuIndx=0);

	// Summary: This member function is called whenever the browse button is pressed,
	//			and can be overridden to perform custom browse functions.
    virtual void OnBrowse();

	// Ignore:
	//{{AFX_VIRTUAL(CXTBrowseEdit)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

    virtual void ChooseDirectory();
    virtual void ChooseFile();
    virtual void PopupMenu();
    virtual void PositionBrowseButton(bool bSizeEdit=false);
    
	// Summary: Defers control initialization
	void DeferInitialUpdate();

    // Ignore:
	//{{AFX_MSG(CXTBrowseEdit)
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG

	afx_msg LRESULT OnInitControl(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTBrowseEdit::SetGap(int nGap) {
    ASSERT(nGap >= 0); m_nGap = nGap;
}
AFX_INLINE void CXTBrowseEdit::SetFileDialog(CFileDialog *pFileDialog/*=NULL*/) {
	m_pFileDialog = pFileDialog;
}
AFX_INLINE void CXTBrowseEdit::SetDlgOpenFile(BOOL bOpenFileDialog/*=TRUE*/) {
	m_bOpenFileDialog = bOpenFileDialog;
}
AFX_INLINE void CXTBrowseEdit::SetDlgDefExt(LPCTSTR strDefExt/*=NULL*/) {
	m_strDefExt = strDefExt;
}
AFX_INLINE void CXTBrowseEdit::SetDlgFileName(LPCTSTR strFileName/*=NULL*/) {
	m_strFileName = strFileName;
}
AFX_INLINE void CXTBrowseEdit::SetDlgStyle(DWORD dwFlags/*=OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT*/) {
	m_dwFlags = dwFlags;
}
AFX_INLINE void CXTBrowseEdit::SetDlgFilter(LPCTSTR strFilter/*=NULL*/) {
	m_strFilter = strFilter;
}
AFX_INLINE void CXTBrowseEdit::SetDlgOwner(CWnd* pParentWnd/*=NULL*/) {
	m_pParentWnd = pParentWnd;
}
AFX_INLINE void CXTBrowseEdit::SetDlgTitle(LPCTSTR strTitle/*=NULL*/) {
	m_strTitle = strTitle;
}
AFX_INLINE void CXTBrowseEdit::SetBrowseStyle(DWORD dwBStyle, UINT nMenu, int nSubMenuIndx) {
	m_dwBStyle = dwBStyle; m_nMenu = nMenu; m_nSubMenuIndx = nSubMenuIndx;
}
AFX_INLINE bool CXTBrowseEdit::IsBrowsing() {
	return m_bBrowsing;
}

//////////////////////////////////////////////////////////////////////
// Summary: CXTItemEdit is a CXTBrowseEdit derived class.  It is used to create
//			a CXTItemEdit window that can be used as an "in-place" edit field that
//			can be dynamically created for controls such as a list box.
class _XT_EXT_CLASS CXTItemEdit : public CXTBrowseEdit
{
	DECLARE_DYNAMIC(CXTItemEdit)
	
	// Used internally to determine if a WM_CLOSE message has been sent.
	
	bool m_bClosePosted;

public:

	// Input:	pParent - Points to the parent window.
	//			rect - Size of the edit item.
	//			strWindowText - Text to be initially displayed in the edit field.
	//			dwBStyle - Specifies the browse edit style for the in-place edit field.
	//			See CXTBrowseEdit::SetBrowseStyle for available styles.
	//			bAutoDelete - Set to true if the object is to be self deleting.
	// Summary: Constructs a CXTItemEdit object that can be used as an "in-place" edit
	//			field, and can be dynamically created for controls such as a list box.
	CXTItemEdit(CWnd* pParent, const CRect& rect, CString& strWindowText,
        DWORD dwBStyle=BES_XT_CHOOSEDIR, bool bAutoDelete=true);

	// Summary: Destroys a CXTItemEdit object, handles cleanup and de-allocation.
    virtual ~CXTItemEdit();

public:

    bool     m_bModified;     // true if the item was modified.
    bool     m_bAutoDelete;   // true if self deleting.
    bool     m_bEscapeKey;    // true if the edit window was closed with the escape key.
    CString& m_strWindowText; // The edit controls text.

public:

	// Summary: This member function is called whenever the control loses focus. 
	//			This will destroy the window, and notify the parent via WM_COMMAND
	//			that the editing has been completed. The two possible commands are:
	//			ON_BEN_XT_LABELEDITEND and ON_BEN_XT_LABELEDITCANCEL;
	virtual void EndLabelEdit();

    // Ignore:
	//{{AFX_VIRTUAL(CXTItemEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

protected:

    // Ignore:
	//{{AFX_MSG(CXTItemEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTBROWSEEDIT_H__)
