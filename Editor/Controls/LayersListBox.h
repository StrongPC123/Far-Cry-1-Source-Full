////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   layerslistbox.h
//  Version:     v1.00
//  Created:     10/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __layerslistbox_h__
#define __layerslistbox_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "Objects\ObjectLayer.h"

#define LBN_LAYERS_RBUTTON_DOWN 10
#define LBN_LAYERS_RBUTTON_UP 11

// CLayersListBox dialog
/*!
 *	CLayerListBox	is a special owner draw version of list box to display layers.
 */
class CLayersListBox : public CListBox
{
	DECLARE_DYNAMIC(CLayersListBox)

public:
	typedef Functor0 UpdateCallback;

	struct SLayerInfo
	{
		CString name;
		bool visible;
		bool usable;
		bool expanded;
		bool childs;
		bool lastchild;
		TSmartPtr<CObjectLayer> pLayer;
		int indent;

		SLayerInfo()
		{
			childs = false;
			expanded = false;
			visible = false;
			usable = false;
			lastchild = false;
			indent = 0;
		}
	};
	typedef std::vector<SLayerInfo> Layers;

	CLayersListBox();   // standard constructor
	virtual ~CLayersListBox();

	void ReloadLayers();
	void SelectLayer( const CString &layerName );
	CObjectLayer* GetCurrentLayer();

	void SetUpdateCallback( UpdateCallback &cb );

	DECLARE_MESSAGE_MAP()
	afx_msg void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT /*lpMeasureItemStruct*/);
	afx_msg int CompareItem( LPCOMPAREITEMSTRUCT lpCompareItemStruct );
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

protected:
	virtual void PreSubclassWindow();
	void Init();
	//void UpdateLayers();
	void AddLayerRecursively( CObjectLayer *pLayer,int level );
	void ReloadListItems();
	void OnModifyLayer( int index );
	void PopupMenu( int item );

	CRect GetButtonRect( CRect &rcItem,int id );
	CRect GetButtonsFrame( CRect &rcItem );
	CRect GetExpandButtonRect( int item );
	void DrawCheckButton( CDC &dc,CRect &rcItem,int id,bool state );
	int GetButtonFromPoint( CPoint point);

	bool HandleMouseClick( UINT nFlags,CPoint point );

	std::vector<SLayerInfo> m_layersInfo;
	CImageList m_imageList;

	int m_itemHeight;

	UpdateCallback m_updateCalback;
	HCURSOR m_hHandCursor;
	bool m_handCursor;
	bool m_noReload;
	
	int m_mousePaintFlags;
	bool m_mousePaintValue;

	int m_draggingItem;
	int m_rclickedItem;

	CFont m_externalFont;
};

#endif // __layerslistbox_h__