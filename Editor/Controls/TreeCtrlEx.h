////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   treectrlex.h
//  Version:     v1.00
//  Created:     1/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History: Assembled from contributions fro www.codeguru.com
//
////////////////////////////////////////////////////////////////////////////

#ifndef __treectrlex_h__
#define __treectrlex_h__

#if _MSC_VER > 1000
#pragma once
#endif

/** CTreeCtrlEx is and extended version of CTreeCtrl,
		It allows Drag&Drop of leaf items, and copying of items.
*/
class CTreeCtrlEx : public CTreeCtrl
{
	DECLARE_DYNAMIC(CTreeCtrlEx)

public:
	CTreeCtrlEx();
	virtual ~CTreeCtrlEx();
	
	/** Get next item using standart CTreeCtrl call.
	*/
	HTREEITEM GetNextItem( HTREEITEM hItem, UINT nCode );

	/** Get next item as if outline was completely expanded.
			@return	The item immediately below the reference item.
			@param	The reference item.
	*/
	HTREEITEM GetNextItem( HTREEITEM hItem);

	/** FindNextItem traverses a tree searching for an item matching all the item attributes set in a TV_ITEM structure.
			In the TV_ITEM structure, the mask member specifies which attributes make up the search criteria.
			If a match is found, the function returns the handle otherwise NULL.
			The function only searches in one direction (down) and if hItem is NULL starts from the root of the tree.
	*/
	HTREEITEM FindNextItem(TV_ITEM* pItem, HTREEITEM hItem);

	/** Copies an item to a new location
			@return	Handle of the new item.
			@parans hItem	Item to be copied.
			@param	htiNewParent	Handle of the parent for new item.
			@param	htiAfter	Item after which the new item should be created.
	*/
	HTREEITEM CopyItem( HTREEITEM hItem, HTREEITEM htiNewParent, HTREEITEM htiAfter=TVI_LAST );

	/** Copies all items in a branch to a new location.
			@return	The new branch node.
			@param	htiBranch	The node that starts the branch.
			@param	htiNewParent	Handle of the parent for new branch.
			@param	htiAfter	Item after which the new branch should be created.
	*/
	HTREEITEM CopyBranch( HTREEITEM htiBranch, HTREEITEM htiNewParent,HTREEITEM htiAfter=TVI_LAST );

	/** When set only leafs are allowed to be dragged.
	*/
	void SetOnlyLeafsDrag( bool bEnable ) { m_bOnlyLeafsDrag = bEnable; }

protected:
	DECLARE_MESSAGE_MAP()

	//! Callback caled when items is copied.
	virtual void OnItemCopied( HTREEITEM hItem, HTREEITEM hNewItem );
	//! Check if drop source.
	virtual BOOL IsDropSource( HTREEITEM hItem );
	//! Get item which is target for drop of this item.
	virtual HTREEITEM GetDropTarget(HTREEITEM hItem);

	BOOL CompareItems(TV_ITEM* pItem, TV_ITEM& tvTempItem);

	//////////////////////////////////////////////////////////////////////////
	// Message Handlers.
	//////////////////////////////////////////////////////////////////////////
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	//////////////////////////////////////////////////////////////////////////
	// For dragging.
	CImageList*	m_pDragImage;
	bool				m_bLDragging;
	HTREEITEM	m_hitemDrag,m_hitemDrop;
	HCURSOR    m_dropCursor,m_noDropCursor;
	bool m_bOnlyLeafsDrag;

};


#endif // __treectrlex_h__