// XTListCtrl.h interface for the CXTListCtrl class.
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

#if !defined(__XTLISTCTRL_H__)
#define __XTLISTCTRL_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTListCtrlBase is a non-inherited class.  It implements the extended
//			list control services provided by the Xtreme Toolkit list control classes.
class _XT_EXT_CLASS CXTListCtrlBase
{

public:

    // Summary: Constructs a CXTListCtrlBase object.
    CXTListCtrlBase();

    // Summary: Destroys a CXTListCtrlBase object, handles cleanup and de-allocation.
    virtual ~CXTListCtrlBase();

	BOOL m_bAutoFont; // If TRUE, the font will automatically be set for the control.

protected:

    class CListCtrl_Friendly : public CListCtrl
    {
        // Needed in order to access protected functions.

		friend class CXTListCtrlBase; 
    };

	void ImplAttach(CListCtrl *pListCtrl);
	CListCtrl_Friendly *m_pListCtrl;

	int					m_nMinColWidth; // Minimum width for columns.
	int					m_nMaxColWidth; // Maximum width for columns.
	int					m_nSortedCol;	// Index of the currently selected column.
	bool				m_bNoColSizing; // If true, column sizing is disabled.
	bool				m_bAscending;	// Flag to determine the sorting order.
	bool				m_bAutoSave;	// true if saving the column widths.
	CString				m_strSection;	// Registry section name.
	CString				m_strEntry;		// Registry entry name.
	CString				m_strDefault;	// Registry default value.
	COLORREF			m_clrText;		// RGB value representing the row text color.
	COLORREF			m_clrTextBk;	// RGB value representing the row text background color.
	CXTFlatHeaderCtrl	m_flatHeader;	// Subclassed flat header control.


public:

	// Returns: A pointer to a CXTFlatHeaderCtrl if subclassed, otherwise returns NULL.
    // Summary:	Call this member function to get a pointer to the flat header control.
    CXTFlatHeaderCtrl* GetFlatHeaderCtrl() const;

	// Input:	nCol - Zero-based index of the column to set the sort image for.
    //			bAsc - TRUE if ascending, otherwise FALSE.
	// Returns: The zero-based index of the previously sorted column.
    // Summary:	This member function will set the sort image for the specified column.
    virtual int SetSortImage(int nCol,BOOL bAsc);

	// Input:	nCol - Zero-based index of the column.
	// Returns:	The column width stored in the registry.
    // Summary:	This member function will return the column width stored in the
	//			registry.
    virtual int GetStoredWidth(int nCol);

	// Input:	nCol - Zero-based index of the column.
    // Summary:	This member function will set the column width from the registry
	//			settings.
    virtual void SetStoredWidth(int nCol);

    // Summary: This member function loads the column widths from the registry.
    virtual void LoadColumnWidths();

    // Summary: This member function saves the column widths to the registry.
    virtual void SaveColumnWidths();

	// Input:	nCol - Zero-based index of the column.
    // Summary:	This member function will automatically size a given column. Pass
	//			in a -1 to auto-fit all columns.
    virtual void AutoSizeColumn(int nCol=-1);

	// Input:	lpszSection - Registry section name.  If NULL, the default is "Settings".
	//			lpszEntry - Registry entry name.  If NULL, the default is "Column Info".
	//			lpszDefault - Registry default value.  If NULL, the default ID.
    // Summary:	This member function will initialize the autosave feature.  If called,
	//			column widths will be saved to the system registry when the list control
	//			window is destroyed.
    virtual void AutoSaveColumns(LPCTSTR lpszSection=NULL,LPCTSTR lpszEntry=NULL,LPCTSTR lpszDefault=NULL);

	// Input:	bBoldFont - true to use a bold font in the header control.
    // Summary:	Call this member function to subclass the flat header control.
    virtual bool SubclassHeader(bool bBoldFont=false);

	// Input:	nCol - Passed in from the control.  Index of the column clicked.
    //			bAscending - Passed in from the control.  true if sort order should 
    //			be ascending.
    // Summary:	Override this member function in your derived class to perform custom
	//			sort routines.
    virtual bool SortList(int nCol, bool bAscending);

	// Input:	dwExStyle  - DWORD value that specifies the extended list-view control style. This
	//			parameter can be a combination of Extended List-View Styles.
    // Summary:	This member function is called to set extended styles for the list
	//			control, ie: LVS_EX_FULLROWSELECT, LVS_EX_GRIDLINES, etc. See MSDN
	//			documentation for a complete list of available styles.
    virtual void SetExtendedStyle(DWORD dwExStyle);

	// Returns:	A DWORD value that represents the extended style of the list control.
    // Summary:	This member function is called to return the list control extended
	// style.
    virtual DWORD GetExtendedStyle();

	// Input:	bDisable - true to disable column sizing.  false to enable.
    // Summary:	Call this member function to enable or disable column sizing.
    virtual void DisableColumnSizing(bool bDisable);

	// Input:	nSize - Size, in pixels, of the column's minimum size.
    // Summary:	Call this member function to set a minimum column size.
    void SetMinimumColSize(int nSize);

	// Returns: An integer value that represents the minimum column size for the list control.
	// Summary:	Call this member function to get the minimum column size for the list
	//			control. 
	int GetMinimumColSize();

	// Returns: An integer value that represents the maximum column size for the list control.
	// Summary:	Call this member function to get the maximum column size for the list control. 
	int GetMaximumColSize();

	// Input:	point - Point to be tested.
	//			col - Address of the column index
	// Returns: The row index, or -1 if 'point' is not over a row.
    // Summary:	Call this member function to determine the row index and column
	//			index for a 'point'. 
    virtual int HitTestEx(CPoint &point,int *col) const;

	// Input:	lpszColHeading - The column's heading.
	//			nWidth - Width of the column, in pixels.  If this 
    //			parameter is -1, the column width is the 
    //			same as the previous column.
	//			nFormat - The alignment of the column.  It can be LVCFMT_LEFT,
    //			LVCFMT_RIGHT, or LVCFMT_CENTER.
	// Returns: The index of the new column if successful, or -1 if unsuccessful.
    // Summary:	Call this member function to add a column after the right most column.
    virtual int AddColumn(LPCTSTR lpszColHeading,int nWidth = -1,int nFormat=LVCFMT_LEFT);

	// Input:	nCols - Number of columns the list control contains.
	//			nWidth - 'int' array containing the column widths.
	//			strColString  - CString array containing the string for each column.
	// Returns: true if successful, or false if an error occurred.
    // Summary:	This member function will build the columns for the list control.
    virtual bool BuildColumns(int nCols,int* nWidth,CString* strColString );

	// Input:	nCols - Number of columns the list control contains.
	//			nWidth - 'int' array containing the column widths.
	//			nColString  - 'int' array containing the string resource ID 
    //			for each column.
	// Returns: true if successful, or false if an error occurred.
    // Summary:	This member function will build the columns for the list control.
    virtual bool BuildColumns(int nCols,int* nWidth,int* nColString );

	// Input:	clrText - RGB value representing the row text color.
	//			clrTextBk - RGB value representing the row background color.
    // Summary:	This member function sets the row, text, and background colors for
	//			the list control.
    virtual void SetRowColors(COLORREF clrText,COLORREF clrTextBk);

	// Returns: An RGB value that represents the background color.
	// Summary:	This member function gets the RGB value defined for the list control's
	//			background color. 
	COLORREF GetBackColor();

	// Returns: An RGB value that represents the text color.
	// Summary:	This member function gets the RGB value defined for the list control's
	//			text color. 
	COLORREF GetTextColor();

	// Returns: An integer value that represents the number of columns in the list.
    // Summary:	This member function gets the number of columns in the list.
    virtual int GetColumnCount();

	// Input:	nFrom - Index of the source row.
    //			nTo - Index of the destination row.
	// Returns: The index of the source row specified by 'nFrom'.
    // Summary:	This member function copies a row to a new location. 
    virtual int CopyRow(int nFrom,int nTo);

	// Input:	nFrom - Index of the source row
    //			nTo - Index of the destination row
	// Returns: true if successful, or false if an error occurred.
    // Summary:	This member function moves a row to a new location. 
    virtual bool MoveRow(int nFrom,int nTo);

	// Returns: A CHeaderCtrl pointer to the associated header control. 
    // Summary:	This member function is a utility function to retrieve the associated
	//			header control.  
    CHeaderCtrl *_xtGetHeaderCtrl() const;

#if _MSC_VER < 1200 //Ignore: MFC 5.0
	POSITION GetFirstSelectedItemPosition() const;
	int GetNextSelectedItem(POSITION& pos) const;
#endif //Ignore: MFC 5.0

protected:

    // message implementation

    virtual BOOL OnNotifyImpl(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    virtual void PreSubclassWindowImpl_Post();
    afx_msg void OnCustomDrawImpl(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroyImpl_Pre();
    afx_msg BOOL OnEraseBkgndImpl(CDC* pDC);
    afx_msg void OnPaintImpl();
    afx_msg int OnCreateImpl_Post(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSettingChangeImpl_Post(UINT uFlags, LPCTSTR lpszSection);
};

//////////////////////////////////////////////////////////////////////
// Summary:  CXTListCtrl is a multiple inheritance class derived from CListCtrl and
//			 CXTListCtrlBase.  This class implements flat header and generic sorting
//			 routines.  See CXTListCtrlBase for additional functionality.
// See Also: CXTListCtrlBase
class _XT_EXT_CLASS CXTListCtrl : public CListCtrl, public CXTListCtrlBase
{
    DECLARE_DYNAMIC(CXTListCtrl)

public:
    
    // Summary: Constructs a CXTListCtrl object.
    CXTListCtrl();

    // Summary: Destroys a CXTListCtrl object, handles cleanup and de-allocation.
    virtual ~CXTListCtrl();

	// Input:	dwExStyle  - DWORD value that specifies the extended list-view control style. This
	//			parameter can be a combination of Extended List-View Styles.
    // Summary:	This member function is called to set extended styles for the list
	//			control, ie: LVS_EX_FULLROWSELECT, LVS_EX_GRIDLINES, etc. See MSDN
	//			documentation for a complete list of available styles.
    virtual void SetExtendedStyle(DWORD dwExStyle );

	// Returns: A DWORD data type that represents the extended style of the list control.
    // Summary:	This member function is called to retrieve the list control extended
	//			style. 
    DWORD GetExtendedStyle();

protected:

	bool m_bInitControl;     // true for initialization.
	
    // Ignore:
	//{{AFX_VIRTUAL(CXTListCtrl)
    protected:
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    virtual void PreSubclassWindow();
    //}}AFX_VIRTUAL

    // Ignore:
	//{{AFX_MSG(CXTListCtrl)
    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnPaint();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	
	afx_msg LRESULT OnInitControl(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE DWORD CXTListCtrl::GetExtendedStyle() {
	return CXTListCtrlBase::GetExtendedStyle();
}
AFX_INLINE void CXTListCtrl::SetExtendedStyle(DWORD dwExStyle){
	CXTListCtrlBase::SetExtendedStyle(dwExStyle);
}

//////////////////////////////////////////////////////////////////////

AFX_INLINE DWORD CXTListCtrlBase::GetExtendedStyle() {
    ASSERT(::IsWindow(m_pListCtrl->GetSafeHwnd())); return (DWORD)m_pListCtrl->SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
}
AFX_INLINE void CXTListCtrlBase::SetExtendedStyle(DWORD dwExStyle) {
    ASSERT(::IsWindow(m_pListCtrl->GetSafeHwnd())); m_pListCtrl->SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, GetExtendedStyle()|dwExStyle);
}
AFX_INLINE int CXTListCtrlBase::SetSortImage( int nCol, BOOL bAsc ) {
    ASSERT(::IsWindow(m_pListCtrl->GetSafeHwnd())); return GetFlatHeaderCtrl()->SetSortImage(nCol, bAsc);
} 
AFX_INLINE void CXTListCtrlBase::SetRowColors(COLORREF clrText, COLORREF clrTextBk) {
    m_clrText = clrText; m_clrTextBk = clrTextBk; m_pListCtrl->Invalidate();
}
AFX_INLINE void CXTListCtrlBase::SetMinimumColSize(int nSize) {
    m_nMinColWidth = nSize; GetFlatHeaderCtrl()->SetMinSize(nSize);
}
AFX_INLINE int CXTListCtrlBase::GetMinimumColSize() {
	return m_nMinColWidth;
}
AFX_INLINE int CXTListCtrlBase::GetMaximumColSize() {
	return m_nMaxColWidth;
}
AFX_INLINE void CXTListCtrlBase::DisableColumnSizing(bool bDisable) {
    m_bNoColSizing = bDisable;
}
AFX_INLINE CXTFlatHeaderCtrl* CXTListCtrlBase::GetFlatHeaderCtrl() const
{
	CHeaderCtrl* pHeaderCtrl = _xtGetHeaderCtrl();
	if (pHeaderCtrl && pHeaderCtrl->IsKindOf(RUNTIME_CLASS(CXTFlatHeaderCtrl))) {
		return (CXTFlatHeaderCtrl*)pHeaderCtrl;
	}
	return NULL;
}
AFX_INLINE CHeaderCtrl* CXTListCtrlBase::_xtGetHeaderCtrl() const 
{
	CHeaderCtrl * const pHeaderCtrl = 
        (CHeaderCtrl*)m_pListCtrl->GetDlgItem(0);

    // If the following assertion fails, you are performing an operation that
	// requires columns to be added, before you added columns.
    // For example, if you called SubclassHeader(), you need to make sure
    // that the call to SubClassHeader() is placed after calls to 
    // InsertColumn().
    
	ASSERT(pHeaderCtrl != NULL);
	return pHeaderCtrl;
}
AFX_INLINE COLORREF CXTListCtrlBase::GetBackColor() {
	return m_clrTextBk;
}
AFX_INLINE COLORREF CXTListCtrlBase::GetTextColor() {
	return m_clrText;
}
//////////////////////////////////////////////////////////////////////

#if _MSC_VER < 1200 // MFC 5.0
AFX_INLINE POSITION CXTListCtrlBase::GetFirstSelectedItemPosition() const {
	ASSERT(::IsWindow(m_pListCtrl->GetSafeHwnd())); return (POSITION) (1+m_pListCtrl->GetNextItem(-1, LVIS_SELECTED));
}
AFX_INLINE int CXTListCtrlBase::GetNextSelectedItem(POSITION& pos) const {
	ASSERT(::IsWindow(m_pListCtrl->GetSafeHwnd())); int nOldPos = (int)pos-1; pos = (POSITION) (1+m_pListCtrl->GetNextItem(nOldPos, LVIS_SELECTED)); return nOldPos;
}
#endif // MFC 5.0

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTLISTCTRL_H__)