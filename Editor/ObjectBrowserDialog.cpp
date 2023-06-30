////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   objectbrowserdialog.cpp
//  Version:     v1.00
//  Created:     27/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "ObjectBrowserDialog.h"

#include "Objects\ObjectManager.h"

#define IMAGE_INDEX_LAYER 0
#define IMAGE_INDEX_TYPE 1
#define IMAGE_INDEX_OBJECT 2

// CObjectBrowserDialog dialog

IMPLEMENT_DYNAMIC(CObjectBrowserDialog, CDialog)
CObjectBrowserDialog::CObjectBrowserDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CObjectBrowserDialog::IDD, pParent)
{
	m_font.CreateFont( 14, 0, 0, 0,
		FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH, "Arial");
}

CObjectBrowserDialog::~CObjectBrowserDialog()
{
	m_font.DeleteObject();
}

void CObjectBrowserDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OBJECTS, m_tree);
}


BEGIN_MESSAGE_MAP(CObjectBrowserDialog, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CObjectBrowserDialog message handlers


BOOL CObjectBrowserDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_imageList.Create( MAKEINTRESOURCE(IDB_OBJECTS_BROWSER),16,1,RGB(255,255,255) );
	m_tree.SetMultiSelect( true );
	m_tree.SetImageList( &m_imageList,TVSIL_NORMAL );
	m_tree.SetFont( &m_font );

	CRect rc;
	GetClientRect( rc );
	rc.DeflateRect( 6,6,6,6 );
	m_tree.MoveWindow( rc );

	ReloadObjects();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CObjectBrowserDialog::ReloadObjects()
{
	//m_tree.DeleteAllItems();

	m_layersMap.clear();
	m_itemsMap.clear();
	m_objectsMap.clear();

	IObjectManager *pObjManager = GetIEditor()->GetObjectManager();

	int i;
	std::vector<CBaseObject*> objects;
	std::vector<CObjectLayer*> layers;

	pObjManager->GetObjects( objects );
	pObjManager->GetLayersManager()->GetLayers( layers );

	// Add Object Layers to Tree.
	for (i = 0; i < layers.size(); i++)
	{
		CObjectLayer *pLayer = layers[i];
		HTREEITEM hItem = m_tree.InsertItem( pLayer->GetName(),IMAGE_INDEX_LAYER,IMAGE_INDEX_LAYER,TVI_ROOT,TVI_SORT );
		m_tree.SetItemState( hItem,TVIS_BOLD,TVIS_BOLD );
		LayerItem li;
		li.hItem = hItem;
		li.objectCount = 0;
		m_layersMap[pLayer] = li;

		Item item(ITEM_LAYER);
		item.layer = pLayer;
		m_itemsMap[hItem] = item;
	}

	LayerItem defLayerItem;
	defLayerItem.hItem = 0;

	// Add Objects to Tree.
	for (i = 0; i < objects.size(); i++)
	{
		CBaseObject *pObject = objects[i];
		if (!pObject->GetParent())
			AddObject( pObject,0 );
	}

	// Change Layers text.
	{
		for (LayersMap::iterator it = m_layersMap.begin(); it != m_layersMap.end(); ++it)
		{
			CObjectLayer *pLayer = it->first;
			LayerItem &layerItem = it->second;
			CString layerName;
			layerName.Format( "%s (%d)",(const char*)pLayer->GetName(),layerItem.objectCount );
			m_tree.SetItemText( layerItem.hItem,layerName );

			// Change types text.
			for (std::map<CString,TypeItem>::iterator tit = layerItem.typeMap.begin(); tit != layerItem.typeMap.end(); ++tit)
			{
				TypeItem &typeItem = tit->second;
				CString typeName;
				typeName.Format( "%s (%d)",(const char*)tit->first,typeItem.objectCount );
				m_tree.SetItemText( typeItem.hItem,typeName );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectBrowserDialog::AddObject( CBaseObject *pObject,CBaseObject *pParent )
{
	CObjectLayer *pLayer = pObject->GetLayer();

	LayersMap::iterator it = m_layersMap.find(pLayer);
	assert( it != m_layersMap.end() );
	LayerItem &layerItem = it->second;
	layerItem.objectCount++;

	HTREEITEM hParentItem = 0;
	if (!pParent)
	{
		TypeItem ti;
		ZeroStruct(ti);

		CString typeName = pObject->GetTypeDescription();
		TypeItem &typeItem = stl::find_in_map_ref( layerItem.typeMap,typeName,ti );
		if (!typeItem.hItem)
		{
			// Add new Type item.
			HTREEITEM hItem = m_tree.InsertItem( typeName,IMAGE_INDEX_TYPE,IMAGE_INDEX_TYPE,layerItem.hItem,TVI_SORT );
			typeItem.hItem = hItem;
			typeItem.objectCount = 1;
			layerItem.typeMap[typeName] = typeItem;

			Item item(ITEM_TYPE);
			item.typeName = typeName;
			m_itemsMap[hParentItem] = item;
		}
		else
		{
			// Type item found.
			typeItem.objectCount++;
		}
		hParentItem = typeItem.hItem;
	}
	else
	{
		// Count number of objects in layer (
		hParentItem = stl::find_in_map( m_objectsMap,pParent,(HTREEITEM)0 );
	}
	HTREEITEM hItem = m_tree.InsertItem( pObject->GetName(),IMAGE_INDEX_OBJECT,IMAGE_INDEX_OBJECT,hParentItem,TVI_SORT );
	m_objectsMap[pObject] = hItem;

	Item item(ITEM_OBJECT);
	item.object = pObject;
	m_itemsMap[hItem] = item;


	// Add all childs.
	int numChilds = pObject->GetChildCount();
	for (int i = 0; i < numChilds; i++)
	{
		AddObject( pObject->GetChild(i),pObject );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectBrowserDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (m_tree.m_hWnd)
	{
		CRect rc;
		GetClientRect( rc );
		rc.DeflateRect( 6,6,6,6 );
		m_tree.MoveWindow( rc );
	}
}
