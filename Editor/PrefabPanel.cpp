////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PrefabPanel.cpp
//  Version:     v1.00
//  Created:     14/11/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PrefabPanel.h"

#include "Objects\ObjectManager.h"
#include "Objects\PrefabObject.h"
#include "Prefabs\PrefabItem.h"

IMPLEMENT_DYNCREATE(CPrefabPanel, CXTResizeDialog)

CPrefabPanel::CPrefabPanel(CWnd* pParent /*=NULL*/)
: CXTResizeDialog(CPrefabPanel::IDD, pParent)
, m_type(0)
{
	m_object = 0;
	Create( IDD,pParent );
}

CPrefabPanel::~CPrefabPanel()
{
}

void CPrefabPanel::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXTRACT_SELECTED, m_extractSelectedBtn);
	DDX_Control(pDX, IDC_EXTRACT_ALL, m_extractAllBtn);
	DDX_Control(pDX, IDC_PREFAB, m_prefabNameBtn);
	DDX_Control(pDX, IDC_OBJECTS, m_tree );
	DDX_Control(pDX, IDC_NUM_OBJECTS, m_objectsText );
}

BOOL CPrefabPanel::OnInitDialog()
{
	CXTResizeDialog::OnInitDialog();

	SetResize( IDC_OBJECTS,SZ_RESIZE(1) );
	SetResize( IDC_NUM_OBJECTS,SZ_RESIZE(1) );
	SetResize( IDC_OBJECT_INFO,SZ_RESIZE(1) );
	SetResize( IDC_PREFAB,SZ_RESIZE(1) );

	SetResize( IDC_NAME,SZ_RESIZE(1) );
	SetResize( IDC_CLASS,SZ_RESIZE(1) );
	SetResize( IDC_TYPE,SZ_RESIZE(1) );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPrefabPanel, CXTResizeDialog)
	ON_BN_CLICKED(IDC_EXTRACT_SELECTED, OnBnClickedExtractSelected)
	ON_BN_CLICKED(IDC_EXTRACT_ALL, OnBnClickedExtractAll)
	ON_BN_CLICKED(IDC_PREFAB, OnBnClickedPrefab)
	ON_NOTIFY(TVN_SELCHANGED, IDC_OBJECTS, OnSelChangedTree)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
void CPrefabPanel::SetObject( CPrefabObject *object )
{
	assert( object );
	m_object = object;
	UpdateData(FALSE);
		
	ReloadObjects();
}

//////////////////////////////////////////////////////////////////////////
inline void RecursivelyGetAllChilds( CBaseObject *obj,std::vector<CBaseObject*> &childs )
{
	for (int i = 0; i < obj->GetChildCount(); i++)
	{
		CBaseObject *c = obj->GetChild(i);
		childs.push_back(c);
		RecursivelyGetAllChilds( c,childs );
	}
}

//////////////////////////////////////////////////////////////////////////
static HTREEITEM AddPrefabTreeItem( CTreeCtrl &tree,CBaseObject *pObject,CBaseObject *pTopParent,std::map<CBaseObject*,HTREEITEM> &itemsMap )
{
	HTREEITEM hParent = TVI_ROOT;

	CBaseObject *pParent = pObject->GetParent();
	if (pParent && pParent != pTopParent)
	{
		hParent = stl::find_in_map( itemsMap,pParent, (HTREEITEM)0 );
		if (!hParent)
		{
			hParent = AddPrefabTreeItem( tree,pParent,pTopParent,itemsMap );
		}
	}
	CString str;
	//str = CString(pObject->GetName()) + " (" + pObject->GetTypeDescription() + ")";
	str = (pObject->GetName()) + " (" + pObject->GetClassDesc()->ClassName() + ")";
	HTREEITEM hItem = tree.InsertItem( str,hParent );
	itemsMap[pObject] = hItem;
	tree.SetItemData( hItem,(DWORD_PTR)pObject );
	tree.Expand( hItem,TVE_EXPAND );
	return hItem;
}

//////////////////////////////////////////////////////////////////////////
void CPrefabPanel::ReloadObjects()
{
	m_tree.SetRedraw(FALSE);
	m_tree.DeleteAllItems();

	std::vector<CBaseObject*> childs;
	childs.reserve(20);
	RecursivelyGetAllChilds( m_object,childs );

	// items map
	std::map<CBaseObject*,HTREEITEM> itemsMap;

	// Fill tree with objects.
	for (int i = 0; i < childs.size(); i++)
	{
		if (childs[i]->CheckFlags(OBJFLAG_PREFAB))
			AddPrefabTreeItem( m_tree,childs[i],m_object,itemsMap );
	}
	m_tree.Expand( TVI_ROOT,TVE_EXPAND );
	m_tree.SetRedraw(TRUE);

	CString text;
	int numChilds = childs.size();
	text.Format( "%d Object(s):",numChilds );

	//SetDlgItemText( IDC_NAME,SZ_RESIZE(1) );
	//SetResize( IDC_CLASS,SZ_RESIZE(1) );
	//SetResize( IDC_TYPE,SZ_RESIZE(1) );

	m_objectsText.SetWindowText( text );
	CPrefabItem *pItem = m_object->GetPrefab();
	if (pItem)
	{
		m_prefabNameBtn.SetWindowText( pItem->GetFullName() );
	}
	else
	{
		m_prefabNameBtn.SetWindowText( "None" );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPrefabPanel::OnSelChangedTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM selectedItem = pNMTreeView->itemNew.hItem;
	if (selectedItem && m_object != 0)
	{
		CBaseObject *pObject = (CBaseObject*)m_tree.GetItemData(selectedItem);
		if (pObject)
		{
			SetDlgItemText( IDC_NAME,pObject->GetName() );
			SetDlgItemText( IDC_CLASS,pObject->GetClassDesc()->ClassName() );
			SetDlgItemText( IDC_TYPE,pObject->GetTypeDescription() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CPrefabPanel::OnBnClickedExtractSelected()
{
	if (m_object)
	{
		HTREEITEM hItem = m_tree.GetSelectedItem();
		if (hItem)
		{
			CBaseObject *pObj = (CBaseObject*)m_tree.GetItemData( hItem );
			if (pObj)
			{
				// Extract selected object.
				m_object->ExtractObject(pObj);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CPrefabPanel::OnBnClickedExtractAll()
{
	if (m_object)
		m_object->ExtractAll();
}

//////////////////////////////////////////////////////////////////////////
void CPrefabPanel::OnBnClickedPrefab()
{
	if (m_object)
	{
		GetIEditor()->OpenDataBaseLibrary( EDB_PREFAB_LIBRARY,m_object->GetPrefab() );
	}
}