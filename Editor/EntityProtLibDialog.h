////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   entityprotlibdialog.h
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __entityprotlibdialog_h__
#define __entityprotlibdialog_h__
#pragma once

#include "BaseLibraryDialog.h"
#include "Controls\SplitterWndEx.h"
#include "Controls\TreeCtrlEx.h"
#include "Controls\PropertyCtrl.h"
#include "Controls\PreviewModelCtrl.h"
#include "EntityScriptDialog.h"

class CEntityPrototypeManager;
class CEntityPrototype;
struct IEntitySystem;
struct IEntity;

/** Dialog which hosts entity prototype library.
*/
class CEntityProtLibDialog : public CBaseLibraryDialog
{
	DECLARE_DYNAMIC(CEntityProtLibDialog)
public:
	CEntityProtLibDialog( CWnd *pParent );
	~CEntityProtLibDialog();

	// Called every frame.
	void Update();
	virtual UINT GetDialogMenuID();

protected:
	void InitToolbar();
	void DoDataExchange(CDataExchange* pDX);
	BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnAddPrototype();
	afx_msg void OnPlay();
	afx_msg void OnUpdatePlay( CCmdUI* pCmdUI );
	afx_msg void OnShowDescription();
	afx_msg void OnDescriptionChange();
	afx_msg void OnAssignToSelection();
	afx_msg void OnSelectAssignedObjects();
	afx_msg void OnNotifyTreeRClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCopy();
	afx_msg void OnPaste();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	void SelectItem( CBaseLibraryItem *item,bool bForceReload=false );

	//////////////////////////////////////////////////////////////////////////
	// IDocListener listener implementation
	//////////////////////////////////////////////////////////////////////////
	virtual	void OnNewDocument();
	virtual	void OnLoadDocument();
	virtual void OnCloseDocument();

	//////////////////////////////////////////////////////////////////////////
	void SpawnEntity( CEntityPrototype *prototype );
	void ReleaseEntity();
	void OnUpdateProperties( IVariable *var );
	void OnReloadEntityScript();
	CString SelectEntityClass();
	CEntityPrototype* GetSelectedPrototype();
	

	DECLARE_MESSAGE_MAP()

	CSplitterWndEx m_wndHSplitter;
	CSplitterWndEx m_wndVSplitter;
	CSplitterWndEx m_wndScriptPreviewSplitter;
	
	//CModelPreview
	CEntityScriptDialog m_scriptDialog;
	CPreviewModelCtrl m_previewCtrl;
	CPropertyCtrl m_propsCtrl;
	CXTEdit m_descriptionEditBox;
	CImageList m_imageList;

	//! Selected Prototype.
	IEntity* m_entity;
	IEntitySystem *m_pEntitySystem;
	CString m_visualObject;

	bool m_bEntityPlaying;
	bool m_bShowDescription;

	// Prototype manager.
	CEntityPrototypeManager *m_pEntityManager;
};

#endif // __entityprotlibdialog_h__
