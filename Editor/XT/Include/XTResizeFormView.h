// XTResizeFormView.h: interface for the CXTResizeFormView class.
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

#if !defined(__XTRESIZEFORMVIEW_H__)
#define __XTRESIZEFORMVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTResizeFormView is a multiple inheritance class derived from CFormView
//			and CXTResize. CXTResizeFormView is used to create a resizable CFormView
//			type object that allows its form items to be resized or moved dynamically.
class _XT_EXT_CLASS CXTResizeFormView : public CFormView, public CXTResize
{
	DECLARE_DYNCREATE(CXTResizeFormView)

public:

	// Input:	nID - Contains the ID number of a dialog template resource.
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
	//			       for group boxes.[/li]
	//			[/ul]
    // Summary:	Constructs a CXTResizeFormView object.
	CXTResizeFormView(const UINT nID = 0,const UINT nFlags = 0);

protected:

	// Ignore:
	//{{AFX_VIRTUAL(CXTResizeFormView)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

	// Ignore:
	//{{AFX_MSG(CXTResizeFormView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO *lpMMI);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

#endif // !defined(__XTRESIZEFORMVIEW_H__)