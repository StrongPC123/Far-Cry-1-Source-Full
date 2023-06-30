////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   BaseLibraryDialog.h
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __baselibrarydialog_h__
#define __baselibrarydialog_h__
#pragma once

//#include "XTToolkit.h"
#include "ToolbarDialog.h"
#include "Controls\SplitterWndEx.h"
#include "Controls\TreeCtrlEx.h"
#include "Controls\PropertyCtrl.h"
#include "Controls\PreviewModelCtrl.h"

class CBaseLibrary;
class CBaseLibraryItem;
class CBaseLibraryManager;

/** Base class for all BasLibrary base dialogs.
		Provides common methods for handling library items.
*/
class CBaseLibraryDialog : public CToolbarDialog,public IDocListener
{
	DECLARE_DYNAMIC(CBaseLibraryDialog);
public:
	CBaseLibraryDialog( UINT nID,CWnd *pParent );
	~CBaseLibraryDialog();

	//! Reload all data in dialog.
	virtual void Reload();

	// Called every frame.
	virtual void Update();

	//! This dialog is activated.
	virtual void SetActive( bool bActive );
	virtual void SelectLibrary( const CString &library );
	virtual void SelectItem( CBaseLibraryItem *item,bool bForceReload=false );
	virtual bool CanSelectItem( CBaseLibraryItem *pItem );

	//! Returns menu for this dialog.
	virtual UINT GetDialogMenuID() { return 0; };

protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	void DoDataExchange(CDataExchange* pDX);
	BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelChangedItemTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDownItemTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateSelected( CCmdUI* pCmdUI );
	afx_msg void OnUpdatePaste( CCmdUI* pCmdUI );

	virtual afx_msg void OnAddLibrary();
	virtual afx_msg void OnRemoveLibrary();
	virtual afx_msg void OnAddItem();
	virtual afx_msg void OnRemoveItem();
	virtual afx_msg void OnRenameItem();
	virtual afx_msg void OnChangedLibrary();
	virtual afx_msg void OnExportLibrary();
	virtual afx_msg void OnSave();
	virtual afx_msg void OnReloadLib();
	virtual afx_msg void OnLoadLibrary();

	//////////////////////////////////////////////////////////////////////////
	// Must be overloaded in derived classes.
	//////////////////////////////////////////////////////////////////////////
	virtual CBaseLibrary* FindLibrary( const CString &libraryName );
	virtual CBaseLibrary* NewLibrary( const CString &libraryName );
	virtual void DeleteLibrary( CBaseLibrary *pLibrary );
	virtual void DeleteItem( CBaseLibraryItem *pItem );

	//////////////////////////////////////////////////////////////////////////
	// Some functions can be overriden to modify standart functionality.
	//////////////////////////////////////////////////////////////////////////
	virtual void InitToolbar();
	virtual void ReloadLibs();
	virtual void ReloadItems();
	virtual HTREEITEM InsertItemToTree( CBaseLibraryItem *pItem,HTREEITEM hParent );
	virtual void SetItemName( CBaseLibraryItem *item,const CString &groupName,const CString &itemName );

	//////////////////////////////////////////////////////////////////////////
	// IDocListener listener implementation
	//////////////////////////////////////////////////////////////////////////
	virtual	void OnNewDocument();
	virtual	void OnLoadDocument();
	virtual void OnCloseDocument();
	virtual void OnMissionChange() {};
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Copying and cloning of items.
	//////////////////////////////////////////////////////////////////////////
	virtual void OnCopy() = 0;
	virtual void OnPaste() = 0;
	virtual void OnCut();
	virtual void OnClone();
	
	
	DECLARE_MESSAGE_MAP()

	// Dialog Toolbar.
	CDlgToolBar m_toolbar;

	// Tree control.
	CTreeCtrl m_treeCtrl;
	// Library control.
	CComboBox m_libraryCtrl;

	// Map of library items to tree ctrl.
	typedef std::map<CBaseLibraryItem*,HTREEITEM> ItemsToTreeMap;
	ItemsToTreeMap m_itemsToTree;

	//CXTToolBar m_toolbar;
	// Name of currently selected library.
	CString m_selectedLib;
	CString m_selectedGroup;
	bool m_bLibsLoaded;

	//! Selected library.
	TSmartPtr<CBaseLibrary> m_pLibrary;
	//! Selected Item.
	TSmartPtr<CBaseLibraryItem> m_pCurrentItem;
	//! Pointer to item manager.
	CBaseLibraryManager* m_pItemManager;

	//////////////////////////////////////////////////////////////////////////
	// Dragging support.
	//////////////////////////////////////////////////////////////////////////
	CBaseLibraryItem *m_pDraggedItem;
	CImageList* m_dragImage;

	bool m_bIgnoreSelectionChange;

	HCURSOR m_hCursorDefault;
	HCURSOR m_hCursorNoDrop;
	HCURSOR m_hCursorCreate;
	HCURSOR m_hCursorReplace;
};

#endif // __baselibrarydialog_h__
