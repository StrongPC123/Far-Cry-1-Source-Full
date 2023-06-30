// XTListView.h interface for the CXTListView class.
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

#if !defined(__XTLISTVIEW_H__)
#define __XTLISTVIEW_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary:  CXTListView is a multiple inheritance class derived from CListView and
//			 CXTListCtrlBase.  This class implements flat header and generic sorting
//			 routines.  
// See Also: CXTListCtrlBase
class _XT_EXT_CLASS CXTListView : public CListView, public CXTListCtrlBase
{
    DECLARE_DYNCREATE(CXTListView)

public:
    
    // Summary: Constructs a CXTListView object.
    CXTListView();

    // Summary: Destroys a CXTListView object, handles cleanup and de-allocation.
    virtual ~CXTListView();

	// Input:	dwExStyle  - DWORD value that specifies the extended list-view control style. This
	//			parameter can be a combination of Extended List-View Styles.
    // Summary:	This member function is called to set extended styles for the list
	//			control, ie: LVS_EX_FULLROWSELECT, LVS_EX_GRIDLINES, etc. See MSDN
	//			documentation for a complete list of available styles.
    virtual void SetExtendedStyle(DWORD dwExStyle );

	// Returns: A DWORD value representing the extended style of the list control.
    // Summary:	This member function is called to return the list control extended
	//			style. 
    DWORD GetExtendedStyle();

protected:

public:
    // Ignore:
	//{{AFX_VIRTUAL(CXTListView)
    public:
    virtual void OnInitialUpdate();
    protected:
    virtual void OnDraw(CDC* pDC);      
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    //}}AFX_VIRTUAL

#ifdef _DEBUG
    
	virtual void AssertValid() const;
    
	virtual void Dump(
		CDumpContext& dc) const;

#endif

    // Ignore:
	//{{AFX_MSG(CXTListView)
    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnPaint();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE DWORD CXTListView::GetExtendedStyle() {
	return CXTListCtrlBase::GetExtendedStyle();
}
AFX_INLINE void CXTListView::SetExtendedStyle(DWORD dwExStyle){
	CXTListCtrlBase::SetExtendedStyle(dwExStyle);
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTLISTVIEW_H__)
