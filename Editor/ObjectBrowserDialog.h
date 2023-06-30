////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   objectbrowserdialog.h
//  Version:     v1.00
//  Created:     27/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __objectbrowserdialog_h__
#define __objectbrowserdialog_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "Controls\MltiTree.h"

// CObjectBrowserDialog dialog
class CBaseObject;
class CObjectLayer;

class CObjectBrowserDialog : public CDialog
{
	DECLARE_DYNAMIC(CObjectBrowserDialog)

public:
	CObjectBrowserDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CObjectBrowserDialog();

// Dialog Data
	enum { IDD = IDD_OBJECT_BROWSER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnSize(UINT nType, int cx, int cy);

	void AddObject( CBaseObject *pObject,CBaseObject *pParent );
	void ReloadObjects();

	DECLARE_MESSAGE_MAP()

private:
	CMultiTree m_tree;
	CImageList m_imageList;
	CFont m_font;

	enum ItemType
	{
		ITEM_OBJECT,
		ITEM_TYPE,
		ITEM_LAYER,
	};
	struct Item
	{
		int type;
		CString typeName;
		CBaseObject *object;
		CObjectLayer *layer;
		Item() {
			object = 0;
			layer = 0;
			type = 0;
		}
		Item( ItemType _type )
		{
			type = _type;
			object = 0;
			layer = 0;
		};
	};

	struct TypeItem
	{
		HTREEITEM hItem;
		int objectCount;
	};
	struct LayerItem
	{
		HTREEITEM hItem;
		std::map<CString,TypeItem> typeMap;
		int objectCount;
	};

	typedef std::map<HTREEITEM,Item> ItemsMap;
	ItemsMap m_itemsMap;

	typedef std::map<CObjectLayer*,LayerItem> LayersMap;
	LayersMap m_layersMap;

	typedef std::map<CBaseObject*,HTREEITEM> ObjectsMap;
	ObjectsMap m_objectsMap;
};

#endif // __objectbrowserdialog_h__