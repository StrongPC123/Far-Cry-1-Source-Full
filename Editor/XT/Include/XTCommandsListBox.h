// XTCommandsListBox.h : interface for the CXTCommandsListBox class.
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

#if !defined(__XTCOMMANDSLISTBOX_H__)
#define __XTCOMMANDSLISTBOX_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CXTCustomGroupItemInfo;

//////////////////////////////////////////////////////////////////////
// Summary: CXTCommandsListBox is a CListBox derived class. It is used during toolbar
//			customization to display a list of currently available commands.
class _XT_EXT_CLASS CXTCommandsListBox : public CListBox
{

public:

	// Summary: This object site API.
	interface ISite
	{
		// Input:	nCmdID - Command ID whose icon is queried for.
		//			hIcon - destination to store command icon, valid if function returns true
		//			hDisabledIcon - destination to store disabled command icon, can be NULL, 
		//			valid if function returns true
		//			hHotIcon - destination to store the hot command icon, can be NULL, valid if the
		//			function returns true.
		// Returns: true if found, false otherwise
		// Summary:	This member function gets a command icon. 
		virtual bool GetCommandIcon(UINT nCmdID,HICON& hIcon, HICON& hDisabledIcon,HICON& hHotIcon) = 0;

		// Input:	pos - Position (item data) of the item in the list box.
		// Returns: A UINT object.
		// Summary:	This member function gets command ID by its position.  
		virtual UINT GetItemCommand(int pos) = 0;

		// Input:	pos - Position (item data) of the item in the list box.
		// Summary:	This member function performs a drag and drop of an item at the position
		//			provided.
		virtual void DragNDrop(int pos) = 0;
	};

private:

	CSize		 m_sizeIcon;
	ISite* const m_site;
	
public:


	ISite* GetSite() const {
		return m_site;
	}
	CSize& GetIconSize() {
		return m_sizeIcon;
	}

	// Input:	site - Points to an ISite object.
    // Summary:	Constructs a CXTCommandsListBox object.
	CXTCommandsListBox(ISite* site);

    // Summary: Destroys a CXTCommandsListBox object, handles cleanup and de-allocation.
	virtual ~CXTCommandsListBox();

public:

	// Ignore:
	//{{AFX_VIRTUAL(CXTCommandsListBox)
	public:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    //}}AFX_VIRTUAL

protected:
	
    // Ignore:
	//{{AFX_MSG(CXTCommandsListBox)
	afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTCOMMANDSLISTBOX_H__)
