// XTCustomToolBar.h interface for the CXTCustomToolBar class.
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

#if !defined(__XT_CUSTOMTOOLBAR__)
#define __XT_CUSTOMTOOLBAR__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// Summary: CXTCustomToolbar is a toolbar that gets dynamically created from
//			within toolbar customization page
class _XT_EXT_CLASS CXTCustomToolBar : public CXTToolBar
{
	DECLARE_DYNCREATE(CXTCustomToolBar)

// Construction

public:

	// Summary: Constructs a CXTCustomToolBar object.
	CXTCustomToolBar();

// Attributes

private:

    CString     m_strTitle;       // Persistently stored title
    CImageList  m_images;         // images of the added buttons
    CImageList  m_disabledImages; // disabled images of the added buttons, created only when CreateDisabledImageList() gets called

// Operations

public:

	// Input:	strNewName - Reference to the new name for the toolbar.
	// Summary:	Persistently renames this toolbar
	void RenameCustomBar(const CString& strNewName);

	// Summary: Deletes this toolbar
	void DeleteCustomBar();

	// Returns:	True if successful, otherwise returns false.
	// Summary:	Creates and enables disabled image list
	bool CreateDisabledImageList();

// Overrides

	// Input:	pInfo - Pointer to a CXTCustomControlBarInfo object.
	// Summary:	Fills in supported customizable features
	virtual void GetCustomBarInfo(CXTCustomControlBarInfo* pInfo);

	// ClassWizard generated virtual function overrides
	// Ignore:
	//{{AFX_VIRTUAL(CXTCustomToolBar)
	//}}AFX_VIRTUAL

// Implementation

public:

	// Summary: Destroys a CXTCustomToolBar object, handles cleanup and de-allocation.
	virtual ~CXTCustomToolBar();
	virtual void PostNcDestroy();
	virtual void ReportCustomGroups(CXTCustomGroups& groups);

	// Generated message map functions

protected:

	// Ignore:
	//{{AFX_MSG(CXTCustomToolBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__XT_CUSTOMTOOLBAR__)