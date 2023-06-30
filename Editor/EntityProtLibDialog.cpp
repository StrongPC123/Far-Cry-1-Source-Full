////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   entityprotlibdialog.cpp
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EntityProtLibDialog.h"

#include "Objects\EntityScript.h"
#include "Objects\ObjectManager.h"
#include "Objects\ProtEntityObject.h"
#include "StringDlg.h"

#include "EntityPrototypeManager.h"
#include "EntityPrototypeLibrary.h"
#include "EntityPrototype.h"
#include "SelectEntityClsDialog.h"
#include "Clipboard.h"
#include "ViewManager.h"

#include <IEntitySystem.h>
#include <EntityDesc.h>

#define IDC_PROTOTYPES_TREE AFX_IDW_PANE_FIRST

IMPLEMENT_DYNAMIC(CEntityProtLibDialog,CBaseLibraryDialog);
//////////////////////////////////////////////////////////////////////////
// CEntityProtLibDialog implementation.
//////////////////////////////////////////////////////////////////////////
CEntityProtLibDialog::CEntityProtLibDialog( CWnd *pParent )
	: CBaseLibraryDialog(IDD_DB_ENTITY, pParent)
{
	m_entity = 0;
	m_pEntityManager = GetIEditor()->GetEntityProtManager();
	m_pItemManager = m_pEntityManager;
	m_bEntityPlaying = false;
	m_bShowDescription = false;

	// Immidiatly create dialog.
	Create( IDD_DB_ENTITY,pParent );
}

CEntityProtLibDialog::~CEntityProtLibDialog()
{
}

void CEntityProtLibDialog::DoDataExchange(CDataExchange* pDX)
{
	CBaseLibraryDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEntityProtLibDialog, CBaseLibraryDialog)
	ON_COMMAND( ID_DB_ADD,OnAddPrototype )
	ON_COMMAND( ID_DB_SAVE,OnSave )
	ON_COMMAND( ID_DB_PLAY,OnPlay )
	ON_COMMAND( ID_DB_LOADLIB,OnLoadLibrary )
	ON_COMMAND( ID_DB_RELOAD,OnReloadEntityScript )
	ON_COMMAND( ID_DB_DESCRIPTION,OnShowDescription )
	ON_UPDATE_COMMAND_UI( ID_DB_PLAY,OnUpdatePlay )
	ON_COMMAND( ID_DB_ASSIGNTOSELECTION,OnAssignToSelection )
	ON_UPDATE_COMMAND_UI( ID_DB_ASSIGNTOSELECTION,OnUpdateSelected )
	//ON_EN_CHANGE( IDC_DESCRIPTION,OnDescriptionChange )
	ON_EN_CHANGE( AFX_IDW_PANE_FIRST+1,OnDescriptionChange ) // Second plane

	ON_COMMAND( ID_DB_SELECTASSIGNEDOBJECTS,OnSelectAssignedObjects )
	ON_UPDATE_COMMAND_UI( ID_DB_SELECTASSIGNEDOBJECTS,OnUpdateSelected )

	ON_NOTIFY(TVN_BEGINDRAG, IDC_PROTOTYPES_TREE, OnBeginDrag)
	ON_NOTIFY(NM_RCLICK , IDC_PROTOTYPES_TREE, OnNotifyTreeRClick)

	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnDestroy()
{
	CBaseLibraryDialog::OnDestroy();
}

// CTVSelectKeyDialog message handlers
BOOL CEntityProtLibDialog::OnInitDialog()
{
	CBaseLibraryDialog::OnInitDialog();

	m_pEntitySystem = GetIEditor()->GetSystem()->GetIEntitySystem();

	InitToolbar();

	CRect rc;
	GetClientRect(rc);
	//int h2 = rc.Height()/2;
	int h2 = 300;

	m_wndVSplitter.CreateStatic( this,1,2,WS_CHILD|WS_VISIBLE );
	m_wndHSplitter.CreateStatic( &m_wndVSplitter,2,1,WS_CHILD|WS_VISIBLE );
	m_wndScriptPreviewSplitter.CreateStatic( &m_wndHSplitter,1,2,WS_CHILD|WS_VISIBLE );

	//m_imageList.Create(IDB_MATERIAL_TREE, 16, 1, RGB (255, 0, 255));
	CMFCUtils::LoadTrueColorImageList( m_imageList,IDB_ENTITY_TREE,16,RGB(255,0,255) );

	// Attach it to the control
	m_treeCtrl.SetParent( &m_wndVSplitter );
	m_treeCtrl.SetImageList(&m_imageList,TVSIL_NORMAL);

	CRect scriptDlgRc;
	m_scriptDialog.Create( CEntityScriptDialog::IDD,&m_wndScriptPreviewSplitter );
	m_scriptDialog.ShowWindow( SW_SHOW );
	m_scriptDialog.GetClientRect( scriptDlgRc );
	m_scriptDialog.SetOnReloadScript( functor(*this,&CEntityProtLibDialog::OnReloadEntityScript) );
	
	m_previewCtrl.Create( &m_wndScriptPreviewSplitter,rc,WS_CHILD|WS_VISIBLE );
	//m_previewCtrl.Create( WS_VISIBLE|WS_CHILD,rc,&m_wndHSplitter,1 );
	m_propsCtrl.Create( WS_VISIBLE|WS_CHILD|WS_BORDER,rc,&m_wndHSplitter,2 );

	m_descriptionEditBox.InitOnCreate(true);
	m_descriptionEditBox.Create( ES_MULTILINE|ES_WANTRETURN|WS_CHILD|WS_TABSTOP,rc,this,IDC_DESCRIPTION );
	m_descriptionEditBox.ShowWindow( SW_HIDE );

	m_wndScriptPreviewSplitter.SetPane( 0,0,&m_scriptDialog,CSize(scriptDlgRc.Width()+4,scriptDlgRc.Height()+4) );
	//m_wndScriptPreviewSplitter.SetPane( 0,1,&m_previewCtrl,CSize(200,h2) );

	m_previewCtrl.SetDlgCtrlID(100);
	m_previewCtrl.ShowWindow( SW_HIDE );
	m_wndScriptPreviewSplitter.SetPane( 0,1,&m_descriptionEditBox,CSize(200,100) );
	m_descriptionEditBox.ShowWindow( SW_SHOW );
	m_bShowDescription = true;

	m_wndHSplitter.SetPane( 0,0,&m_wndScriptPreviewSplitter,CSize(100,h2) );
	m_wndHSplitter.SetPane( 1,0,&m_propsCtrl,CSize(100,h2) );

	m_wndVSplitter.SetPane( 0,0,&m_treeCtrl,CSize(200,100) );
	m_wndVSplitter.SetPane( 0,1,&m_wndHSplitter,CSize(200,100) );
	//m_wndVSplitter.CreateView( 1,1,

	RecalcLayout();

	ReloadLibs();
	ReloadItems();

	m_scriptDialog.Invalidate();
	m_scriptDialog.ShowWindow( SW_SHOW );

	m_wndVSplitter.RecalcLayout();
	m_wndHSplitter.RecalcLayout();
	m_wndScriptPreviewSplitter.RecalcLayout();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
UINT CEntityProtLibDialog::GetDialogMenuID()
{
	return IDR_DB_ENTITY;
};

//////////////////////////////////////////////////////////////////////////
// Create the toolbar
void CEntityProtLibDialog::InitToolbar()
{
	VERIFY( m_toolbar.CreateEx(this, TBSTYLE_FLAT|TBSTYLE_WRAPABLE,
		WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC) );
	VERIFY( m_toolbar.LoadToolBar24(IDR_DB_ENTITY_BAR) );

	m_toolbar.SetButtonStyle( m_toolbar.CommandToIndex(ID_DB_PLAY),TBBS_CHECKBOX );

	CBaseLibraryDialog::InitToolbar();
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnSize(UINT nType, int cx, int cy)
{
	CBaseLibraryDialog::OnSize(nType, cx, cy);
	
	// resize splitter window.
	if (m_wndVSplitter.m_hWnd)
	{
		CRect rc;
		GetClientRect(rc);
		m_wndVSplitter.MoveWindow(rc,FALSE);

		m_toolbar.SetWindowPos(NULL, 0, 0, rc.right, 70, SWP_NOZORDER);
		//m_scriptDialog.Invalidate();
	}
	RecalcLayout();

	if (m_wndVSplitter.m_hWnd)
	{
		m_scriptDialog.Invalidate();
		Invalidate();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnNewDocument()
{
	// Clear all prototypes and libraries.
	GetIEditor()->GetEntityProtManager()->ClearAll();
	CBaseLibraryDialog::OnNewDocument();
};

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnLoadDocument()
{
	CBaseLibraryDialog::OnLoadDocument();
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnCloseDocument()
{
	CBaseLibraryDialog::OnCloseDocument();
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnAddPrototype()
{
	if (!m_pLibrary)
		return;
	CString library = m_selectedLib;
	if (library.IsEmpty())
		return;
	CString entityClass = SelectEntityClass();
	if (entityClass.IsEmpty())
		return;

	CStringGroupDlg dlg( _T("New Entity Name"),this );
	dlg.SetGroup( m_selectedGroup );
	dlg.SetString( entityClass );
	if (dlg.DoModal() != IDOK || dlg.GetString().IsEmpty())
	{
		return;
	}

	CEntityPrototype *prototype = (CEntityPrototype*)m_pEntityManager->CreateItem( m_pLibrary );
	
	// Make prototype name.
	SetItemName( prototype,dlg.GetGroup(),dlg.GetString() );
	// Assign entity class to prototype.
	prototype->SetEntityClassName( entityClass );

	ReloadItems();
	SelectItem( prototype );
}

//////////////////////////////////////////////////////////////////////////
CString CEntityProtLibDialog::SelectEntityClass()
{
	CSelectEntityClsDialog dlg;
	if (dlg.DoModal() == IDOK)
	{
		return dlg.GetEntityClass();
	}
	return "";
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::SelectItem( CBaseLibraryItem *item,bool bForceReload )
{
	bool bChanged = item != m_pCurrentItem || bForceReload;
	CBaseLibraryDialog::SelectItem( item,bForceReload );
	
	if (!bChanged)
		return;

	CEntityPrototype *prototype = (CEntityPrototype*)item;

	// Empty preview control.
	m_previewCtrl.SetEntity(0);
	m_previewCtrl.LoadFile( "",false );

	m_propsCtrl.DeleteAllItems();
	if (prototype && prototype->GetProperties())
	{
		CString scriptName;
		if (prototype->GetScript())
		{
			scriptName = prototype->GetScript()->GetName();
			scriptName += " ";
		}
		m_propsCtrl.AddVarBlock( prototype->GetProperties() );
		m_propsCtrl.SetRootName( scriptName + "Properties" );
		m_propsCtrl.ExpandAll();
		m_propsCtrl.SetUpdateCallback( functor(*this,&CEntityProtLibDialog::OnUpdateProperties) );
	}

	if (prototype)
	{
		m_descriptionEditBox.SetWindowText( prototype->GetDescription() );
	}

	ReleaseEntity();
	if (prototype)
	{
		CRect rc;
		m_previewCtrl.GetClientRect(rc);
		if (rc.Width() > 5 && rc.Height() > 5 && !m_bShowDescription)
			SpawnEntity( prototype );
		else
		{
			if (prototype->GetScript())
				m_scriptDialog.SetScript( prototype->GetScript(),0 );
		}
	}

	if (!m_visualObject.IsEmpty())
		m_previewCtrl.LoadFile( m_visualObject,false );
	if (m_entity)
		m_previewCtrl.SetEntity(m_entity);
	m_previewCtrl.SetGrid(true);
	m_previewCtrl.EnableUpdate(true);
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::SpawnEntity( CEntityPrototype *prototype )
{
	assert( prototype );

	ReleaseEntity();

	CEntityScript *script = prototype->GetScript();
	if (!script)
		return;
	// Load script if its not loaded yet.
	if (!script->IsValid())
	{
		if (!script->Load())
			return;
	}
	CEntityDesc ed;
	ed.ClassId = script->GetClsId();
	ed.name = prototype->GetName();
	
	m_entity = m_pEntitySystem->SpawnEntity( ed,false );
	if (m_entity)
	{
		if (prototype->GetProperties())
		{
			// Assign to entity properties of prototype.
			script->SetProperties( m_entity,prototype->GetProperties(),false );
		}
		// Initialize properties.
		if (!m_pEntitySystem->InitEntity( m_entity,ed ))
			m_entity = 0;

		//////////////////////////////////////////////////////////////////////////
		// Make visual object for this entity.
		//////////////////////////////////////////////////////////////////////////
		m_visualObject = script->GetVisualObject();

		m_scriptDialog.SetScript( script,m_entity );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::ReleaseEntity()
{
	m_visualObject = "";
	if (m_entity)
	{
		m_entity->SetDestroyable(true);
		m_pEntitySystem->RemoveEntity( m_entity->GetId(),true );
	}
	m_entity = 0;
	m_scriptDialog.SetScript( 0,0 );
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::Update()
{
	if (!m_bEntityPlaying)
		return;

	// Update preview control.
	if (m_entity)
	{
		m_previewCtrl.Update();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnUpdateProperties( IVariable *var )
{
	CEntityPrototype *prototype = GetSelectedPrototype();
	if (prototype != 0)
	{
		// Mark prototype library modified.
		prototype->GetLibrary()->SetModified();

		CEntityScript *script = prototype->GetScript();
		CVarBlock *props = prototype->GetProperties();
		if (script && props && m_entity != 0)
		{
			// Set entity properties.
			script->SetProperties( m_entity,props,true );
		}
		prototype->Update();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnShowDescription()
{
	m_previewCtrl.SetDlgCtrlID(100);
	m_wndScriptPreviewSplitter.SetPane( 0,1,&m_descriptionEditBox,CSize(200,100) );
	m_wndScriptPreviewSplitter.RecalcLayout();
	m_descriptionEditBox.ShowWindow( SW_SHOW );
	m_previewCtrl.ShowWindow( SW_HIDE );
	m_bShowDescription = true;
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnPlay()
{
	m_bEntityPlaying = !m_bEntityPlaying;

	if (m_bShowDescription)
	{
		m_descriptionEditBox.SetDlgCtrlID(101);
		m_wndScriptPreviewSplitter.SetPane( 0,1,&m_previewCtrl,CSize(200,100) );
		m_descriptionEditBox.ShowWindow( SW_HIDE );
		m_previewCtrl.ShowWindow( SW_SHOW );
		m_bShowDescription = false;
		m_wndScriptPreviewSplitter.RecalcLayout();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnUpdatePlay( CCmdUI* pCmdUI )
{
	if (m_bEntityPlaying)
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnReloadEntityScript()
{
	CEntityPrototype *prototype = GetSelectedPrototype();
	if (prototype)
	{
		prototype->Reload();
		TSmartPtr<CEntityPrototype> sel = prototype;
		SelectItem( prototype );
	}
}

//////////////////////////////////////////////////////////////////////////
CEntityPrototype* CEntityProtLibDialog::GetSelectedPrototype()
{
	CBaseLibraryItem* item = m_pCurrentItem;
	return (CEntityPrototype*)item;
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnDescriptionChange()
{
	CString desc;
	m_descriptionEditBox.GetWindowText( desc );
	if (GetSelectedPrototype())
	{
		GetSelectedPrototype()->SetDescription( desc );
		GetSelectedPrototype()->GetLibrary()->SetModified();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnAssignToSelection()
{
	CEntityPrototype *pPrototype = GetSelectedPrototype();
	if (!pPrototype)
		return;

	CUndo undo( "Assign Archetype" );

	CSelectionGroup *pSel = GetIEditor()->GetSelection();
	if (!pSel->IsEmpty())
	{
		for (int i = 0; i < pSel->GetCount(); i++)
		{
			CBaseObject *pObject = pSel->GetObject(i);
			if (pObject->IsKindOf(RUNTIME_CLASS(CProtEntityObject)))
			{
				CProtEntityObject *pProtEntity = (CProtEntityObject*)pObject;
				pProtEntity->SetPrototype( pPrototype,true );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnSelectAssignedObjects()
{
	CEntityPrototype *pItem = GetSelectedPrototype();
	if (!pItem)
		return;

	CBaseObjectsArray objects;
	GetIEditor()->GetObjectManager()->GetObjects( objects );
	for (int i = 0; i < objects.size(); i++)
	{
		CBaseObject *pObject = objects[i];
		if (pObject->IsKindOf(RUNTIME_CLASS(CProtEntityObject)))
		{
			CProtEntityObject *protEntity = (CProtEntityObject*)pObject;
			if (protEntity->GetPrototype() != pItem)
				continue;
			if (pObject->IsHidden() || pObject->IsFrozen())
				continue;
			GetIEditor()->GetObjectManager()->SelectObject( pObject );
		}
	}
}
	
//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnNotifyTreeRClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Show helper menu.
	CPoint point;

	CEntityPrototype *pItem = 0;

	// Find node under mouse.
	GetCursorPos( &point );
	m_treeCtrl.ScreenToClient( &point );
	// Select the item that is at the point myPoint.
	UINT uFlags;
	HTREEITEM hItem = m_treeCtrl.HitTest(point,&uFlags);
	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		pItem = (CEntityPrototype*)m_treeCtrl.GetItemData(hItem);
	}

	if (!pItem)
		return;

	SelectItem( pItem );

	// Create pop up menu.
	CMenu menu;
	menu.CreatePopupMenu();

	if (pItem)
	{
		CClipboard clipboard;
		bool bNoPaste = clipboard.IsEmpty();
		int pasteFlags = 0;
		if (bNoPaste)
			pasteFlags |= MF_GRAYED;

		menu.AppendMenu( MF_STRING,ID_DB_CUT,"Cut" );
		menu.AppendMenu( MF_STRING,ID_DB_COPY,"Copy" );
		menu.AppendMenu( MF_STRING|pasteFlags,ID_DB_PASTE,"Paste" ); 
		menu.AppendMenu( MF_STRING,ID_DB_CLONE,"Clone" ); 
		menu.AppendMenu( MF_SEPARATOR,0,"" );
		menu.AppendMenu( MF_STRING,ID_DB_RENAME,"Rename" );
		menu.AppendMenu( MF_STRING,ID_DB_REMOVE,"Delete" );
		menu.AppendMenu( MF_SEPARATOR,0,"" );
		menu.AppendMenu( MF_STRING,ID_DB_ASSIGNTOSELECTION,"Assign to Selected Objects" );
		menu.AppendMenu( MF_STRING,ID_DB_SELECTASSIGNEDOBJECTS,"Select Assigned Objects" );
		menu.AppendMenu( MF_STRING,ID_DB_RELOAD,"Reload" );
	}

	GetCursorPos( &point );
	menu.TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this );
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	CBaseLibraryItem *pItem = (CEntityPrototype*)m_treeCtrl.GetItemData(hItem);
	if (!pItem)
		return;

	m_pDraggedItem = pItem;

	m_treeCtrl.Select( hItem,TVGN_CARET );

	m_dragImage = m_treeCtrl.CreateDragImage( hItem );
	if (m_dragImage)
	{
		m_dragImage->BeginDrag(0, CPoint(-10, -10));

		CRect rc;
		AfxGetMainWnd()->GetWindowRect( rc );

		CPoint p = pNMTreeView->ptDrag;
		ClientToScreen( &p );
		p.x -= rc.left;
		p.y -= rc.top;

		m_dragImage->DragEnter( AfxGetMainWnd(),p );
		SetCapture();
		GetIEditor()->EnableUpdate( false );
	}

	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_dragImage)
	{
		CRect rc;
		AfxGetMainWnd()->GetWindowRect( rc );

		CPoint p = point;
		ClientToScreen( &p );
		p.x -= rc.left;
		p.y -= rc.top;
		m_dragImage->DragMove( p );

		SetCursor( m_hCursorDefault );
		// Check if can drop here.
		{
			CPoint p;
			GetCursorPos( &p );
			CViewport* viewport = GetIEditor()->GetViewManager()->GetViewportAtPoint( p );
			if (viewport)
			{
				SetCursor( m_hCursorCreate );
				CPoint vp = p;
				viewport->ScreenToClient(&vp);
				ObjectHitInfo hit( viewport,vp );
				if (viewport->HitTest( vp,hit,0 ))
				{
					if (hit.object && hit.object->IsKindOf(RUNTIME_CLASS(CProtEntityObject)))
					{
						SetCursor( m_hCursorReplace );
					}
				}
			}
		}
	}

	CBaseLibraryDialog::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_dragImage)
	{
		CPoint p;
		GetCursorPos( &p );

		GetIEditor()->EnableUpdate( true );

		m_dragImage->DragLeave( AfxGetMainWnd() );
		m_dragImage->EndDrag();
		delete m_dragImage;
		m_dragImage = 0;
		ReleaseCapture();

		CViewport* viewport = GetIEditor()->GetViewManager()->GetViewportAtPoint( p );
		if (viewport)
		{
			bool bHit = false;
			CPoint vp = p;
			viewport->ScreenToClient(&vp);
			// Drag and drop into one of views.
			// Start object creation.
			ObjectHitInfo hit( viewport,vp );
			if (viewport->HitTest( vp,hit,0 ))
			{
				if (hit.object)
				{
					if (hit.object->IsKindOf(RUNTIME_CLASS(CProtEntityObject)))
					{
						bHit = true;
						CUndo undo( "Assign Archetype" );
						((CProtEntityObject*)hit.object)->SetPrototype( (CEntityPrototype*)m_pDraggedItem,false );
					}
				}
			}
			if (!bHit)
			{
				CUndo undo( "Create EntityPrototype" );
				CString guid = GuidUtil::ToString( m_pDraggedItem->GetGUID() );
				GetIEditor()->StartObjectCreation( "PrototypeEntity",guid );
			}
		}
		m_pDraggedItem = 0;
	}

	CBaseLibraryDialog::OnLButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnCopy()
{
	CBaseLibraryItem *pItem = m_pCurrentItem;
	if (pItem)
	{
		CClipboard clipboard;
		XmlNodeRef node = new CXmlNode( "EntityPrototype" );
		CBaseLibraryItem::SerializeContext ctx(node,false);
		ctx.bCopyPaste = true;
		pItem->Serialize( ctx );
		clipboard.Put( node );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityProtLibDialog::OnPaste()
{
	if (!m_pLibrary)
		return;

	CClipboard clipboard;
	XmlNodeRef node = clipboard.Get();
	if (!node)
		return;

	if (strcmp(node->getTag(),"EntityPrototype") == 0)
	{
		// This is material node.
		CBaseLibrary *pLib = m_pLibrary;
		CEntityPrototype *pItem = m_pEntityManager->LoadPrototype( (CEntityPrototypeLibrary*)pLib,node );
		if (pItem)
		{
			pItem->SetName( m_pEntityManager->MakeUniqItemName(pItem->GetName()) );
			ReloadItems();
			SelectItem(pItem);
		}
	}
}