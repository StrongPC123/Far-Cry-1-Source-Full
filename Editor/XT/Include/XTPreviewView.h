// XTPreviewView.h interface for the CXTPreviewView class.
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

#if !defined(__XTPREVIEWVIEW_H__)
#define __XTPREVIEWVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTPreviewView is a CPreviewView derived class.  CXTPreviewView is used
//			to create an office style print preview window.  To use the CXTPreviewView
//			class add the following code to your CView derived class:
// 
//			<pre>void CPrintPreviewView::OnFilePrintPreview() 
//			{
//			    // show print preview.
//			    _xtAfxShowPrintPreview( this );
//			}</pre>
class _XT_EXT_CLASS CXTPreviewView : public CPreviewView
{
	DECLARE_DYNCREATE(CXTPreviewView)

protected:

    // Summary: Constructs a CXTPreviewView object.
	CXTPreviewView();
	
    // Summary: Destroys a CXTPreviewView object, handles cleanup and de-allocation.
	virtual ~CXTPreviewView();

public:

	// Ignore:
	//{{AFX_VIRTUAL(CXTPreviewView)
	//}}AFX_VIRTUAL

	// Summary: This member function is called by the print preview view
	//			to update the toolbar button icon to represent the current
	//			view display state, either 1 or 2 pages.
	virtual void UpdateNumPageIcon();

protected:
	
	int			m_i1PageIndex; // Icon index of the single page toolbar button
	int			m_i2PageIndex; // Icon index of the dual page toolbar button
	CXTToolBar	m_wndToolBar;  // Toolbar used by print preview.

	// Ignore:
	//{{AFX_MSG(CXTPreviewView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnUpdateNumPageChange(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTPREVIEWVIEW_H__)