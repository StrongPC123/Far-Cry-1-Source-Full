////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   paneldisplaylayer.cpp
//  Version:     v1.00
//  Created:     9/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////


// PanelDisplayLayer.cpp : implementation file
//

#include "stdafx.h"
#include "PanelDisplayLayer.h"
#include "ObjectLayerPropsDialog.h"

#include "Objects\ObjectManager.h"
#include "Objects\ObjectLayerManager.h"

#define ITEM_COLOR_NORMAL RGB(0,0,0)
#define ITEM_COLOR_SELECTED RGB(255,0,0)
#define ITEM_COLOR_FROZEN RGB(196,196,196)

// CPanelDisplayLayer dialog

IMPLEMENT_DYNAMIC(CPanelDisplayLayer, CXTCBarDialog)
CPanelDisplayLayer::CPanelDisplayLayer(CWnd* pParent /*=NULL*/)
	: CXTCBarDialog(CPanelDisplayLayer::IDD, pParent)
{
	m_pLayerManager = GetIEditor()->GetObjectManager()->GetLayersManager();
	m_bLayersValid = false;
	m_bIgnoreSelectItem = false;

	Create( IDD,pParent );

	// Register callback.
	m_pLayerManager->AddUpdateListener( functor(*this,&CPanelDisplayLayer::OnLayerUpdate) );
	
	xtAfxData.bControlBarMenus = TRUE; // Turned off in constructor of CXTCBarDialog.
}

CPanelDisplayLayer::~CPanelDisplayLayer()
{
	m_pLayerManager->RemoveUpdateListener( functor(*this,&CPanelDisplayLayer::OnLayerUpdate) );
}

void CPanelDisplayLayer::DoDataExchange(CDataExchange* pDX)
{
	CXTCBarDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_LAYERS, m_treeCtrl);
	DDX_Control(pDX,IDC_LAYERS,m_layersCtrl );
}


BEGIN_MESSAGE_MAP(CPanelDisplayLayer, CXTCBarDialog)
	ON_LBN_SELCHANGE(IDC_LAYERS, OnSelChanged)
	//ON_NOTIFY(LBN_LAYERS_RBUTTON_DOWN, IDC_LAYERS, OnLayersRButtonDown)
	ON_NOTIFY(LBN_LAYERS_RBUTTON_UP, IDC_LAYERS, OnLayersRButtonUp)
	
	ON_COMMAND( ID_PANEL_LAYERS_NEW,OnBnClickedNew )
	ON_COMMAND( ID_PANEL_LAYERS_RENAME,OnBnClickedRename )
	ON_COMMAND( ID_PANEL_LAYERS_DELETE,OnBnClickedDelete )
	ON_COMMAND( ID_PANEL_LAYERS_EXPORT,OnBnClickedExport )
	ON_COMMAND( ID_PANEL_LAYERS_IMPORT,OnBnClickedImport )
	
//	ON_NOTIFY(TVN_CHKCHANGE, IDC_OBJECTS, OnTvnCheckBox)
END_MESSAGE_MAP()

BOOL CPanelDisplayLayer::OnInitDialog()
{
	CXTCBarDialog::OnInitDialog();

	m_hPrevSelected = 0;

	//m_treeCtrl.EnableMultiSelect(TRUE);
	//m_treeCtrl.SetBkColor( RGB(0xE0,0xE0,0xE0) );
	m_layersCtrl.SetBkColor( GetSysColor(COLOR_BTNFACE) );
//	m_layers.SetUpdateCallback( functor(*this,OnLayersUpdate) );
	ReloadLayers();

	m_toolbar.Create( this,WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY );
	m_toolbar.LoadToolBar( IDR_PANEL_LAYERS );
	RecalcBarLayout();

	// Autoresize layers.
	SetResize( IDC_LAYERS,SZ_RESIZE(1) );
	SetResize( IDC_INFOTEXT,SZ_REPOS(1) );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::OnLayerUpdate( int event,CObjectLayer *pLayer )
{
	switch (event)
	{
	case CObjectLayerManager::ON_LAYER_SELECT:
		SelectLayer( pLayer );
		break;
	case CObjectLayerManager::ON_LAYER_MODIFY:
		UpdateLayerItem( pLayer );
		break;
	default:
		m_bLayersValid = false;
		ReloadLayers();
	}
}

//////////////////////////////////////////////////////////////////////////
HTREEITEM CPanelDisplayLayer::AddLayer( CObjectLayer *pLayer )
{
	return 0;
	m_bIgnoreSelectItem = true;
	HTREEITEM hRoot = TVI_ROOT;
	if (pLayer->GetParent())
	{
		hRoot = stl::find_in_map( m_layerToItemMap,pLayer->GetParent(),hRoot );
	}
	HTREEITEM hItem = m_treeCtrl.InsertItem( pLayer->GetName(),0,0,hRoot );
	m_layerToItemMap[pLayer] = hItem;
	m_treeCtrl.SetItemData( hItem,(DWORD_PTR)pLayer );
	m_bIgnoreSelectItem = false;
	
	SetItemState( hItem );

	// Add child layers.
	for (int i = 0; i < pLayer->GetChildCount(); i++)
	{
		AddLayer( pLayer->GetChild(i) );
	}
	return hItem;
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::UpdateLayerItem( CObjectLayer *pLayer )
{
	HTREEITEM hItem = stl::find_in_map( m_layerToItemMap,pLayer,(HTREEITEM)0 );
	if (hItem)
		SetItemState(hItem);
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::SetItemState( HTREEITEM hItem )
{
	return;
	CObjectLayer *pLayer = (CObjectLayer*)m_treeCtrl.GetItemData(hItem);
	assert(pLayer);

	if (pLayer->IsVisible())
	{
		m_treeCtrl.SetCheck( hItem,1 );
	}
	else
	{
		m_treeCtrl.SetCheck( hItem,0 );
	}

	
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));       // zero out structure
//	lf.lfHeight = 12;                      // request a 12-pixel-height font
	if (pLayer->IsFrozen())
		lf.lfItalic = TRUE;
//	strcpy(lf.lfFaceName, "Tahoma");        // request a face name "Arial"

	m_treeCtrl.SetItemFont( hItem,lf );
	m_treeCtrl.RedrawWindow();
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::ReloadLayers()
{
	m_layersCtrl.ReloadLayers();
	return;

	m_bIgnoreSelectItem = true;
	m_treeCtrl.SetRedraw(FALSE);
	m_treeCtrl.DeleteAllItems();
	m_bIgnoreSelectItem = false;

	m_hPrevSelected = 0;
	m_layerToItemMap.clear();

	m_bLayersValid = true;
	std::vector<CObjectLayer*> layers;
	m_pLayerManager->GetLayers( layers );

	CObjectLayer *pCurrent = m_pLayerManager->GetCurrentLayer();

	CString selectedLayerName;
	
	// Add root layers.
	CLayersListBox::Layers layersInfo;
	for (int i = 0; i < layers.size(); i++)
	{
		CObjectLayer *pLayer = layers[i];
		if (pLayer->GetParent())
			continue;

		CLayersListBox::SLayerInfo layerInfo;
		layerInfo.name = pLayer->GetName();
		layerInfo.visible = pLayer->IsVisible();
		layerInfo.usable = !pLayer->IsFrozen();
		layersInfo.push_back(layerInfo);

		if (pCurrent == pLayer)
		{
			selectedLayerName = m_currentLayer->GetName();
		}

		HTREEITEM hItem = AddLayer( pLayer );
		m_treeCtrl.Expand( hItem,TVE_EXPAND );
	}
	
	SelectLayer( m_pLayerManager->GetCurrentLayer() );

	int sel = m_layersCtrl.FindString( -1,selectedLayerName );
	if (sel != LB_ERR)
	{
		m_layersCtrl.SetCurSel(sel);
	}
//	m_layers.SetLayers( layersInfo );
	//m_layers.SelectLayer( selectedLayerName );
	m_treeCtrl.SetRedraw(TRUE);
}

//////////////////////////////////////////////////////////////////////////
CString CPanelDisplayLayer::GetSelectedLayer()
{
	/*
	CLayersListBox::Layers layersInfo;
	m_layers.GetLayers( layersInfo );
	int sel = m_layers.GetCurSel();
	if (sel != LB_ERR)
	{
		return layersInfo[sel].name;
	}
	*/
	return "";
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::OnLayersUpdate()
{
	/*
	// Update layers info.
	CUndo undo( "Layer Modify" );
	CLayersListBox::Layers layersInfo;
	m_layers.GetLayers( layersInfo );
	for (int i = 0; i < layersInfo.size(); i++)
	{
		CObjectLayer *pLayer = m_pLayerManager->FindLayerByName(layersInfo[i].name);
		if (!pLayer)
			continue;
		pLayer->SetVisible( layersInfo[i].visible );
		pLayer->SetFrozen( !layersInfo[i].usable );
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::SelectLayer( CObjectLayer *pLayer )
{
	// Current layer possibly change.	
	assert(pLayer);
	m_pLayerManager->SetCurrentLayer( pLayer );

	m_bIgnoreSelectItem = true;
	CString selectedLayerName = pLayer->GetName();
	int sel = m_layersCtrl.FindString( -1,selectedLayerName );
	if (sel != LB_ERR)
	{
		m_layersCtrl.SetCurSel(sel);
	}
	m_bIgnoreSelectItem = false;

	return;

	HTREEITEM hItem = stl::find_in_map( m_layerToItemMap,pLayer,(HTREEITEM)0 );
	assert( hItem );

	m_bIgnoreSelectItem = true;

	if (m_hPrevSelected)
	{
		m_treeCtrl.SetItemState(m_hPrevSelected,0,TVIS_SELECTED|TVIS_FOCUSED);
		m_treeCtrl.SetItemBold( m_hPrevSelected,FALSE );
		m_treeCtrl.SetItemColor( m_hPrevSelected,ITEM_COLOR_NORMAL );
	}
	m_treeCtrl.SelectItem( hItem );
	m_treeCtrl.SetItemBold( hItem,TRUE );
	m_treeCtrl.SetItemColor( hItem,ITEM_COLOR_SELECTED );
	m_hPrevSelected = hItem;

	SetItemState( hItem );

	m_bIgnoreSelectItem = false;
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::OnSelChanged()
{
	CObjectLayer *pLayer = m_layersCtrl.GetCurrentLayer();
	if (pLayer)
	{
		m_pLayerManager->SetCurrentLayer( pLayer );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::OnBnClickedNew()
{
	// Create a new layer.
	CObjectLayerPropsDialog dlg( this );
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.m_name.IsEmpty())
			return;
		// Check if layer with such name already exists.
		if (m_pLayerManager->FindLayerByName(dlg.m_name))
		{
			Warning( "Layer %s already exist, choose different layer name",(const char*)dlg.m_name );
			return;
		}
		CUndo undo( "New Layer" );
		CObjectLayer *pLayer = m_pLayerManager->CreateLayer();
		pLayer->SetName( dlg.m_name );
		pLayer->SetExternal( dlg.m_bExternal );
		pLayer->SetExportable( dlg.m_bExportToGame );
		pLayer->SetFrozen( dlg.m_bFrozen );
		pLayer->SetVisible( dlg.m_bVisible );
		ReloadLayers();
	}
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::OnBnClickedDelete()
{
	CObjectLayer *pLayer = m_pLayerManager->GetCurrentLayer();
	CString selectedLayer = pLayer->GetName();
	CString str;
	str.Format( "Deleting of layer will also delete all child layers and objects assigned to them.\r\nDelete Layer %s?",
		(const char*)selectedLayer );
	if (MessageBox( str,"Confirm Delete Layer",MB_YESNO|MB_ICONQUESTION	) == IDYES)
	{
		// Delete selected layer.
		CUndo undo( "Delete Layer" );
		m_pLayerManager->DeleteLayer( pLayer );
		ReloadLayers();
	}
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::OnBnClickedExport()
{
	// Export selected layer.
	CObjectLayer *pLayer = m_pLayerManager->GetCurrentLayer();
	if (!pLayer)
		return;

	char szFilters[] = "Object Layer (*.lyr)|*.lyr||";

	CString filename = "*.lyr";
	if (CFileUtil::SelectSaveFile( szFilters,"lyr","",filename ))
	{
		CWaitCursor wait;
		
		XmlNodeRef root = new CXmlNode( "ObjectLayer" );

		XmlNodeRef layerDesc = root->newChild("Layer");
		CObjectArchive ar(GetIEditor()->GetObjectManager(),layerDesc,FALSE);
		GetIEditor()->GetObjectManager()->GetLayersManager()->ExportLayer( pLayer,ar,true );
		root->saveToFile( filename );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::OnBnClickedImport()
{
	// Import layer.
	char szFilters[] = "Object Layer (*.lyr)|*.lyr||";

	CString file;
	if (CFileUtil::SelectFile( "Object Layer (*.lyr)|*.lyr|All Files|*.*||","",file ))
	{
		CWaitCursor wait;

		XmlParser parser;
		XmlNodeRef root = parser.parse( file );
		if (!root)
			return;

		// Start recording errors.
		CErrorsRecorder errorsRecorder;

		CUndo undo( "Import Layer(s)" );
		CObjectArchive archive( GetIEditor()->GetObjectManager(),root,true );
		for (int i = 0; i < root->getChildCount(); i++)
		{
			XmlNodeRef layerDesc = root->getChild(i);
			if (stricmp(layerDesc->getTag(),"Layer") != 0)
				continue;

			archive.node = layerDesc;
			GetIEditor()->GetObjectManager()->GetLayersManager()->ImportLayer( archive,false );
		}
		GetIEditor()->GetObjectManager()->GetLayersManager()->ResolveLayerLinks();
		archive.ResolveObjects();
	}
}
	
//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::OnBnClickedRename()
{
	// Export selected layer.
	CObjectLayer *pLayer = m_pLayerManager->GetCurrentLayer();
	if (!pLayer)
		return;

	CObjectLayerPropsDialog dlg( this );
	dlg.m_name = pLayer->GetName();
	dlg.m_bExportToGame = pLayer->IsExportable();
	dlg.m_bExternal = pLayer->IsExternal();
	dlg.m_bFrozen = pLayer->IsFrozen();
	dlg.m_bVisible = pLayer->IsVisible();
	dlg.m_bMainLayer = pLayer == m_pLayerManager->GetMainLayer();
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.m_name.IsEmpty())
			return;

		CUndo undo("Rename Layer");

		CObjectLayer *pOtherLayer = m_pLayerManager->FindLayerByName(dlg.m_name);
		// Check if layer with such name already exists.
		if (pOtherLayer && pOtherLayer != pLayer)
		{
			Warning( "Layer %s already exist, choose different layer name",(const char*)dlg.m_name );
			return;
		}

		pLayer->SetName( dlg.m_name );
		pLayer->SetExternal( dlg.m_bExternal );
		pLayer->SetExportable( dlg.m_bExportToGame );
		pLayer->SetFrozen( dlg.m_bFrozen );
		pLayer->SetVisible( dlg.m_bVisible );

		ReloadLayers();
	}
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayLayer::OnLayersRButtonUp(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Assume that layer was already selected.
	CMenu menu;
	menu.CreatePopupMenu();

	menu.AppendMenu( MF_STRING,ID_PANEL_LAYERS_RENAME,"Settings" );
	menu.AppendMenu( MF_STRING,ID_PANEL_LAYERS_EXPORT,"Export" );
	menu.AppendMenu( MF_STRING,ID_PANEL_LAYERS_DELETE,"Delete" );

	CPoint pos;
	GetCursorPos( &pos );

	int cmd = menu.TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON,pos.x,pos.y,this );

	m_layersCtrl.Invalidate();

	*pResult = 1;
}