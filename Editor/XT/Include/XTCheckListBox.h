// XTCheckListBox.h : interface for the CXTCheckListBox class.
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

#if !defined(__XTCHECKLISTBOX_H__)
#define __XTCHECKLISTBOX_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// CXTCheckListState - helper class

class _XT_EXT_CLASS CXTCheckListState : public CNoTrackObject
{
public:
	CXTCheckListState(UINT uBitmapID);
	virtual ~CXTCheckListState();
	HBITMAP m_hbitmapCheck;
	CSize m_sizeCheck;
};

//////////////////////////////////////////////////////////////////////
// Summary: The CXTCheckListBox class is a CXTListBox derived class.  It provides
//			the functionality of a Windows checklist box.  A “checklist box” displays
//			a list of items, such as filenames. Each item in the list has a check
//			box next to it that the user can check or clear.
//
//			CXTCheckListBox is only for owner-drawn controls because the list contains 
//			more than text strings.  At its simplest, a checklist box contains text
//			strings and check boxes, but you do not need to have text at all.  For
//			example, you could have a list of small bitmaps with a check box next
//			to each item.
//
//			To create your own checklist box, you must derive your own class from
//			CXTCheckListBox.  To derive your own class, write a constructor for
//			the derived class, then call Create. 
//
//			If your checklist box is a default checklist box (a list of strings
//			with the default-sized checkboxes to the left of each), you can use
//			the default CXTCheckListBox::DrawItem to draw the checklist box. Otherwise,
//			you must override the CListBox::CompareItem function and the 
//			CXTCheckListBox::DrawItem and CXTCheckListBox::MeasureItem functions. 
class _XT_EXT_CLASS CXTCheckListBox : public CXTListBox
{
	DECLARE_DYNAMIC(CXTCheckListBox)
		
public:
	
	// Input:	uBitmapID - Resource identifier of the bitmap to use for checkmarks.
    // Summary: Constructs a CXTCheckListBox object.
	CXTCheckListBox(UINT uBitmapID=XT_IDB_CHECKLISTBOX);
	
	// Summary: Destroys a CXTCheckListBox object, handles cleanup and de-allocation.
	virtual ~CXTCheckListBox();
	
protected:
	
    int               m_cyText;
    UINT              m_nStyle;
    CXTCheckListState m_checkListState;
	
public:
	

	// Input:	dwStyle - Specifies the style of the checklist box. The style must be
	//			either LBS_OWNERDRAWFIXED (all items in the list are the same height) 
	//			or LBS_OWNERDRAWVARIABLE (items in the list are of varying heights). 
	//			This style can be combined with other list-box styles.
	//			rect - Specifies the checklist-box size and position. Can be either a 
	//			CRect object or a RECT structure.
	//			pParentWnd - Specifies the checklist box’s parent window (usually a 
	//			CDialog object). It must not be NULL.
	//			nID - Specifies the checklist box’s control ID.
	// Returns: Nonzero if successful; otherwise 0.
	// Summary: Creates the Windows checklist box and attaches it to the CXTListBox object.
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	
	// BULLETED LIST:

	// Input:	nStyle - Determines the style of check boxes in the checklist box.  Valid styles are:
	//			[ul]
	//			[li]<b>BS_CHECKBOX</b>[/li]
	//			[li]<b>BS_AUTOCHECKBOX</b>[/li]
	//			[li]<b>BS_AUTO3STATE</b>[/li]
	//			[li]<b>BS_3STATE</b>[/li]
	//			[/ul]
	//			For information on these styles, see Button Styles.
	// Summary:	Call this function to set the style of check boxes in the checklist box.
	void SetCheckStyle(UINT nStyle);
	
	// Returns:  The style of the control’s check boxes.
	// Summary:  Call this function to get the checklist box’s style. For information on 
	//			 possible styles, see SetCheckStyle.  
	// See Also: SetCheckStyle
	UINT GetCheckStyle();
	
	// Input:	nIndex - Index of the item whose check box is to be set.
	//			nCheck - State of the check box: 0 for clear, 1 for checked, and 2 for
	//			indeterminate.
	// Summary: Call this function to set the check box of the item specified by nIndex.
	void SetCheck(int nIndex, int nCheck);

	// Input:	nIndex - Index of the item whose check status is to be retrieved.
	// Returns: Zero if the item is not checked, 1 if it is checked, and 2 if it is 
	//			indeterminate.
	// Summary: Call this function to determine the check state of an item.  
	int GetCheck(int nIndex);

	// Input:	nIndex - Index of the checklist box item to be enabled.
	//			bEnabled - Specifies whether the item is enabled or disabled.
	// Summary: Call this function to enable or disable a checklist box item.
	void Enable(int nIndex, BOOL  = TRUE);
	
	// Input:	nIndex - Index of the item.
	// Returns: Nonzero if the item is enabled; otherwise 0.
	// Summary: Call this function to determine whether an item is enabled.  
	BOOL IsEnabled(int nIndex);
	
	// Input:	rectItem - The position and size of the list item.
	//			rectCheckBox - The default position and size of an item's check box.
	// Example: The following function overrides the default and puts the check box on the
	//			right of the item, makes it the same height as the item (minus a pixel 
	//			offset at the top and bottom), and makes it the standard check box width:
    //          <pre>
    //          CRect CMyCheckListBox::OnGetCheckPosition(CRect rectItem, CRect rectCheckBox)
    //          {
    //             CRect rectMyCheckBox;
    //             rectMyCheckBox.top = rectItem.top -1;
    //             rectMyCheckBox.bottom = rectItem.bottom -1;
    //             rectMyCheckBox.right = rectItem.right -1;
    //             rectMyCheckBox.left = rectItem.right -1 - rectCheckBox.Width();
    //             return rectMyCheckBox;
    //          }
    //          </pre>
	// Returns: The position and size of an item's check box.
	// Summary: The framework calls this function to get the position and size of the 
	//			check box in an item.
	//
	//			The default implementation only returns the default position and size 
	//			of the check box (rectCheckBox). By default, a check box is aligned in 
	//			the upper-left corner of an item and is the standard check box size. There 
	//			may be cases where you want the check boxes on the right, or want a larger
	//			or smaller check box. In these cases, override OnGetCheckPosition to change
	//			the check box position and size within the item. 
	virtual CRect OnGetCheckPosition(CRect rectItem, CRect rectCheckBox);
	
protected:
	
    // Ignore:
	//{{AFX_VIRTUAL(CXTCheckListBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	virtual int GetFontHeight();
	virtual int PreCompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual int CalcMinimumItemHeight();
	virtual int CheckFromPoint(CPoint point, BOOL& bInCheck);
	virtual void PreDrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void PreMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void PreDeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);
	virtual void SetSelectionCheck( int nCheck );
	virtual void InvalidateCheck(int nIndex);
	virtual void InvalidateItem(int nIndex);
	
protected:

	// Ignore:
	//{{AFX_MSG(CXTCheckListBox)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBAddString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBFindString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBFindStringExact(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBGetItemData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBGetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBInsertString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBSelectString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBSetItemData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBSetItemHeight(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

AFX_INLINE UINT CXTCheckListBox::GetCheckStyle() {
	return m_nStyle;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTCHECKLISTBOX_H__)