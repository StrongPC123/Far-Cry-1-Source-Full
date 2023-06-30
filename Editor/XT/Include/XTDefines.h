// XTDefines.h interface for the XT_AUX_DATA struct.
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

#if !defined(__XTDEFINES_H__)
#define __XTDEFINES_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxres.h>

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTColorRef"
//////////////////////////////////////////////////////////////////////////

const UINT XPCOLOR_BASE                     =  30;           // An RGB value that represents the XP base color.
const UINT XPCOLOR_BARFACE                  =  XPCOLOR_BASE; // An RGB value that represents the XP toolbar background color.
const UINT XPCOLOR_HIGHLIGHT                =  31;           // An RGB value that represents the XP menu item selected color.
const UINT XPCOLOR_HIGHLIGHT_BORDER         =  32;           // An RGB value that represents the XP menu item selected border color.
const UINT XPCOLOR_HIGHLIGHT_PUSHED         =  33;           // An RGB value that represents the XP menu item pushed color.
const UINT XPCOLOR_ICONSHADDOW              =  34;           // An RGB value that represents the XP menu item icon shadow
const UINT XPCOLOR_GRAYTEXT                 =  35;           // An RGB value that represents the XP menu item disabled text color.
const UINT XPCOLOR_HIGHLIGHT_CHECKED        =  36;           // An RGB value that represents the XP menu item checked color.
const UINT XPCOLOR_HIGHLIGHT_CHECKED_BORDER =  37;           // An RGB value that represents the XP menu item checked border color.
const UINT XPCOLOR_GRIPPER                  =  38;           // An RGB value that represents the XP toolbar gripper color.
const UINT XPCOLOR_SEPARATOR                =  39;           // An RGB value that represents the XP toolbar separator color.
const UINT XPCOLOR_DISABLED                 =  40;           // An RGB value that represents the XP menu icon disabled color.
const UINT XPCOLOR_MENUTEXT_BACK            =  41;           // An RGB value that represents the XP menu item text background color.
const UINT XPCOLOR_MENU_EXPANDED            =  42;           // An RGB value that represents the XP hidden menu commands background color.
const UINT XPCOLOR_MENU_BORDER              =  43;           // An RGB value that represents the XP menu border color.
const UINT XPCOLOR_MENUTEXT                 =  44;           // An RGB value that represents the XP menu item text color.
const UINT XPCOLOR_HIGHLIGHT_TEXT           =  45;           // An RGB value that represents the XP menu item selected text color.
const UINT XPCOLOR_BARTEXT                  =  46;           // An RGB value that represents the XP toolbar text color.
const UINT XPCOLOR_BARTEXT_PUSHED           =  47;           // An RGB value that represents the XP toolbar pushed text color.
const UINT XPCOLOR_TAB_INACTIVE_BACK        =  48;           // An RGB value that represents the XP inactive tab background color.
const UINT XPCOLOR_TAB_INACTIVE_TEXT        =  49;           // An RGB value that represents the XP inactive tab text color.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTControlBar"
//////////////////////////////////////////////////////////////////////////

const UINT XT_IDW_CONTROLBAR_MIN   = AFX_IDW_CONTROLBAR_FIRST;	// Minimum control ID value for a CXTControlBar (59392).
const UINT XT_IDW_CONTROLBAR_MAX   = AFX_IDW_CONTROLBAR_LAST;	// Maximum control ID value for a CXTControlBar (59647).
const UINT XT_IDC_TOOLSMANAGER_MIN = 0xCF08;					// Minimum command ID for tools manager user defined commands (53000).
const UINT XT_IDC_TOOLSMANAGER_MAX = 0xD002;					// Maximum command ID for tools manager user defined commands (53250).

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTSplitterDock"
//////////////////////////////////////////////////////////////////////////

const UINT XT_SPLITTER_VERT        = 0x0001; // Vertical docking window splitter.
const UINT XT_SPLITTER_HORZ        = 0x0002; // Horizontal docking window splitter.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTSplitterWnd"
//////////////////////////////////////////////////////////////////////////

const UINT XT_SPLIT_DOTTRACKER     = 0x0001; // Has a dotted tracker.
const UINT XT_SPLIT_NOFULLDRAG     = 0x0002; // Show pane contents while dragging.
const UINT XT_SPLIT_NOBORDER       = 0x0004; // Do not draw a border around the pane.
const UINT XT_SPLIT_NOSIZE         = 0x0008; // Do allow sizing.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTDockWindow"
//////////////////////////////////////////////////////////////////////////

const UINT CBRS_XT_GRIPPER         = 0x0001; // Has a gripper when docked.
const UINT CBRS_XT_GRIPPER_TEXT    = 0x0002; // Draws text in the gripper area when docked.  Only valid with CBRS_XT_GRIPPER styles.
const UINT CBRS_XT_TWOGRIP         = 0x0004; // Has a gripper with two grip bars.
const UINT CBRS_XT_GRIPPER_FLAT    = 0x0008; // Has a flat gripper.  Must be used with CBRS_XT_GRIPPER.
const UINT CBRS_XT_GRIPPER_GRAD    = 0x0010; // Has a gradient style gripper.  Must be used with CBRS_XT_GRIPPER_FLAT.
const UINT CBRS_XT_BUTTONS         = 0x0020; // Has min, max, and close frame buttons.
const UINT CBRS_XT_BUTTONS_FLAT    = 0x0040; // Draws buttons flat.  Only valid with CBRS_XT_BUTTONS style.
const UINT CBRS_XT_BORDERS_FLAT    = 0x0080; // Draws the borders flat.
const UINT CBRS_XT_SEMIFLAT        = 0x0100; // Draws buttons and borders with a thin 3D border.
const UINT CBRS_XT_CLIENT_OUTLINE  = 0x0200; // Draws an outline around the client area.
const UINT CBRS_XT_CLIENT_STATIC   = 0x0400; // Draws a static rect around the client area.
const UINT CBRS_XT_CLIENT_MODAL    = 0x0800; // Draws a modal rect around the client area.
const UINT CBRS_XT_NEWDOCKED       = 0x1000; // Used internally by docking windows.
const UINT CBRS_XT_DEFAULT         = 0x0023; // Same as CBRS_XT_BUTTONS | CBRS_XT_GRIPPER | CBRS_XT_GRIPPER_TEXT.
const UINT CBRS_XT_ALL_FLAT        = 0x00EB; // Same as CBRS_XT_DEFAULT | CBRS_XT_GRIPPER_FLAT | CBRS_XT_BUTTONS_FLAT | CBRS_XT_BORDERS_FLAT.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTDockBar"
//////////////////////////////////////////////////////////////////////////

const UINT CBRS_XT_NONFLAT         = 0x0000; // 3D borders.
const UINT CBRS_XT_FLAT            = 0x0080; // Flat borders.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTButton"
//////////////////////////////////////////////////////////////////////////

const UINT BS_XT_FLAT              = 0x0001; // Draws a flat button.
const UINT BS_XT_SEMIFLAT          = 0x0002; // Draws a semi-flat button.
const UINT BS_XT_TWOROWS           = 0x0004; // Draws images and text are centered.

// OBSOLETE

const UINT BS_XT_CENTER            = 0x0008; 

const UINT BS_XT_SHOWFOCUS         = 0x0010; // Draws a focus rect when the button has input focus.
const UINT BS_XT_HILITEPRESSED     = 0x0020; // Highlights the button when pressed.
const UINT BS_XT_XPFLAT            = 0x0040; // Draws a flat button ala Office XP.
const UINT BS_XT_WINXP_COMPAT      = 0x1000; // Uses Windows XP themes if available.  This setting overrides BS_XT_FLAT and BS_XT_SEMIFLAT, but <i>does not</i> override BS_XT_XPFLAT.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTBrowseEdit"
//////////////////////////////////////////////////////////////////////////

const UINT BES_XT_CHOOSEDIR        = 0x0001; // Display the choose folder dialog.
const UINT BES_XT_CHOOSEFILE       = 0x0002; // Display the choose file dialog.
const UINT BES_XT_POPUPMENU        = 0x0004; // Display a user defined context menu.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTColorPopup"
//////////////////////////////////////////////////////////////////////////

const UINT CPS_XT_NOFILL           = 0x0001; // Use No Fill style in place of Automatic.
const UINT CPS_XT_EXTENDED         = 0x0002; // Display extended colors.
const UINT CPS_XT_MORECOLORS       = 0x0004; // Display more colors button.
const UINT CPS_XT_USERCOLORS       = 0x0020; // Display a list of user selected colors.

// *** Used internally by the color popup window.

const UINT CPS_XT_PICKBOX          = 0x0008; 
const UINT CPS_XT_DEFAULT          = 0x0010; 

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTColorDialog"
//////////////////////////////////////////////////////////////////////////

const UINT CPS_XT_SHOWHEXVALUE     = 0x0020; // Displays color in hex format.
const UINT CPS_XT_SHOW3DSELECTION  = 0x0040; // Shows new / current display box with a 3D border.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTEditListBox"
//////////////////////////////////////////////////////////////////////////

const UINT LBS_XT_DEFAULT          = 0x0000; // Standard edit field.
const UINT LBS_XT_CHOOSEDIR        = 0x0001; // Choose directory browse edit field.
const UINT LBS_XT_CHOOSEFILE       = 0x0002; // Choose file browse edit field.
const UINT LBS_XT_NOTOOLBAR        = 0x0008; // Do not display edit toolbar.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTTipWindow"
//////////////////////////////////////////////////////////////////////////

const UINT TWS_XT_THICKBORDER      = 0x0001; // Show a 3D border.
const UINT TWS_XT_DROPSHADOW       = 0x0002; // Show a drop shadow when the window is active.
const UINT TWS_XT_ALPHASHADOW      = 0x0004; // Show alpha style drop shadow.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTCaption"
//////////////////////////////////////////////////////////////////////////

const UINT CPWS_EX_GROOVE_EDGE     = 0x0001; // Show the caption with a sunken border.
const UINT CPWS_EX_RAISED_EDGE     = 0x0002; // Show the caption with a raised border.
const UINT CPWS_EX_CLOSEBUTTON     = 0x0004; // Caption has a close button.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTFlatTabCtrl"
//////////////////////////////////////////////////////////////////////////

const UINT FTS_XT_BOTTOM           = 0x0001; // Show tabs on bottom.
const UINT FTS_XT_HASARROWS        = 0x0002; // Show back and next arrows.
const UINT FTS_XT_HASHOMEEND       = 0x0004; // Show home and end arrows.  Used with FTS_XT_HASARROWS.
const UINT FTS_XT_TOOLTIPS         = 0x0008; // Show tooltips.
const UINT FTS_XT_DEFAULT          = 0x000F; // Same as FTS_XT_BOTTOM | FTS_XT_HASARROWS | FTS_XT_HASHOMEEND | FTS_XT_TOOLTIPS.
const UINT FTS_XT_HSCROLL          = 0x0010; // Show a horizontal scroll bar.
const UINT FTS_XT_MASK             = 0x001F; // All FTS_ mask items.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTFrameWnd, CXTMDIFrameWnd, CXTOleIPFrameWnd"
//////////////////////////////////////////////////////////////////////////

const UINT CUST_XT_TOOLBARS        = 0x0001;																			 // Displays the Toolbars tab in the toolbar customize property sheet.
const UINT CUST_XT_COMMANDS        = 0x0002;																			 // Displays the Commands tab in the toolbar customize property sheet.
const UINT CUST_XT_KEYBOARD        = 0x0004;																			 // Displays the Keyboard tab in the toolbar customize property sheet.
const UINT CUST_XT_TOOLS           = 0x0008;																			 // Displays the Tools tab in the toolbar customize property sheet.
const UINT CUST_XT_OPTIONS         = 0x0010;																			 // Displays the Options tab in the toolbar customize property sheet.
const UINT CUST_XT_DEFAULT         = (CUST_XT_TOOLBARS|CUST_XT_COMMANDS|CUST_XT_KEYBOARD|CUST_XT_TOOLS|CUST_XT_OPTIONS); // Same as CUST_XT_TOOLBARS|CUST_XT_COMMANDS|CUST_XT_KEYBOARD|CUST_XT_TOOLS|CUST_XT_OPTIONS.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTOutBarCtrl"
//////////////////////////////////////////////////////////////////////////

const UINT OBS_XT_SMALLICON        = 0x0001; // Sets small icon mode.
const UINT OBS_XT_LARGEICON        = 0x0002; // Sets large icon mode.
const UINT OBS_XT_EDITGROUPS       = 0x0004; // Enables folder local editing (renaming).
const UINT OBS_XT_EDITITEMS        = 0x0008; // Enables item local editing (renaming).
const UINT OBS_XT_REMOVEGROUPS     = 0x0010; // Enables the "Remove" command for folders in context menu.
const UINT OBS_XT_REMOVEITEMS      = 0x0020; // Enables the "Remove" command for items in context menu.
const UINT OBS_XT_ADDGROUPS        = 0x0040; // Enables folder insertion.
const UINT OBS_XT_DRAGITEMS        = 0x0080; // Enables item dragging to rearrange position.
const UINT OBS_XT_ANIMATION        = 0x0100; // Enables animation while changing folder selection.
const UINT OBS_XT_SELHIGHLIGHT     = 0x0200; // Enables dimmed highlight of last pressed item.
const UINT OBS_XT_DEFAULT          = 0x00FC; // Same as OBS_XT_DRAGITEMS | OBS_XT_EDITGROUPS | OBS_XT_EDITITEMS | OBS_XT_REMOVEGROUPS | OBS_XT_REMOVEITEMS | OBS_XT_ADDGROUPS.

//////////////////////////////////////////////////////////////////////////
//:Associate with "Styles - CXTTreeCtrl"
//////////////////////////////////////////////////////////////////////////

const UINT TVIS_FOCUSED            = 0x0001; // For determining tree item focus state.

//:Associate with "Notification Handlers - CXTColorPicker"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			color selection has changed.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_CPN_XT_SELCHANGE(IDC_CTRL_ID, OnSelChange)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnSelChange()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnSelChange( );</b></pre>
const UINT CPN_XT_SELCHANGE = (WM_APP + 2500); 

//:Associate with "Notification Handlers - CXTColorPicker"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			color selection window is displayed.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_CPN_XT_DROPDOWN(IDC_CTRL_ID, OnDropDown)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//
//			<pre>void CAppDialog::OnDropDown()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnDropDown( );</b></pre>
const UINT CPN_XT_DROPDOWN = (WM_APP + 2501); 

//:Associate with "Notification Handlers - CXTColorPicker"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			color selection window has closed.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_CPN_XT_CLOSEUP(IDC_CTRL_ID, OnCloseUp)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnCloseUp()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnCloseUp( );</b></pre>
const UINT CPN_XT_CLOSEUP = (WM_APP + 2502); 

//:Associate with "Notification Handlers - CXTColorPicker"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever a
//			color selection has been made.
//
// Example:	Here is an example of how an application would handle this message. 
// 
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_CPN_XT_SELENDOK(IDC_CTRL_ID, OnSelEndOk)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnSelEndOk()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnSelEndOk( );</b></pre>
const UINT CPN_XT_SELENDOK = (WM_APP + 2503); 

//:Associate with "Notification Handlers - CXTColorPicker"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever a
//			color selection has been canceled.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_CPN_XT_SELENDCANCEL(IDC_CTRL_ID, OnSelEndCancel)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnSelEndCancel()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnSelEndCancel( );</b></pre>
const UINT CPN_XT_SELENDCANCEL = (WM_APP + 2504); 

//:Associate with "Notification Handlers - CXTColorPicker"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			no fill / automatic color selection has been made.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_CPN_XT_SELNOFILL(IDC_CTRL_ID, OnSelNoFill)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnSelNoFill()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnSelNoFill( );</b></pre>
const UINT CPN_XT_SELNOFILL = (WM_APP + 2505); 

#define ON_CPN_XT_SELCHANGE(id, memberFxn) \
    ON_CONTROL(CPN_XT_SELCHANGE, id, memberFxn)
#define ON_CPN_XT_DROPDOWN(id, memberFxn) \
    ON_CONTROL(CPN_XT_DROPDOWN, id, memberFxn)
#define ON_CPN_XT_CLOSEUP(id, memberFxn) \
    ON_CONTROL(CPN_XT_CLOSEUP, id, memberFxn)
#define ON_CPN_XT_SELENDOK(id, memberFxn) \
    ON_CONTROL(CPN_XT_SELENDOK, id, memberFxn)
#define ON_CPN_XT_SELENDCANCEL(id, memberFxn) \
    ON_CONTROL(CPN_XT_SELENDCANCEL, id, memberFxn)
#define ON_CPN_XT_SELNOFILL(id, memberFxn) \
    ON_CONTROL(CPN_XT_SELNOFILL, id, memberFxn)

//:Associate with "Notification Handlers - CXTBrowseEdit"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			label edit operation has ended.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_BEN_XT_LABELEDITEND(IDC_CTRL_ID, OnLabelEditEnd)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnLabelEditEnd()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnLabelEditEnd( );</b></pre>
const UINT BEN_XT_LABELEDITEND = (WM_APP + 2506); 

//:Associate with "Notification Handlers - CXTBrowseEdit"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			label edit operation has been canceled.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_BEN_XT_LABELEDITCANCEL(IDC_CTRL_ID, OnLabelEditCancel)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnLabelEditCancel()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnLabelEditCancel( );</b></pre>
const UINT BEN_XT_LABELEDITCANCEL = (WM_APP + 2507); 

#define ON_BEN_XT_LABELEDITEND(id, memberFxn) \
    ON_CONTROL(BEN_XT_LABELEDITEND, id, memberFxn)
#define ON_BEN_XT_LABELEDITCANCEL(id, memberFxn) \
    ON_CONTROL(BEN_XT_LABELEDITCANCEL, id, memberFxn)

//:Associate with "Notification Handlers - CXTEditListBox"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			label edit operation has ended.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_LBN_XT_LABELEDITEND(IDC_CTRL_ID, OnLabelEditEnd)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnLabelEditEnd()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnLabelEditEnd( );</b></pre>
const UINT LBN_XT_LABELEDITEND = BEN_XT_LABELEDITEND; 

//:Associate with "Notification Handlers - CXTEditListBox"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			label edit operation has been canceled.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_LBN_XT_LABELEDITCANCEL(IDC_CTRL_ID, OnLabelEditCancel)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnLabelEditCancel()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnLabelEditCancel( );</b></pre>
const UINT LBN_XT_LABELEDITCANCEL = BEN_XT_LABELEDITCANCEL; 

//:Associate with "Notification Handlers - CXTEditListBox"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			new item button has been pressed.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_LBN_XT_NEWITEM(IDC_CTRL_ID, OnNewItem)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnNewItem()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnNewItem( );</b></pre>
const UINT LBN_XT_NEWITEM = (WM_APP + 2508); 

//:Associate with "Notification Handlers - CXTEditListBox"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner just before an item
//			is deleted.  This is useful for retrieving information about the selected
//			item, such as item data, before it is removed from the edit list control.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_LBN_XT_PREDELETEITEM(IDC_CTRL_ID, OnPreDeleteItem)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnPreDeleteItem()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnPreDeleteItem( );</b></pre>
const UINT LBN_XT_PREDELETEITEM = (WM_APP + 2509); 

//:Associate with "Notification Handlers - CXTEditListBox"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			delete item button has been pressed.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_LBN_XT_DELETEITEM(IDC_CTRL_ID, OnDeleteItem)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnDeleteItem()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnDeleteItem( );</b></pre>
const UINT LBN_XT_DELETEITEM = (WM_APP + 2510); 

//:Associate with "Notification Handlers - CXTEditListBox"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			move item up button has been pressed.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_LBN_XT_MOVEITEMUP(IDC_CTRL_ID, OnMoveItemUp)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnMoveItemUp()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnMoveItemUp( );</b></pre>
const UINT LBN_XT_MOVEITEMUP = (WM_APP + 2511); 

//:Associate with "Notification Handlers - CXTEditListBox"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			move item down button has been pressed.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_LBN_XT_MOVEITEMDOWN(IDC_CTRL_ID, OnMoveItemDown)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnMoveItemDown()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnMoveItemDown( );</b></pre>
const UINT LBN_XT_MOVEITEMDOWN = (WM_APP + 2512); 

#define ON_LBN_XT_LABELEDITEND(id, memberFxn) \
    ON_CONTROL(LBN_XT_LABELEDITEND, id, memberFxn)
#define ON_LBN_XT_LABELEDITCANCEL(id, memberFxn) \
    ON_CONTROL(LBN_XT_LABELEDITCANCEL, id, memberFxn)
#define ON_LBN_XT_NEWITEM(id, memberFxn) \
    ON_CONTROL(LBN_XT_NEWITEM, id, memberFxn)
#define ON_LBN_XT_PREDELETEITEM(id, memberFxn) \
    ON_CONTROL(LBN_XT_PREDELETEITEM, id, memberFxn)
#define ON_LBN_XT_DELETEITEM(id, memberFxn) \
    ON_CONTROL(LBN_XT_DELETEITEM, id, memberFxn)
#define ON_LBN_XT_MOVEITEMUP(id, memberFxn) \
    ON_CONTROL(LBN_XT_MOVEITEMUP, id, memberFxn)
#define ON_LBN_XT_MOVEITEMDOWN(id, memberFxn) \
    ON_CONTROL(LBN_XT_MOVEITEMDOWN, id, memberFxn)

//:Associate with "Notification Handlers - CXTTrayIcon"

/////////////////////////////////////////////////////////////////////////////
// Input:	uID - Value of wParam specifies the resource ID of the icon associated with the
//			CXTTrayIcon object.
//			uMouseMsg - Value of lParam specifies the mouse or keyboard message associated with the event.
//
// Returns:	If the application is to process this message, the return value should be 0.
//
// Remarks:	When a mouse or keyboard event occurs on a tray icon, the TIN_XT_TRAYICON
//			message is sent to the tray icon's owner window. 
//
// Example:	Here is an example of how an application would process the TIN_XT_TRAYICON 
//			message. 
//
//			<pre>BEGIN_MESSAGE_MAP(CMainFrame, CXTMDIFrameWnd)
//			    //{{AFX_MSG_MAP(CMainFrame)
//			    ON_MESSAGE(TIN_XT_TRAYICON, OnTrayIconNotify)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>LRESULT CMainFrame::OnTrayIconNotify(WPARAM wParam, LPARAM lParam) 
//			{
//			    UINT uID = (UINT) wParam;        // resource ID of the tray icon.
//			    UINT uMouseMsg = (UINT) lParam;  // mouse message that was sent.
//				
//			    switch( uMouseMsg )
//			    {
//			        case WM_RBUTTONUP:
//			        {
//			            CMenu menu;
//			            VERIFY(menu.LoadMenu(IDR_MAINFRAME));
//						
//			            CMenu* pPopup = menu.GetSubMenu(0);
//			            ASSERT(pPopup != NULL);
//			            CWnd* pWndPopupOwner = this;
//						
//			            // Insert the restore menu command into the popup.
//			            if (m_bIsVisible == false)
//			            {
//			                pPopup->InsertMenu(0, MF_BYPOSITION,
//			                    IDR_RESTORE, _T("&Restore Window..."));
//			
//			                pPopup->InsertMenu(1, MF_BYPOSITION|MF_SEPARATOR,
//			                    IDR_RESTORE);
//			
//			                // Make restore command bold.
//			                ::SetMenuDefaultItem(pPopup->m_hMenu, IDR_RESTORE, FALSE);
//			            }
//			            else
//			            {
//			                // Make the exit command bold.
//			                ::SetMenuDefaultItem(pPopup->m_hMenu, ID_APP_EXIT, FALSE);
//			            }
//			
//			            // Display the menu at the current mouse location. There's a "bug"
//			            // (Microsoft calls it a feature) in Windows 95 that requires calling
//			            // SetForegroundWindow. To find out more, search for Q135788 in MSDN.
//			            //
//			            CPoint point;
//			            GetCursorPos( &point );
//			            ::SetForegroundWindow(m_hWnd);
//			
//			            while (pWndPopupOwner->GetStyle() & WS_CHILD)
//			                pWndPopupOwner = pWndPopupOwner->GetParent();
//			
//			            int iCmd = pPopup->TrackPopupMenu(
//			                TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
//			                point.x, point.y, pWndPopupOwner);
//			
//			            // at this point we want to make sure that the app is visible
//			            // after the user makes a menu selection - this is just one way
//			            // of doing so:
//			
//			            switch (iCmd)
//			            {
//			                case 0:
//			                    // user cancelled menu - do nothing
//			                    break;
//			
//			                // these are commands for which we don't need to make the
//			                // main app window visible
//			                case ID_APP_EXIT:
//			                    PostMessage(WM_COMMAND, iCmd, 0);  // just post the command
//			                    break;
//			
//			                // for all other menu commands - make sure the window is
//			                // visible before we continue
//			                default:
//			                    // make main window visible
//			                    OnRestore();
//			                    PostMessage(WM_COMMAND, iCmd, 0); // post the command
//			                    break;
//			            }
//			
//			            return 1; // let the tray icon know that we handled this message.
//			        }
//			
//			        case WM_LBUTTONDBLCLK:
//			            OnRestore();
//			            return 1; // let the tray icon know that we handled this message.
//			    }
//			
//			    return 0;
//			}</pre>
// Summary: The TIN_XT_TRAYICON message is sent to the CXTTrayIcon window's owner
//			whenever a mouse event occurs in the CXTTrayIcon. 
//   
//			<pre>TIN_XT_TRAYICON 
//			uID = (UINT) wParam;        // resource ID of the tray icon.
//			uMouseMsg = (UINT) lParam;  // mouse message that was sent.</pre>
const UINT TIN_XT_TRAYICON = (WM_APP + 2513); 

//:Associate with "Notification Handlers - CXTControlBar"

/////////////////////////////////////////////////////////////////////////////
// Input:	pBar - Value of wParam specifies a CXTControlBar object that represents the toolbar
//			where the context menu event occurred.
//  
// Returns:	If the application is to process this message, the return value should be 0.
//
// Remarks:	When the user right clicks on a control bar, the CBRN_XT_CONTEXTMENU
//			message is sent to the control bar's owner window. 
//
// Example:	Here is an example of how an application would process the CBRN_XT_CONTEXTMENU 
//			message. 
//
//			<pre>BEGIN_MESSAGE_MAP(CMainFrame, CXTMDIFrameWnd)
//			    //{{AFX_MSG_MAP(CMainFrame)
//			    ON_MESSAGE(CBRN_XT_CONTEXTMENU, OnBarContextMenu)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>LRESULT CMainFrame::OnBarContextMenu(WPARAM wParam, LPARAM lParam)
//			{
//			    UNREFERENCED_PARAMETER(lParam);
//			
//			    CXTControlBar* pBar = (CXTControlBar*)wParam;
//			    ASSERT_KINDOF(CXTControlBar, pBar);
//			
//			    return 0;
//			}</pre>
//
// Summary: The CBRN_XT_CONTEXTMENU message is sent to the CXTControlBar window's owner
//			whenever a context menu event occurs in the CXTControlBar. 
//   
//			<pre>CBRN_XT_CONTEXTMENU 
//			pBar = (NMTOOLBAR*)wParam;  // pointer to the control bar context menu owner</pre>
const UINT CBRN_XT_CONTEXTMENU = (WM_APP + 2514); 

//:Associate with "Notification Handlers - CXTToolBar"

/////////////////////////////////////////////////////////////////////////////
// Input:	pNMTB - Value of wParam specifies a NMTOOLBAR structure that represents the toolbar
//			Button where the dropdown event occurred.
//
//			pRect - Value of lParam points to a CRect object that represents the size and location
//			of the toolbar button where the dropdown event occurred.
//
// Returns:	Dropdown is displaying a modal window, the return value should be 0, otherwise
//			return 1 for menu style displays.
//
// Remarks:	When the user clicks on a dropdown arrow for a toolbar button, the CBRN_XT_DROPDOWN
//			message is sent to the toolbar's owner window. 
//
// Example:	Here is an example of how an application would process the CBRN_XT_DROPDOWN 
//			message. 
//
//			<pre>BEGIN_MESSAGE_MAP(CMainFrame, CXTMDIFrameWnd)
//			    //{{AFX_MSG_MAP(CMainFrame)
//			    ON_MESSAGE(CBRN_XT_DROPDOWN, OnDropDown)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>LRESULT CMainFrame::OnDropDown(WPARAM wParam, LPARAM lParam)
//			{
//			    NMTOOLBAR* pNMTB = (NMTOOLBAR*)wParam;
//			    ASSERT(pNMTB != NULL);
//			
//			    CRect* pRect = (CRect*)lParam;
//			    ASSERT(pRect != NULL);
//			
//			    // TODO: Handle toolbar dropdown notification (click on REDO drop arrow).
//			
//			    return 0;
//			}</pre>
// Summary: The CBRN_XT_DROPDOWN message is sent to the CXTToolBar window 
//			whenever a dropdown event occurs in the CXTToolBar. 
//   
//			<pre>CBRN_XT_DROPDOWN 
//			pNMTB = (NMTOOLBAR*)wParam;  // pointer to a NMTOOLBAR struct 
//			pRect = (CRect*)lParam;      // pointer to a CRect object</pre>
const UINT CBRN_XT_DROPDOWN = (WM_APP + 2515); 

//:Associate with "Notification Handlers - CXTCaptionPopupWnd"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	When the user makes a selection of the push pin button, the CPWN_XT_PUSHPINBUTTON message is 
//			sent to the caption popup window's owner window. 
//
// Example:	Here is an example of how an application would process the TCN_XT_SELCHANGE 
//			message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CMainFrame, CXTMDIFrameWnd)
//			    //{{AFX_MSG_MAP(CMainFrame)
//			    ON_MESSAGE_VOID(CPWN_XT_PUSHPINBUTTON, OnPushPinButton)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CMainFrame::OnPushPinButton()
//			{
//			    // TODO: Add your message handler code here and/or call default
//			    m_wndSplitter2.ShowColumn();
//			    m_nColumn = 1;
//			}</pre>
// Summary: The CPWN_XT_PUSHPINBUTTON message is sent to the owner of a CXTCaptionPopupWnd whenever
//			the push pin button selection has been made.
const UINT CPWN_XT_PUSHPINBUTTON = (WM_APP + 2516); 

//:Associate with "Notification Handlers - CXTCaptionPopupWnd"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	When the user cancels a selection of the push pin button, the CPWN_XT_PUSHPINCANCEL message is 
//			sent to the caption popup window's owner window. 
//
// Example:	Here is an example of how an application would process the TCN_XT_SELCHANGE 
//			message. 
//
//			<pre>BEGIN_MESSAGE_MAP(CMainFrame, CXTMDIFrameWnd)
//			    //{{AFX_MSG_MAP(CMainFrame)
//			    ON_MESSAGE_VOID(CPWN_XT_PUSHPINCANCEL, OnPushPinCancel)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CMainFrame::OnPushPinCancel()
//			{
//			    // TODO: Add your message handler code here and/or call default
//			}</pre>
// Summary: The CPWN_XT_PUSHPINCANCEL message is sent to the owner of a CXTCaptionPopupWnd whenever
//			the push pin button selection has been canceled.
const UINT CPWN_XT_PUSHPINCANCEL = (WM_APP + 2517); 

//:Associate with "Notification Handlers - CXTOutBarCtrl"

// BULLETED LIST:

/////////////////////////////////////////////////////////////////////////////
// Input:	nBarAction - Value of wParam specifies an Outlook bar value that indicates the users 
//			request. This parameter can be one of the following values:
//			[ul]
//			[li]<b>OBN_XT_ITEMCLICK</b> The user has selected an item in the
//			Outlook bar.[/li]
//			[li]<b>OBN_XT_ONLABELENDEDIT</b> The user has completed editing an
//			item's label.[/li]
//			[li]<b>OBN_XT_ONGROUPENDEDIT</b> The user has completed editing a
//			folder's label.[/li]
//			[li]<b>OBN_XT_DRAGITEM</b> The user has dragged an item to a new
//			location in the Outlook bar.[/li]
//			[li]<b>OBN_XT_FOLDERCHANGE</b> The user has selected a new folder
//			item.[/li]
//			[li]<b>OBN_XT_ITEMHOVER</b> The user's mouse is hovering over an item
//			in the Outlook bar.[/li]
//			[li]<b>OBN_XT_DELETEITEM</b> The user has chosen to delete an item
//			from the Outlook bar.[/li]
//			[li]<b>OBN_XT_DELETEFOLDER</b> The user has chosen to delete a folder
//			from the Outlook bar.[/li]
//			[li]<b>OBN_XT_ITEMRCLICK</b> The user has right clicked on the Outlook
//			bar folder.[/li]
//			[/ul]
//			pOBInfo - Value of lParam points to an XT_OUTBAR_INFO structure that contains information for the 
//			specified item. The item can either be a folder group or icon item, depending on the 
//			value of the nBarAction parameter.  This pointer should <b>never</b> be NULL.
//  
// Returns:	If the application is to process this message, the return value should be TRUE. If the 
//			return value is FALSE, the user's action is ignored. 
//
// Remarks:	When the user performs an action in the Outlook bar, the XTWM_OUTBAR_NOTIFY message is 
//			sent to the Outlook bar's owner window. 
//
// Example:	Here is an example of how an application would process the XTWM_OUTBAR_NOTIFY 
//			message. 
//
//			<pre>int nBarAction = (int)wParam;</pre>
//			 
//			<pre>// Cast the lParam to an XT_OUTBAR_INFO* struct pointer.
//			XT_OUTBAR_INFO* pOBInfo = (XT_OUTBAR_INFO*)lParam;
//			ASSERT(pOBInfo);</pre>
//			
//			<pre>switch (nBarAction)
//			{
//			    case OBN_XT_ITEMCLICK:
//			    case OBN_XT_FOLDERCHANGE:
//			    case OBN_XT_ONLABELENDEDIT:
//			    case OBN_XT_ONGROUPENDEDIT:
//			    case OBN_XT_DRAGITEM:
//			    case OBN_XT_ITEMHOVER:
//			    case OBN_XT_ITEMRCLICK:
//			        TRACE2( "Index: %d, Name: %s.\n", pOBInfo->nIndex, pOBInfo->lpszText);
//			        break;
//			
//			    case OBN_XT_DELETEITEM:
//			        if (AfxMessageBox(_T("Remove this folder shortcut?"),
//			            MB_ICONWARNING|MB_YESNO) == IDNO)
//			        {
//			            return FALSE; // Return FALSE to abort the action.
//			        }
//			        break;
//			
//			    case OBN_XT_DELETEFOLDER:
//			        if (AfxMessageBox(_T("Remove the specified folder?"),
//			            MB_ICONWARNING|MB_YESNO) == IDNO)
//			        {
//			            return FALSE; // Return FALSE to abort the action.
//			        }
//			        break;
//			}
//			return TRUE;</pre>
// Summary: The XTWM_OUTBAR_NOTIFY message is sent to the CXTOutBarCtrl owner window 
//			whenever an action occurs within the CXTOutBarCtrl. 
//   
//			<pre>XTWM_OUTBAR_NOTIFY 
//			nBarAction = (int) wParam;           // Outlook bar action 
//			pOBInfo = (XT_OUTBAR_INFO*) lParam;  // pointer to an XT_OUTBAR_INFO struct</pre>
const UINT XTWM_OUTBAR_NOTIFY = (WM_APP + 2518); 
const UINT OBN_XT_ITEMCLICK        = 1;  // Item was selected.  See XTWM_OUTBAR_NOTIFY.
const UINT OBN_XT_ONLABELENDEDIT   = 2;  // Item label edit completed.  See XTWM_OUTBAR_NOTIFY.
const UINT OBN_XT_ONGROUPENDEDIT   = 3;  // Folder label edit completed.  See XTWM_OUTBAR_NOTIFY.
const UINT OBN_XT_DRAGITEM         = 4;  // Item drag operation in process.  See XTWM_OUTBAR_NOTIFY.
const UINT OBN_XT_FOLDERCHANGE     = 5;  // Folder selection changed.  See XTWM_OUTBAR_NOTIFY.
const UINT OBN_XT_ITEMHOVER        = 6;  // Mouse is hovering over item.  See XTWM_OUTBAR_NOTIFY.
const UINT OBN_XT_DELETEITEM       = 7;  // Item deleted.  See XTWM_OUTBAR_NOTIFY.
const UINT OBN_XT_DELETEFOLDER     = 8;  // Folder deleted.  See XTWM_OUTBAR_NOTIFY.
const UINT OBN_XT_BEGINDRAG        = 9;  // Item drag operation begun.  See XTWM_OUTBAR_NOTIFY.
const UINT OBN_XT_ITEMRCLICK       = 10; // Right click on Outlook bar occurred.  See XTWM_OUTBAR_NOTIFY.

//:Associate with "Notification Handlers - CXTShellListCtrl, CXTShellListView, CXTShellTreeCtrl, CXTShellTreeView"

// BULLETED LIST:

/////////////////////////////////////////////////////////////////////////////
// Input:	nShellAction - Value of wParam specifies a shell tree value that indicates the users 
//			request. This parameter can be one of the following values:
//			[ul]
//			[li]<b>SHN_XT_SHELLMENU</b> Shell context menu selection made.[/li]
//			[li]<b>SHN_XT_TREESELCHANGE</b> Tree selection made.[/li]
//			[li]<b>SHN_XT_SELECTCHILD</b> Child tree node selected.[/li]
//			[li]<b>SHN_XT_NOFOLDER</b> Item selected was not a folder.[/li]
//			[li]<b>SHN_XT_INETFOLDER</b> Item selected was the internet folder.[/li]
//			[/ul]
//			pItemData - Value of lParam points to an XT_TVITEMDATA structure that contains information for the 
//			specified item. Depending on the action, this pointer can be NULL.
//
// Returns:	If the application is to process this message, the return value should be 0.
//
// Remarks:	When the user performs an action in the shell tree, the XTWM_SHELL_NOTIFY message is 
//			sent to the shell tree's owner window. 
//
// Example:	Here is an example of how an application would process the XTWM_SHELL_NOTIFY 
//			message. 
//
//			<pre>int nShellAction = (int)wParam;</pre>
//			 
//			<pre>// Cast the lParam to an XT_TVITEMDATA* struct pointer.
//			XT_TVITEMDATA* pItemData = (XT_TVITEMDATA*)lParam;
//			ASSERT(pItemData);</pre>
//			
//			<pre>switch (nBarAction)
//			{
//			    case SHN_XT_SHELLMENU:
//			    case SHN_XT_TREESELCHANGE:
//			    case SHN_XT_SELECTCHILD:
//			    case SHN_XT_NOFOLDER:
//			    case SHN_XT_INETFOLDER:
//			        // TODO: Handle shell notification message.
//			        TRACE0("Shell notification was sent.\n");
//			        break;
//			}
//			return 0;</pre>
// Summary: The XTWM_SHELL_NOTIFY message is sent to the CXTShellTree and list owner window 
//			whenever an action occurs within the CXTShellTree and list. 
//			<pre>XTWM_SHELL_NOTIFY 
//			nShellAction = (int) wParam;         // tree or list action 
//			pItemData = (XT_TVITEMDATA*) lParam;  // pointer to an XT_TVITEMDATA struct</pre>
const UINT XTWM_SHELL_NOTIFY = (WM_APP + 2519); 
const UINT SHN_XT_SHELLMENU        = 1; // Shell context menu selection made.  See XTWM_SHELL_NOTIFY
const UINT SHN_XT_TREESELCHANGE    = 2; // Tree selection made.  See XTWM_SHELL_NOTIFY.
const UINT SHN_XT_SELECTCHILD      = 3; // Child tree node selected.  See XTWM_SHELL_NOTIFY.
const UINT SHN_XT_NOFOLDER         = 4; // Item selected was not a folder.  See XTWM_SHELL_NOTIFY.
const UINT SHN_XT_INETFOLDER       = 5; // Item selected was the internet folder.  See XTWM_SHELL_NOTIFY.

//:Associate with "Notification Handlers - CXTTabCtrl"

/////////////////////////////////////////////////////////////////////////////
// Input:	nID - Dialog control ID for the tab control.
//			pTabCtrl - Points to the tab control whose selection has changed.
//
// Remarks:	When the user selects a new tab in the tab control, the TCN_XT_SELCHANGE message is 
//			sent to the tab control's owner window. 
//
// Example:	Here is an example of how an application would process the TCN_XT_SELCHANGE 
//			message. 
//
//			<pre>BEGIN_MESSAGE_MAP(CMainFrame, CXTMDIFrameWnd)
//			    //{{AFX_MSG_MAP(CMainFrame)
//			    ON_MESSAGE(TCN_XT_SELCHANGE, OnTabSelChange)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>LRESULT CMainFrame::OnTabSelChange(WPARAM wParam, LPARAM lParam)
//			{
//			    CXTTabCtrl* pTabCtrl = (CXTTabCtrl*)lParam;
//			    ASSERT_VALID(pTabCtrl);
//			
//			    int nID = (int)wParam;
//			    if (nID == ID_VIEW_WORKSPACEBAR)
//			    {
//				      // TODO: handle TCN_SELCHANGE from tab control bar.
//			    }
//			    return 0;
//			}</pre>
// Summary: The TCN_XT_SELCHANGE message is sent to the owner of a CXTTabCtrl whenever
//			the tab selection has changed.
//   
//			<pre>TCN_XT_SELCHANGE 
//			nID = (int) wParam;               // dialog control ID 
//			pTabCtrl = (CXTTabCtrl*) lParam;  // pointer to a CXTTabCtrl object</pre>
const UINT TCN_XT_SELCHANGE = (WM_APP + 2520); 

//:Associate with "Notification Handlers - CXTTabCtrl"

/////////////////////////////////////////////////////////////////////////////
// Input:	nID - Dialog control ID for the tab control.
//			pTabCtrl - Points to the tab control whose selection is about to change.
//
// Remarks:	When the user selects a new tab in the tab control, the TCN_XT_SELCHANGING message is 
//			sent to the tab control's owner window. 
//
// Example:	Here is an example of how an application would process the TCN_XT_SELCHANGING 
//			message. 
//
//			<pre>BEGIN_MESSAGE_MAP(CMainFrame, CXTMDIFrameWnd)
//			    //{{AFX_MSG_MAP(CMainFrame)
//			    ON_MESSAGE(TCN_XT_SELCHANGING, OnTabSelChanging)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>LRESULT CMainFrame::OnTabSelChanging(WPARAM wParam, LPARAM lParam)
//			{
//			    CXTTabCtrl* pTabCtrl = (CXTTabCtrl*)lParam;
//			    ASSERT_VALID(pTabCtrl);
//			
//			    int nID = (int)wParam;
//			
//			    if (nID == ID_VIEW_WORKSPACEBAR)
//			    {
//				      // TODO: handle TCN_SELCHANGE from tab control bar.
//			    }
//			    return 0;
//			}</pre>
// Summary: The TCN_XT_SELCHANGING message is sent to the owner of a CXTTabCtrl whenever
//			the tab selection is about to change.
//   
//			<pre>TCN_XT_SELCHANGING 
//			nID = (int) wParam;               // dialog control ID 
//			pTabCtrl = (CXTTabCtrl*) lParam;  // pointer to a CXTTabCtrl object</pre>
const UINT TCN_XT_SELCHANGING = (WM_APP + 2521); 

//:Associate with "Notification Handlers - CXTSearchOptionsCtrl"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			search options control is expanding.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_SON_XT_ITEMEXPANDING(IDC_CTRL_ID, OnItemExpanding)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnItemExpanding()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnItemExpanding( );</b></pre>
const UINT SON_XT_ITEMEXPANDING = (WM_APP + 2522); 

//:Associate with "Notification Handlers - CXTSearchOptionsCtrl"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			search options control has expanded.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_SON_XT_ITEMEXPAND(IDC_CTRL_ID, OnItemExpand)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnItemExpand()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnItemExpand( );</b></pre>
const UINT SON_XT_ITEMEXPAND = (WM_APP + 2523); 

#define ON_SON_XT_ITEMEXPANDING(id, memberFxn) \
    ON_CONTROL(SON_XT_ITEMEXPANDING, id, memberFxn)
#define ON_SON_XT_ITEMEXPAND(id, memberFxn) \
    ON_CONTROL(SON_XT_ITEMEXPAND, id, memberFxn)

//:Associate with "Notification Handlers - CXTSearchOptionsCtrl"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			search options control is contracting.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_SON_XT_ITEMCONTRACTING(IDC_CTRL_ID, OnItemContracting)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnItemContracting()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary:  <pre>function prototype: <b>afx_msg void OnItemContracting( );</b></pre>
const UINT SON_XT_ITEMCONTRACTING = (WM_APP + 2524); 

//:Associate with "Notification Handlers - CXTSearchOptionsCtrl"

/////////////////////////////////////////////////////////////////////////////
// Remarks:	This command handler is called to inform the owner whenever the
//			search options control has contracted.
//
// Example:	Here is an example of how an application would handle this message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CAppDialog, CDialog
//			    //{{AFX_MSG_MAP(CAppDialog)
//			    ON_SON_XT_ITEMCONTRACT(IDC_CTRL_ID, OnItemContract)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>void CAppDialog::OnItemContract()
//			{
//			    // TODO: Handle command.
//			}</pre>
// Summary: <pre>function prototype: <b>afx_msg void OnItemContract( );</b></pre>
const UINT SON_XT_ITEMCONTRACT = (WM_APP + 2525); 

#define ON_SON_XT_ITEMCONTRACTING(id, memberFxn) \
    ON_CONTROL(SON_XT_ITEMCONTRACTING, id, memberFxn)
#define ON_SON_XT_ITEMCONTRACT(id, memberFxn) \
    ON_CONTROL(SON_XT_ITEMCONTRACT, id, memberFxn)

//:Associate with "Notification Handlers - General"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Summary: This message handler is used to send initial update notifications to
//			controls when they are created. This message is primarily used by the
//			toolkit during initialization, however you can use this message to initialize
//			your own control as well.
const UINT XTWM_INITIAL_UPDATE = (WM_APP + 2526); 

///////////////////////////////////////////////////////////////////////////////////////////////////
// Notification message: toolbar expansion popup window has closed.
// NB: This is a private XT toolkit library message subject to changes.

const UINT XTWM_POPUPCLOSED = (WM_APP + 2527);

//:Associate with "Notification Handlers - CXTReBar"

/////////////////////////////////////////////////////////////////////////////
// Input:	bHorz - TRUE if horizontally oriented, FALSE otherwise.
//			pRBBI - A pointer to a BARBANDINFO structure, the window can modify its parameters as appropriate.
//
// Returns:	If the application is to process this message, the return value should be TRUE.
//
// Remarks:	When a window is added to a rebar, the XTWM_ADDREBAR message is sent to the window. 
//
// Example:	Here is an example of how an application would process the XTWM_ADDREBAR 
//			message. 
//
//			<pre>BEGIN_MESSAGE_MAP(CWidget, CWnd)
//			    //{{AFX_MSG_MAP(CWidget)
//			    ON_MESSAGE(XTWM_ADDREBAR, OnAddReBar)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>LRESULT CWidget::OnAddReBar(WPARAM wParam, LPARAM lParam) 
//			{
//			    BOOL bHorz = (BOOL) wParam;
//			
//			    REBARBANDINFO* pRBBI = (REBARBANDINFO*) lParam;
//			        // TODO: Handle message.
//			
//			    return TRUE;
//			}</pre>
// Summary: The XTWM_ADDREBAR message is sent to a window whenever it is added
//			to a CXTReBar control. 
//   
//			<pre>XTWM_ADDREBAR 
//			bHorz = (BOOL) wParam;            // TRUE if horizontal.
//			pRBBI = (REBARBANDINFO*) lParam;  // Points to a REBARBANDINFO structure.</pre>
const UINT XTWM_ADDREBAR = (WM_APP + 2528); 

//:Associate with "Notification Handlers - CXTReBar"

/////////////////////////////////////////////////////////////////////////////
// Input:	bHorz - TRUE if horizontally oriented, FALSE otherwise.
//			pRBSCI - A pointer to an XT_REBARSIZECHILDINFO structure, the window can modify its parameters as appropriate.
//
// Returns:	If the application is to process this message, the return value should be TRUE.
//
// Remarks:	When a window is added to a rebar, the XTWM_REBAR_SIZECHILD message is sent to the window. 
//
// Example:	Here is an example of how an application would process the XTWM_REBAR_SIZECHILD 
//			message. 
//
//			<pre>BEGIN_MESSAGE_MAP(CWidget, CWnd)
//			    //{{AFX_MSG_MAP(CWidget)
//			    ON_MESSAGE(XTWM_REBAR_SIZECHILD, OnReBarSizeChild)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>LRESULT CWidget::OnReBarSizeChild(WPARAM wParam, LPARAM lParam) 
//			{
//			    BOOL bHorz = (BOOL) wParam;
//			
//			    XT_REBARSIZECHILDINFO* pRBSCI = (XT_REBARSIZECHILDINFO*) lParam;
//			        // TODO: Handle message.
//			
//			    return TRUE;
//			}</pre>
// Summary: The XTWM_REBAR_SIZECHILD message is sent when CXTReBar control has resized 
//			the band in which the recipient resides.
//   
//			<pre>XTWM_REBAR_SIZECHILD 
//			bHorz = (BOOL) wParam; // TRUE if horizontal.
//			pRBSCI = (XT_REBARSIZECHILDINFO*) lParam;  // Points to an XT_REBARSIZECHILDINFO structure.</pre>
const UINT XTWM_REBAR_SIZECHILD = (WM_APP + 2529); 

//:Associate with "Notification Handlers - CXTDockContext"

/////////////////////////////////////////////////////////////////////////////
// Returns: TRUE if the control bar shall be fully visualized, FALSE if just a wire frame must be rendered.
//
// Remarks:	When a control bar is dragged the XTWM_QUERYVISUALIZE message is sent to the control bar. 
//
// Example:	Here is an example of how an application would process the XTWM_QUERYVISUALIZE 
//			message. 
//  
//			<pre>BEGIN_MESSAGE_MAP(CWidget, CXTControlBar)
//			    //{{AFX_MSG_MAP(CWidget)
//			    ON_MESSAGE(XTWM_QUERYVISUALIZE, OnQueryVisualize)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>LRESULT CWidget::OnQueryVisualize(WPARAM wParam, LPARAM lParam) 
//			{
//			    UNREFERENCED_PARAMETER( wParam );
//			    UNREFERENCED_PARAMETER( lParam );
//			    // Allow full visualizations.
//			    return TRUE;
//			}</pre>
// Summary: CXTDockContext sends the XTWM_QUERYVISUALIZE message to a control bar to 
//			check if the control bar shall be visualized while dragging or just a wire 
//			frame shall be rendered.
//   
//			<pre>XTWM_QUERYVISUALIZE 
//			wParam - not used, must be zero
//			lParam - not used, must be zero</pre>
const UINT XTWM_QUERYVISUALIZE = (WM_APP + 2530);

//:Associate with "Notification Handlers - CXTDockContext"

/////////////////////////////////////////////////////////////////////////////
// Returns: TRUE if control bar has a hot spot, FALSE if it does not.
//
// Remarks: CXTDockContext sends the XTWM_QUERYHOTSPOT message the control bar to check 
//			for a hot spot. 
//
// Example: Here is an example of how an application would process the XTWM_QUERYHOTSPOT 
//			message. 
//
//			<pre>BEGIN_MESSAGE_MAP(CWidget, CXTControlBar)
//			    //{{AFX_MSG_MAP(CWidget)
//			    ON_MESSAGE(XTWM_QUERYHOTSPOT, OnQueryHotSpot)
//			    //}}AFX_MSG_MAP
//			END_MESSAGE_MAP()</pre>
//			
//			<pre>LRESULT CWidget::OnQueryHotSpot(WPARAM wParam, LPARAM lParam) 
//			{
//			    UNREFERENCED_PARAMETER( wParam );
//			    LPSIZE lpSize = (LPSIZE) lParam;  // Points to a LPSIZE structure.
//			    // TODO: Handle message.
//			    return TRUE;
//			}</pre>
// Summary: CXTDockContext sends the XTWM_QUERYHOTSPOT message to a control bar to 
//			check if the control bar has a hot spot, a point that must match the position 
//			of the cursor.
//   
//			<pre>XTWM_QUERYHOTSPOT 
//			wParam - not used, must be zero
//			lpSize = (LPSIZE) lParam;  // IN  - Extent of the rectangle in which 
//			                           //       hot spot is to be defined
//			                           // OUT - Offset of the hot spot location 
//			                           //       within the rectangle.</pre>
const UINT XTWM_QUERYHOTSPOT = (WM_APP + 2531);

//:Associate with "Notification Handlers - CXTPropertyGrid"

// BULLETED LIST:

/////////////////////////////////////////////////////////////////////////////
// Input:	nGridAction - Value of wParam specifies an Outlook bar value that indicates the users 
//			request. This parameter can be one of the following values:
//			[ul]
//			[li]<b>XT_PGN_SORTORDER_CHANGED</b> The sort order has changed in the property grid.[/li]
//			[li]<b>XT_PGN_SELECTION_CHANGED</b> The selection has changed in the property grid.[/li]
//			[li]<b>XT_PGN_ITEMVALUE_CHANGED</b> The value has changed for pItem in the property grid.[/li]
//			[li]<b>XT_PGN_EDIT_CHANGED</b> The edit value has changed in the property grid.[/li]
//			[/ul]
//			pItem - Value of lParam points to an CXTPropertyGridItem object that contains information for the 
//			specified item. This pointer should <b>never</b> be NULL.
//  
// Returns: If the application is to process this message, the return value should be TRUE; otherwise the 
//			return value is FALSE. 
//
// Remarks: When the user performs an action in the property grid, the XTWM_PROPERTYGRID_NOTIFY message is 
//			sent to the property grid's owner window. 
//
// Example: Here is an example of how an application would process the XTWM_PROPERTYGRID_NOTIFY 
//			message. 
//
//			<pre>int nGridAction = (int)wParam;</pre>
//			 
//			<pre>// Cast the lParam to an CXTPropertyGridItem* object pointer.
//			CXTPropertyGridItem* pItem = (CXTPropertyGridItem*)lParam;
//			ASSERT(pItem);</pre>
//			
//			<pre>switch (nGridAction)
//			{
//			    case XT_PGN_SORTORDER_CHANGED:
//					  {
//			             m_nSort = m_wndXTPropertyGrid.GetPropertySort();
//			             UpdateData(FALSE);
//					  }
//			        break;
//			    case XT_PGN_SELECTION_CHANGED:
//					  {
//			             TRACE(_T("Value Changed. Caption = %s, ID = %i, Value = %s\n"),
//			                 pItem->GetCaption(), pItem->GetID(), pItem->GetValue());
//					  }
//			        break;
//			    case XT_PGN_ITEMVALUE_CHANGED:
//					  {
//			             TRACE(_T("Selection Changed. Item = %s\n"), pItem->GetCaption());
//					  }
//			        break;
//			}
//			return TRUE;</pre>
// Summary: The XTWM_PROPERTYGRID_NOTIFY message is sent to the CXTPropertyGrid owner window 
//			whenever an action occurs within the CXTPropertyGrid. 
//   
//			<pre>XTWM_PROPERTYGRID_NOTIFY 
//			nGridAction = (int) wParam;             // Property grid action 
//			pItem = (CXTPropertyGridItem*) lParam;  // pointer to an CXTPropertyGridItem object</pre>
const UINT XTWM_PROPERTYGRID_NOTIFY = (WM_APP + 2533);
const UINT XT_PGN_SORTORDER_CHANGED = 1; // The sort order has changed in the property grid.
const UINT XT_PGN_SELECTION_CHANGED = 2; // The selection has changed in the property grid.
const UINT XT_PGN_ITEMVALUE_CHANGED = 3; // The value has changed for pItem in the property grid.
const UINT  XT_PGN_EDIT_CHANGED		= 4; // The edit value has changed in the property grid.

// used internally - notifies rebar control that the ideal size of the 
// embedded control has changed.
// wParam - control's handle, HWND
// lParam - new ideal size, UINT
// Return value is ignored.

const UINT XTWM_IDEALSIZECHANGED = (WM_APP + 2534);

// Note: If your application supports docking controlbars, you should
//  not use the following IDs for your own controlbars.  We suggest that
//  you use 59500 (0xE86C) and higher for your control bar numbering.

#ifndef AFX_IDW_MENUBAR
#define AFX_IDW_MENUBAR                 0xE858
#endif

//////////////////////////////////////////////////////////////////////
// Summary: This macro will free the dynamically allocated memory specified by 
//			'p' and set its value to NULL. 
//#define SAFE_DELETE(p)  if(p) { delete p; p = NULL; }
#define _delete			SAFE_DELETE			 

//////////////////////////////////////////////////////////////////////
// Summary: This macro will free the dynamically allocated memory for an array 
//			specified by 'p' and set its value to NULL. 
#define SAFE_DELETE_AR(p)	if(p) { delete [] p; p = NULL; }
#define _deleteArray		SAFE_DELETE_AR

//////////////////////////////////////////////////////////////////////
// Summary: This macro will release the resources for the COM object specified 
//			by 'p' and set its value to NULL. 
//#define SAFE_RELEASE(p)		if(p) { (p)->Release(); (p)=NULL; }
 
/////////////////////////////////////////////////////////////////////////////
// UNICODE support definitions:

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// Windows 2000 or greater tray icon support definitions:

#ifdef NOTIFYICONDATA_V1_SIZE
#define SYSTRAY_USEW2K
#endif//NOTIFYICONDATA_V1_SIZE

#ifndef NIIF_NONE
#define NIIF_NONE				0x00000000
#endif//NIIF_NONE

#ifndef NIIF_INFO
#define NIIF_INFO				0x00000001
#endif//NIIF_INFO

#ifndef NIIF_WARNING
#define NIIF_WARNING			0x00000002
#endif//NIIF_WARNING

#ifndef NIIF_ERROR
#define NIIF_ERROR				0x00000003
#endif//NIIF_ERROR

#ifndef NIN_SELECT
#define NIN_SELECT				(WM_USER + 0)
#endif//NIN_SELECT

#ifndef NINF_KEY
#define NINF_KEY				0x1
#endif//NINF_KEY

#ifndef NIN_KEYSELECT
#define NIN_KEYSELECT			(NIN_SELECT | NINF_KEY)
#endif//NIN_KEYSELECT

#ifndef NIN_BALLOONSHOW
#define NIN_BALLOONSHOW			(WM_USER + 2)
#endif//NIN_BALLOONSHOW

#ifndef NIN_BALLOONHIDE
#define NIN_BALLOONHIDE			(WM_USER + 3)
#endif//NIN_BALLOONHIDE

#ifndef NIN_BALLOONTIMEOUT
#define NIN_BALLOONTIMEOUT		(WM_USER + 4)
#endif//NIN_BALLOONTIMEOUT

#ifndef NIN_BALLOONUSERCLICK
#define NIN_BALLOONUSERCLICK	(WM_USER + 5)
#endif//NIN_BALLOONUSERCLICK

#ifndef IDANI_CAPTION
#define IDANI_CAPTION			3
#endif//IDANI_CAPTION

/////////////////////////////////////////////////////////////////////////////
// VC5 support definitions:

#ifndef VERSION_WIN4
#define VERSION_WIN4                    MAKELONG(0, 4)
#endif
#ifndef VERSION_IE401
#define VERSION_IE401                   MAKELONG(72,4)
#endif
#ifndef CBRS_GRIPPER
#define CBRS_GRIPPER                    0x00400000L
#endif
#ifndef AFX_IDW_REBAR
#define AFX_IDW_REBAR                   0xE804
#endif
#ifndef ID_VIEW_REBAR
#define ID_VIEW_REBAR                   0xE804
#endif
#ifndef AFX_IDW_DIALOGBAR
#define AFX_IDW_DIALOGBAR               0xE805
#endif
#ifndef COLOR_GRADIENTACTIVECAPTION
#define COLOR_GRADIENTACTIVECAPTION     27
#endif
#ifndef COLOR_GRADIENTINACTIVECAPTION
#define COLOR_GRADIENTINACTIVECAPTION   28
#endif
#ifndef SPI_GETGRADIENTCAPTIONS
#define SPI_GETGRADIENTCAPTIONS         0x1008
#endif
#ifndef SPI_GETMENUUNDERLINES
#define SPI_GETMENUUNDERLINES           0x100A
#endif
#ifndef SPI_GETFLATMENU
#define SPI_GETFLATMENU                 0x1022
#endif
#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE              0x0040
#endif
#ifndef AFX_INLINE
#define AFX_INLINE inline
#endif
#ifndef AFX_STATIC
#define AFX_STATIC static
#endif
#ifndef BTNS_WHOLEDROPDOWN
#define BTNS_WHOLEDROPDOWN				0x80  /* draw dropdown arrow, but without split arrow section */
#endif
// Summary: The I_IMAGENONE constant is used when defining text-only toolbar buttons with no
//			space allocated to the button icon
//			The constant is normally defined in version 5.81 and greater SDK headers
#ifndef I_IMAGENONE
#define I_IMAGENONE (-2)
#endif//I_IMAGENONE

//////////////////////////////////////////////////////////////////////

#endif // #if !defined(__XTDEFINES_H__)