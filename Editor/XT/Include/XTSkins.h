// XTSkins.h : header file
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

#if !defined(__XTSKINS_H__)
#define __XTSKINS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// forwards

class CXTToolBar;
class CXTFlatEdit;
class CXTFlatComboBox;
class CXTCommandsListBox;

//////////////////////////////////////////////////////////////////////
// Summary: IXTFlatComboProxy is a stand alone interface class.  It is an API for
//			skin related operations for a flat combo.
interface IXTFlatComboProxy
{
	// Input:	pCombo - CXTFlatComboBox pointer to the combo to draw.
	//			pDC - CDC pointer to the device context to render on.
	//			bHilight - Flag that tells if the combo must be highlighted.
    // Summary:	This member function renders the appearance of the combo box.
    virtual void DrawCombo(CXTFlatComboBox* pCombo,CDC* pDC,BOOL bHilight) = 0;
};

//////////////////////////////////////////////////////////////////////
// Summary: IXTFlatEditProxy is a stand alone interface class.  It is an API for
//			skin related operations for a flat edit.
interface IXTFlatEditProxy
{

	// Input:	pCombo - CXTFlatEdit pointer to the combo to draw.
	//			pDC - CDC pointer to the device context to render on.
	//			bHilight - Flag that tells if the combo must be highlighted.
    // Summary:	This member function renders the appearance of the combo box.
    virtual void DrawEdit(CXTFlatEdit* pCombo,CDC* pDC,BOOL bHilight) = 0;
};

//////////////////////////////////////////////////////////////////////
// Summary: IXTToolbarProxy is a stand alone interface class.  It is used to create
//			an API for skin related operations for a toolbar item.
interface IXTToolbarProxy
{

	// Input:	pDC - CDC pointer to the device context to draw on.
	//			pOwner - CXTToolBar pointer to a toolbar whose item is being drawn.
	//			nIndex - Index of the item (a button) to draw.
    // Summary:	This member function fills in a rectangle allocated to a toolbar control.
    virtual void FillControlRect(CDC* pDC,CXTToolBar* pOwner,UINT nIndex) = 0;

	// Input:	pDC - CDC pointer to the device context to draw on.
	//			pOwner - CXTToolBar pointer to a toolbar whose item is being drawn.
	//			nIndex - Index of the item (a button) to draw.
	// Returns: TRUE if the item has been drawn, or returns FALSE otherwise.
    // Summary:	This member function draws a toolbar item.  
    virtual BOOL DrawButton(CDC* pDC,CXTToolBar* pOwner,UINT nIndex) = 0;

	// Input:	pDC - CDC pointer to the device context to draw on.
	//			pOwner - CXTToolBar pointer to a toolbar whose item is being drawn.
	//			nIndex - Index of the item (separator) to draw.
    // Summary:	This member function draws a toolbar separator.
    virtual void DrawSeparator(CDC* pDC,CXTToolBar* pOwner,UINT nIndex) = 0;

	// Input:	pDC - CDC pointer to the device context to draw on.
	//			pOwner - CXTToolBar pointer to a toolbar whose item is being drawn.
	//			nIndex - Index of the item (button) to draw.
    // Summary:	This member function draws special effects for a dropped item (shadow).
    virtual void DrawDroppedEffects(CDC* pDC,CXTToolBar* pOwner,UINT nIndex) = 0;

	// Input:	pOwner - CXTToolBar pointer to a toolbar that needs to be refreshed.
	//			nIndex - Index of the item (button) to refresh.
    // Summary:	This member function removes special effects around a selected item.
    virtual void RemoveDroppedEffects(CXTToolBar* pOwner,UINT nIndex) = 0;

};

//////////////////////////////////////////////////////////////////////
// Summary: IXTCommandsListBoxProxy is a stand alone interface class.  It is used to create
//			an API for skin related operations for the commands list box.
interface IXTCommandsListBoxProxy
{

	// Input:	lpDIS - LPDRAWITEMSTRUCT pointer that contains information about the
	//			item being drawn.
	//			pOwner - CXTCommandsListBox pointer to the commands list box whose item
	//			is being drawn.
    // Summary:	This member function draws each item in the commands list box.
    virtual void OnDrawItem(LPDRAWITEMSTRUCT lpDIS,CXTCommandsListBox* pOwner) = 0;

	// Input:	pOwner - CXTCommandsListBox pointer to the commands list box that needs
	//			to be refreshed.
    // Summary:	This member function paints the background and handles default
	//			drawing for the commands list box.
	virtual void OnPaint(CXTCommandsListBox* pOwner) = 0;
};

//////////////////////////////////////////////////////////////////////
// Summary: IXTSkin is a stand alone interface class.  It is used to create a skin
//			for rendering UI elements.
interface IXTSkin
{

	// Returns: A proxy for drawing a flat edit control.
    // Summary:	This member function is used to get a proxy for drawing a flat edit
	//			control. Implementation can return NULL to indicate default control
	//			behavior shall be used. 
    virtual IXTFlatEditProxy* GetFlatEditProxy() = 0;

	// Returns: A proxy for drawing a flat combo box. 
    // Summary:	This member function is used to get a proxy for drawing a flat combo
	//			box. Implementation can return NULL to indicate default control behavior
	//			shall be used. 
    virtual IXTFlatComboProxy* GetFlatComboProxy() = 0;

	// Returns: A proxy for drawing a toolbar button.
    // Summary:	This member function is used to get a proxy for drawing a toolbar
	//			button. Implementation can return NULL to indicate default control
	//			behaviour shall be used. 
    virtual IXTToolbarProxy* GetToolbarProxy() = 0;

	// Returns: A proxy for drawing a commands list box.
    // Summary:	This member function is used to get a proxy for drawing a commands list box
	//			control. Implementation can return NULL to indicate default control
	//			behavior shall be used. 
    virtual IXTCommandsListBoxProxy* GetCommandsListBoxProxy() = 0;

	// Input:	bTracking - Flag designating current operation state. TRUE if tracking
	//			toolbar is pending, or FALSE otherwise.
    // Summary:	This member function notifies that a toolbar is currently being
	//			dragged or resized.
    virtual void NotifyToolbarTracking(BOOL bTracking) = 0;

	// Returns: An IXTSkin pointer.
    // Summary:	This member function is a singleton type instantiation. 
    static _XT_EXT_CLASS IXTSkin* GetInstance();
};

#endif // !defined(__XTSKINS_H__)