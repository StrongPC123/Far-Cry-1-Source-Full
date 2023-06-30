////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   vegetationpanel.cpp
//  Version:     v1.00
//  Created:     31/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VegetationPanel.h"
#include "VegetationTool.h"
#include "Heightmap.h"
#include "VegetationMap.h"
#include "StringDlg.h"
#include "Viewport.h"
#include "PanelPreview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVegetationPanel dialog
//////////////////////////////////////////////////////////////////////////
CVegetationPanel::CVegetationPanel(CVegetationTool *tool,CWnd* pParent /*=NULL*/)
	: CXTCBarDialog(CVegetationPanel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVegetationPanel)
	//}}AFX_DATA_INIT
	xtAfxData.bControlBarMenus = TRUE; // Turned off in constructor of CXTCBarDialog.

	assert( tool );
	m_tool = tool;
	m_category = "";
	m_previewPanel = NULL;
	m_vegetationMap = GetIEditor()->GetHeightmap()->GetVegetationMap();

	Create( IDD,pParent );
}

void CVegetationPanel::DoDataExchange(CDataExchange* pDX)
{
	CXTCBarDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVegetationPanel)
	//}}AFX_DATA_MAP

	DDX_Control(pDX, IDC_RADIUS, m_radius);
	DDX_Control(pDX, IDC_PAINT, m_paintButton);	
	DDX_Control(pDX, IDC_OBJECTS, m_objectsTree);
	DDX_Control(pDX, IDC_PROPERTIES, m_propertyCtrl);
	DDX_Control(pDX, IDC_INFO, m_info);
}


BEGIN_MESSAGE_MAP(CVegetationPanel, CXTCBarDialog)
	//{{AFX_MSG_MAP(CVegetationPanel)
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_RADIUS, OnReleasedcaptureRadius)
	ON_BN_CLICKED(IDC_PAINT, OnPaint)

	ON_NOTIFY(TVN_SELCHANGED, IDC_OBJECTS, OnObjectSelectionChanged)
	ON_NOTIFY(TVN_CHKCHANGE, IDC_OBJECTS, OnTvnHideObjects)
	ON_NOTIFY(TVN_KEYDOWN, IDC_OBJECTS, OnTvnKeydownObjects)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_OBJECTS, OnTvnBeginlabeleditObjects)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_OBJECTS, OnTvnEndlabeleditObjects)

	ON_COMMAND( ID_PANEL_VEG_ADD,OnAdd )
	ON_COMMAND( ID_PANEL_VEG_ADDCATEGORY,OnNewCategory )
	ON_COMMAND( ID_PANEL_VEG_CLONE,OnClone )
	ON_COMMAND( ID_PANEL_VEG_REPLACE,OnReplace )
	ON_COMMAND( ID_PANEL_VEG_REMOVE,OnRemove )

	ON_COMMAND( ID_PANEL_VEG_DISTRIBUTE,OnDistribute )
	ON_COMMAND( ID_PANEL_VEG_CLEAR,OnClear )
	ON_COMMAND( ID_PANEL_VEG_IMPORT,OnBnClickedImport )
	ON_COMMAND( ID_PANEL_VEG_EXPORT,OnBnClickedExport )
	ON_COMMAND( ID_PANEL_VEG_SCALE,OnBnClickedScale )
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CVegetationPanel message handlers

void CVegetationPanel::OnReleasedcaptureRadius(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int radius = m_radius.GetPos();
	m_tool->SetBrushRadius(radius/2.0f);

	AfxGetMainWnd()->SetFocus();

	*pResult = 0;
}

void CVegetationPanel::OnPaint() 
{
	bool paint = m_paintButton.GetCheck()==0;
	m_tool->SetMode( paint );
	if (m_paintButton.GetCheck() == 0)
		m_paintButton.SetCheck(1);
	else
		m_paintButton.SetCheck(0);
	AfxGetMainWnd()->SetFocus();
}

BOOL CVegetationPanel::OnInitDialog() 
{
	CXTCBarDialog::OnInitDialog();

	m_toolbar.Create( this,WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY );
	m_toolbar.LoadToolBar( IDR_PANEL_VEGETATION );
	RecalcBarLayout();

	m_treeImageList.Create( IDB_VEGETATION_TREE, 12, 1, RGB (255,255,255));
	m_treeStateImageList.Create( IDB_VEGETATION_TREE_STATE, 12, 1, RGB(255,255,255) );
	m_objectsTree.SetImageList(&m_treeImageList,TVSIL_NORMAL);
	m_objectsTree.SetImageList( &m_treeStateImageList, TVSIL_STATE );
	m_objectsTree.SetIndent( 0 );


	//DWORD style = ::GetWindowLong(m_objectsTree.GetSafeHwnd(),GWL_STYLE);
	//::SetWindowLong( m_objectsTree.GetSafeHwnd(),GWL_STYLE,style|TVS_CHECKBOXES );

	m_paintButton.SetCheck(0);
	m_paintButton.SetToolTip( "Hold Ctrl to remove" );

	m_radius.SetRange( 1,300 );
	m_radius.SetPos( m_tool->GetBrushRadius()*2.0f );

	ReloadObjects();

	/*
	CRect rc;
	GetDlgItem(IDC_PREVIEW)->GetWindowRect(rc);
	ScreenToClient( rc );
	GetDlgItem(IDC_PREVIEW)->ShowWindow(SW_HIDE);
	if (gSettings.bPreviewGeometryWindow)
		m_previewCtrl.Create( this,rc,WS_VISIBLE|WS_CHILD|WS_BORDER );
		*/

	SendToControls();
	UpdateInfo();

	SetResize( IDC_OBJECTS,SZ_HORRESIZE(1) );
	SetResize( IDC_PROPERTIES,SZ_HORRESIZE(1) );
	SetResize( IDC_INFO,SZ_HORRESIZE(1) );
	SetResize( IDC_PAINT,SZ_HORRESIZE(1) );
	SetResize( IDC_RADIUS,SZ_HORRESIZE(1) );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::GetObjectsInCategory( const CString &category,std::vector<CVegetationObject*> &objects )
{
	objects.clear();
	for (int i = 0; i < m_vegetationMap->GetObjectCount(); i++)
	{
		CVegetationObject *obj = m_vegetationMap->GetObject(i);
		if (category == obj->GetCategory())
		{
			objects.push_back(obj);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnNewCategory()
{
	CStringDlg dlg( "New Category" );
	if (dlg.DoModal() == IDOK)
	{
		if (m_categoryMap.find(dlg.GetString()) == m_categoryMap.end())
		{
			m_categoryMap[dlg.GetString()] = 0;
			ReloadObjects();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnDistribute()
{
	CUndo undo( "Vegetation Distribute" );
	BeginWaitCursor();
	if (m_tool)
		m_tool->Distribute();
	EndWaitCursor();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnDistributeMask()
{
	CUndo undo( "Vegetation Distribute Mask" );
	CString file;
	if (CFileUtil::SelectSingleFile( EFILE_TYPE_TEXTURE,file ))
	{
		BeginWaitCursor();
		if (m_tool)
			m_tool->DistributeMask( file );
		EndWaitCursor();
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnClear()
{
	CUndo undo( "Vegetation Clear" );
	BeginWaitCursor();
	if (m_tool)
		m_tool->Clear();
	EndWaitCursor();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnClearMask()
{
	CUndo undo( "Vegetation Clear Mask" );
	CString file;
	if (CFileUtil::SelectSingleFile( EFILE_TYPE_TEXTURE,file ))
	{
		BeginWaitCursor();
		if (m_tool)
			m_tool->ClearMask( file );
		EndWaitCursor();
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::SetObjectPanel( CVegetationObjectPanel *panel )
{
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnBnClickedScale()
{
	CUndo undo( "Vegetation Scale" );
	if (m_tool)
		m_tool->ScaleObjects();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnBnClickedImport()
{
	CUndo undo( "Vegetation Import" );
	CString file;
	if (CFileUtil::SelectFile( "Vegetation Objects(*.veg)|*.veg|All Files|*.*||",GetIEditor()->GetLevelFolder(),file ))
	{
		XmlParser parser;
		XmlNodeRef root = parser.parse(file);
		if (!root)
			return;

		m_propertyCtrl.DeleteAllItems();

		CWaitCursor wait;
		CUndo undo( "Import Vegetation" );
    for (int i = 0; i < root->getChildCount(); i++)
		{
			m_vegetationMap->ImportObject(root->getChild(i));
		}
		ReloadObjects();
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnBnClickedExport()
{
	CUndo undo( "Vegetation Export" );
	Selection objects;
	GetSelectedObjects( objects );
	if (objects.empty())
	{
		AfxMessageBox( "Select Objects For Export" );
		return;
	}

	CString fileName;
	if (CFileUtil::SelectSaveFile( "Vegetation Objects (*.veg)|*.veg||","veg",GetIEditor()->GetLevelFolder(),fileName ))
	{
		CWaitCursor wait;
		XmlNodeRef root = new CXmlNode( "Vegetation" );
		for (int i = 0; i < objects.size(); i++)
		{
			XmlNodeRef node = root->newChild( "VegetationObject" );
			m_vegetationMap->ExportObject( objects[i],node );
		}
		root->saveToFile( fileName );
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::ReloadObjects()
{
	std::map<CString,HTREEITEM>::iterator cit;
	
	m_objectsTree.DeleteAllItems();

	// Clear all selections.
	for (int i = 0; i < m_vegetationMap->GetObjectCount(); i++)
	{
		m_vegetationMap->GetObject(i)->SetSelected(false);
	}

	// Clear items within category.
	for (cit = m_categoryMap.begin(); cit != m_categoryMap.end(); ++cit)
	{
		HTREEITEM hRoot = m_objectsTree.InsertItem( cit->first,0,1 );
		m_objectsTree.SetItemData( hRoot,(DWORD_PTR)0 );
		m_objectsTree.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);
		cit->second = hRoot;
	}

	for (int i = 0; i < m_vegetationMap->GetObjectCount(); i++)
	{
		CVegetationObject* object = m_vegetationMap->GetObject(i);
		AddObjectToTree( object,false );
	}

	// Set initial category.
	if (m_category.IsEmpty() && !m_categoryMap.empty())
	{
		m_category = m_categoryMap.begin()->first;
	}

	m_objectsTree.Invalidate();
	
	// Give focus back to main view.
	AfxGetMainWnd()->SetFocus();
}


void CVegetationPanel::AddObjectToTree( CVegetationObject *object,bool bExpandCategory )
{
	CString str;
	char filename[_MAX_PATH];

	std::map<CString,HTREEITEM>::iterator cit;
	CString category = object->GetCategory();

	HTREEITEM hRoot = TVI_ROOT;
	cit = m_categoryMap.find(category);
	if (cit != m_categoryMap.end())
	{
		hRoot = cit->second;
	}
	if (hRoot == TVI_ROOT)
	{
		hRoot = m_objectsTree.InsertItem( category,0,1 );
		m_objectsTree.SetItemData( hRoot,(DWORD_PTR)0 );
		m_objectsTree.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);
		m_categoryMap[category] = hRoot;
	}

	// Add new category item.
	_splitpath( object->GetFileName(),0,0,filename,0 );
	str.Format( "%s (%d)",filename,object->GetNumInstances() );
	HTREEITEM hItem = m_objectsTree.InsertItem( str,2,3,hRoot );
	m_objectsTree.SetItemData( hItem,(DWORD_PTR)object );
	m_objectsTree.SetCheck( hItem,!object->IsHidden() );

	if (hRoot != TVI_ROOT)
	{
		if (bExpandCategory)
			m_objectsTree.Expand(hRoot,TVE_EXPAND);

		// Determine check state of category.
		std::vector<CVegetationObject*> objects;
		GetObjectsInCategory( category,objects );
		bool anyChecked = false;
		for (int i = 0; i < objects.size(); i++)
		{
			if (!objects[i]->IsHidden())
				anyChecked = true;
		}
		m_objectsTree.SetCheck( hRoot,anyChecked );
	}
}

void CVegetationPanel::UpdateObjectInTree( CVegetationObject *object,bool bUpdateInfo )
{
	CString str;
	char filename[_MAX_PATH];

	// Find object.
	TV_ITEM item;
	ZeroStruct(item);
	item.mask = TVIF_PARAM;
	item.lParam = (LPARAM)object;
	HTREEITEM hItem = m_objectsTree.FindNextItem( &item,NULL );
	if (hItem)
	{
		// Add new category item.
		_splitpath( object->GetFileName(),0,0,filename,0 );
		str.Format( "%s (%d)",filename,object->GetNumInstances() );
		m_objectsTree.SetItemText( hItem,str );
	}

	if (bUpdateInfo)
		UpdateInfo();
}

void CVegetationPanel::UpdateAllObjectsInTree()
{
	for (int i = 0; i < m_vegetationMap->GetObjectCount(); i++)
	{
		UpdateObjectInTree( m_vegetationMap->GetObject(i),false );
	}
	UpdateInfo();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::SelectObject( CVegetationObject *object )
{
	HTREEITEM hItem = m_objectsTree.GetSelectedItem();
	if (hItem)
	{
		// If already selected.
		if (m_objectsTree.GetItemData(hItem) == (DWORD_PTR)object)
			return;
	}

	// Find object.
	TV_ITEM item;
	ZeroStruct(item);
	item.mask = TVIF_PARAM;
	item.lParam = (LPARAM)object;
	hItem = m_objectsTree.FindNextItem( &item,NULL );
	if (hItem)
	{
		m_objectsTree.Select(hItem,TVGN_CARET);
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::UpdateInfo()
{
	CString str;
	int numObjects = m_vegetationMap->GetObjectCount();
	int numInstance = m_vegetationMap->GetNumInstances();
	str.Format( "Total Objects: %d\nTotal Instances: %d",numObjects,numInstance );
	// Update info.
	m_info.SetWindowText( str );
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::RemoveObjectFromTree( CVegetationObject *object )
{
	// Find object.
	TV_ITEM item;
	ZeroStruct(item);
	item.mask = TVIF_PARAM;
	item.lParam = (LPARAM)object;
	HTREEITEM hItem = m_objectsTree.FindNextItem( &item,NULL );
	if (hItem)
	{
		m_objectsTree.DeleteItem(hItem);
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnAdd()
{
	////////////////////////////////////////////////////////////////////////
	// Add another static object to the list
	////////////////////////////////////////////////////////////////////////
	std::vector<CString> files;
	if (!CFileUtil::SelectMultipleFiles( EFILE_TYPE_GEOMETRY,files ))
		return;

	CString category = m_category;

	CWaitCursor wait;

	CUndo undo( "Add VegObject(s)" );
	for (int i = 0; i < files.size(); i++)
	{
		// Create a new static object settings class
		CVegetationObject *obj = m_vegetationMap->CreateObject();
		if (!obj)
			continue;
		obj->SetFileName( files[i] );
		if (!category.IsEmpty())
			obj->SetCategory( m_category );

		AddObjectToTree( obj );
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnClone()
{
	Selection objects;
	GetSelectedObjects( objects );

	CUndo undo( "Clone VegObject" );
	for (int i = 0; i < objects.size(); i++)
	{
		CVegetationObject *object = objects[i];
		CVegetationObject *newObject = m_vegetationMap->CreateObject( object );
		if (newObject)
			AddObjectToTree( newObject );
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnReplace()
{
	HTREEITEM hItem = m_objectsTree.GetSelectedItem();
	if (hItem)
	{
		// Check if category selected.
		if (m_objectsTree.GetItemData(hItem) == 0)
		{
			m_category = m_objectsTree.GetItemText(hItem);
			// Rename category.
			CStringDlg dlg("Rename Category",this);
			dlg.m_strString = m_category;
			if (dlg.DoModal() == IDOK && m_category != dlg.GetString())
			{
				CString oldCategory = m_category;
				m_category = dlg.GetString();
				Selection objects;
				GetObjectsInCategory( oldCategory,objects );
				for (int i = 0; i < objects.size(); i++)
				{
					objects[i]->SetCategory( m_category );
				}
				m_categoryMap[m_category] = m_categoryMap[oldCategory];
				m_categoryMap.erase(oldCategory);
				ReloadObjects();
			}
			return;
		}
	}


	Selection objects;
	GetSelectedObjects( objects );
	if (objects.size() != 1)
		return;

	CVegetationObject *object = objects[0];
	if (!object)
		return;

	CString objectFolder = Path::GetPath(object->GetFileName());

	CString relFile;
	if (!CFileUtil::SelectSingleFile( EFILE_TYPE_GEOMETRY,relFile,"",objectFolder ))
		return;

	CWaitCursor wait;
	CUndo undo( "Replace VegObject" );
	object->SetFileName( relFile );
	m_vegetationMap->RepositionObject( object );

	RemoveObjectFromTree( object );
	AddObjectToTree( object );
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnRemove()
{
	Selection objects;
	GetSelectedObjects( objects );

	if (objects.empty())
		return;

	// validate
	if (AfxMessageBox( _T("Delete Vegetation Object(s)?"), MB_YESNO ) != IDYES)
	{
		return;
	}

	// Unselect all instances.
	m_tool->ClearThingSelection();

	CWaitCursor wait;
	CUndo undo( "Remove VegObject(s)" );

	for (int i = 0; i < objects.size(); i++)
	{
		RemoveObjectFromTree( objects[i] );
		m_propertyCtrl.DeleteAllItems();
		m_vegetationMap->RemoveObject( objects[i] );
		//if (!reloadObjects)
	}
	//ReloadObjects();

	GetIEditor()->UpdateViews( eUpdateStatObj );
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnObjectSelectionChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	// Clear all selections.
	for (int i = 0; i < m_vegetationMap->GetObjectCount(); i++)
	{
		m_vegetationMap->GetObject(i)->SetSelected(false);
	}

	HTREEITEM hItem = m_objectsTree.GetFirstSelectedItem();
	while (hItem)
	{
		CVegetationObject *object = (CVegetationObject*)m_objectsTree.GetItemData(hItem);
		if (object)
		{
			m_category = object->GetCategory();
			object->SetSelected(true);
		}
		else
		{
			// Category selected.
			m_category = m_objectsTree.GetItemText(hItem);

			Selection objects;
			GetObjectsInCategory( m_category,objects );
			for (int i = 0; i < objects.size(); i++)
			{
				objects[i]->SetSelected(true);
				//m_objectsTree.SelectChildren(hItem);
			}

			m_objectsTree.SelectChildren(hItem);
		}

		hItem = m_objectsTree.GetNextSelectedItem(hItem);
	}

	SendToControls();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnTvnKeydownObjects(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
	
	if (pTVKeyDown->wVKey == VK_DELETE)
	{
		// Delete current item.
		OnRemove();

		*pResult = TRUE;
		return;
	}
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnTvnBeginlabeleditObjects(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	if (pTVDispInfo->item.lParam || pTVDispInfo->item.pszText == 0)
	{
		// Not Category.
		// Cancel editing.
		*pResult = TRUE;
		return;
	}
	m_category = pTVDispInfo->item.pszText;
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnTvnEndlabeleditObjects(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	if (pTVDispInfo->item.lParam == 0 && pTVDispInfo->item.pszText != 0)
	{
		if (m_categoryMap.find(pTVDispInfo->item.pszText) != m_categoryMap.end())
		{
			// Cancel.
			*pResult = 0;
			return;
		}

		// Accept category name change.
		Selection objects;
		GetObjectsInCategory( m_category,objects );
		for (int i= 0; i < objects.size(); i++)
		{
			objects[i]->SetCategory( pTVDispInfo->item.pszText );
		}
		// Replace item in m_category map.
		m_categoryMap[pTVDispInfo->item.pszText] = m_categoryMap[m_category];
		m_categoryMap.erase( m_category );

		*pResult = TRUE; // Accept change.
		return;
	}
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::OnTvnHideObjects(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// Check if clicked on state image so we may be changed Checked state.
	*pResult = 0;

	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	CWaitCursor wait;

	CVegetationObject *object = (CVegetationObject*)pNMTreeView->itemNew.lParam;
	if (object)
	{
		bool bHidden = m_objectsTree.GetCheck(hItem) != TRUE;
		// Object.
		m_vegetationMap->HideObject( object,bHidden );
	}
	else
	{
		bool bHidden = m_objectsTree.GetCheck(hItem) != TRUE;
		// Category.
		CString category = m_objectsTree.GetItemText(hItem);
		std::vector<CVegetationObject*> objects;
		GetObjectsInCategory( category,objects );
		for (int i = 0; i < objects.size(); i++)
		{
			m_vegetationMap->HideObject( objects[i],bHidden );
		}
		// for all childs of this item set same check.
		hItem = m_objectsTree.GetNextItem(hItem,TVGN_CHILD);
		while (hItem)
		{
			m_objectsTree.SetCheck( hItem,!bHidden );
			hItem = m_objectsTree.GetNextSiblingItem(hItem);
		}
	}
}

void CVegetationPanel::GetSelectedObjects( std::vector<CVegetationObject*> &objects )
{
	objects.clear();
	for (int i = 0; i < m_vegetationMap->GetObjectCount(); i++)
	{
		CVegetationObject *obj = m_vegetationMap->GetObject(i);
		if (obj->IsSelected())
		{
			objects.push_back(obj);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationPanel::SendToControls()
{
	Selection objects;
	GetSelectedObjects( objects );

	// Delete all var block.
	m_varBlock = 0;
	m_propertyCtrl.DeleteAllItems();

	if (objects.empty())
	{
		if (m_previewPanel)
			m_previewPanel->LoadFile( "" );
		m_propertyCtrl.EnableWindow(FALSE);
		return;
	}
	else
	{
		m_propertyCtrl.EnableWindow(TRUE);
	}

	/*
	if (objects.size() == 1)
	{
		CVegetationObject *object = objects[0];
		if (m_previewPanel)
			m_previewPanel->LoadFile( object->GetFileName() );
		
		m_propertyCtrl.DeleteAllItems();
		m_propertyCtrl.AddVarBlock( object->GetVarBlock() );
		m_propertyCtrl.ExpandAll();

		m_propertyCtrl.SetDisplayOnlyModified(false);
	}
	else
	*/
	{
		m_varBlock = objects[0]->GetVarBlock()->Clone(true);
		m_varBlock->Wire( objects[0]->GetVarBlock() );

		for (int i = 1; i < objects.size(); i++)
		{
			m_varBlock->Wire( objects[i]->GetVarBlock() );
		}
		// Add variable blocks of all objects.
		m_propertyCtrl.AddVarBlock( m_varBlock );
		m_propertyCtrl.ExpandAll();

		if (objects.size() > 1)
		{
			m_propertyCtrl.SetDisplayOnlyModified(true);
		}
		else
		{
			m_propertyCtrl.SetDisplayOnlyModified(false);
		}
	}
	if (objects.size() == 1)
	{
		CVegetationObject *object = objects[0];
		if (m_previewPanel)
			m_previewPanel->LoadFile( object->GetFileName() );
	}
	else
	{
		if (m_previewPanel)
			m_previewPanel->LoadFile( "" );
	}
}