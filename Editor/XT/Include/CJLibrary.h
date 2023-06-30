//:Ignore
// CJLibrary.h : header file
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

#if !defined(__CJLIBRARY_H__)
#define __CJLIBRARY_H__

//:Ignore
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
//:End Ignore


#include <XTToolkit.h>

//////////////////////////////////////////////////////////////////////
// PROGRAMMERS NOTE:
// This file is intended to offer support for users who have upgraded from using
// CJLibrary, it is not intended for use on new projects, and only as a temporary
// solution.

#define CCJBrowseButton					CXTBrowseButton
#define CCJBrowseEdit					CXTBrowseEdit
#define CCJCaption						CXTCaption
#define CCJCaptionButton				CXTCaptionButton
#define CCJCaptionPopupWnd				CXTCaptionPopupWnd
#define CCJCBarButton					// obsolete in XTreme toolkit
#define CCJColorPicker					CXTColorPicker
#define CCJColorPopup					CXTColorPopup
#define CCJComboBoxEx					CXTComboBoxEx
#define CCJControlBar					CXTDockWindow
#define CCJDateEdit						CXTDateEdit
#define CCJDateTimeCtrl					CXTDateTimeCtrl
#define CCJDockBar						CXTDockBar
#define CCJDockContext					CXTDockContext
#define CCJExplorerBar					// obsolete in XTreme toolkit
#define CCJFlatButton					CXTButton
#define CCJFlatComboBox					CXTFlatComboBox
#define CCJFlatHeaderCtrl				CXTFlatHeaderCtrl
#define CCJFlatSplitterWnd				CXTSplitterWnd
#define CCJFlatTabCtrl					CXTFlatTabCtrl
#define CCJFontCombo					CXTFontCombo
#define CCJFrameWnd						CXTFrameWnd
#define CCJHexEdit						CXTHexEdit
#define CCJHtmlView						CXTHtmlView
#define CCJHyperLink					CXTHyperLink
#define CCJListBox						CXTListBox
#define CCJListCtrl						CXTListCtrl
#define CCJListView						CXTListView
#define CCJLogoPane						CXTLogoPane
#define CCJMaskEdit						CXTMaskEdit
#define CCJMDIChildWnd					CXTMDIChildWnd
#define CCJMDIFrameWnd					CXTMDIFrameWnd
#define CCJMemFile						CXTMemFile
#define CCJMenu							// obsolete in XTreme toolkit
#define CCJMenuBar						CXTMenuBar
#define CCJMenuBarFrameHook				CXTMBarWndHook
#define CCJMenuData						// obsolete in XTreme toolkit
#define CCJMetaFileButton				// obsolete in XTreme toolkit
#define CCJMiniDockFrameWnd				CXTMiniDockFrameWnd
#define CCJMonthCalCtrl					CXTMonthCalCtrl
#define CCJOutlookBar					CXTOutlookBar
#define CCJPagerCtrl					CXTPagerCtrl
#define CCJReBar						CXTReBar
#define CCJReBarCtrl					CXTReBarCtrl
#define CCJShell						// obsolete in XTreme toolkit
#define CCJShellList					CXTShellListCtrl
#define CCJShellTree					CXTShellTreeCtrl
#define CCJSizeDockBar					CXTDockBar
#define CCJSortClass					CXTSortClass
#define CCJStatusBar					CXTStatusBar
#define CCJStatusBarPane				// obsolete in XTreme toolkit
#define CCJStatusBarPaneControlInfo		// obsolete in XTreme toolkit
#define CCJTabCtrl						CXTTabCtrl
#define CCJTabCtrlBar					CXTTabCtrlBar
#define CCJTabView						CXTTabView
#define CCJTimeEdit						CXTTimeEdit
#define CCJToolBar						CXTToolBar
#define CCJToolBarBase					CXTControlBar
#define CCJToolBarCtrl					CXTToolBarCtrl
#define CCJTreeCtrl						CXTTreeCtrl
#define CCJWindowPlacement				CXTWindowPos
#define CContentItems					CXTContentItems
#define CGfxGroupEdit					CXTEditItem
#define CMenuItemInfo					// obsolete in XTreme toolkit
#define ColorTableEntry					XT_PICK_BUTTON
#define CSubclassWnd					CXTWndHook
#define CSubclassWndMap					CXTWindowMap
#define DROPDOWNBUTTON					CXTDropDownButton
#define AFX_OLDTOOLINFO					XT_OLDTOOLINFO
#define CTV_ITEM						XT_TCB_ITEM
#define LVITEMDATA						XT_LVITEMDATA
#define LPLVITEMDATA					XT_LPLVITEMDATA
#define TCB_ITEM						XT_TCB_ITEM
#define TVITEMDATA						XT_TVITEMDATA
#define LPTVITEMDATA					XT_LPTVITEMDATA
#define TOOLBARINFO						// obsolete in XTreme toolkit
#define CJX_COLORMAP					XT_COLORMAP
#define AFX_DLLVERSIONINFO				XT_DLLVERSIONINFO
#define CJX_CONTROLPOS					XT_CONTROLPOS
#define CToolBarData					XT_TOOLBARDATA
#define CGfxSplitterWnd                 CXTSplitterWndEx

class CGfxOutBarCtrl : public CXTOutBarCtrl
{
public:
	enum {  fSmallIcon    = OBS_XT_SMALLICON,
			fLargeIcon    = OBS_XT_LARGEICON,
			fEditGroups   = OBS_XT_EDITGROUPS,
			fEditItems    = OBS_XT_EDITITEMS,
			fRemoveGroups = OBS_XT_REMOVEGROUPS,
			fRemoveItems  = OBS_XT_REMOVEITEMS,
			fAddGroups    = OBS_XT_ADDGROUPS,
			fDragItems    = OBS_XT_DRAGITEMS,
			fAnimation    = OBS_XT_ANIMATION,
			fSelHighlight = OBS_XT_SELHIGHLIGHT };

	enum {  ircIcon		  = 1 // RC_OB_ICON,
			ircLabel	  = 2 // RC_OB_LABEL,
			ircAll		  = 3 // RC_OB_BOTH };
};

struct OUTBAR_INFO : public XT_OUTBAR_INFO
{
	OUTBAR_INFO() : index(nIndex), iDragFrom(nDragFrom), iDragTo(nDragTo), cText(lpszText) {}
	int&		index;
	int&		iDragFrom;
	int&		iDragTo;
	LPCTSTR&	cText;
};

typedef enum
{
	NONE	= 0,
	OUTLINE	= CBRS_XT_CLIENT_OUTLINE,
	SUNKEN	= CBRS_XT_CLIENT_STATIC,
	RAISED	= CBRS_XT_CLIENT_MODAL
}
CHILD_BORDER;

//////////////////////////////////////////////////////////////////////
// Global functions:

#define _LoadSysColorBitmap(hInst, hRsrc, bMono)						_xtAfxLoadSysColorBitmap(hInst, hRsrc, bMono)
#define DDX_CJMonthCalCtrl(pDX, nIDC, value)							DDX_XTMonthCalCtrl(pDX, nIDC, value)
#define DDX_CJColorPicker(pDX, nIDC, value)								DDX_XTColorPicker(pDX, nIDC, value)
#define DDX_CJDateTimeCtrl(pDX, nIDC, value)							DDX_XTDateTimeCtrl(pDX, nIDC, value)
#define DDX_OleDateTime(pDX, nIDC, rControl, rDateTime)					DDX_XTOleDateTime(pDX, nIDC, rControl, rDateTime)
#define DDV_CJMinMaxDateTime(pDX, refValue, pMinRange, pMaxRange)		DDV_XTMinMaxDateTime(pDX, refValue, pMinRange, pMaxRange)
#define DDV_CJMinMaxMonth(pDX, refValue, pMinRange, pMaxRange)			DDV_XTMinMaxMonth(pDX, refValue, pMinRange, pMaxRange)

//////////////////////////////////////////////////////////////////////
// Definitions:

#define _ComCtlVersion				_xtAfxComCtlVersion
#define _GetComCtlVersion()			_xtAfxGetComCtlVersion()
#define _dropDownWidth				// obsolete in XTreme toolkit
#define _GetDropDownWidth()			// obsolete in XTreme toolkit
#define AFX_IDW_SIZEBAR_LEFT		AFX_IDW_DOCKBAR_LEFT
#define AFX_IDW_SIZEBAR_RIGHT		AFX_IDW_DOCKBAR_RIGHT
#define AFX_IDW_SIZEBAR_TOP			AFX_IDW_DOCKBAR_TOP
#define AFX_IDW_SIZEBAR_BOTTOM		AFX_IDW_DOCKBAR_BOTTOM
#define CM_ONPUSHPINBUTTON			CPWN_XT_PUSHPINBUTTON
#define CM_ONPUSHPINCANCEL			CPWN_XT_PUSHPINCANCEL
#define WS_EX_FLATEDGE				// obsolete in XTreme toolkit
#define CPN_SELCHANGE				CPN_XT_SELCHANGE
#define CPN_DROPDOWN				CPN_XT_DROPDOWN
#define CPN_CLOSEUP					CPN_XT_CLOSEUP
#define CPN_SELENDOK				CPN_XT_SELENDOK
#define CPN_SELENDCANCEL			CPN_XT_SELENDCANCEL
#define FTS_BOTTOM					FTS_XT_BOTTOM
#define FTS_HASARROWS				FTS_XT_HASARROWS
#define FTS_HASHOMEEND				FTS_XT_HASHOMEEND
#define _CJX_EXT_CLASS				_XT_EXT_CLASS
#define _CJXLIB_INLINE				inline
#define WM_SHELL_NOTIFY				XTWM_SHELL_NOTIFY
#define NM_SH_SHELLMENU				SHN_XT_SHELLMENU
#define WM_OUTBAR_NOTIFY			XTWM_OUTBAR_NOTIFY
#define NM_OB_ITEMCLICK				OBN_XT_ITEMCLICK
#define NM_OB_ONLABELENDEDIT		OBN_XT_ONLABELENDEDIT
#define NM_OB_ONGROUPENDEDIT		OBN_XT_ONGROUPENDEDIT
#define NM_OB_DRAGITEM				OBN_XT_DRAGITEM
#define NM_FOLDERCHANGE				OBN_XT_FOLDERCHANGE
#define NM_OB_ITEMHOVER				OBN_XT_ITEMHOVER
#define BT_POPUPMENU				BES_XT_POPUPMENU
#define BT_DIRECTORY				BES_XT_CHOOSEDIR
#define BT_FILE						BES_XT_CHOOSEFILE
#define BROWSE_TYPE					DWORD
#define CP_MODE_TEXT				// obsolete in XTreme toolkit
#define CP_MODE_BK					// obsolete in XTreme toolkit
#define BTN_IMG_INDEX				// obsolete in XTreme toolkit
#define CTabList					// obsolete in XTreme toolkit
#define CMDIMenuList				// obsolete in XTreme toolkit
#define DATA_TYPE					XT_DATA_TYPE
#define CTabViews					// obsolete in XTreme toolkit
#define CListViews					// obsolete in XTreme toolkit

#endif // #if !defined(__CJLIBRARY_H__)
//:End Ignore
