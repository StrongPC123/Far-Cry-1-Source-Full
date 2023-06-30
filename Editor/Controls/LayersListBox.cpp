////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   layerslistbox.cpp
//  Version:     v1.00
//  Created:     10/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LayersListBox.h"

#include "Objects\ObjectManager.h"

// CLayersListBox dialog
#define INDENT_SIZE 16
#define ITEM_EXPANDED_BITMAP 2
#define ITEM_COLLAPSED_BITMAP 3
#define ITEM_LEAF_BITMAP 4

#define BUTTON_VISIBLE 0
#define BUTTON_USABLE 1
#define BUTTON_EXPAND 2

IMPLEMENT_DYNAMIC(CLayersListBox, CListBox)

//////////////////////////////////////////////////////////////////////////
CLayersListBox::CLayersListBox()
{
	m_itemHeight = 24;
	m_handCursor = false;
	m_mousePaintFlags = 0;
	m_mousePaintValue = false;
	m_noReload = false;
	m_draggingItem = -1;
	m_rclickedItem = -1;
}

//////////////////////////////////////////////////////////////////////////
CLayersListBox::~CLayersListBox()
{
	DeleteObject( m_hHandCursor );
}


BEGIN_MESSAGE_MAP(CLayersListBox, CListBox)
	ON_WM_DRAWITEM_REFLECT()
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_COMPAREITEM_REFLECT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()


// CLayersListBox message handlers

void CLayersListBox::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
	//dis->
	if (lpdis->itemAction == ODA_SELECT || lpdis->itemAction == ODA_DRAWENTIRE)
	{
		CRect rc = lpdis->rcItem;

		CDC dc;
		dc.Attach(lpdis->hDC);

		SLayerInfo &layerInfo = m_layersInfo[lpdis->itemData];

		CString text = layerInfo.name;	

		CBrush brush( GetSysColor(COLOR_BTNFACE) );
		//CBrush *prevBrush = dc.SelectObject( &brush );
		HPEN prevPen = (HPEN)dc.SelectObject( (HPEN)GetStockObject(WHITE_PEN) );

		CRect buttonRc = GetButtonsFrame(rc);
		CRect nameRc;
		nameRc.SubtractRect(rc,buttonRc);

		CRect btnrc = buttonRc;
		btnrc.top += 1;
		dc.Draw3dRect( btnrc,GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DDKSHADOW));

		if (lpdis->itemState & ODS_SELECTED)
		{
			// Selected
			dc.FillSolidRect( nameRc,GetSysColor(COLOR_HIGHLIGHT) ); // blue.
			//dc.DrawRect(rc,GetSysColor(COLOR_3DDKSHADOW),GetSysColor(COLOR_3DHILIGHT) );
			dc.SetTextColor( GetSysColor(COLOR_HIGHLIGHTTEXT) );
		}
		else
		{
			// Normal.
			dc.FillSolidRect( nameRc,GetSysColor(COLOR_BTNFACE) ); // blue.
			//dc.Draw3dRect(rc,GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DDKSHADOW));
			dc.SetTextColor( GetSysColor(COLOR_BTNTEXT) );
		}
		
		// Draw separator white line.
		dc.MoveTo( nameRc.left,nameRc.top );
		dc.LineTo( nameRc.right,nameRc.top );

		//int indent = max(0,layerInfo.indent-1);
		int indent = layerInfo.indent;

		// Draw text.
		int textX = nameRc.left + 3 + indent*INDENT_SIZE;
		int textY = nameRc.top + (nameRc.bottom-nameRc.top)/2 - 8;
		
		/*
		if (layerInfo.indent > 0)
		{
			CPen pen(PS_SOLID,1,RGB(128,128,128) );
			CPen *oldPen = dc.SelectObject( &pen );
			CPoint center = nameRc.CenterPoint();
			dc.MoveTo( textX-2,center.y );
			dc.LineTo( textX-10,center.y );
			//dc.LineTo( textX-10,nameRc.top );
			dc.MoveTo( textX-10,rc.top );
			if (layerInfo.lastchild)
				dc.LineTo( textX-10,center.y );
			else
				dc.LineTo( textX-10,rc.bottom );
			dc.SelectObject( oldPen );
		}
		*/

		if (layerInfo.childs)
		{
			CRect expandRect = GetExpandButtonRect( lpdis->itemData );
			//expandRect.left -= 14;
			// Draw Expand icon.
			if (layerInfo.expanded)
				m_imageList.Draw( &dc,ITEM_EXPANDED_BITMAP,CPoint(expandRect.left,expandRect.top+5),ILD_TRANSPARENT );
			else
				m_imageList.Draw( &dc,ITEM_COLLAPSED_BITMAP,CPoint(expandRect.left,expandRect.top+5),ILD_TRANSPARENT );
			textX += 14;
		}

		CFont *prevFont = NULL;
		if (layerInfo.pLayer->IsExternal())
		{
			if (!m_externalFont.GetSafeHandle())
			{
				VERIFY(m_externalFont.CreateFont(-::MulDiv(8, GetDeviceCaps(dc.m_hDC, LOGPIXELSY), 72), 0, 0, 0, 
					FW_DONTCARE, FALSE, TRUE, FALSE,
					ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
					DEFAULT_PITCH, "Tahoma"));
			}
			prevFont = dc.SelectObject( &m_externalFont );
		}

		m_imageList.Draw( &dc,ITEM_LEAF_BITMAP,CPoint(textX,textY),ILD_TRANSPARENT );
		dc.TextOut( textX+2+16,textY+2,text );

		DrawCheckButton( dc,rc,0,m_layersInfo[lpdis->itemData].visible );
		DrawCheckButton( dc,rc,1,m_layersInfo[lpdis->itemData].usable );

		dc.SelectObject( prevPen );
		if (prevFont)
			dc.SelectObject( prevFont );

		if (lpdis->itemData == m_layersInfo.size()-1)
		{
			HPEN prevPen = (HPEN)dc.SelectObject( (HPEN)GetStockObject(BLACK_PEN) );
			// For last item draw black line at bottom.
			dc.MoveTo( nameRc.left,nameRc.bottom-1 );
			dc.LineTo( nameRc.right,nameRc.bottom-1 );
			dc.SelectObject(prevPen);
		}
		//dc.SelectObject(prevBrush);
		dc.Detach();
	}
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
//	lpMeasureItemStruct->itemWidth = 0;
	lpMeasureItemStruct->itemHeight = m_itemHeight;
	//lpMeasureItemStruct->itemData = 0;
}

//////////////////////////////////////////////////////////////////////////
int CLayersListBox::CompareItem( LPCOMPAREITEMSTRUCT lpCompareItemStruct )
{
	//lpCompareItemStruct.
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::PreSubclassWindow()
{
	CListBox::PreSubclassWindow();
	Init();
}

//////////////////////////////////////////////////////////////////////////
int CLayersListBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	Init();

	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::Init()
{
	// Initialization.
	SetItemHeight(0,m_itemHeight);
//	SetFont( CFont::FromHandle( (HFONT)::GetStockObject(SYSTEM_FONT)) );
	//m_imageList.Create( IDB_LAYER_BUTTONS,16,16,RGB(255,0,255) );
	CMFCUtils::LoadTrueColorImageList( m_imageList,IDB_LAYER_BUTTONS,16,RGB(255,0,255) );
	m_hHandCursor = LoadCursor( NULL,MAKEINTRESOURCE(IDC_HAND) );

	CRect rc; 
	GetClientRect(rc);
}

//////////////////////////////////////////////////////////////////////////
inline int CompareLayersInfo( const CLayersListBox::SLayerInfo &layer1,const CLayersListBox::SLayerInfo &layer2 )
{
	return layer1.name < layer2.name;
}

//////////////////////////////////////////////////////////////////////////
CRect CLayersListBox::GetButtonRect( CRect &rcItem,int id )
{
	int ofsx = 3;
	int ofsy = 3;
	int sz = m_itemHeight-ofsy;
	CRect rc;
	rc.left = rcItem.left + (sz+ofsx)*id + ofsx;
	rc.right = rc.left + sz;
	rc.top = rcItem.top + ofsy;
	rc.bottom = rcItem.top + sz;

	return rc;
}

CRect CLayersListBox::GetButtonsFrame( CRect &rcItem )
{
	CRect buttonRc;
	CRect brc = GetButtonRect(rcItem,1);
	buttonRc = rcItem;
	buttonRc.right = brc.right + 4;
	return buttonRc;
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::DrawCheckButton( CDC &dc,CRect &rcItem,int id,bool state )
{
	CRect rc = GetButtonRect( rcItem,id );	

	dc.Draw3dRect(rc,GetSysColor(COLOR_3DDKSHADOW),GetSysColor(COLOR_3DHILIGHT));
	if (state)
	{
		m_imageList.Draw( &dc,id,CPoint(rc.left+2,rc.top+1),ILD_TRANSPARENT );
	}
	else
	{
		//dc.Draw3dRect(rc,GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DDKSHADOW));
	}
}

//////////////////////////////////////////////////////////////////////////
CRect CLayersListBox::GetExpandButtonRect( int item )
{
	assert( item >= 0 && item < m_layersInfo.size() );
	CRect rcItem;
	GetItemRect( item,rcItem );

	CRect rcBtn(0,0,0,0);

	SLayerInfo &layerInfo = m_layersInfo[item];
	if (layerInfo.childs)
	{
		// See if hit expand button.
		rcBtn = GetButtonsFrame(rcItem);
		rcBtn.left = rcBtn.right + 3 + layerInfo.indent*INDENT_SIZE;
		rcBtn.right = rcBtn.left + 14;
	}
	return rcBtn;
}

//////////////////////////////////////////////////////////////////////////
int CLayersListBox::GetButtonFromPoint( CPoint point )
{
	BOOL bOutside;
	int button = -1;
	// Find item where we clicked.
	int item = ItemFromPoint( point,bOutside );
	if (item != LB_ERR && !bOutside)
	{
		CRect rcItem;
		GetItemRect( item,rcItem );

		for (int i = 0; i < 2; i++)
		{
			CRect buttonRc = GetButtonRect(rcItem,i);
			buttonRc.InflateRect(0,5);
			if (buttonRc.PtInRect(point))
			{
				button = i;
				break;
			}
		}

		if (button < 0)
		{
			if (GetExpandButtonRect(item).PtInRect(point))
				button = BUTTON_EXPAND;
		}
	}
	return button;
}

//////////////////////////////////////////////////////////////////////////
bool CLayersListBox::HandleMouseClick( UINT nFlags,CPoint point )
{
	bool bNoSelect = false;
	BOOL bOutside;
	
	// Find item where we clicked.
	int item = ItemFromPoint( point,bOutside );
	if (item != LB_ERR && !bOutside)
	{
		CRect rcItem;
		GetItemRect( item,rcItem );

		/*
		if (GetSel(item))
		{
			m_layersInfo[item].visible = true;
			m_layersInfo[item].usable = true;
			return true;
		}
		*/

		CRect buttonRc = GetButtonsFrame(rcItem);
		if (buttonRc.PtInRect(point))
		{
			// Clicked in button.
			bNoSelect = true;
		}

		int button = GetButtonFromPoint(point);
		if (button == BUTTON_VISIBLE)
		{
			m_mousePaintFlags = (1<<31) | button;
			m_layersInfo[item].visible = !m_layersInfo[item].visible;
			m_mousePaintValue = m_layersInfo[item].visible;
			OnModifyLayer( item );
		}
		else if (button == BUTTON_USABLE)
		{
			m_mousePaintFlags = (1<<31) | button;
			m_layersInfo[item].usable = !m_layersInfo[item].usable;
			m_mousePaintValue = m_layersInfo[item].usable;
			OnModifyLayer( item );
		} else if (button == BUTTON_EXPAND)
		{
			bNoSelect = true;
			m_layersInfo[item].pLayer->Expand( !m_layersInfo[item].pLayer->IsExpanded() );
			ReloadLayers();
		}

		if (!bNoSelect && (nFlags & MK_CONTROL))
		{
			// Can only drag removable layers.
			if (m_layersInfo[item].pLayer->IsRemovable())
			{
				// Start dragging.
				m_draggingItem = item;
				
				CPoint pos;
				GetCursorPos(&pos);
				// Initialize the drag image (usually called from WM_LBUTTONDOWN).
				m_imageList.BeginDrag( ITEM_LEAF_BITMAP, CPoint(-10,-10));
				m_imageList.DragEnter( this,point );
				SetCapture();
				Invalidate();
			}
		}
	}
	return bNoSelect;
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!HandleMouseClick(nFlags,point))
		CListBox::OnLButtonDown(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_mousePaintFlags = 0;

	if (m_draggingItem >= 0)
	{
		m_imageList.DragLeave( this );
		m_imageList.EndDrag();
		ReleaseCapture();
		Invalidate();

		BOOL bOutside;
		// Find item where we clicked.
		int item = ItemFromPoint( point,bOutside );
		if (item != LB_ERR && !bOutside)
		{
			if (item != m_draggingItem)
			{
				SLayerInfo &src = m_layersInfo[m_draggingItem];
				SLayerInfo &trg = m_layersInfo[item];
				if (!trg.pLayer->IsChildOf(src.pLayer))
				{
					trg.pLayer->AddChild(src.pLayer);
					trg.pLayer->Expand(true);
					ReloadLayers();
				}
				m_draggingItem = -1;
			}
		}
		else
		{
			SLayerInfo &src = m_layersInfo[m_draggingItem];
			if (src.pLayer->GetParent())
			{
				src.pLayer->GetParent()->RemoveChild(src.pLayer);
				ReloadLayers();
			}
		}
		m_draggingItem = -1;
	}
	CListBox::OnLButtonUp(nFlags, point);
}


//////////////////////////////////////////////////////////////////////////
void CLayersListBox::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	BOOL bOutside;
	// Find item where we clicked.
	int item = ItemFromPoint( point,bOutside );
	if (item != LB_ERR && !bOutside)
	{
		int button = GetButtonFromPoint(point);
		if (button < 0 || button == BUTTON_EXPAND)
		{
			// Toggle expand status.
			m_layersInfo[item].pLayer->Expand( !m_layersInfo[item].pLayer->IsExpanded() );
			ReloadLayers();
		}
		else if (button == BUTTON_VISIBLE)
		{
			CUndo undo("Layer Modify");
			bool bValue = m_layersInfo[item].visible;
			// Set all layers to this value.
			std::vector<CObjectLayer*> layers;
			GetIEditor()->GetObjectManager()->GetLayersManager()->GetLayers( layers );
			for (int i = 0; i < layers.size(); i++)
			{
				layers[i]->SetVisible(bValue,!layers[i]->IsExpanded() );
			}
			ReloadLayers();
		}	else if (button == BUTTON_USABLE)
		{
			CUndo undo("Layer Modify");
			bool bUsable = m_layersInfo[item].usable;
			// Set all layers to this value.
			std::vector<CObjectLayer*> layers;
			GetIEditor()->GetObjectManager()->GetLayersManager()->GetLayers( layers );
			for (int i = 0; i < layers.size(); i++)
			{
				layers[i]->SetFrozen(!bUsable,!layers[i]->IsExpanded());
			}
			ReloadLayers();
		}
	}
  CListBox::OnLButtonDblClk(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::SetUpdateCallback( UpdateCallback &cb )
{
	m_updateCalback = cb;
}

BOOL CLayersListBox::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default
	if (!m_handCursor)
	{
		return CListBox::OnSetCursor(pWnd, nHitTest, message);
	}
	SetCursor( m_hHandCursor );

	return TRUE;
}

void CLayersListBox::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_draggingItem >= 0)
	{
		CPoint pos = point;
		ClientToScreen(&pos);
		m_imageList.DragMove( point );
		return;
	}

	int button = GetButtonFromPoint(point);
	if (button >= 0)
	{
		m_handCursor = true;
	}
	else
		m_handCursor = false;

	if (m_mousePaintFlags != 0)
	{
		BOOL bOutside;
		int item = ItemFromPoint( point,bOutside );
		if ((item != LB_ERR && !bOutside) && (m_mousePaintFlags & 0xF) == button)
		{
			if (button == 0)
			{
				m_mousePaintFlags = (1<<31) | button;
				if (m_layersInfo[item].visible != m_mousePaintValue)
				{
					m_layersInfo[item].visible = m_mousePaintValue;
					OnModifyLayer( item );
				}
			}
			else if (button == 1)
			{
				m_mousePaintFlags = (1<<31) | button;
				if (m_layersInfo[item].usable != m_mousePaintValue)
				{
					m_layersInfo[item].usable = m_mousePaintValue;
					OnModifyLayer( item );
				}
			}
		}
	}

	CListBox::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::SelectLayer( const CString &layerName )
{
	for (int i = 0; i < m_layersInfo.size(); i++)
	{
		if (stricmp(layerName,m_layersInfo[i].name) == 0)
		{
			SetCurSel(i);
			break;
		}
	}
}

inline bool CompareLayers( CObjectLayer *l1,CObjectLayer *l2 )
{
	return stricmp(l1->GetName(),l2->GetName()) < 0;
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::ReloadLayers()
{
	if (m_noReload)
		return;
	/*
	m_layersInfo.clear();
	for (int i = 0; i < layers.size(); i++)
	{
	}
	*/
	m_layersInfo.clear();

	CObjectLayerManager *pLayerManager = GetIEditor()->GetObjectManager()->GetLayersManager();
	std::vector<CObjectLayer*> layers;
	pLayerManager->GetLayers( layers );

	std::sort( layers.begin(),layers.end(),CompareLayers );

	for (int i = 0; i < layers.size(); i++)
	{
		CObjectLayer *pLayer = layers[i];
		if (!pLayer)
			continue;
		if (pLayer->GetParent())
			continue;
		
		AddLayerRecursively( pLayer,0 );
	}

	ReloadListItems();
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::AddLayerRecursively( CObjectLayer *pLayer,int level )
{
	CLayersListBox::SLayerInfo layerInfo;
	layerInfo.name = pLayer->GetName();
	layerInfo.visible = pLayer->IsVisible();
	layerInfo.usable = !pLayer->IsFrozen();
	layerInfo.pLayer = pLayer;
	layerInfo.indent = level;
	layerInfo.childs = pLayer->GetChildCount() != 0;
	layerInfo.expanded = pLayer->IsExpanded();
	m_layersInfo.push_back(layerInfo);

	if (pLayer->IsExpanded())
	{
		int numLayers = pLayer->GetChildCount();
		for (int i = 0; i < numLayers; i++)
		{
			CObjectLayer *pChild = pLayer->GetChild(i);
			if (pChild)
				AddLayerRecursively( pChild,level+1 );

			if (i == numLayers-1)
			{
				m_layersInfo[m_layersInfo.size()-1].lastchild = true;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::ReloadListItems()
{
	CObjectLayerManager *pLayerManager = GetIEditor()->GetObjectManager()->GetLayersManager();
	CString selectedLayerName = pLayerManager->GetCurrentLayer()->GetName();
	int sel = LB_ERR;
	SetRedraw(FALSE);
	ResetContent();
	for (int i = 0; i < m_layersInfo.size(); i++)
	{
		int id = AddString( m_layersInfo[i].name );
		if (m_layersInfo[i].name == selectedLayerName)
			sel = id;
		SetItemData(id,i);
	}
	if (sel != LB_ERR)
	{
		SetCurSel(sel);
	}
	SetRedraw(TRUE);
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::OnModifyLayer( int index )
{
	assert( index >= 0 && index < m_layersInfo.size() );

	CUndo undo("Layer Modify");
	m_noReload = true;
	SLayerInfo &li = m_layersInfo[index];
	bool bRecursive = !(li.pLayer->IsExpanded());
	li.pLayer->SetVisible( li.visible,bRecursive );
	li.pLayer->SetFrozen( !li.usable,bRecursive );
	m_noReload = false;

	CRect rcItem;
	GetItemRect( index,rcItem );
	InvalidateRect( rcItem );
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_rclickedItem = -1;
	BOOL bOutside;
	int item = ItemFromPoint( point,bOutside );
	if (item != LB_ERR && !bOutside)
	{
		m_rclickedItem = item;
		SetCurSel( item );
		CObjectLayer *pLayer = GetCurrentLayer();
		if (pLayer)
		{
			GetIEditor()->GetObjectManager()->GetLayersManager()->SetCurrentLayer( pLayer );
		}
	}

	CListBox::OnRButtonDown(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CLayersListBox::OnRButtonUp(UINT nFlags, CPoint point)
{
	BOOL bOutside;
	int item = ItemFromPoint( point,bOutside );
	if (item != LB_ERR && !bOutside)
	{
		if (item == m_rclickedItem)
		{
			// Show context menu.
			NMHDR hdr;
			hdr.code = LBN_LAYERS_RBUTTON_UP;
			hdr.hwndFrom = m_hWnd;
			hdr.idFrom = GetDlgCtrlID();
			LRESULT hres;
			hres = GetParent()->SendMessage( WM_NOTIFY,(WPARAM)GetDlgCtrlID(),(LPARAM)(&hdr) );
			if (hres != 0)
			{
				m_rclickedItem = -1;
				return;
			}
		}
	}
	m_rclickedItem = -1;

	CListBox::OnRButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
CObjectLayer* CLayersListBox::GetCurrentLayer()
{
	int sel = GetCurSel();
	if (sel != LB_ERR)
	{
		return m_layersInfo[sel].pLayer;
	}
	return 0;
}
