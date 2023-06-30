// XTIncludes.h : header file
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

#if !defined(__XTINCLUDES_H__)
#define __XTINCLUDES_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//--------------
// MFC includes:
//--------------

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>
#endif // _AFX_NO_OLE_SUPPORT

#ifdef _MFC_OVERRIDES_NEW
#define _INC_MALLOC
#endif
// MFC support for docking windows
#include <afxpriv.h>		
// MFC template classes
#include <afxtempl.h>		
// MFC ListView / TreeView support
#include <afxcview.h>		

#if _MFC_VER >= 0x0700 //MFC 7.0
// MFC Global data
#include <..\src\mfc\afximpl.h>	
#else
// MFC Global data
#include <..\src\mfc\\afximpl.h>	    
#endif

// MFC support for shell extensions
#include <shlobj.h>			

//////////////////////////////////////////////////////////////////////
// Summary: NOTE: If using the Xtreme Toolkit as a static library linked to an application
//			that is dynamically linked with MFC, you will need to do the following:
//
//			Open the XTToolkit_Lib project workspace and select one of the 
//			Win32 Dynamic build settings and build the library. Add the following lines 
//			of code to your stdafx.h file:
//
//			<pre>#define _XT_STATICLINK
//			#include <XTToolkit.h></pre>
//
//			Add the following line of code to your *.rc2 file after the comment:
//			"Add manually edited resources here...":
//
//			<pre>#include "XTResource.rc"</pre>
#if !defined( _AFXDLL ) || defined( _XT_STATICLINK )
#define _XT_EXT_CLASS
#else
#define _XT_EXT_CLASS	__declspec( dllimport )
#endif // !defined( _AFXDLL ) || defined( _XT_STATICLINK )

//-----------------------------------------
// Xtreme global and resource definitions:
//-----------------------------------------

#include "XTDefines.h"
#include "XTGlobal.h"
#include "XTResource.h"
#include "XTFunctions.h"
#include "XTVersion.h"
#include "XTMemDC.h"

//-------------------------
// Xtreme control classes:
//-------------------------

#include "XTButton.h"
#include "XTWndHook.h"
#include "XTCoolMenu.h"
#include "XTHexEdit.h"
#include "XTFlatComboBox.h"
#include "XTMaskEdit.h"
#include "XTBrowseEdit.h"
#include "XTCaption.h"
#include "XTCaptionPopupWnd.h"
#include "XTColorRef.h"
#include "XTColorSelectorCtrl.h"
#include "XTColorPopup.h"
#include "XTColorPicker.h"
#include "XTColorPageCustom.h"
#include "XTColorPageStandard.h"
#include "XTColorDialog.h"
#include "XTEditListBox.h"
#include "XTFontCombo.h"
#include "XTComboBoxEx.h"
#include "XTTabCtrl.h"
#include "XTFlatTabCtrl.h"
#include "XTTreeCtrl.h"
#include "XTTreeView.h"
#include "XTFlatHeaderCtrl.h"
#include "XTListCtrl.h"
#include "XTOutBarCtrl.h"
#include "XTOutlookBar.h"
#include "XTPagerCtrl.h"
#include "XTDateTimeCtrl.h"
#include "XTHyperLink.h"
#include "XTTipWindow.h"
#include "XTTipOfTheDay.h"
#include "XTSplitterWnd.h"
#include "XTStatusBar.h"
#include "XTLogoPane.h"
#include "XTMDIWndTab.h"
#include "XTOSVersionInfo.h"

//------------------------------------------------------------------------------------
// Xtreme resizing dialog controls
//------------------------------------------------------------------------------------

#include "XTResizeRect.h"
#include "XTResizePoint.h"
#include "XTResize.h"
#include "XTResizeDialog.h"
#include "XTResizeFormView.h"
#include "XTResizePropertyPage.h"
#include "XTResizePropertySheet.h"
#include "XTSearchOptionsCtrl.h"
#include "XTCBarDialog.h"

//------------------------------------------------------------------------------------
// Xtreme manager controls
//------------------------------------------------------------------------------------

#include "XTIconMap.h"
#include "XTToolsManager.h"
#include "XTOptionsManager.h"
#include "XTAccelManager.h"
#include "XTAccelKeyEdit.h"

//-----------------------------------------------
// Xtreme control bars -
// Replaces: CControlBar, CDockBar, CDockContext:
//-----------------------------------------------

#include "XTDelegate.h"
#include "XTCustomizeAPI.h"
#include "XTCallbacks.h"
#include "XTControlBar.h"
#include "XTDialogBar.h"
#include "XTSplitterDock.h"
#include "XTDockBar.h"
#include "XTDockContext.h"
#include "XTDockWindow.h"
#include "XTTabCtrlBar.h"
#include "XTDockColorSelector.h"

//------------------------------------------------------
// Xtreme toolbars - 
// Replaces: CToolBar, CToolBarCtrl, CReBar, CReBarCtrl:
//------------------------------------------------------

#include "XTToolBarCtrl.h"
#include "XTToolBar.h"
#include "XTMenuBar.h"
#include "XTReBar.h"
#include "XTReBarCtrl.h"
#include "XTToolBarPopupWnd.h"
#include "XTPopupTearOffWnd.h"

//------------------------------------------------------------------------------------
// Xtreme frame windows -
// Replaces: CFrameWnd, CMDIFrameWnd, CMDIChildWnd, COleIPFrameWnd, CMiniDockFrameWnd:
//------------------------------------------------------------------------------------

#include "XTFrameImpl.h"
#include "XTFrameWnd.h"
#include "XTMDIFrameWnd.h"
#include "XTMDIChildWnd.h"

#ifndef _AFX_NO_OLE_SUPPORT
#include "XTOleIPFrameWnd.h"
#endif // _AFX_NO_OLE_SUPPORT

#include "XTMiniDockFrameWnd.h"

//-----------------------
// Xtreme customization:
//-----------------------

#include "XTCheckListBox.h"
#include "XTCommandsListBox.h"
#include "XTNewToolbarDlg.h"
#include "XTCustomizePage.h"
#include "XTCustomizeSheet.h"

//-----------------------
// Xtreme CView classes:
//-----------------------

#include "XTHtmlView.h"
#include "XTListView.h"
#include "XTPreviewView.h"
#include "XTTabView.h"

//--------------------------
// Xtreme shell extensions:
//--------------------------

#include "XTDropSource.h"
#include "XTShellPidl.h"
#include "XTShellSettings.h"
#include "XTShellTreeView.h"
#include "XTShellTreeCtrl.h"
#include "XTShellListView.h"
#include "XTShellListCtrl.h"
#include "XTTrayIcon.h"
#include "XTBrowseDialog.h"

//----------------------------
// Xtreme utility classes:
//----------------------------

#include "XTWindowPos.h"
#include "XTMemFile.h"
#include "XTSortClass.h"
#include "XTRegistryManager.h"

/////////////////////////////////////////////////////////////////////////////

#endif // #if !defined(__XTINCLUDES_H__)
