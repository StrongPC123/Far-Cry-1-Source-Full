// XTDialogBar.h interface for the CXTDialogBar class.
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

#if !defined(__XTDIALOGBAR_H__)
#define __XTDIALOGBAR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: The CXTDialogBar class provides the functionality of a Windows modeless
//			dialog box in a control bar. A dialog bar resembles a dialog box in
//			that it contains standard Windows controls that the user can tab between.
//			Another similarity is that you create a dialog template to represent
//			the dialog bar.
//
//			Creating and using a dialog bar is similar to creating and using a CFormView
//			object. First, use the dialog editor (described in the MSDN Visual C++
//			User's Guide) to define a dialog template with the style WS_CHILD and
//			no other style. The template must not have the style WS_VISIBLE. In
//			your application code, call the constructor to construct the CXTDialogBar
//			object, then call Create to create the dialog bar window and attach
//			it to the CXTDialogBar object.
class _XT_EXT_CLASS CXTDialogBar : public CXTControlBar
{
    DECLARE_DYNAMIC(CXTDialogBar)

public:

    // Summary: Constructs a CXTDialogBar object.
    CXTDialogBar();

    // Summary: Destroys a CXTDialogBar object, handles cleanup and de-allocation.
    virtual ~CXTDialogBar();

protected:

    CWnd* m_pParentWnd;  // Points to the parent window.
    CSize m_sizeDefault; // Default size of the dialog bar.

public:

	// BULLETED LIST:

	// Input:	pParentWnd - A pointer to the parent CWnd object.
	//			lpszTemplateName - A pointer to the name of the CXTDialogBar 
    //			object’s dialog box resource template.
	//			nStyle - The alignment style of the dialog bar. The 
    //			following styles are supported:
	//			[ul]
    //			[li]<b>CBRS_TOP</b> Control bar is at the top of the frame
	//			window.[/li]
    //			[li]<b>CBRS_BOTTOM</b> Control bar is at the bottom of the
	//			frame window.[/li]
    //			[li]<b>CBRS_NOALIGN</b> Control bar is not repositioned when
	//			the parent is resized.[/li]
    //			[li]<b>CBRS_LEFT</b> Control bar is at the left of the frame
	//			window.[/li]
    //			[li]<b>CBRS_RIGHT</b> Control bar is at the right of the frame
	//			window.[/li]
	//			[/ul]
	//			nID - The control ID of the dialog bar.
	// Returns: Nonzero if successful, otherwise returns zero.
    // Summary:	This member function loads the dialog box resource template specified
	//			by 'lpszTemplateName'. It creates the dialog bar window, sets
	//			its style, and associates it with the CXTDialogBar object.
    //
    //			If you specify the CBRS_TOP or CBRS_BOTTOM alignment style, the 
    //			dialog bar’s width is that of the frame window and its height is 
    //			that of the resource specified by 'lpszTemplateName'. If you specify the 
    //			CBRS_LEFT or CBRS_RIGHT alignment style, the dialog bar’s height is 
    //			that of the frame window and its width is that of the resource 
    //			specified by 'lpszTemplateName'.
    virtual BOOL Create(CWnd* pParentWnd,LPCTSTR lpszTemplateName,UINT nStyle = CBRS_ALIGN_TOP,UINT_PTR nID = AFX_IDW_DIALOGBAR);

	// BULLETED LIST:

	// Input:	pParentWnd - A pointer to the parent CWnd object.
	//			nIDTemplate - The resource ID of the CXTDialogBar object’s 
    //			dialog box template.
	//			nStyle - The alignment style of the dialog bar. The 
    //			following styles are supported:
	//			[ul]
    //			[li]<b>CBRS_TOP</b> Control bar is at the top of the frame
	//			window.[/li]
    //			[li]<b>CBRS_BOTTOM</b> Control bar is at the bottom of the
	//			frame window.[/li]
    //			[li]<b>CBRS_NOALIGN</b> Control bar is not repositioned when
	//			the parent is resized.[/li]
    //			[li]<b>CBRS_LEFT</b> Control bar is at the left of the frame
	//			window.[/li]
    //			[li]<b>CBRS_RIGHT</b> Control bar is at the right of frame
	//			window.[/li]
	//			[/ul]
    //			nID - The control ID of the dialog bar.
	// Returns: Nonzero if successful, otherwise returns zero.
    // Summary:	This member function loads the dialog box resource template specified
	//			by 'nIDTemplate'. It creates the dialog bar window, sets
	//			its style, and associates it with the CXTDialogBar object.
    //
    //			If you specify the CBRS_TOP or CBRS_BOTTOM alignment style, the 
    //			dialog bar’s width is that of the frame window and its height is 
    //			that of the resource specified by 'nIDTemplate'. If you specify the 
    //			CBRS_LEFT or CBRS_RIGHT alignment style, the dialog bar’s height is 
    //			that of the frame window and its width is that of the resource 
    //			specified by 'nIDTemplate'.
    virtual BOOL Create(CWnd* pParentWnd,UINT nIDTemplate,UINT nStyle = CBRS_ALIGN_TOP,UINT nID = AFX_IDW_DIALOGBAR);

protected:

    // Ignore:
	//{{AFX_VIRTUAL(CXTDialogBar)
    virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
    //}}AFX_VIRTUAL

#ifndef _AFX_NO_OCC_SUPPORT
	//////////////////////////////////////////////////////////////////////    
	// Data and functions necessary for OLE control containment
    //////////////////////////////////////////////////////////////////////

	_AFX_OCC_DIALOG_INFO* m_pOccDialogInfo;
	LPCTSTR m_lpszTemplateName;

	virtual BOOL SetOccDialogInfo(
		_AFX_OCC_DIALOG_INFO* pOccDialogInfo);

#endif //!_AFX_NO_OCC_SUPPORT

    // Ignore:
	//{{AFX_MSG(CXTDialogBar)
#ifndef _AFX_NO_OCC_SUPPORT
    afx_msg LRESULT HandleInitDialog(WPARAM wParam, LPARAM lParam);
#endif 
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE BOOL CXTDialogBar::Create(CWnd* pParentWnd, UINT nIDTemplate, UINT nStyle, UINT nID) {
    return Create(pParentWnd, MAKEINTRESOURCE(nIDTemplate), nStyle, nID);
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
#endif // #if !defined(__XTDIALOGBAR_H__)