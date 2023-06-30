// EditModeToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "EditModeToolBar.h"
#include "Objects\ObjectManager.h"
#include "UndoDropDown.h"
#include "ViewManager.h"
#include "GridSettingsDialog.h"
#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditModeToolBar

CEditModeToolBar::CEditModeToolBar()
{
	//{{AFX_DATA_INIT(CEditModeToolBar)
	m_szSelection = "";
	//}}AFX_DATA_INIT

	m_coordSys = (RefCoordSys)-1;
	m_objectSelectionMask = 0;
}

CEditModeToolBar::~CEditModeToolBar()
{
}


BEGIN_MESSAGE_MAP(CEditModeToolBar, CXTToolBar)
	//{{AFX_MSG_MAP(CEditModeToolBar)
	ON_WM_CREATE()
	ON_CBN_SELENDOK( IDC_SELECTION,OnSelectionChanged )
	ON_CBN_SELCHANGE( IDC_SELECTION,OnSelectionChanged )
	ON_NOTIFY( CBEN_ENDEDIT, IDC_SELECTION, OnNotifyOnSelectionChanged)
	ON_NOTIFY_REFLECT( TBN_DROPDOWN, OnToolbarDropDown )
	ON_UPDATE_COMMAND_UI(ID_REF_COORDS_SYS, OnUpdateCoordsRefSys)
	ON_CBN_SELENDOK( ID_REF_COORDS_SYS,OnCoordsRefSys )

	ON_UPDATE_COMMAND_UI(IDC_SELECTION_MASK, OnUpdateSelectionMask)
	ON_CBN_SELENDOK( IDC_SELECTION_MASK,OnSelectionMask )

	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CEditModeToolBar::DoDataExchange(CDataExchange* pDX)
{
	CXTToolBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditModeToolBar)
	DDX_CBString(pDX, IDC_SELECTION, m_szSelection);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CEditModeToolBar message handlers

BOOL CEditModeToolBar::Create( CWnd *pParentWnd,DWORD dwStyle,UINT nID )
{
	if (!CXTToolBar::CreateEx( pParentWnd,TBSTYLE_FLAT|TBSTYLE_TRANSPARENT|TBSTYLE_LIST,dwStyle,CRect(0,0,0,0),nID ))
		return FALSE;
	
	LoadToolBar( IDR_EDIT_MODE );

	/*
	// Set up hot bar image lists.
	CImageList	toolbarImageList;
	CBitmap	toolbarBitmap;
	toolbarBitmap.LoadBitmap(IDR_EDIT_MODE);
	toolbarImageList.Create(16, 15, ILC_COLORDDB|ILC_MASK, 13, 1);
	toolbarImageList.Add(&toolbarBitmap,TOOLBAR_TRANSPARENT_COLOR);
	GetToolBarCtrl().SetImageList( &toolbarImageList );
	toolbarImageList.Detach();
	toolbarBitmap.Detach();
	*/
	
	GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS|TBSTYLE_EX_MIXEDBUTTONS);
	//GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

	// Replace buttons to check boxes.
	for (int i = 0; i < GetCount(); i++)
	{
		int style = GetButtonStyle(i);

		if (style & TBBS_SEPARATOR)
			continue;

		int id = GetItemID(i);
		if (/*(id == ID_EDITMOD_LINK)||(id == ID_EDITMODE_UNLINK)||*/(id == ID_EDITMODE_SELECT)||(id == ID_EDITMODE_MOVE)||(id == ID_EDITMODE_ROTATE)||(id == ID_EDITMODE_SCALE))
		{
			if (style == TBBS_BUTTON)
			{
				style = TBBS_CHECKGROUP;
				SetButtonStyle(i,style);
			}
		}
	}

	int iIndex;

	// Layer select button.
	SetButtonStyle( CommandToIndex(ID_LAYER_SELECT),BTNS_BUTTON|BTNS_SHOWTEXT );
	//SetButtonText( CommandToIndex(ID_LAYER_SELECT),"        " );

	// Snap.
	iIndex = CommandToIndex(ID_SNAP_TO_GRID);
	SetButtonStyle( iIndex,GetButtonStyle(iIndex)|TBSTYLE_DROPDOWN|BTNS_SHOWTEXT );
	SetButtonText( iIndex," " );

	// Terrain Axis.
	//iIndex = CommandToIndex(ID_SELECT_AXIS_TERRAIN);
	//SetButtonStyle( iIndex,GetButtonStyle(iIndex)|TBSTYLE_DROPDOWN );


	// Place drop down buttons for Undo and Redo.
	iIndex = CommandToIndex(ID_UNDO);
	SetButtonStyle( iIndex,GetButtonStyle(iIndex)|TBSTYLE_DROPDOWN );

	iIndex = CommandToIndex(ID_REDO);
	SetButtonStyle( iIndex,GetButtonStyle(iIndex)|TBSTYLE_DROPDOWN );

	// Create controls in the animation bar
	CRect rect;
  // Get the index of the keyframe slider position in the toolbar
	iIndex = CommandToIndex(IDC_SELECTION);
	
	/*
	if (iIndex >= 0)
	{	 
		// Convert that button to a seperator and get its position
		SetButtonInfo(iIndex, IDC_SELECTION, TBBS_SEPARATOR, 100);
		GetItemRect(iIndex, &rect);
	}
	*/

	CalcLayout(LM_HORZ);

	rect.SetRect( 2,0,100,150 );
	m_selections.Create(WS_CHILD|WS_VISIBLE|CBS_DROPDOWN|CBS_SORT,rect,this,IDC_SELECTION);

	InsertControl( &m_selections );
	//SetButtonCtrl( iIndex,&m_selections,FALSE );
	m_selections.SetItemHeight( -1, 16 ); 
  //m_selections.SetFont( &g_PaintManager->m_FontNormal ); 

	rect.SetRect( 2,0,60,150 );
	m_refCoords.Create(WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST,rect,this,ID_REF_COORDS_SYS);
	m_refCoords.AddString("View");
	m_refCoords.AddString("Local");
	m_refCoords.AddString("World");

	InsertControl( &m_refCoords );
	//SetButtonCtrl( iIndex,&m_selections,FALSE );
	//m_refCoords.SetItemHeight( -1, 16 );


	//////////////////////////////////////////////////////////////////////////
	// Initialize selection mask.
	//////////////////////////////////////////////////////////////////////////
	int id;
	rect.SetRect( 2,0,80,300 );
	m_selectionMask.Create(WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST,rect,this,IDC_SELECTION_MASK);
	//////////////////////////////////////////////////////////////////////////
	id = m_selectionMask.AddString("Select All");
	m_selectionMask.SetItemData( id,OBJTYPE_ANY );
	//////////////////////////////////////////////////////////////////////////
	id = m_selectionMask.AddString("No Brushes");
	m_selectionMask.SetItemData( id,(~OBJTYPE_BRUSH) );
	//////////////////////////////////////////////////////////////////////////
	id = m_selectionMask.AddString("Brushes");
	m_selectionMask.SetItemData( id,OBJTYPE_BRUSH );
	//////////////////////////////////////////////////////////////////////////
	id = m_selectionMask.AddString("Entities");
	m_selectionMask.SetItemData( id,OBJTYPE_ENTITY );
	//////////////////////////////////////////////////////////////////////////
	id = m_selectionMask.AddString("Prefabs");
	m_selectionMask.SetItemData( id,OBJTYPE_PREFAB );
	//////////////////////////////////////////////////////////////////////////
	id = m_selectionMask.AddString("Areas");
	m_selectionMask.SetItemData( id,OBJTYPE_VOLUME );
	//////////////////////////////////////////////////////////////////////////
	id = m_selectionMask.AddString("AI Points");
	m_selectionMask.SetItemData( id,OBJTYPE_AIPOINT );

	InsertControl( &m_selectionMask );
	//////////////////////////////////////////////////////////////////////////
	
	/*
	{
		CMenu menu;
		menu.CreatePopupMenu();
		int size = 1;
		for (int i = 0; i < 7; i++)
		{
			CString str;
			str.Format( "%d",size );
			menu.AppendMenu( MF_STRING,1+i,str );
			size *= 2;
		}
		menu.AppendMenu( MF_SEPARATOR );
		menu.AppendMenu( MF_STRING,100,"Setup grid" );
		SetButtonMenu( CommandToIndex(ID_SNAP_TO_GRID),menu.Detach(),FALSE );
	}
	*/

	CalcLayout(LM_HORZ);
	GetToolBarCtrl().AutoSize();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CEditModeToolBar::SetGridSize( float size )
{
	// Snap.
	int iIndex = CommandToIndex(ID_SNAP_TO_GRID);
	CString str;
	str.Format( "%g",size );
	SetButtonText( iIndex,str );
	AutoSize();
}

//////////////////////////////////////////////////////////////////////////
void CEditModeToolBar::SetCurrentLayer( const CString &layerName )
{
	int iIndex = CommandToIndex(ID_LAYER_SELECT);
	if (iIndex >= 0)
	{
		SetButtonText( iIndex,CString(" ")+layerName );
		//m_layerName.SetWindowText( layerName );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEditModeToolBar::OnSelectionChanged()
{
	CString selection = GetSelection();
	if (!selection.IsEmpty())
		GetIEditor()->GetObjectManager()->SetSelection( selection );
}

void CEditModeToolBar::OnNotifyOnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMCBEENDEDIT *endEdit = (NMCBEENDEDIT*)pNMHDR;
	if (endEdit->iWhy == CBENF_RETURN || endEdit->iWhy == CBENF_KILLFOCUS)
	{
		// Add new selection.
		CString selection = endEdit->szText;
		if (!selection.IsEmpty())
		{
			AddSelection( selection );
			GetIEditor()->GetObjectManager()->NameSelection( selection );
		}
	}
}

CString CEditModeToolBar::GetSelection()
{
	UpdateData( TRUE );
	if (m_selections.m_hWnd)
	{
		int sel = m_selections.GetCurSel();
		if (sel != CB_ERR)
		{
			m_selections.GetLBText( sel,m_szSelection );
		}
	}
	return m_szSelection;
}

void CEditModeToolBar::AddSelection( const CString &name )
{
	if (m_selections.FindStringExact( 0,name ) == CB_ERR)
	{
		//char str[1024];
		//strcpy( str,name );
		//m_selections.AddString( name );
		//COMBOBOXEXITEM item;
		//memset(&item,0,sizeof(item));
		//item.mask = CBEIF_TEXT;
		//item.pszText = str;
		//item.cchTextMax = name.GetLength();
		m_selections.AddString( name );
	}
}

void CEditModeToolBar::RemoveSelection( const CString &name )
{
	int i = m_selections.FindStringExact( 0,name );
	if (i != CB_ERR)
		m_selections.DeleteString( i );
}

void CEditModeToolBar::SetSelection( const CString &name )
{
	m_szSelection = name;
	UpdateData( FALSE );
}

void CEditModeToolBar::OnRButtonUp(UINT nFlags, CPoint point) 
{
	/*
	CRect rc;
	for (int i = 0; i < GetCount(); i++)
	{
		GetItemRect(i,rc);
		if (rc.PtInRect(point))
		{
			int id = GetButtonID(i);
			if ((id == ID_EDITMODE_MOVE)||(id == ID_EDITMODE_ROTATE)||(id == ID_EDITMODE_SCALE))
			{
				if (GetToolBarCtrl().IsButtonChecked(id))
				{
					CLogFile::WriteLine( "Clicked" );
				}
			}
		}
	}
	*/
	
	CXTToolBar::OnRButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CEditModeToolBar::OnToolbarDropDown(NMHDR* pnhdr, LRESULT *plr)
{
	CRect rc;
	CPoint pos;
	GetCursorPos( &pos );

	NMTOOLBAR* pnmtb = (NMTOOLBAR*)pnhdr;

	//GetItemRect( pnmtb->iItem,rc );
	rc = pnmtb->rcButton;
	ClientToScreen( rc );
	pos.x = rc.left;
	pos.y = rc.bottom;

	// Switch on button command id's.
	switch (pnmtb->iItem)
	{
	case ID_UNDO:
		{
			CUndoDropDown undoDialog( pos,true,AfxGetMainWnd() );
			undoDialog.DoModal();
		}
		break;
	case ID_REDO:
		{
			CUndoDropDown undoDialog( pos,false,AfxGetMainWnd() );
			undoDialog.DoModal();
		}
		break;
	case ID_SELECT_AXIS_TERRAIN:
		//OnAxisTerrainMenu(pos);
		break;
	case ID_SNAP_TO_GRID:
		{
			// Display drop down menu with snap values.
			OnSnapMenu(pos);
		}
		break;
	default:
		return;
	}
	*plr = TBDDRET_DEFAULT;
}

//////////////////////////////////////////////////////////////////////////
void CEditModeToolBar::OnAxisTerrainMenu( CPoint pos )
{
	/*
	CMenu menu;
	menu.CreatePopupMenu();

	menu.AppendMenu( MF_STRING,1,"Terrain And Objects" );
	menu.AppendMenu( MF_STRING,2,"Only Terrain" );

	int cmd = menu.TrackPopupMenu( TPM_RETURNCMD|TPM_LEFTALIGN|TPM_LEFTBUTTON,pos.x,pos.y,this );
	if (cmd == 1)
	{
		GetIEditor()->SetTerrainAxisIgnoreObjects(false);
	}
	if (cmd == 1)
	{
		GetIEditor()->SetTerrainAxisIgnoreObjects(true);
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CEditModeToolBar::OnSnapMenu( CPoint pos )
{
	CMenu menu;
	menu.CreatePopupMenu();

	float startSize = 0.125;
	int steps = 10;

	double size = startSize;
	for (int i = 0; i < steps; i++)
	{
		CString str;
		str.Format( "%g",size );
		menu.AppendMenu( MF_STRING,1+i,str );
		size *= 2;
	}
	menu.AppendMenu( MF_SEPARATOR );
	menu.AppendMenu( MF_STRING,100,"Setup grid" );

	int cmd = menu.TrackPopupMenu( TPM_RETURNCMD|TPM_LEFTALIGN|TPM_LEFTBUTTON,pos.x,pos.y,this );
	if (cmd >= 1 && cmd < 100)
	{
		size = startSize;
		for (int i = 0; i < cmd-1; i++)
		{
			size *= 2;
		}
		// Set grid to size.
		GetIEditor()->GetViewManager()->GetGrid()->size = size;
	}
	if (cmd == 100)
	{
		// Setup grid dialog.
		CGridSettingsDialog dlg;
		dlg.DoModal();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEditModeToolBar::OnUpdateCoordsRefSys(CCmdUI *pCmdUI)
{
	if (m_coordSys != GetIEditor()->GetReferenceCoordSys() && m_refCoords.m_hWnd)
	{
		int sel = LB_ERR;
		m_coordSys = GetIEditor()->GetReferenceCoordSys();
		switch (m_coordSys)
		{
		case COORDS_VIEW:
			sel = 0;
			break;
		case COORDS_LOCAL:
			sel = 1;
			break;
		case COORDS_WORLD:
			sel = 2;
			break;
		};
		if (sel != LB_ERR)
		{
			m_refCoords.SetCurSel(sel);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEditModeToolBar::OnCoordsRefSys()
{
	int sel = m_refCoords.GetCurSel();
	if (sel != LB_ERR)
	{
		switch (sel)
		{
		case 0:
			GetIEditor()->SetReferenceCoordSys( COORDS_VIEW );
			break;
		case 1:
			GetIEditor()->SetReferenceCoordSys( COORDS_LOCAL );
			break;
		case 2:
			GetIEditor()->SetReferenceCoordSys( COORDS_WORLD );
			break;
		};
	}
}

//////////////////////////////////////////////////////////////////////////
void CEditModeToolBar::OnUpdateSelectionMask(CCmdUI *pCmdUI)
{
	if (m_objectSelectionMask != gSettings.objectSelectMask && m_selectionMask.m_hWnd)
	{
		m_objectSelectionMask = gSettings.objectSelectMask;
		int sel = LB_ERR;
		for (int i = 0; i < m_selectionMask.GetCount(); i++)
		{
			if (m_selectionMask.GetItemData(i) == gSettings.objectSelectMask)
			{
				sel = i;
				break;
			}
		}
		if (sel != LB_ERR && sel != m_selectionMask.GetCurSel())
		{
			m_selectionMask.SetCurSel(sel);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEditModeToolBar::OnSelectionMask()
{
	int sel = m_selectionMask.GetCurSel();
	if (sel != LB_ERR)
	{
		gSettings.objectSelectMask = m_selectionMask.GetItemData(sel);
		m_objectSelectionMask = gSettings.objectSelectMask;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEditModeToolBar::NextSelectMask()
{
	int sel = m_selectionMask.GetCurSel();
	if (sel == LB_ERR || sel == m_selectionMask.GetCount()-1)
	{
		sel = 0;
	}
	else
		sel++;
	m_selectionMask.SetCurSel(sel);
	OnSelectionMask();
}