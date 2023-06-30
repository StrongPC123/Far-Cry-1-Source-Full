// XTNewToolbarDlg.h interface for the CXTNewToolbarDlg class.
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

#if !defined(__XTNEWTOOLBARDLG_H__)
#define __XTNEWTOOLBARDLG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTNewToolbarDlg is a multiple inheritance class derived from CXTDialogState
//			and CDialog. CXTNewToolbarDlg is used to create the customize dialog
//			that is used during toolbar customization.
class _XT_EXT_CLASS CXTNewToolbarDlg : CXTDialogState, public CDialog
{

public:

	// Input:	pWndParent - Points to the top-level frame window.
    // Summary:	Constructs a CXTNewToolbarDlg object.
    CXTNewToolbarDlg(CFrameWnd* pWndParent=NULL);

    //{{AFX_DATA(CXTNewToolbarDlg)

    enum { IDD = XT_IDD_NEWTOOLBAR };
    CXTEdit   m_editToolbar;
    CString m_strToolbar;
    //}}AFX_DATA

	int			m_nNewID;			// ID for a newly created toolbar.
	CString		m_strExistingName;	// A NULL terminated string that represents the toolbar name.
	CFrameWnd*	m_pFrameWnd;		// Pointer to the top-level frame window.

    // Summary: This member function is called by the dialog to determine the next
	//			available suggested name to be displayed in the new toolbar dialog.
    void SetSuggestedName();

    // Ignore:
	//{{AFX_VIRTUAL(CXTNewToolbarDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Ignore:
	//{{AFX_MSG(CXTNewToolbarDlg)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTNEWTOOLBARDLG_H__)