////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   MaterialDialog.cpp
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MusicEditorDialog.h"

#include "StringDlg.h"
#include "NumberDlg.h"

#include "MusicManager.h"
#include "MusicThemeLibrary.h"
#include "MusicThemeLibItem.h"

#include "ViewManager.h"
#include "Clipboard.h"

#include <Isound.h>

#include "MusicEditorUI.h"

#define IDC_TREE_CTRL AFX_IDW_PANE_FIRST
#define IDC_FILE_TREE_CTRL AFX_IDW_PANE_FIRST + 1*16

#define IMAGE_THEME 0
#define IMAGE_MOOD 1
#define IMAGE_PATTERN_SET 2
#define IMAGE_LAYER 3
#define IMAGE_PATTERN 4

//////////////////////////////////////////////////////////////////////////
static CMusicEditorUI_Theme sMusicUI_Theme;
static CMusicEditorUI_Mood sMusicUI_Mood;
static CMusicEditorUI_PatternSet sMusicUI_PatternSet;
static CMusicEditorUI_Layer sMusicUI_Layer;
static CMusicEditorUI_Pattern sMusicUI_Pattern;

IMPLEMENT_DYNAMIC(CMusicEditorDialog,CBaseLibraryDialog);

//////////////////////////////////////////////////////////////////////////
// CMusicEditorDialog implementation.
//////////////////////////////////////////////////////////////////////////
CMusicEditorDialog::CMusicEditorDialog( CWnd *pParent )
	: CBaseLibraryDialog(IDD_DB_ENTITY, pParent)
{
	m_pMusicManager = GetIEditor()->GetMusicManager();
	m_pMusicData = m_pMusicManager->GetMusicData();
	m_pItemManager = m_pMusicManager;

	m_hDropItem = NULL;
	m_hDraggedItem = NULL;
	m_pSound = 0;

	m_bPlaying = false;

	m_bHandleDeleteItem = false;

	m_lastItemType = (ETreeItemType)-1;

	m_musicFilesPath = "Music";

	m_pMusicSystem = GetIEditor()->GetSystem()->GetIMusicSystem();

	// Immidiatly create dialog.
	Create( IDD_DB_ENTITY,pParent );
}

CMusicEditorDialog::~CMusicEditorDialog()
{
}

void CMusicEditorDialog::DoDataExchange(CDataExchange* pDX)
{
	CBaseLibraryDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMusicEditorDialog, CBaseLibraryDialog)
	ON_COMMAND( ID_DB_ADD,OnAddItem )
	ON_UPDATE_COMMAND_UI( ID_DB_PLAY,OnUpdatePlay )
	ON_COMMAND( ID_DB_MTL_ADDSUBMTL,OnAddSubItem )

	ON_COMMAND( ID_DB_PLAY,OnPlay )
	ON_COMMAND( ID_DB_STOP,OnStop )
	ON_COMMAND( ID_DB_MUSIC_LOADFROMLUA,OnLoadFromLua )

	ON_COMMAND( ID_DB_CUT,OnCut )
	ON_COMMAND( ID_DB_COPY,OnCopy )
	ON_COMMAND( ID_DB_PASTE,OnPaste )
	ON_COMMAND( ID_DB_CLONE,OnClone )
	ON_COMMAND( ID_DB_MUSIC_RELOADFILES,OnRealodMusicFiles )

	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_CTRL, OnSelChangedItemTree)

	ON_NOTIFY(TVN_BEGINDRAG, IDC_TREE_CTRL, OnBeginDrag)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE_CTRL, OnBeginLabelEdit)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE_CTRL, OnEndLabelEdit)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TREE_CTRL, OnTreeDeleteItem )
	ON_NOTIFY(NM_RCLICK , IDC_TREE_CTRL, OnNotifyTreeRClick)
	ON_NOTIFY(NM_DBLCLK , IDC_TREE_CTRL, OnNotifyTreeDblClick)

	//////////////////////////////////////////////////////////////////////////
	// File tree.
	ON_NOTIFY(NM_CLICK , IDC_FILE_TREE_CTRL, OnNotifyFileTreeClick)
	ON_NOTIFY(TVN_SELCHANGED, IDC_FILE_TREE_CTRL, OnSelectFileTree)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_FILE_TREE_CTRL, OnFilesBeginDrag)
	ON_NOTIFY(NM_KILLFOCUS, IDC_FILE_TREE_CTRL, OnFileTreeKillFocus)

	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnDestroy()
{
	int temp;
	int HSplitter,VSplitter;
	m_wndHSplitter.GetRowInfo( 0,HSplitter,temp );
	m_wndVSplitter.GetColumnInfo( 0,VSplitter,temp );
	AfxGetApp()->WriteProfileInt("Dialogs\\MusicEditor","HSplitter",HSplitter );
	AfxGetApp()->WriteProfileInt("Dialogs\\MusicEditor","VSplitter",VSplitter );

	CBaseLibraryDialog::OnDestroy();
}

// CTVSelectKeyDialog message handlers
BOOL CMusicEditorDialog::OnInitDialog()
{
	CBaseLibraryDialog::OnInitDialog();

	m_pMusicSystem = GetIEditor()->GetSystem()->GetIMusicSystem();

	InitToolbar();

	CRect rc;
	GetClientRect(rc);
	//int h2 = rc.Height()/2;
	int h2 = 200;

	int VSplitter = AfxGetApp()->GetProfileInt("Dialogs\\MusicEditor","VSplitter",300 );
	int HSplitter = AfxGetApp()->GetProfileInt("Dialogs\\MusicEditor","HSplitter",100 );

	//m_imageList.Create(IDB_MUSIC_TREE, 16, 1, RGB (255, 0, 255));
	CMFCUtils::LoadTrueColorImageList( m_imageList,IDB_MUSIC_TREE,16,RGB(255,0,255) );

	m_wndVSplitter.CreateStatic( this,1,2,WS_CHILD|WS_VISIBLE );
	m_wndHSplitter.CreateStatic( &m_wndVSplitter,2,1,WS_CHILD|WS_VISIBLE );

	// TreeCtrl must be already created.
	//m_treeCtrl.EnableMultiSelect(TRUE);
	m_treeCtrl.ModifyStyle( 0,TVS_EDITLABELS );
	m_treeCtrl.SetParent( &m_wndVSplitter );
	m_treeCtrl.SetImageList(&m_imageList,TVSIL_NORMAL);

	m_propsCtrl.Create( WS_VISIBLE|WS_CHILD|WS_BORDER,rc,&m_wndVSplitter,1 );
	//m_vars = gMatVars.CreateVars();
	//m_propsCtrl.AddVarBlock( m_vars );
	m_propsCtrl.EnableWindow(FALSE);

	//////////////////////////////////////////////////////////////////////////
	// Attach it to the control
	m_filesImageList.Create(IDB_SOUND_FILES, 16, 1, RGB (255, 0, 255));
	m_filesTreeCtrl.Create( WS_VISIBLE|WS_CHILD|WS_TABSTOP|WS_BORDER|TVS_HASBUTTONS|TVS_LINESATROOT|TVS_HASLINES,CRect(),this,IDC_FILE_TREE_CTRL );
	m_filesTreeCtrl.SetParent( &m_wndHSplitter );
	m_filesTreeCtrl.SetImageList(&m_filesImageList,TVSIL_NORMAL);
	//////////////////////////////////////////////////////////////////////////

	m_wndHSplitter.SetPane( 0,0,&m_propsCtrl,CSize(100,HSplitter) );
	m_wndHSplitter.SetPane( 1,0,&m_filesTreeCtrl,CSize(100,HSplitter) );

	m_wndVSplitter.SetPane( 0,0,&m_treeCtrl,CSize(VSplitter,100) );
	m_wndVSplitter.SetPane( 0,1,&m_wndHSplitter,CSize(VSplitter,100) );

	RecalcLayout();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
UINT CMusicEditorDialog::GetDialogMenuID()
{
	return IDR_DB_ENTITY;
};

//////////////////////////////////////////////////////////////////////////
// Create the toolbar
void CMusicEditorDialog::InitToolbar()
{
	VERIFY( m_toolbar.CreateEx(this, TBSTYLE_FLAT|TBSTYLE_WRAPABLE,
		WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC) );
	VERIFY( m_toolbar.LoadToolBar24(IDR_DB_MUSIC_BAR) );

	// Resize the toolbar
	CRect rc;
	GetClientRect(rc);
	m_toolbar.SetWindowPos(NULL, 0, 0, rc.right, 70, SWP_NOZORDER);
	CSize sz = m_toolbar.CalcDynamicLayout(TRUE,TRUE);
	//m_toolbar.SetButtonStyle( m_toolbar.CommandToIndex(ID_DB_PLAY),TBBS_CHECKBOX );

	CBaseLibraryDialog::InitToolbar();
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnSize(UINT nType, int cx, int cy)
{
	// resize splitter window.
	if (m_wndVSplitter.m_hWnd)
	{
		CRect rc;
		GetClientRect(rc);
		m_wndVSplitter.MoveWindow(rc,FALSE);
	}
	CBaseLibraryDialog::OnSize(nType, cx, cy);
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnNewDocument()
{
	//////////////////////////////////////////////////////////////////////////
	CBaseLibraryDialog::OnNewDocument();
	if (m_pSound)
		m_pSound->Stop();
	m_pSound = 0;

	if (m_pMusicSystem) // [marco] crashes in previewer
		m_pMusicSystem->SetData( m_pMusicData,true );
};

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnLoadDocument()
{
	CBaseLibraryDialog::OnLoadDocument();
	if (m_pSound)
		m_pSound->Stop();
	m_pSound = 0;
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnCloseDocument()
{
	CBaseLibraryDialog::OnCloseDocument();
	if (m_pSound)
		m_pSound->Stop();
	m_pSound = 0;
}

//////////////////////////////////////////////////////////////////////////
HTREEITEM CMusicEditorDialog::InsertItemToTree( CBaseLibraryItem *pItem,HTREEITEM hParent )
{
	/*
	CMusicThemeLibItem *pMtl = (CMusicThemeLibItem*)pItem;
	if (pMtl->GetParent())
	{
		if (!hParent || hParent == TVI_ROOT || m_treeCtrl.GetItemData(hParent) == 0)
			return 0;
	}

	HTREEITEM hMtlItem = CBaseLibraryDialog::InsertItemToTree( pItem,hParent );

	for (int i = 0; i < pMtl->GetSubMaterialCount(); i++)
	{
		CMusicThemeLibItem *pSubMtl = pMtl->GetSubMaterial(i);
		CBaseLibraryDialog::InsertItemToTree( pSubMtl,hMtlItem );
	}
	return hMtlItem;
	*/
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::SelectItem( CBaseLibraryItem *item,bool bForceReload )
{
	/*
	bool bChanged = item != m_pCurrentItem || bForceReload;
	CBaseLibraryDialog::SelectItem( item,bForceReload );
	
	if (!bChanged)
		return;

	// Empty preview control.
	m_previewCtrl.SetEntity(0);
	m_pMatManager->SetCurrentMaterial( (CMusicThemeLibItem*)item );

	if (!item)
	{
		m_propsCtrl.EnableWindow(FALSE);
		return;
	}

	if (!m_pEntityRender)
	{
		LoadGeometry( m_geometryFile );
	}
	if (m_pEntityRender)
		m_previewCtrl.SetEntity( m_pEntityRender );

	m_propsCtrl.EnableWindow(TRUE);
	m_propsCtrl.EnableUpdateCallback(false);

	// Render preview geometry with current material
	CMusicThemeLibItem *mtl = GetSelectedMaterial();

	AssignMtlToGeometry();

	// Update variables.
	m_propsCtrl.EnableUpdateCallback(false);
	gMatVars.SetFromMaterial( mtl );
	m_propsCtrl.EnableUpdateCallback(true);

	//gMatVars.m_onSetCallback = functor(*this,OnUpdateProperties);

	if (m_publicVarsItems)
	{
		m_propsCtrl.DeleteItem( m_publicVarsItems );
		m_publicVarsItems = 0;
	}

	m_publicVars = mtl->GetPublicVars();
	if (m_publicVars)
	{
		m_publicVarsItems = m_propsCtrl.AddVarBlock( m_publicVars,"Shader Params" );
	}
	m_propsCtrl.ExpandAllChilds( m_propsCtrl.GetRootItem(),false );

	m_propsCtrl.SetUpdateCallback( functor(*this,OnUpdateProperties) );
	m_propsCtrl.EnableUpdateCallback(true);
	*/
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::Update()
{
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnUpdateProperties( IVariable *var )
{
	ItemDesc *pItemDesc = GetCurrentItem();
	switch (pItemDesc->type)
	{
	case EITEM_THEME:
		sMusicUI_Theme.Get( pItemDesc->m_pTheme );
		break;
	case EITEM_MOOD:
		sMusicUI_Mood.Get( pItemDesc->m_pMood );
		break;
	case EITEM_PATTERN_SET:
		sMusicUI_PatternSet.Get( pItemDesc->m_pPatternSet );
		break;
	case EITEM_LAYER:
		sMusicUI_Layer.Get( pItemDesc->m_pPatternSet,pItemDesc->layerType );
		break;
	case EITEM_PATTERN:
		{
			sMusicUI_Pattern.Get(	pItemDesc->m_pPattern );

			// Update total probabilities.
			SMusicPatternSet *pPatternSet = pItemDesc->m_pPatternSet;
			CalcProbabilities( pPatternSet );
			// Update pattern description in Music.
			m_pMusicSystem->UpdatePattern( pItemDesc->m_pPattern );
		}
		break;
	};
	SetModified();

	/*
	CMusicThemeLibItem *mtl = GetSelectedMaterial();
	if (!mtl)
		return;

	bool bLightingOnChanged = &gMatVars.bLighting == var;
	bool bShaderChanged = (var == &gMatVars.shader);

	// Assign new public vars to material.
	if (m_publicVarsItems != NULL && m_publicVars != NULL)
	{
		if (!bShaderChanged)
		{
			// No need to change public vars.
			mtl->SetPublicVars( m_publicVars );
		}
	}

	gMatVars.SetToMaterial( mtl );
	if (bShaderChanged || bLightingOnChanged)
	{
		gMatVars.SetFromMaterial( mtl );
	}
	gMatVars.SetTextureNames( mtl );

	AssignMtlToGeometry();

	// When shader changed.
	if (bShaderChanged)
	{
		// Delete old public params and add new ones. 
		if (m_publicVarsItems)
		{
			m_propsCtrl.DeleteItem( m_publicVarsItems );
			m_publicVarsItems = 0;
		}
		m_publicVars = mtl->GetPublicVars();
		if (m_publicVars)
		{
			m_publicVarsItems = m_propsCtrl.AddVarBlock( m_publicVars,"Shader Params" );
		}
	}
	if (bLightingOnChanged)
		m_propsCtrl.Invalidate();

	*/
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnPlay()
{
	m_bPlaying = true;

	m_pMusicSystem->Pause(false);

	ItemDesc *pItem = GetCurrentItem();
	if (pItem)
	{
		if (m_bPlaying)
		{
			if (pItem->type == EITEM_PATTERN)
			{
				m_pMusicSystem->PlayPattern( pItem->m_pPattern->sName.c_str(),false );
			}
			else
			{
				if (pItem->m_pTheme)
					m_pMusicSystem->SetTheme( pItem->m_pTheme->sName.c_str() );
				if (pItem->m_pMood)
					m_pMusicSystem->SetMood( pItem->m_pMood->sName.c_str() );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnUpdatePlay( CCmdUI* pCmdUI )
{
	if (m_bPlaying)
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnStop()
{
	m_pMusicSystem->Silence();
	m_pMusicSystem->Pause(true);
	m_bPlaying = false;
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	/*
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	CMusicThemeLibItem* pMtl = (CMusicThemeLibItem*)m_treeCtrl.GetItemData(hItem);
	if (!pMtl)
		return;


	m_treeCtrl.Select( hItem,TVGN_CARET );

	m_hDropItem = 0;
	m_dragImage = m_treeCtrl.CreateDragImage( hItem );
	if (m_dragImage)
	{
		m_hDraggedItem = hItem;
		m_hDropItem = hItem;
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
	*/
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_dragImage)
	{
		CPoint p;

		p = point;
		ClientToScreen( &p );
		m_treeCtrl.ScreenToClient( &p );

		TVHITTESTINFO hit;
		ZeroStruct(hit);
		hit.pt = p;
		HTREEITEM hHitItem = m_treeCtrl.HitTest( &hit );
		if (m_hDropItem != hHitItem)
		{
			if (m_hDropItem)
			{
				m_treeCtrl.SetItem( m_hDropItem,TVIF_STATE,0,0,0,0,TVIS_DROPHILITED,0 );
				m_treeCtrl.Invalidate();
			}
		}
		if (hHitItem)
		{
			if (m_hDropItem != hHitItem)
			{
				m_hDropItem = 0;

				ItemDesc *pItemDesc = (ItemDesc*)m_treeCtrl.GetItemData(hHitItem);
				if (pItemDesc && (pItemDesc->type == EITEM_PATTERN || pItemDesc->type == EITEM_LAYER))
				{
					// Set state of this item to drop target.
					m_treeCtrl.SetItem( hHitItem,TVIF_STATE,0,0,0,TVIS_DROPHILITED,TVIS_DROPHILITED,0 );
					m_treeCtrl.Invalidate();
					m_hDropItem = hHitItem;
				}
			}
		}

		CRect rc;
		AfxGetMainWnd()->GetWindowRect( rc );
		p = point;
		ClientToScreen( &p );
		p.x -= rc.left;
		p.y -= rc.top;
		m_dragImage->DragMove( p );
	}

	CBaseLibraryDialog::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	//CXTResizeDialog::OnLButtonUp(nFlags, point);

	if (m_hDropItem)
	{
		m_treeCtrl.SetItem( m_hDropItem,TVIF_STATE,0,0,0,0,TVIS_DROPHILITED,0 );
		m_hDropItem = 0;
	}

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

		CPoint treepoint = p;
		m_treeCtrl.ScreenToClient( &treepoint );

		TVHITTESTINFO hit;
		ZeroStruct(hit);
		hit.pt = treepoint;
		HTREEITEM hHitItem = m_treeCtrl.HitTest( &hit );
		if (hHitItem)
		{
			DropToItem( hHitItem,m_hDraggedItem );
			m_hDraggedItem = 0;
			return;
		}
	}
	m_hDraggedItem = 0;

	CBaseLibraryDialog::OnLButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnNotifyTreeRClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	// Show helper menu.
	CPoint point;

	ItemDesc *pItemDesc = 0;

	// Find node under mouse.
	GetCursorPos( &point );
	m_treeCtrl.ScreenToClient( &point );
	// Select the item that is at the point myPoint.
	UINT uFlags;
	HTREEITEM hItem = m_treeCtrl.HitTest(point,&uFlags);
	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		pItemDesc = (ItemDesc*)m_treeCtrl.GetItemData(hItem);
	}

	m_treeCtrl.Select( hItem,TVGN_CARET );

	// Create pop up menu.
	CMenu menu;
	menu.CreatePopupMenu();
	
	CString pasteStr = "Paste";

	CClipboard clipboard;
	bool bNoPaste = clipboard.IsEmpty();
	int pasteFlags = 0;
	if (bNoPaste)
		pasteFlags |= MF_GRAYED;
	else
	{
		pasteStr = pasteStr + " " + clipboard.GetTitle();
	}

	if (pItemDesc)
	{
		switch (pItemDesc->type)
		{
		case EITEM_THEME:
			menu.AppendMenu( MF_STRING,ID_DB_PLAY,"Play Theme" );
			menu.AppendMenu( MF_STRING,ID_DB_ADD,"New Theme" );
			menu.AppendMenu( MF_STRING,ID_DB_MTL_ADDSUBMTL,"New Mood" );
			menu.AppendMenu( MF_STRING,ID_DB_MTL_ADDSUBMTL,"New Bridge" );
			menu.AppendMenu( MF_SEPARATOR,0,"" );
			break;
		case EITEM_MOOD:
			menu.AppendMenu( MF_STRING,ID_DB_PLAY,"Play Mood" );
			//menu.AppendMenu( MF_STRING,ID_DB_PLAY,"Play As Default Mood" );
			menu.AppendMenu( MF_STRING,ID_DB_ADD,"New Mood" );
			menu.AppendMenu( MF_STRING,ID_DB_MTL_ADDSUBMTL,"New Pattern Set" );
			menu.AppendMenu( MF_SEPARATOR,0,"" );
			break;
		case EITEM_PATTERN_SET:
			menu.AppendMenu( MF_STRING,ID_DB_PLAY,"Play Mood" );
			menu.AppendMenu( MF_STRING,ID_DB_ADD,"New Pattern Set" );
			menu.AppendMenu( MF_SEPARATOR,0,"" );
			break;
		case EITEM_LAYER:
			menu.AppendMenu( MF_STRING,ID_DB_PLAY,"Play Mood" );
			menu.AppendMenu( MF_STRING,ID_DB_ADD,"New Pattern" );
			menu.AppendMenu( MF_SEPARATOR,0,"" );
			break;
		case EITEM_PATTERN:
			menu.AppendMenu( MF_STRING,ID_DB_PLAY,"Play Pattern" );
			menu.AppendMenu( MF_STRING,ID_DB_ADD,"New Pattern" );
			menu.AppendMenu( MF_SEPARATOR,0,"" );
			break;
		};

		if (pItemDesc->type != EITEM_LAYER)
		{
			menu.AppendMenu( MF_STRING,ID_DB_CUT,"Cut" );
			menu.AppendMenu( MF_STRING,ID_DB_COPY,"Copy" );
			menu.AppendMenu( MF_STRING|pasteFlags,ID_DB_PASTE,pasteStr );
			menu.AppendMenu( MF_STRING,ID_DB_CLONE,"Clone" ); 
			menu.AppendMenu( MF_SEPARATOR,0,"" );
			menu.AppendMenu( MF_STRING,ID_DB_RENAME,"Rename" );
			menu.AppendMenu( MF_STRING,ID_DB_REMOVE,"Delete" );
		}
		else
		{
			menu.AppendMenu( MF_STRING|pasteFlags,ID_DB_PASTE,pasteStr );
			menu.AppendMenu( MF_SEPARATOR,0,"" );
			menu.AppendMenu( MF_STRING,ID_DB_REMOVE,"Delete Patterns" );
		}
	}
	else
	{
		// No items.
		menu.AppendMenu( MF_STRING,ID_DB_ADD,"New Theme" );
		menu.AppendMenu( MF_SEPARATOR,0,"" );
		menu.AppendMenu( MF_STRING|pasteFlags,ID_DB_PASTE,pasteStr );
	}

	GetCursorPos( &point );
	menu.TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this );
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnNotifyTreeDblClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	// Show helper menu.
	CPoint point;

	ItemDesc *pItemDesc = 0;

	// Find node under mouse.
	GetCursorPos( &point );
	m_treeCtrl.ScreenToClient( &point );
	// Select the item that is at the point myPoint.
	UINT uFlags;
	HTREEITEM hItem = m_treeCtrl.HitTest(point,&uFlags);
	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		pItemDesc = (ItemDesc*)m_treeCtrl.GetItemData(hItem);
	}

	if (!pItemDesc)
		return;
	m_treeCtrl.Select( hItem,TVGN_CARET );

	/*
	if (pItemDesc->type == EITEM_PATTERN)
	{
		bool bExclusive = !m_bPlaying;
		m_pMusicSystem->PlayPattern( pItemDesc->m_pPattern->sName.c_str(),bExclusive );
		if (bExclusive)
			m_bPlaying = false;
	}
	else
	*/
		if (m_bPlaying)
			OnPlay();
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnCopy()
{
	CClipboard clipboard;

	ItemDesc *pItemDesc = GetCurrentItem();
	if (!pItemDesc)
		return;
	switch (pItemDesc->type)
	{
	case EITEM_THEME:
		// Copy Theme.
		{
			XmlNodeRef node = new CXmlNode( "Theme" );
			CBaseLibraryItem::SerializeContext ctx( node,false );
			ctx.bCopyPaste = true;
			((CMusicThemeLibItem*)(pItemDesc->pItem))->Serialize( ctx );
			clipboard.Put( node );
		}
		break;
	case EITEM_MOOD:
		// Copy Mood.
		{
			XmlNodeRef node = new CXmlNode( "Mood" );
			CBaseLibraryItem::SerializeContext ctx( node,false );
			ctx.bCopyPaste = true;
			CMusicThemeLibItem::SerializeMood( ctx,pItemDesc->m_pMood );
			clipboard.Put( node );
		}
		break;
	case EITEM_PATTERN_SET:
		// Copy pattern set.
		{
			XmlNodeRef node = new CXmlNode( "PatternSet" );
			CBaseLibraryItem::SerializeContext ctx( node,false );
			ctx.bCopyPaste = true;
			CMusicThemeLibItem::SerializePatternSet( ctx,pItemDesc->m_pPatternSet );
			clipboard.Put( node );
		}
		break;
	case EITEM_LAYER:
		
		break;
	case EITEM_PATTERN:
		{
			XmlNodeRef node = new CXmlNode( "Pattern" );
			CBaseLibraryItem::SerializeContext ctx( node,false );
			ctx.bCopyPaste = true;
			CMusicThemeLibItem::SerializePattern( ctx,pItemDesc->m_pPattern );
			clipboard.Put( node );
		}
		break;
	};
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnPaste()
{
	if (!m_pLibrary)
		return;

	CClipboard clipboard;
	if (clipboard.IsEmpty())
		return;
	XmlNodeRef node = clipboard.Get();
	if (!node)
		return;

	ItemDesc *pItemDesc = GetCurrentItem();

	if (node->isTag("Pattern"))
	{
		if (pItemDesc && pItemDesc->m_pPatternSet)
			NewPattern( pItemDesc->m_pPatternSet,pItemDesc->layerType,NULL,node );
	}
	else if (node->isTag("PatternSet"))
	{
		if (pItemDesc && pItemDesc->m_pMood)
			PastePatternSet( pItemDesc->m_pMood,node );
	}
	else if (node->isTag("Mood"))
	{
		if (pItemDesc && pItemDesc->m_pTheme)
			PasteMood( pItemDesc->m_pTheme,node );
	}
	else if (node->isTag("Theme"))
	{
		PasteTheme( node );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::DropToItem( HTREEITEM hItem,HTREEITEM hSrcItem )
{
	if (m_pLibrary)
	{
		m_pLibrary->SetModified();
	}
	CString filename = stl::find_in_map( m_musicFilesMap,hSrcItem,"" );
	if (!filename.IsEmpty())
	{
		ItemDesc *pItemDesc = (ItemDesc*)m_treeCtrl.GetItemData(hItem);
		if (pItemDesc)
		{
			switch(pItemDesc->type) {
			case EITEM_LAYER:
				{
					CString fname = Path::GetFileName(filename);
					SPatternDef *pPattern = NewPattern( pItemDesc->m_pPatternSet,pItemDesc->layerType,fname );
					pPattern->sFilename = (const char*)filename;
					m_pMusicSystem->UpdatePattern( pPattern );
				}
				break;
			case EITEM_PATTERN:
				pItemDesc->m_pPattern->sFilename = (const char*)filename;
				m_pMusicSystem->UpdatePattern( pItemDesc->m_pPattern );
				break;
			}

			if (GetCurrentItem() == pItemDesc)
				SelectItem( pItemDesc );
		}
	}
	SetModified();
}

//////////////////////////////////////////////////////////////////////////
HTREEITEM CMusicEditorDialog::InsertItem_Theme( CMusicThemeLibItem *pItem )
{
	SMusicTheme *pTheme = pItem->GetTheme();
	HTREEITEM hItem = m_treeCtrl.InsertItem( pTheme->sName.c_str(),IMAGE_THEME,IMAGE_THEME,TVI_ROOT );
	ItemDesc *pItemDesc = new ItemDesc;
	pItemDesc->type = EITEM_THEME;
	pItemDesc->pItem = pItem;
	pItemDesc->m_pTheme = pTheme;
	m_treeCtrl.SetItemData( hItem,(DWORD_PTR)pItemDesc );

	m_ThemeToItemMap[pTheme] = hItem;
	
	for (TMoodMap::iterator it = pTheme->mapMoods.begin(); it != pTheme->mapMoods.end(); ++it)
	{
		InsertItem_Mood( pItemDesc,it->second,hItem );
	}
	return hItem;
}

//////////////////////////////////////////////////////////////////////////
HTREEITEM CMusicEditorDialog::InsertItem_Mood( ItemDesc *pParent,SMusicMood *pMood,HTREEITEM parentItem )
{
	if (!parentItem)
	{
		parentItem = stl::find_in_map( m_ThemeToItemMap,pParent->m_pTheme, (HTREEITEM)0 );
		if (!parentItem)
			parentItem = TVI_ROOT;
	}
	HTREEITEM hItem = m_treeCtrl.InsertItem( pMood->sName.c_str(),IMAGE_MOOD,IMAGE_MOOD,parentItem );
	ItemDesc *pItemDesc = new ItemDesc( *pParent );
	pItemDesc->type = EITEM_MOOD;
	pItemDesc->m_pMood = pMood;
	m_treeCtrl.SetItemData( hItem,(DWORD_PTR)pItemDesc );

	m_MoodToItemMap[pMood] = hItem;

	for (int i = 0; i < pMood->vecPatternSets.size();i++)
	{
		InsertItem_PatternSet( pItemDesc,pMood->vecPatternSets[i],hItem );
	}
	return hItem;
}

//////////////////////////////////////////////////////////////////////////
HTREEITEM CMusicEditorDialog::InsertItem_PatternSet( ItemDesc *pParent,SMusicPatternSet *pPatterns,HTREEITEM parentItem )
{
	if (!parentItem)
	{
		parentItem = stl::find_in_map( m_MoodToItemMap,pParent->m_pMood, (HTREEITEM)0 );
		if (!parentItem)
			parentItem = TVI_ROOT;
	}
	HTREEITEM hItem = m_treeCtrl.InsertItem( "Pattern Set",IMAGE_PATTERN_SET,IMAGE_PATTERN_SET,parentItem );
	ItemDesc *pItemDesc = new ItemDesc( *pParent );
	pItemDesc->type = EITEM_PATTERN_SET;
	pItemDesc->m_pPatternSet = pPatterns;
	m_treeCtrl.SetItemData( hItem,(DWORD_PTR)pItemDesc );

	m_PatternSetToItemMap[pPatterns] = hItem;

	// Add 3 pattern layers.
	HTREEITEM hMainLayer = m_treeCtrl.InsertItem( "Main Layer",IMAGE_LAYER,IMAGE_LAYER,hItem );
	HTREEITEM hRhythmicLayer = m_treeCtrl.InsertItem( "Rhytmic Layer",IMAGE_LAYER,IMAGE_LAYER,hItem );
	HTREEITEM hIncidentalLayer = m_treeCtrl.InsertItem( "Incidental Layer",IMAGE_LAYER,IMAGE_LAYER,hItem );

	ItemDesc *pItemDesc1 = new ItemDesc(*pItemDesc,ELAYER_MAIN);
	ItemDesc *pItemDesc2 = new ItemDesc(*pItemDesc,ELAYER_RHYTHMIC);
	ItemDesc *pItemDesc3 = new ItemDesc(*pItemDesc,ELAYER_INCIDENTAL);

	m_treeCtrl.SetItemData( hMainLayer,(DWORD_PTR)(pItemDesc1) );
	m_treeCtrl.SetItemData( hRhythmicLayer,(DWORD_PTR)(pItemDesc2) );
	m_treeCtrl.SetItemData( hIncidentalLayer,(DWORD_PTR)(pItemDesc3) );

	int i;
	for (i = 0; i < pPatterns->vecMainPatterns.size();i++)
	{
		InsertItem_Pattern( pItemDesc1,pPatterns->vecMainPatterns[i],hMainLayer );
	}
	for (i = 0; i < pPatterns->vecRhythmicPatterns.size();i++)
	{
		InsertItem_Pattern( pItemDesc2,pPatterns->vecRhythmicPatterns[i],hRhythmicLayer );
	}
	for (i = 0; i < pPatterns->vecIncidentalPatterns.size();i++)
	{
		InsertItem_Pattern( pItemDesc3,pPatterns->vecIncidentalPatterns[i],hIncidentalLayer );
	}
	return hItem;
}

//////////////////////////////////////////////////////////////////////////
HTREEITEM CMusicEditorDialog::InsertItem_Pattern( ItemDesc *pParent,SPatternDef *pPattern,HTREEITEM parentItem )
{
	if (!parentItem)
	{
		if (!parentItem)
			parentItem = TVI_ROOT;
	}
	// Find this pattern.
	//m_pMusicData->vecPatternDef
	HTREEITEM hItem = m_treeCtrl.InsertItem( pPattern->sName.c_str(),IMAGE_PATTERN,IMAGE_PATTERN,parentItem );
	ItemDesc *pItemDesc = new ItemDesc( *pParent );
	pItemDesc->type = EITEM_PATTERN;
	pItemDesc->m_pPattern = pPattern;
	m_treeCtrl.SetItemData( hItem,(DWORD_PTR)pItemDesc );

	m_PatternToItemMap[pPattern] = hItem;
	return hItem;
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::ReloadItems()
{
	m_bIgnoreSelectionChange = true;
	m_pCurrentItem = 0;
	m_itemsToTree.clear();
	m_treeCtrl.DeleteAllItems();
	m_bIgnoreSelectionChange = false;

	if (!m_pLibrary)
		return;

	m_bIgnoreSelectionChange = true;

	m_treeCtrl.SetRedraw(FALSE);
	m_treeCtrl.DeleteAllItems();

	// Clear all maps.
	m_PatternsMap.clear();
	// Clear all maps.
	m_ThemeToItemMap.clear();
	m_MoodToItemMap.clear();
	m_PatternSetToItemMap.clear();
	m_PatternToItemMap.clear();

	if (!m_pMusicData)
		return;
	for (int i = 0; i < m_pMusicData->vecPatternDef.size(); i++)
	{
		SPatternDef *pPattern = m_pMusicData->vecPatternDef[i];
		m_PatternsMap[pPattern->sName.c_str()] = pPattern;
	}

	//////////////////////////////////////////////////////////////////////////
	HTREEITEM hLibItem = TVI_ROOT;
	for (int i = 0; i < m_pLibrary->GetItemCount(); i++)
	{
		CMusicThemeLibItem *pItem = (CMusicThemeLibItem*)m_pLibrary->GetItem(i);

		InsertItem_Theme( pItem );
	}
	//////////////////////////////////////////////////////////////////////////


	m_treeCtrl.SetRedraw(TRUE);
	m_bIgnoreSelectionChange = false;
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnSelChangedItemTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_bIgnoreSelectionChange)
		return;
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (m_pSound)
		m_pSound->Stop();
	m_pSound = 0;

	m_pCurrentItem = 0;
	if (m_treeCtrl)
	{
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		if (hItem != 0 && hItem != TVI_ROOT)
		{
			// Change currently selected item.
			ItemDesc *desc = (ItemDesc*)m_treeCtrl.GetItemData(hItem);
			assert(desc);
			SelectItem( desc );
		}
	}

	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::SelectItem( ItemDesc *pItemDesc )
{
	assert(pItemDesc);

	m_propsCtrl.EnableUpdateCallback(false);
	
	if (m_lastItemType != pItemDesc->type)
	{
		m_lastItemType = pItemDesc->type;
		// Recreate UI variables.
		m_propsCtrl.DeleteAllItems();
		m_vars = 0;
		switch (pItemDesc->type)
		{
		case EITEM_THEME:
			m_vars = sMusicUI_Theme.CreateVars();
			break;
		case EITEM_MOOD:
			m_vars = sMusicUI_Mood.CreateVars();
			break;
		case EITEM_PATTERN_SET:
			m_vars = sMusicUI_PatternSet.CreateVars();
			break;
		case EITEM_LAYER:
			m_vars = sMusicUI_Layer.CreateVars();
			break;
		case EITEM_PATTERN:
			m_vars = sMusicUI_Pattern.CreateVars();
			break;
		}
		if (m_vars)
		{
			m_propsCtrl.AddVarBlock( m_vars );
			m_propsCtrl.ExpandAll();
			m_propsCtrl.EnableWindow(TRUE);
		}
		else
			m_propsCtrl.EnableWindow(FALSE);
	}

	switch (pItemDesc->type)
	{
	case EITEM_THEME:
		if (m_vars)
		{
			m_propsCtrl.DeleteAllItems();
			sMusicUI_Theme.Set( pItemDesc->m_pTheme );
			m_propsCtrl.AddVarBlock( m_vars );
			m_propsCtrl.ExpandAll();
		}
		break;
	case EITEM_MOOD:
		sMusicUI_Mood.Set( pItemDesc->m_pMood );
		break;
	case EITEM_PATTERN_SET:
		sMusicUI_PatternSet.Set( pItemDesc->m_pPatternSet );
		break;
	case EITEM_LAYER:
		sMusicUI_Layer.Set( pItemDesc->m_pPatternSet,pItemDesc->layerType );
		break;
	case EITEM_PATTERN:
		sMusicUI_Pattern.Set( pItemDesc->m_pPattern );
		break;
	}
	m_propsCtrl.SetUpdateCallback( functor(*this, &CMusicEditorDialog::OnUpdateProperties) );
	m_propsCtrl.EnableUpdateCallback(true);
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVDISPINFO *pNM = (NMTVDISPINFO*)pNMHDR;
	
	*pResult = 0;

	ItemDesc *pItemDesc = (ItemDesc*)m_treeCtrl.GetItemData( pNM->item.hItem );
	if (!pItemDesc)
	{
		*pResult = TRUE;
		return;
	}
	switch (pItemDesc->type)
	{
	case EITEM_LAYER:
	case EITEM_PATTERN_SET:
		*pResult = TRUE;
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVDISPINFO *pNM = (NMTVDISPINFO*)pNMHDR;

	*pResult = 0;

	const char *sText = pNM->item.pszText;
	if (!sText)
		return;

	ItemDesc *pItemDesc = (ItemDesc*)m_treeCtrl.GetItemData( pNM->item.hItem );
	if (!pItemDesc)
		return;

	switch (pItemDesc->type)
	{
	case EITEM_THEME:
		m_pMusicData->mapThemes.erase( pItemDesc->m_pTheme->sName.c_str() );
		m_pMusicData->mapThemes[sText] = pItemDesc->m_pTheme;
		pItemDesc->m_pTheme->sName = sText;
		m_treeCtrl.SetItemText( pNM->item.hItem,sText );
		pItemDesc->pItem->SetName( sText );
		m_pMusicSystem->UpdateTheme( pItemDesc->m_pTheme );
		break;
	case EITEM_MOOD:
		pItemDesc->m_pTheme->mapMoods.erase( pItemDesc->m_pMood->sName );
		pItemDesc->m_pMood->sName = sText;
		pItemDesc->m_pTheme->mapMoods[sText] = pItemDesc->m_pMood;
		m_treeCtrl.SetItemText( pNM->item.hItem,sText );
		m_pMusicSystem->UpdateMood( pItemDesc->m_pMood );
		break;
	case EITEM_PATTERN_SET:
		//pItemDesc->m_pPatternSet->s
		break;
	case EITEM_PATTERN:
		// New name must be unique.
		if (m_PatternsMap.find(sText) == m_PatternsMap.end())
		{
			// Rename pattern.
			m_pMusicSystem->RenamePattern( pItemDesc->m_pPattern->sName.c_str(),sText );
			m_PatternsMap.erase(pItemDesc->m_pPattern->sName.c_str() ); // Old remove from map.
			m_PatternsMap[sText] = pItemDesc->m_pPattern; // New add to map.
			pItemDesc->m_pPattern->sName = sText;
			m_treeCtrl.SetItemText( pNM->item.hItem,sText );
		}
		break;
	}
	SetModified();
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::SetModified()
{
	if (m_pLibrary)
		m_pLibrary->SetModified();
	GetIEditor()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
CMusicEditorDialog::ItemDesc* CMusicEditorDialog::GetCurrentItem()
{
	//if (m_treeCtrl.GetSelectedCount() != 1)
		//return 0;

	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();
	if (!hItem)
		return 0;

	ItemDesc *pItemDesc = (ItemDesc*)m_treeCtrl.GetItemData( hItem );
	return pItemDesc;
}

SMusicTheme* CMusicEditorDialog::NewTheme( const char *sBaseName )
{
	SMusicTheme *pTheme = new SMusicTheme;
	if (!sBaseName || strlen(sBaseName) == 0)
		sBaseName = "Theme0";
	pTheme->sName = MakeUniqThemeName(sBaseName);
	pTheme->fDefaultMoodTimeout = 0;
	m_pMusicData->mapThemes[pTheme->sName] = pTheme;

	CMusicThemeLibItem *pItem = (CMusicThemeLibItem*)m_pMusicManager->CreateItem( m_pLibrary );
	pItem->SetTheme( pTheme );

	m_treeCtrl.SetRedraw(FALSE);
	HTREEITEM hNewItem = InsertItem_Theme( pItem );
	m_treeCtrl.Select( hNewItem,TVGN_CARET );
	m_treeCtrl.SetRedraw(TRUE);

	m_pMusicSystem->UpdateTheme( pTheme );

	return pTheme;
}

//////////////////////////////////////////////////////////////////////////
SMusicMood* CMusicEditorDialog::NewMood( SMusicTheme *pTheme,const char *sBaseName,bool bNewPatternSet )
{
	SMusicMood *pMood = new SMusicMood;
	if (!sBaseName)
		sBaseName = "Mood0";
	pMood->sName = MakeUniqMoodName(pTheme,sBaseName);
	pMood->nPriority = 0;
	pMood->fFadeOutTime = DEFAULT_CROSSFADE_TIME;
	pMood->bPlaySingle = false;
	pMood->pCurrPatternSet = 0;

	pTheme->mapMoods[pMood->sName] = pMood;

	m_treeCtrl.SetRedraw(FALSE);
	//////////////////////////////////////////////////////////////////////////
	// Add item to tree.
	//////////////////////////////////////////////////////////////////////////
	HTREEITEM hTheme = stl::find_in_map( m_ThemeToItemMap,pTheme, (HTREEITEM)0 );
	if (hTheme)
	{
		ItemDesc *pThemeItemDesc = (ItemDesc*)m_treeCtrl.GetItemData(hTheme);
		if (pThemeItemDesc)
		{
			HTREEITEM hNewItem = InsertItem_Mood( pThemeItemDesc,pMood,hTheme );
			m_treeCtrl.Select( hNewItem,TVGN_CARET );
		}
	}

	// Make one pattern set in this mood.
	if (bNewPatternSet)
		NewPatternSet( pMood );
	m_treeCtrl.SetRedraw(TRUE);

	return pMood;
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::CalcProbabilities( SMusicPatternSet *pPatternSet )
{
	assert(pPatternSet);
	pPatternSet->fTotalMainPatternProbability = 0.0f;
	pPatternSet->fTotalRhythmicPatternProbability = 0.0f;
	pPatternSet->fTotalIncidentalPatternProbability = 0.0f;
	for (int j = 0; j < pPatternSet->vecMainPatterns.size(); j++)
	{
		pPatternSet->fTotalMainPatternProbability += pPatternSet->vecMainPatterns[j]->fProbability;
	}
	for (int j = 0; j < pPatternSet->vecRhythmicPatterns.size(); j++)
	{
		pPatternSet->fTotalRhythmicPatternProbability += pPatternSet->vecRhythmicPatterns[j]->fProbability;
	}
	for (int j = 0; j < pPatternSet->vecIncidentalPatterns.size(); j++)
	{
		pPatternSet->fTotalIncidentalPatternProbability += pPatternSet->vecIncidentalPatterns[j]->fProbability;
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::PasteTheme( XmlNodeRef &node )
{
	const char *sBaseName = node->getAttr( "Name" );
	SMusicTheme *pTheme = NewTheme(sBaseName);

	pTheme->sDefaultMood = node->getAttr( "DefaultMood" );
	node->getAttr( "DefaultMoodTimeout",pTheme->fDefaultMoodTimeout );

	// Load moods.
	XmlNodeRef nodeMoods = node->findChild("Moods");
	if (nodeMoods)
	{
		for (int i = 0; i < nodeMoods->getChildCount(); i++)
		{
			XmlNodeRef moodNode = nodeMoods->getChild(i);
			SMusicMood *pMood = PasteMood( pTheme,moodNode );
			pTheme->mapMoods.erase( pMood->sName.c_str() ); // Erase uniq mood name.
			pMood->sName = moodNode->getAttr("Name"); // Load name from xml node.
			pTheme->mapMoods[pMood->sName.c_str()] = pMood; // Assign xml node name.
		}
	}

	// Load bridges.
	XmlNodeRef nodeBridges = node->findChild( "Bridges" );
	if (nodeBridges)
	{
		for (int i = 0; i < nodeBridges->getChildCount(); i++)
		{
			XmlNodeRef nodeBridge = nodeBridges->getChild(i);
			CString sPattern;
			if (nodeBridge->getAttr( "Pattern",sPattern))
			{
				pTheme->mapBridges[nodeBridges->getTag()] = (const char*)sPattern;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
SMusicMood* CMusicEditorDialog::PasteMood( SMusicTheme *pTheme,XmlNodeRef &node )
{
	// Loading.
	CString sBaseName = node->getAttr( "Name" );
	SMusicMood *pMood = NewMood( pTheme,sBaseName,false );

	node->getAttr( "PlaySingle",pMood->bPlaySingle );
	node->getAttr( "Priority",pMood->nPriority );
	node->getAttr( "FadeOutTime",pMood->fFadeOutTime );
	if (node->getChildCount() > 0)
	{
		// Load pattern sets.
		for (int i = 0; i < node->getChildCount(); i++)
		{
			PastePatternSet( pMood,node->getChild(i) );
		}
	}
	return pMood;
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::PastePatternSet( SMusicMood *pMood,XmlNodeRef &nodePtrnSet )
{
	SMusicPatternSet *pPatternSet = NewPatternSet( pMood );

	nodePtrnSet->getAttr( "MaxTimeout",pPatternSet->fMaxTimeout );
	nodePtrnSet->getAttr( "MinTimeout",pPatternSet->fMinTimeout );
	nodePtrnSet->getAttr( "IncidentalLayerProbability",pPatternSet->fIncidentalLayerProbability );
	nodePtrnSet->getAttr( "RhythmicLayerProbability",pPatternSet->fRhythmicLayerProbability );
	nodePtrnSet->getAttr( "MaxSimultaneousIncidentalPatterns",pPatternSet->nMaxSimultaneousIncidentalPatterns );
	nodePtrnSet->getAttr( "MaxSimultaneousRhythmicPatterns",pPatternSet->nMaxSimultaneousRhythmicPatterns );

	// Save patterns.
	XmlNodeRef nodeMainLayer = nodePtrnSet->findChild( "MainLayer" );
	XmlNodeRef nodeRhythmicLayer = nodePtrnSet->findChild( "RhythmicLayer" );
	XmlNodeRef nodeIncidentalLayer = nodePtrnSet->findChild( "IncidentalLayer" );

	if (nodeMainLayer)
	{
		for (int j = 0; j < nodeMainLayer->getChildCount(); j++)
		{
			NewPattern( pPatternSet,ELAYER_MAIN,NULL,nodeMainLayer->getChild(j) );
		}
	}
	if (nodeRhythmicLayer)
	{
		for (int j = 0; j < nodeRhythmicLayer->getChildCount(); j++)
		{
			NewPattern( pPatternSet,ELAYER_RHYTHMIC,NULL,nodeRhythmicLayer->getChild(j) );
		}
	}
	if (nodeIncidentalLayer)
	{
		for (int j = 0; j < nodeIncidentalLayer->getChildCount(); j++)
		{
			NewPattern( pPatternSet,ELAYER_INCIDENTAL,NULL,nodeIncidentalLayer->getChild(j) );
		}
	}
	CalcProbabilities(pPatternSet);
}

//////////////////////////////////////////////////////////////////////////
SMusicPatternSet* CMusicEditorDialog::NewPatternSet( SMusicMood *pMood )
{
	SMusicPatternSet *pPatternSet = new SMusicPatternSet;
	pMood->vecPatternSets.push_back( pPatternSet );

	pPatternSet->fMinTimeout = 60;
	pPatternSet->fMaxTimeout = 120;
	pPatternSet->fTotalMainPatternProbability = 0;
	//TPatternInfoVec vecMainPatterns;
	pPatternSet->nMaxSimultaneousRhythmicPatterns = 1;
	pPatternSet->fRhythmicLayerProbability = 100;
	pPatternSet->fTotalRhythmicPatternProbability = 0;
	//TPatternInfoVec vecRhythmicPatterns;
	pPatternSet->nMaxSimultaneousIncidentalPatterns = 1;
	pPatternSet->fIncidentalLayerProbability = 100;
	pPatternSet->fTotalIncidentalPatternProbability = 0;
	//TPatternInfoVec vecIncidentalPatterns;

	//////////////////////////////////////////////////////////////////////////
	// Add item to tree.
	//////////////////////////////////////////////////////////////////////////
	HTREEITEM hMood = stl::find_in_map( m_MoodToItemMap,pMood,(HTREEITEM)0 );
	if (hMood)
	{
		ItemDesc *pMoodItemDesc = (ItemDesc*)m_treeCtrl.GetItemData(hMood);
		if (pMoodItemDesc)
		{
			HTREEITEM hNewItem = InsertItem_PatternSet( pMoodItemDesc,pPatternSet,hMood );
			m_treeCtrl.Select( hNewItem,TVGN_CARET );
		}
	}
	return pPatternSet;
}

//////////////////////////////////////////////////////////////////////////
SPatternDef* CMusicEditorDialog::NewPattern( SMusicPatternSet *pPatternSet,ELayerType layer,const char *sBaseName,XmlNodeRef &node )
{
	SPatternDef *pPattern = new SPatternDef;
	if (!sBaseName)
		sBaseName = "Pattern0";

	if (node)
	{
		sBaseName = node->getAttr( "Name" );
		// If paste from node.
		CBaseLibraryItem::SerializeContext ctx( node,true );
		ctx.bCopyPaste = true;
		CMusicThemeLibItem::SerializePattern( ctx,pPattern );
	}
	else
	{
		pPattern->nLayeringVolume = 255; // Max volume.
	}

	CString uniqName = MakeUniqPatternName( sBaseName );
	pPattern->sName = (const char*)uniqName;

	m_PatternsMap[uniqName] = pPattern;
	m_pMusicData->vecPatternDef.push_back(pPattern);

	if (layer == ELAYER_MAIN)
		pPatternSet->vecMainPatterns.push_back( pPattern );
	else if (layer == ELAYER_RHYTHMIC)
		pPatternSet->vecRhythmicPatterns.push_back( pPattern );
	else if (layer == ELAYER_INCIDENTAL)
		pPatternSet->vecIncidentalPatterns.push_back( pPattern );

	CalcProbabilities( pPatternSet );

	m_pMusicSystem->UpdatePattern( pPattern );

	//////////////////////////////////////////////////////////////////////////
	// Insert item to tree.
	//////////////////////////////////////////////////////////////////////////
	HTREEITEM hPatternSet = stl::find_in_map( m_PatternSetToItemMap,pPatternSet,(HTREEITEM)0 );
	if (hPatternSet)
	{
		ItemDesc *pItemDesc = (ItemDesc*)m_treeCtrl.GetItemData(hPatternSet);
		if (pItemDesc)
		{
			HTREEITEM hLayer = m_treeCtrl.GetNextItem(hPatternSet,TVGN_CHILD);
			for (int i = 0; i < layer && hLayer; i++)
			{
				hLayer = m_treeCtrl.GetNextItem(hLayer,TVGN_NEXT);
			}
			if (hLayer)
			{
				ItemDesc *pLayerDesc = (ItemDesc*)m_treeCtrl.GetItemData(hLayer);
				if (pLayerDesc)
				{
					HTREEITEM hNewItem = InsertItem_Pattern( pLayerDesc,pPattern,hLayer );
					m_treeCtrl.Select( hNewItem,TVGN_CARET );
				}
			}
		}
	}

	return pPattern;
}

//////////////////////////////////////////////////////////////////////////
CString CMusicEditorDialog::MakeUniqPatternName( const CString &baseName )
{
	if (m_PatternsMap.find(baseName) == m_PatternsMap.end())
		return baseName;

	// Remove all numbers from the end of name.
	CString typeName = baseName;
	int len = typeName.GetLength();
	while (len > 0 && isdigit(typeName[len-1]))
		len--;

	typeName = typeName.Left(len);

	CString tpName = typeName;
	int num = 0;

	for (int i = 0; i < m_pMusicData->vecPatternDef.size(); i++)
	{
		SPatternDef *pPattern = m_pMusicData->vecPatternDef[i];
		const char *name = pPattern->sName.c_str();
		if (strncmp(name,tpName,len) == 0)
		{
			int n = atoi(name+len) + 1;
			num = MAX( num,n );
		}
	}
	CString str;
	str.Format( "%s%d",(const char*)typeName,num );
	return str;
}

//////////////////////////////////////////////////////////////////////////
CString CMusicEditorDialog::MakeUniqThemeName( const CString &baseName )
{
	if (m_pMusicData->mapThemes.find((const char*)baseName) == m_pMusicData->mapThemes.end())
		return baseName;

	// Remove all numbers from the end of name.
	CString typeName = baseName;
	int len = typeName.GetLength();
	while (len > 0 && isdigit(typeName[len-1]))
		len--;

	typeName = typeName.Left(len);

	CString tpName = typeName;
	int num = 0;

	for (TThemeMap::iterator it = m_pMusicData->mapThemes.begin(); it != m_pMusicData->mapThemes.end(); ++it)
	{
		SMusicTheme *pTheme = it->second;
		const char *name = pTheme->sName.c_str();
		if (strncmp(name,tpName,len) == 0)
		{
			int n = atoi(name+len) + 1;
			num = MAX( num,n );
		}
	}
	CString str;
	str.Format( "%s%d",(const char*)typeName,num );
	return str;
}

//////////////////////////////////////////////////////////////////////////
CString CMusicEditorDialog::MakeUniqMoodName( SMusicTheme *pTheme,const CString &baseName )
{
	if (pTheme->mapMoods.find((const char*)baseName) == pTheme->mapMoods.end())
		return baseName;

	// Remove all numbers from the end of name.
	CString typeName = baseName;
	int len = typeName.GetLength();
	while (len > 0 && isdigit(typeName[len-1]))
		len--;

	typeName = typeName.Left(len);

	CString tpName = typeName;
	int num = 0;

	for (TMoodMap::iterator it = pTheme->mapMoods.begin(); it != pTheme->mapMoods.end(); ++it)
	{
		SMusicMood *pMood = it->second;
		const char *name = pMood->sName.c_str();
		if (strncmp(name,tpName,len) == 0)
		{
			int n = atoi(name+len) + 1;
			num = MAX( num,n );
		}
	}
	CString str;
	str.Format( "%s%d",(const char*)typeName,num );
	return str;
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnAddItem()
{
	ItemDesc *pItemDesc = GetCurrentItem();
	if (!pItemDesc)
	{
		// Add Theme.
		NewTheme();
		return;
	}

	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();
	if (!hItem)
		return;

	switch (pItemDesc->type)
	{
	case EITEM_THEME:
		NewTheme();
		break;
	case EITEM_MOOD:
		NewMood( pItemDesc->m_pTheme );
		break;
	case EITEM_PATTERN_SET:
		NewPatternSet( pItemDesc->m_pMood );
		break;
	case EITEM_LAYER:
		NewPattern( pItemDesc->m_pPatternSet,pItemDesc->layerType );
		break;
	case EITEM_PATTERN:
		NewPattern( pItemDesc->m_pPatternSet,pItemDesc->layerType,pItemDesc->m_pPattern->sName.c_str() );
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnAddSubItem()
{
	ItemDesc *pItemDesc = GetCurrentItem();
	if (!pItemDesc)
		return;

	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();
	if (!hItem)
		return;

	switch (pItemDesc->type)
	{
	case EITEM_THEME:
		// Add mood
		NewMood( pItemDesc->m_pTheme );
		break;
	case EITEM_MOOD:
		// Add pattern set.
		NewPatternSet( pItemDesc->m_pMood );
		break;
	case EITEM_PATTERN_SET:
		break;
	case EITEM_LAYER:
		// Add Pattern
		NewPattern( pItemDesc->m_pPatternSet,pItemDesc->layerType );
		break;
	}
	SetModified();
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::DeletePattern( ItemDesc *pItemDesc )
{
	SPatternDef *def = pItemDesc->m_pPattern;

	m_PatternToItemMap.erase(def);

	// Delete item.
	SMusicPatternSet *pPatternSet = pItemDesc->m_pPatternSet;
	switch (pItemDesc->layerType)
	{
	case ELAYER_MAIN:
		stl::find_and_erase( pPatternSet->vecMainPatterns,def );
		break;
	case ELAYER_RHYTHMIC:
		stl::find_and_erase( pPatternSet->vecRhythmicPatterns,def );
		break;
	case ELAYER_INCIDENTAL:
		stl::find_and_erase( pPatternSet->vecIncidentalPatterns,def );
		break;
	}

	CalcProbabilities( pPatternSet );

	// Delete pattern from music.
	m_pMusicSystem->DeletePattern( def->sName.c_str() );

	// Delete from music.
	stl::find_and_erase( m_pMusicData->vecPatternDef,def );
	delete def;
	SetModified();
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::DeletePatternSet( ItemDesc *pItemDesc )
{
	SMusicPatternSet *pPatternSet = pItemDesc->m_pPatternSet;
	stl::find_and_erase( pItemDesc->m_pMood->vecPatternSets,pPatternSet );
	m_PatternSetToItemMap.erase( pPatternSet );
  delete pPatternSet;
	SetModified();
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::DeleteMood( ItemDesc *pItemDesc )
{
	SMusicMood *pMood = pItemDesc->m_pMood;
	pItemDesc->m_pTheme->mapMoods.erase( pMood->sName );
	m_MoodToItemMap.erase( pMood );
	delete pMood;
	SetModified();
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::DeleteTheme( ItemDesc *pItemDesc )
{
	DeleteItem( pItemDesc->pItem );
	SMusicTheme *pTheme = pItemDesc->m_pTheme;
	m_pMusicData->mapThemes.erase( pTheme->sName );
	m_ThemeToItemMap.erase( pTheme );
	delete pTheme;
	m_pMusicSystem->UpdateTheme( NULL ); // Get newer themes, data.
	SetModified();
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnTreeDeleteItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	if (!m_bHandleDeleteItem)
		return;

	NMTREEVIEW *pNMTREEVIEW = (NMTREEVIEW*)pNMHDR;

	HTREEITEM hItem = pNMTREEVIEW->itemOld.hItem;
	ItemDesc *pItemDesc = (ItemDesc*)pNMTREEVIEW->itemOld.lParam;
	if (!pItemDesc)
		return;
	//m_treeCtrl.SetItemData( hItem,0 );
	
	switch (pItemDesc->type)
	{
	case EITEM_THEME:
		DeleteTheme( pItemDesc );
		break;
	case EITEM_MOOD:
		DeleteMood( pItemDesc );
		break;
	case EITEM_PATTERN_SET:
		DeletePatternSet( pItemDesc );
		break;
	case EITEM_LAYER:
		break;
	case EITEM_PATTERN:
		DeletePattern( pItemDesc );
		break;
	};

	delete pItemDesc;
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnRemoveItem()
{
	ItemDesc *pItemDesc = GetCurrentItem();
	if (!pItemDesc)
		return;

	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();
	if (!hItem)
		return;

	m_bHandleDeleteItem = true;
	m_treeCtrl.SetRedraw(FALSE);

	switch (pItemDesc->type)
	{
	case EITEM_THEME:
		if (AfxMessageBox( _T("Delete this Theme?"),MB_YESNO ) == IDYES)
			m_treeCtrl.DeleteItem(hItem);
		break;
	case EITEM_MOOD:
		if (AfxMessageBox( _T("Delete this Mood?"),MB_YESNO ) == IDYES)
			m_treeCtrl.DeleteItem(hItem);
		break;
	case EITEM_PATTERN_SET:
		{
			// Check if next item is also pattern set.
			/*
			bool bLastSet = true;
			ItemDesc *pParentItem = (ItemDesc*)m_treeCtrl.GetItemData( m_treeCtrl.GetParentItem(hItem) );
			if (pParentItem)
			{
				if (pParentItem->m_pMood->vecPatternSets.size() > 1) // more then 1 pattern in mood.
					bLastSet = false;
			}
			if (bLastSet)
			*/
			{
				if (AfxMessageBox( _T("Delete this Pattern Set?"),MB_YESNO ) == IDYES)
					m_treeCtrl.DeleteItem(hItem);
			}
		}
		break;
	case EITEM_LAYER:
		// Delete all items in this layer.
		{
			if (AfxMessageBox( _T("Delete all paterns in this Layer?"),MB_YESNO ) == IDYES)
			{
				// Delete all of the children of hmyItem.
				if (m_treeCtrl.ItemHasChildren(hItem))
				{
					HTREEITEM hNextItem;
					HTREEITEM hChildItem = m_treeCtrl.GetChildItem(hItem);
					while (hChildItem != NULL)
					{
						hNextItem = m_treeCtrl.GetNextItem(hChildItem, TVGN_NEXT);
						m_treeCtrl.DeleteItem(hChildItem);
						hChildItem = hNextItem;
					}
				}
			}
		}
		break;
	case EITEM_PATTERN:
		{
			if (AfxMessageBox( _T("Delete this Pattern?"),MB_YESNO ) == IDYES)
				m_treeCtrl.DeleteItem(hItem);
		}
		break;
	};

	m_treeCtrl.SetFocus();
	m_treeCtrl.SetRedraw(TRUE);

	m_bHandleDeleteItem = false;
	SetModified();
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnRenameItem()
{
	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();
	if (!hItem)
		return;	

	m_treeCtrl.SetFocus();
	m_treeCtrl.EditLabel(hItem);
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnLoadFromLua()
{
	if (AfxMessageBox( "Load Music Data from Lua?",MB_YESNO|MB_ICONQUESTION ) != IDYES)
		return;
	CWaitCursor wait;
	m_pMusicManager->LoadFromLua( m_pLibrary );
	ReloadItems();
	SetModified();
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnLoadLibrary()
{
	CBaseLibrary *pPrevLib = m_pLibrary;
	CBaseLibraryDialog::OnLoadLibrary();
	if (m_pLibrary != pPrevLib)
	{
		// Library changed.
		GetIEditor()->GetSystem()->GetIMusicSystem()->SetData( m_pMusicData,true );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnRealodMusicFiles()
{
	FillFileTree();
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::LoadMusicFiles( std::vector<CString> &musicFiles )
{
	CString fileSpec = "*.ogg;*.mp3";
	CString currFileSpec;
	static CFileUtil::FileArray files;

	files.clear();

	// Reserve many files.
	files.resize(0);
	files.reserve( 2000 );

	// Split file spec with ';'
	while (!fileSpec.IsEmpty())
	{
		int splitpos = fileSpec.Find(';');
		if (splitpos < 0)
		{
			currFileSpec = fileSpec;
			CFileUtil::ScanDirectory( m_musicFilesPath,currFileSpec,files,true );
			break;
		}
		CString currFileSpec = fileSpec.Mid(0,splitpos);
		CFileUtil::ScanDirectory( m_musicFilesPath,currFileSpec,files,true );
		fileSpec = fileSpec.Mid(splitpos+1);
	}

	musicFiles.resize(0);
	musicFiles.reserve( files.size() );
	for (int i = 0; i < files.size(); i++)
	{
		musicFiles.push_back( files[i].filename );
	}
	std::sort( musicFiles.begin(),musicFiles.end() );
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::FillFileTree()
{
	CWaitCursor wait;
	std::vector<CString> files;
	LoadMusicFiles( files );

	m_filesTreeCtrl.SetRedraw( FALSE );
	m_musicFilesMap.clear();

	m_filesTreeCtrl.DeleteAllItems();

	CString filename,ext,path;
	std::map<CString,HTREEITEM> pathmap;
	for (int i = 0; i < files.size(); i++)
	{
		Path::Split( files[i],path,filename,ext );

		HTREEITEM hGroup = TVI_ROOT;

		if (!path.IsEmpty())
		{
			hGroup = stl::find_in_map( pathmap,path,(HTREEITEM)0 );
			if (!hGroup)
			{
				hGroup = TVI_ROOT;
				int startPos = 0;
				int prevPos = 0;
				CString subpath;
				while (startPos < path.GetLength())
				{
					prevPos = startPos;
					int pos = path.Find( '\\',startPos );
					if (pos >= 0)
					{
						startPos = pos+1;
					}
					else
						startPos = path.GetLength();
					subpath = path.Mid(0,pos);
					HTREEITEM hItem = stl::find_in_map( pathmap,subpath,(HTREEITEM)0 );
					if (!hItem)
					{
						CString subGroupName = path.Mid(prevPos,startPos-prevPos-1);
						hGroup = m_filesTreeCtrl.InsertItem( subGroupName,0,0,hGroup );
						pathmap[subpath] = hGroup;
					}
					else
						hGroup = hItem;
				}
			}
		}

		int nImage = 1;
		HTREEITEM hNewItem = m_filesTreeCtrl.InsertItem( filename, nImage, nImage, hGroup );
		//m_tree.SetItemState(hNewItem, TVIS_BOLD, TVIS_BOLD);
		CString filename = Path::AddBackslash(m_musicFilesPath) + files[i];
		filename.Replace( '\\','/' );
		m_musicFilesMap[hNewItem] = filename;
	}

	m_filesTreeCtrl.SetRedraw( TRUE );
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::SetActive( bool bActive )
{
	CBaseLibraryDialog::SetActive( bActive );
	if (bActive && m_musicFilesMap.empty())
	{
		FillFileTree();
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::PlaySound( const CString &filename )
{
	if (m_pSound)
		m_pSound->Stop();
	m_pSound = 0;
	if (m_bPlaying)
	{
		ISoundSystem *pSoundSystem = GetIEditor()->GetSystem()->GetISoundSystem();
		if (pSoundSystem)
		{
			m_pSound = pSoundSystem->LoadSound( filename,FLAG_SOUND_2D|FLAG_SOUND_STEREO|FLAG_SOUND_16BITS|FLAG_SOUND_LOAD_SYNCHRONOUSLY );
			if (m_pSound)
				m_pSound->Play();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnNotifyFileTreeClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	pResult = 0;

	if (m_bIgnoreSelectionChange)
		return;
	
	// Find item under mouse.
	CPoint point;
	GetCursorPos( &point );
	m_filesTreeCtrl.ScreenToClient( &point );
	UINT uFlags;
	HTREEITEM hItem = m_filesTreeCtrl.HitTest(point,&uFlags);
	CString filename = stl::find_in_map( m_musicFilesMap,hItem,"" );
	if (!filename.IsEmpty())
	{
		// Play sound.
		PlaySound( filename );
	}
	else
	{
		if (m_pSound)
			m_pSound->Stop();
		m_pSound = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnSelectFileTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	pResult = 0;

	if (m_bIgnoreSelectionChange)
		return;

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (m_pSound)
		m_pSound->Stop();
	m_pSound = 0;

	if (m_filesTreeCtrl)
	{
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		CString filename = stl::find_in_map( m_musicFilesMap,hItem,"" );
		if (!filename.IsEmpty())
		{
			// Play sound.
			PlaySound( filename );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnFileTreeKillFocus(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	if (m_pSound)
		m_pSound->Stop();
	m_pSound = 0;
}

//////////////////////////////////////////////////////////////////////////
void CMusicEditorDialog::OnFilesBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	CString filename = stl::find_in_map( m_musicFilesMap,hItem,"" );
	if (!filename.IsEmpty())
	{
		m_filesTreeCtrl.Select( hItem,TVGN_CARET );

		m_hDropItem = 0;
		m_dragImage = m_filesTreeCtrl.CreateDragImage( hItem );
		if (m_dragImage)
		{
			m_hDraggedItem = hItem;
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
	}

	*pResult = 0;
}