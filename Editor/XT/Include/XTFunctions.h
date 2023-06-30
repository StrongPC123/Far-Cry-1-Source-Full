// XTFunctions.h interface for the XT_AUX_DATA struct.
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

#if !defined(__XTFUNCTIONS_H__)
#define __XTFUNCTIONS_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//:Associate with "Global Functions"

#if defined(XT_INIT_BY_REGULAR_DLL)
//////////////////////////////////////////////////////////////////////
// Summary: This member function will initialize the resources for the Xtreme Toolkit.
//			If using the Xtreme Toolkit as an extension DLL within a regular DLL,
//			add the following two lines to your stdafx.h file. This will give you
//			access to the exported function InitXtremeExtDll():
// 
//			<pre>#define XT_INIT_BY_REGULAR_DLL
//			#include "XTToolkit.h"</pre>
// 
//			You will then need to add XT_INIT_BY_REGULAR_DLL to the Xtreme Toolkit 
//			preprocessor definitions and rebuild it.  After you have done this
//			locate your CWinApp::InitInstance() method for your regular DLL and make
//			the following call:
// 
//			<pre>InitXtremeExtDLL();</pre>
// 
//			This will initialize the resources for the Xtreme Toolkit.
extern _XT_EXT_CLASS void WINAPI InitXtremeExtDLL();
#endif // defined(XT_INIT_BY_REGULAR_DLL)

// Input:	pWnd - Pointer to a valid CWnd object.
//			pFont - Pointer to the new font to set for the window.
// Summary:	This member function will set the font for the window specified by
//			'pWnd' and all of the child windows owned by 'pWnd'.
extern _XT_EXT_CLASS void AFXAPI _xtAfxChangeWindowFont(CWnd* pWnd,CFont* pFont);

// Input:	pDC - Pointer to the current device context.
//			imageList - Address of an image list.
//			nIndex - Index of the image in the image list.
//			point - XY location of where to draw the icon.
//			bInColor - TRUE to draw the item in color, otherwise the
//			icon will be drawn with the default disabled look.
// Summary:	This member function will draw an embossed icon from the image
//			list that is passed in, into the specified device context. Typically
//			used by toolbars and menus to draw a disabled icon in color.
extern _XT_EXT_CLASS void AFXAPI _xtAfxDrawEmbossed(CDC* pDC,CImageList& imageList,int nIndex,CPoint point,BOOL bInColor);

// Input:	hInst - Instance handle of the module that has the required resource.
//			hRsrc - Bitmap resource handle.
//			bMono - Tells if monochrome conversion shall be applied.
// Summary:	This member function loads a bitmap from resource and translates the
//			bitmap color such that they suit requirements for a toolbar bitmap.
extern _XT_EXT_CLASS HBITMAP AFXAPI _xtAfxLoadSysColorBitmap(HINSTANCE hInst,HRSRC hRsrc,BOOL bMono = FALSE);

// Input:	pDC - Points to the current device context.
//			rect - Size of the area to draw.
// Summary:	This member function will draw a shadow rect into the specified
//			device context.
extern _XT_EXT_CLASS void AFXAPI _xtAfxDrawShadedRect(CDC *pDC,CRect& rect);

// Input:	hWnd - HWND handle of the parent window to find the child for.
//			pt - Current cursor position.
// Returns: An HWND handle for the child window at 'pt'.
// Summary:	This member function will retrieve an HWND handle for the child window
//			found directly under the cursor position specified by 'pt'.
extern _XT_EXT_CLASS HWND AFXAPI _xtAfxChildWindowFromPoint(HWND hWnd,POINT pt);

// Input:	strText - A reference to a CString object.
// Summary:	This member function will search a string, strip off the mnemonic
//			'&', and reformat the string.
extern _XT_EXT_CLASS void AFXAPI _xtAfxStripMnemonics(CString& strText);

// Input:	lpszFileName - Fully qualified path for the item to find.
// Returns: TRUE if successful, otherwise returns FALSE.
// Summary:	This member function will test for the existence of a file or folder.
extern _XT_EXT_CLASS BOOL AFXAPI _xtAfxExist(LPCTSTR lpszFileName);

// Input:	pszPath - Pointer to a NULL-terminated string with the path.  Paths are delimited 
//			by backslashes or by the NULL at the end of the path. 
// Returns: A pointer to a NULL-terminated string with the next path component
//			if successful, or NULL otherwise.
// Summary:	This member function parses a path for the next path component, not
//			dependent on shell32.dll, and is a replacement for the windows API 
//			PathFindNextComponent. 
extern _XT_EXT_CLASS LPTSTR AFXAPI _xtAfxPathFindNextComponent(LPCTSTR pszPath);

// Input:	pParentWnd - Points to the parent of the view to be created.  The parent must be
//			valid.
//			pViewClass - CView runtime class.
//			pDocument - CDocument associated with the view. It can be NULL.
//			pContext - Create context for the view. It can be NULL.
//			dwStyle - Default style for the view.
//			pOwnerWnd - Owner of the view. If NULL, 'pParentWnd' is used.
//			nID - Control ID of the view.
// Returns: A CWnd* pointer to the newly created view if successful, 
//			otherwise returns NULL.
// Summary:	This member function dynamically creates a view based on a CRuntimeClass
//			object.
extern _XT_EXT_CLASS CWnd* AFXAPI _xtAfxCreateView(CWnd* pParentWnd,CRuntimeClass *pViewClass, CDocument *pDocument=NULL,CCreateContext *pContext=NULL,DWORD dwStyle=AFX_WS_DEFAULT_VIEW,CWnd* pOwnerWnd=NULL,UINT nID=AFX_IDW_PANE_FIRST);

// Input:	lparam1 - Corresponds to the lParam member of the TV_ITEM structure for the 
//			two items being compared.
//			lparam2 - Corresponds to the lParam member of the TV_ITEM structure for the 
//			two items being compared.
//			lparamSort - The 'lParamSort' member corresponds to the lParam member of TV_SORTCB.
// Returns: A negative value if the first item should precede the second, 
//			a positive value if the first item should follow the second, or zero 
//			if the two items are equivalent.
// Summary:	This member function is a callback function used by CXTShellTreeCtrl
//			and CXTShellTreeView, and is called during a sort operation each
//			time the relative order of two list items needs to be compared.
extern _XT_EXT_CLASS int CALLBACK _xtAfxTreeViewCompareProc(LPARAM lparam1,LPARAM lparam2,LPARAM lparamSort);

// Input:	lparam1 - Corresponds to the lParam member of the LV_ITEM structure for the 
//			two items being compared.
//			lparam2 - Corresponds to the lParam member of the LV_ITEM structure for the 
//			two items being compared.
//			lparamSort - The 'lParamSort' member corresponds to the lParam member of LV_SORTCB.
// Returns: A negative value if the first item should precede the second, 
//			a positive value if the first item should follow the second, or zero 
//			if the two items are equivalent.
// Summary:	This member function is a callback function used by CXTShellListCtrl
//			and CXTShellListView, and is called during a sort operation each time
//			the relative order of two list items needs to be compared.
extern _XT_EXT_CLASS int CALLBACK _xtAfxListViewCompareProc(LPARAM lparam1,LPARAM lparam2,LPARAM lparamSort);

// Input:	pView - Pointer to the view to dispaly print preview for.
// Returns: true if successful, otherwise returns false.
// Summary:	Call this global function to display an office style print preview window 
//			for your CView derived class.  
_XT_EXT_CLASS bool _xtAfxShowPrintPreview(CView* pView);

extern _XT_EXT_CLASS void AFXAPI _xtAfxMakeOleVariant(COleVariant &ov, LPCITEMIDLIST pidl);
#ifdef _DEBUG
extern _XT_EXT_CLASS void AFXAPI _xtAfxAssertValidBarIDs(CFrameWnd* pFrameWnd);
#endif //_DEBUG

//:Associate with "Dialog Data Exchange"

// Input:	pDX - A pointer to a CDataExchange object. The framework supplies this
//			object to establish the context of the data exchange, including its
//			direction.    
//			nIDC - The resource ID of the color picker control associated with the
//			control property.
//			value - A reference to a member variable of the dialog box, form view, or
//			control view object with which data is exchanged.
// Summary:	CXTColorPicker - The DDX_XTColorPicker function manages the transfer
//			of integer data between a color picker control in a dialog box, form view,
//			or control view object and a COLORREF data member of the dialog box,
//			form view, or control view object.
//
//			When DDX_XTColorPicker is called, 'value' is set to the current state
//			of the color picker control.
extern _XT_EXT_CLASS void AFXAPI DDX_XTColorPicker(CDataExchange* pDX,int nIDC,COLORREF& value);

//:Associate with "Dialog Data Exchange"

// Input:	pDX - A pointer to a CDataExchange object.  The framework supplies this
//			object to establish the context of the data exchange, including its
//			direction.  You do not need to delete this object.
//			nIDC - The resource ID of the date and time picker control associated with
//			the member variable.
//			value - A reference to a COleDateTime member variable, dialog box, form
//			view, or control view object with which data is exchanged. 
// Summary:	CXTDateTimeCtrl - The DDX_XTDateTimeCtrl function manages the transfer
//			of date and/or time data between a date and time picker control (CXTDateTimeCtrl)
//			in a dialog box or form view object and a COleDateTime data member of the
//			dialog box or form view object.
//
//			When CXTDateTimeCtrl is called, 'value' is set to the current state
//			of the date and time picker control, or the control is set to 'value',
//			depending on the direction of the exchange.
extern _XT_EXT_CLASS void AFXAPI DDX_XTDateTimeCtrl(CDataExchange* pDX,int nIDC,COleDateTime& value);

//:Associate with "Dialog Data Exchange"

// Input:	pDX - A pointer to a CDataExchange object.  The framework supplies this
//			object to establish  the context of the data exchange, including its
//			direction.  You do not need to delete this object.
//			nIDC - The resource ID of the date and time picker control associated with
//			the member variable.
//			value - A reference to a CTime member variable, dialog box, form view, or
//			control view object with which data is exchanged. 
// Summary:	CXTDateTimeCtrl - The DDX_XTDateTimeCtrl function manages the transfer
//			of date and/or time data between a date and time picker control (CXTDateTimeCtrl)
//			in a dialog box or form view object, and a CTime data member of the dialog
//			box or form view object.
//
//			When CXTDateTimeCtrl is called, 'value' is set to the current state
//			of the date and time picker control, or the control is set to 'value',
//			depending on the direction of the exchange.
extern _XT_EXT_CLASS void AFXAPI DDX_XTDateTimeCtrl(CDataExchange* pDX,int nIDC,CTime& value);

//:Associate with "Dialog Data Validation"

// Input:	pDX - A pointer to a CDataExchange object.  The framework supplies this
//			object to establish the context of the data exchange, including its
//			direction. You do not need to delete this object.
//			refValue - A reference to a CTime object associated with a member variable
//			of the dialog box, form view, or control view object. This object contains
//			the data to be validated.
//			pMinRange - Minimum date/time value allowed.
//			pMaxRange - Maximum date/time value allowed.
// Summary:	CXTDateTimeCtrl - Call DDV_XTMinMaxDateTime to verify that the time/date
//			value in the date and time picker control (CXTDateTimeCtrl) associated
//			with 'refValue' falls between 'pMinRange' and 'pMaxRange'.
extern _XT_EXT_CLASS void AFXAPI DDV_XTMinMaxDateTime(CDataExchange* pDX,CTime& refValue,const CTime* pMinRange,const CTime* pMaxRange);

//:Associate with "Dialog Data Validation"

// Input:	pDX - A pointer to a CDataExchange object.  The framework supplies this
//			object to establish the context of the data exchange, including its
//			direction.   You do not need to delete this object.
//			refValue - A reference to a COleDateTime object associated with a member variable
//			of the dialog box, form view, or control view object.  This object
//			contains the data to be validated.
//			pMinRange - Minimum date/time value allowed.
//			pMaxRange - Maximum date/time value allowed.
// Summary:	CXTDateTimeCtrl - Call DDV_XTMinMaxDateTime to verify that the time/date
//			value in the date and time picker control (CXTDateTimeCtrl) associated
//			with 'refValue' falls between 'pMinRange' and 'pMaxRange'.
extern _XT_EXT_CLASS void AFXAPI DDV_XTMinMaxDateTime(CDataExchange* pDX,COleDateTime& refValue,const COleDateTime* pMinRange,const COleDateTime* pMaxRange);

//:Associate with "Dialog Data Exchange"

// Input:	pDX - A pointer to a CDataExchange object.  The framework supplies this
//			object to establish the context of the data exchange, including its
//			direction. You do not need to delete this object.
//			nIDC - The resource ID of the month calendar control associated with the
//			member variable.
//			value - A reference to a COleDateTime member variable of the dialog box,
//			form view, or control view object with which data is exchanged. 
// Summary:	CXTMonthCalCtrl - The DDX_XTMonthCalCtrl function manages the transfer
//			of date data between a month calendar control (DDX_XTMonthCalCtrl),
//			in a dialog box, form view, or control view object, and a COleDateTime
//			data member, of the dialog box, form view, or control view object.
//
// Remarks: The control manages a date value only.  The time fields in the
//			time object are set to reflect the creation time of the control window,
//			or whatever time was set in the control with a call to CXTMonthCalCtrl::SetCurSel.
//
//			When DDX_XTMonthCalCtrl is called, 'value' is set to the current state of
//			the month calendar control.
extern _XT_EXT_CLASS void AFXAPI DDX_XTMonthCalCtrl(CDataExchange* pDX,	int nIDC,COleDateTime& value);

//:Associate with "Dialog Data Exchange"

// Input:	pDX - A pointer to a CDataExchange object. The framework supplies this
//			object to establish the context of the data exchange, including its
//			direction. You do not need to delete this object.
//			nIDC - The resource ID of the month calendar control associated with the
//			member variable.
//			value - A reference to a CTime member variable of the dialog box, form view,
//			or control view object with which data is exchanged. 
// Remarks: The control manages a date value only.  The time fields in the
//			time object are set to reflect the creation time of the control window,
//			or whatever time was set in the control with a call to CXTMonthCalCtrl::SetCurSel.
//
//			When DDX_XTMonthCalCtrl is called, 'value' is set to the current state of
//			the month calendar control.
// Summary:	CXTMonthCalCtrl - The DDX_XTMonthCalCtrl function manages the transfer
//			of date data between a month calendar control (DDX_XTMonthCalCtrl), in
//			a dialog box, form view, or control view object, and a CTime data
//			member of the dialog box, form view, or control view object.
extern _XT_EXT_CLASS void AFXAPI DDX_XTMonthCalCtrl(CDataExchange* pDX,int nIDC,CTime& value);

//:Associate with "Dialog Data Validation"

// Input:	pDX - A pointer to a CDataExchange object.  The framework supplies this
//			object to establish the context of the data exchange, including its
//			direction.
//			refValue - A reference to an object of type CTime associated with a member
//			variable of the dialog box, form view, or control view object.  This
//			object contains the data to be validated.  MFC passes this reference
//			when DDV_XTMinMaxMonth is called.
//			pMinRange - Minimum date/time value allowed.
//			pMaxRange - Maximum date/time value allowed.
// Summary:	CXTMonthCalCtrl - Call DDV_XTMinMaxMonth to verify that the time/date
//			value in the month calendar control (CXTMonthCalCtrl) associated with
//			'refValue' falls between 'pMinRange' and 'pMaxRange'.
extern _XT_EXT_CLASS void AFXAPI DDV_XTMinMaxMonth(CDataExchange* pDX,CTime& refValue,const CTime* pMinRange,const CTime* pMaxRange);

//:Associate with "Dialog Data Validation"

// Input:	pDX - A pointer to a CDataExchange object.  The framework supplies this
//			object to establish the context of the data exchange, including its
//			direction.
//			refValue - A reference to an object of type CTime associated with a member
//			variable of the dialog box, form view, or control view object.  This
//			object contains the data to be validated. MFC passes this reference
//			when DDV_XTMinMaxMonth is called.
//			pMinRange - Minimum date/time value allowed.
//			pMaxRange - Maximum date/time value allowed.
// Summary:	CXTMonthCalCtrl - Call DDV_XTMinMaxMonth to verify that the time/date
//			value in the month calendar control (CXTMonthCalCtrl) associated with
//			'refValue' falls between 'pMinRange' and 'pMaxRange'.
extern _XT_EXT_CLASS void AFXAPI DDV_XTMinMaxMonth(CDataExchange* pDX,COleDateTime& refValue,const COleDateTime* pMinRange,const COleDateTime* pMaxRange);

class CXTDateEdit;

//:Associate with "Dialog Data Exchange"

// Input:	pDX - A pointer to a CDataExchange object.  The framework supplies this
//			object to establish the context of the data exchange, including its
//			direction.    
//			nIDC - The resource ID of the date edit control associated with the control
//			property.
//			rControl - A reference to a member variable of the dialog box, form view, or
//			control view object with which data is exchanged.
//			rDateTime - A reference to a member variable of the dialog box, form view, or
//			control view object with which data is exchanged.
// Summary:	CXTDateEdit - The DDX_XTOleDateTime function manages the transfer of
//			integer data between a date edit control, in a dialog box, form view, or
//			control view object, and a COleDateTime data member of the dialog box,
//			form view, or control view object.
//
//			When DDX_XTOleDateTime is called, 'value' is set to the current state
//			of the date edit control.
extern _XT_EXT_CLASS void AFXAPI DDX_XTOleDateTime(CDataExchange* pDX,int nIDC,CXTDateEdit& rControl,COleDateTime& rDateTime);

//////////////////////////////////////////////////////////////////////

#endif // #if !defined(__XTFUNCTIONS_H__)
