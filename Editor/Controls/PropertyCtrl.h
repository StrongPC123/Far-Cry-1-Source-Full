////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   propertyctrl.h
//  Version:     v1.00
//  Created:     5/6/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Defines custom control to handle Properties.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __propertyctrl_h__
#define __propertyctrl_h__

#if _MSC_VER > 1000
#pragma once
#endif

// forward declarations.
class CPropertyItem;
class CVarBlock;

/** Costom control to handle Properties hierarchies.
*/
class CPropertyCtrl : public CWnd
{
	DECLARE_DYNAMIC(CPropertyCtrl)
public:
	typedef std::vector<CPropertyItem*> Items;

	// Flags of property control.
	enum Flags
	{
		F_VARIABLE_HEIGHT		= 0x0010,
		F_VS_DOT_NET_STYLE	= 0x0020,	// Provides a look similar to Visual Studio.NET property grid.
	};

	//! When item change, this callback fired with name of item.
	typedef Functor1<XmlNodeRef> UpdateCallback;
	//! When selection changes, this callback is fired with name of item.
	typedef Functor1<XmlNodeRef> SelChangeCallback;
	//! When item change, this callback fired variable that changed.
	typedef Functor1<IVariable*> UpdateVarCallback;

	CPropertyCtrl();
	virtual ~CPropertyCtrl();

	void Create( DWORD dwStyle,const CRect &rc,CWnd *pParent=NULL,UINT nID=0 );

	//! Set control flags.
	//! @param flags @see Flags enum.
	void SetFlags( int flags ) { m_nFlags = flags; };
	//! get control flags.
	int GetFlags() const { return m_nFlags; };

	/** Create Property items from root Xml node
	*/
	void CreateItems( XmlNodeRef &node );

	/** Delete all items from this control.
	*/
	void DeleteAllItems();

	/** Delete item and all its subitems.
	*/
	void DeleteItem( CPropertyItem *pItem );

	/** Add more variables.
			@param szCategory Name of category to place var block, if NULL do not create new category.
			@return Root item where this var block was added.
	*/
	CPropertyItem* AddVarBlock( CVarBlock *varBlock,const char *szCategory=NULL );

	/** Set update callback to be used for this property window.
	*/
	void SetUpdateCallback( UpdateCallback &callback ) { m_updateFunc = callback; }
	
	/** Set update callback to be used for this property window.
	*/
	void SetUpdateCallback( UpdateVarCallback &callback ) { m_updateVarFunc = callback; }
	
	/** Enable of disable calling update callback when some values change.
	*/
	bool EnableUpdateCallback( bool bEnable );

	/** Set selchange callback to be used for this property window.
	*/
	void SetSelChangeCallback( SelChangeCallback &callback ) { m_selChangeFunc = callback; }
	
	/** Enable of disable calling selchange callback when the selection changes.
	*/
	bool EnableSelChangeCallback( bool bEnable );

	/** Expand all categories.
	*/
	void	ExpandAll();

	/** Expand all childs of specified item.
	*/
	void ExpandAllChilds( CPropertyItem *item,bool bRecursive );

	//! Expend this item
	void Expand( CPropertyItem *item,bool bExpand );

	/** Get pointer to root item
	*/
	CPropertyItem* GetRootItem() const { return m_root; };

	/**  Reload values back from xml nodes.
	*/
	void ReloadValues();

	/** Change splitter value.
	*/
	void SetSplitter( int splitter ) { m_splitter = splitter; };

	/** Get current value of splitter.
	*/
	int GetSplitter() const { return m_splitter; };

	/** Get total height of all visible items.
	*/
	int GetVisibleHeight();

	static void RegisterWindowClass();

	void OnItemChange( CPropertyItem *item );

	// Ovveride method defined in CWnd.
	BOOL EnableWindow( BOOL bEnable = TRUE );

	//! When set to true will only display values of modified parameters.
	void SetDisplayOnlyModified( bool bEnable ) { m_bDisplayOnlyModified = bEnable; };

	CRect GetItemValueRect( const CRect &rect );
	void GetItemRect( CPropertyItem *item,CRect &rect );

	//! Set height of item, (When F_VARIABLE_HEIGHT flag is set, this value is ignored)
	void SetItemHeight( int nItemHeight );

	//! Get height of item.
	int GetItemHeight( CPropertyItem *item ) const;

	void ClearSelection();

	CPropertyItem* GetSelectedItem() { return m_selected; }

	void SetRootName( const CString &rootName );

	//! Find item that reference specified property.
	CPropertyItem* FindItemByVar( IVariable *pVar );

	void GetVisibleItems( CPropertyItem *root,Items &items );
	bool IsCategory( CPropertyItem *item );

	CPropertyItem* GetItemFromPoint( CPoint point );
	void SelectItem( CPropertyItem *item );

	void MultiSelectItem( CPropertyItem *pItem );
	void MultiUnselectItem( CPropertyItem *pItem );
	void MultiSelectRange( CPropertyItem *pAnchorItem );

protected:
	friend CPropertyItem;

	void DrawItem( CPropertyItem *item,CDC &dc,CRect &itemRect );
	int CalcOffset( CPropertyItem *item );
	void DrawSign( CDC &dc,CPoint point,bool plus );

	
	void CreateInPlaceControl();
	bool IsOverSplitter( CPoint point );
	void ProcessTooltip( CPropertyItem *item );

	void CalcLayout();
	void Init();

	void CopyItem( XmlNodeRef rootNode,CPropertyItem *pItem,bool bRecursively );
	void OnCopy( bool bRecursively );
	void OnCopyAll();
	void OnPaste();

	DECLARE_MESSAGE_MAP()

	afx_msg UINT OnGetDlgCode(); 
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnGetFont(WPARAM wParam, LPARAM);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
	//////////////////////////////////////////////////////////////////////////
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PreSubclassWindow();
	//////////////////////////////////////////////////////////////////////////

	TSmartPtr<CPropertyItem> m_root;
	XmlNodeRef m_xmlRoot;
	bool m_bEnableCallback;
	UpdateCallback m_updateFunc;
	bool m_bEnableSelChangeCallback;
	SelChangeCallback m_selChangeFunc;
	UpdateVarCallback m_updateVarFunc;
	CImageList m_icons;

	CPropertyItem *m_selected;
	CBitmap m_offscreenBitmap;

	CPropertyItem *m_prevTooltipItem;
	std::vector<CPropertyItem*> m_multiSelectedItems;

	HCURSOR m_leftRightCursor;
	CBrush m_bgBrush;
	int m_splitter;

	CPoint m_mouseDownPos;
	bool m_bSplitterDrag;

	CPoint m_scrollOffset;

	CToolTipCtrl m_tooltip;

	CFont *m_pBoldFont;

	//! When set to true will only display values of modified items.
	bool m_bDisplayOnlyModified;

	//! Timer to track loose of focus.
	int m_nTimer;

	//! Item height.
	int m_nItemHeight;

	//! Control custom flags.
	int m_nFlags;
};


#endif // __propertyctrl_h__