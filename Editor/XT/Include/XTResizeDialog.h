// XTResizeDialog.h: interface for the CXTResizeDialog class.
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//--------------------------------------------------------------------
// Based on the resizable classes created by Torben B. Haagh. Used by permission.
// http://www.codeguru.com/dialog/torbenResizeDialog.shtml
//--------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////

#if !defined(__XTRESIZEDIALOG_H__)
#define __XTRESIZEDIALOG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTResizeDialog is a multiple inheritance class derived from CDialog
//			and CXTResize.  CXTResizeDialog is used to create a resizable CDialog
//			type object that allows its dialog items to be resized or moved dynamically.
class _XT_EXT_CLASS CXTResizeDialog : public CDialog, public CXTResize
{
	DECLARE_DYNCREATE(CXTResizeDialog);

public:

    // Summary: Constructs a CXTResizeDialog object.
	CXTResizeDialog();

	// Input:	nID - Contains the ID number of a dialog box template resource.
	//			pParent - Points to the parent or owner window object, of type CWnd, to which
	//			the dialog object belongs.  If it is NULL, the dialog object’s parent
	//			window is set to the main application window.
	//			nFlags - Flags that are to be passed to CXTResize that specify the attributes
	//			of the resizing property page.  They can be one or more of the following,
	//			and can be combined using the or (|) operator:
	//			[ul]
    //			[li]<b>SZ_NOSIZEICON</b> Do not add size icon.[/li]
    //			[li]<b>SZ_NOHORISONTAL</b> No horizontal resizing.[/li]
    //			[li]<b>SZ_NOVERTICAL</b> No vertical resizing.[/li]
    //			[li]<b>SZ_NOMINSIZE</b> Do not require a minimum size.[/li]
    //			[li]<b>SZ_NOCLIPCHILDREN</b> Do not set clip children style.[/li]
    //			[li]<b>SZ_NOTRANSPARENTGROUP</b> Do not set transparent style
	//			for group boxes.[/li]
	//			[/ul]
    // Summary:	Constructs a CXTResizeDialog object.
	CXTResizeDialog(const UINT nID,CWnd* pParent = 0,const UINT nFlags = 0);

protected:

	UINT m_nDialogID; // Contains the ID number of a dialog box template resource.

	// Ignore:
	//{{AFX_MSG(CXTResizeDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO *lpMMI);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

#endif // !defined(__XTRESIZEDIALOG_H__)
