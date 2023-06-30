////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PrefabDialog.h
//  Version:     v1.00
//  Created:     10/11/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PrefabDialog_h__
#define __PrefabDialog_h__
#pragma once

#include "BaseLibraryDialog.h"
#include "Controls\SplitterWndEx.h"
#include "Controls\TreeCtrlEx.h"
#include "Controls\PropertyCtrl.h"
#include "Controls\PreviewModelCtrl.h"

class CPrefabItem;
class CPrefabManager;

/** Dialog which hosts entity prototype library.
*/
class CPrefabDialog : public CBaseLibraryDialog
{
	DECLARE_DYNAMIC(CPrefabDialog);
public:
	CPrefabDialog( CWnd *pParent );
	~CPrefabDialog();

	// Called every frame.
	void Update();

	virtual UINT GetDialogMenuID();

	CPrefabItem* GetPrefabFromSelection();

protected:
	void DoDataExchange(CDataExchange* pDX);
	BOOL OnInitDialog();

	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	
	afx_msg void OnAddItem();
	afx_msg void OnUpdateItemSelected( CCmdUI* pCmdUI );
	afx_msg void OnUpdateObjectSelected( CCmdUI* pCmdUI );
	afx_msg void OnUpdateAssignToSelection( CCmdUI *pCmdUI );
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNotifyMtlTreeRClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPickPrefab();
	afx_msg void OnUpdatePickPrefab( CCmdUI* pCmdUI );
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnSelectAssignedObjects();
	afx_msg void OnAssignToSelection();
	afx_msg void OnGetFromSelection();
	afx_msg void OnMakeFromSelection();

	//! Update objects to which this prefab is assigned.
	void UpdatePrefabObjects( CPrefabItem *pPrefab );

	//////////////////////////////////////////////////////////////////////////
	// Some functions can be overriden to modify standart functionality.
	//////////////////////////////////////////////////////////////////////////
	virtual void InitToolbar();
	virtual void SelectItem( CBaseLibraryItem *item,bool bForceReload=false );

	//////////////////////////////////////////////////////////////////////////
	CPrefabItem* GetSelectedPrefab();
	void OnUpdateProperties( IVariable *var );
	
	void DropToItem( HTREEITEM hItem,HTREEITEM hSrcItem,CPrefabItem *pMtl );

	//////////////////////////////////////////////////////////////////////////
	// IDocListener listener implementation
	//////////////////////////////////////////////////////////////////////////
	virtual	void OnNewDocument();
	virtual	void OnLoadDocument();
	virtual void OnCloseDocument();
	//////////////////////////////////////////////////////////////////////////

	enum EDrawType
	{
		DRAW_BOX,
		DRAW_SPHERE,
		DRAW_TEAPOT,
		DRAW_SELECTION,
	};
	

	DECLARE_MESSAGE_MAP()

	CSplitterWndEx m_wndHSplitter;
	CSplitterWndEx m_wndVSplitter;
	
	CPropertyCtrl m_propsCtrl;
	CImageList m_imageList;

	CImageList *m_dragImage;

	// Manager.
	CPrefabManager *m_pPrefabManager;

	EDrawType m_drawType;

	HTREEITEM m_hDropItem;
	HTREEITEM m_hDraggedItem;
};

#endif // __PrefabDialog_h__
