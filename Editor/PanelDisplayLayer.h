////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   paneldisplaylayer.h
//  Version:     v1.00
//  Created:     9/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __paneldisplaylayer_h__
#define __paneldisplaylayer_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "XTToolkit.h"
#include "Controls\LayersListBox.h"

class CObjectLayer;
class CObjectLayerManager;
// CPanelDisplayLayer dialog

class CPanelDisplayLayer : public CXTCBarDialog
{
	DECLARE_DYNAMIC(CPanelDisplayLayer)

public:
	CPanelDisplayLayer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPanelDisplayLayer();

// Dialog Data
	enum { IDD = IDD_PANEL_DISPLAY_LAYERS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//! Callback called when layer is updated.
	void OnLayerUpdate( int event,CObjectLayer *pLayer );
	void SelectLayer( CObjectLayer *pLayer );

	void ReloadLayers();
	void OnLayersUpdate();
	CString GetSelectedLayer();

	DECLARE_MESSAGE_MAP()
private:
	// ! ListBox of layers.
	CXTTreeCtrl m_treeCtrl;
	CXTToolBar m_toolbar;
	CColorCtrl<CLayersListBox> m_layersCtrl;

	HTREEITEM AddLayer( CObjectLayer *pLayer );
	void SetItemState( HTREEITEM hItem );
	void UpdateLayerItem( CObjectLayer *pLayer );

	virtual void OnOK() {};
	virtual void OnCancel() {};

	virtual BOOL OnInitDialog();
	afx_msg void OnSelChanged();
	afx_msg void OnLayersRButtonUp(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedNew();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedExport();
	afx_msg void OnBnClickedImport();
	afx_msg void OnBnClickedRename();

	bool m_bLayersValid;

	typedef std::map<CObjectLayer*,HTREEITEM> LayerToItemMap;
	LayerToItemMap m_layerToItemMap;

	CObjectLayer *m_currentLayer;
	CObjectLayerManager *m_pLayerManager;
	bool m_bIgnoreSelectItem;

	HTREEITEM m_hPrevSelected;
};

#endif // __paneldisplaylayer_h__