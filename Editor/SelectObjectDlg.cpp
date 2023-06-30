// SelectObjectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SelectObjectDlg.h"

#include "Objects\Group.h"
#include "Objects\ObjectManager.h"
#include "Objects\Entity.h"
#include "DisplaySettings.h"
#include "Settings.h"
#include "Material\Material.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CHILD_INDENT 1

#define DISPLAY_VISIBLE 0
#define DISPLAY_HIDDEN 1
#define DISPLAY_FROZEN 2

#define COLUMN_NAME 0
#define COLUMN_TYPE 1
#define COLUMN_LAYER 2
#define COLUMN_MATERIAL 3

#define LIST_BITMAP_ANY 0
#define LIST_BITMAP_ENTITY 1
#define LIST_BITMAP_BRUSH 2
#define LIST_BITMAP_TAGPOINT 3
#define LIST_BITMAP_PATH 4
#define LIST_BITMAP_VOLUME 5
#define LIST_BITMAP_GROUP 7

int CSelectObjectDlg::m_sortFlags = 0;
int CSelectObjectDlg::m_displayMode = DISPLAY_VISIBLE;

BOOL	CSelectObjectDlg::m_bEntity = TRUE;
BOOL	CSelectObjectDlg::m_bPrefabs = TRUE;
BOOL	CSelectObjectDlg::m_bOther = TRUE;
BOOL	CSelectObjectDlg::m_bAIPoint = TRUE;
BOOL	CSelectObjectDlg::m_bTagPoint = TRUE;
BOOL	CSelectObjectDlg::m_bGroups = TRUE;
BOOL	CSelectObjectDlg::m_bVolumes = TRUE;
BOOL	CSelectObjectDlg::m_bShapes = TRUE;
BOOL  CSelectObjectDlg::m_bBrushes = TRUE;

BOOL  CSelectObjectDlg::m_bAutoselect = FALSE;
BOOL  CSelectObjectDlg::m_bTree = FALSE;

CSelectObjectDlg* CSelectObjectDlg::m_instance = 0;

/////////////////////////////////////////////////////////////////////////////
// CSelectObjectDlg dialog

CSelectObjectDlg* CSelectObjectDlg::GetInstance()
{
	if (!m_instance)
	{
		m_instance = new CSelectObjectDlg;
	}
	return m_instance;
}

CSelectObjectDlg::CSelectObjectDlg(CWnd* pParent /*=NULL*/)
	: CXTResizeDialog(CSelectObjectDlg::IDD, pParent)
{
	m_bIgnoreObjectCallback = false;
	m_picking = false;
}

void CSelectObjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectObjectDlg)
	DDX_Control(pDX, IDC_OBJECTS, m_list);
	DDX_Control(pDX, IDC_HIDE, m_hideBtn);
	DDX_Control(pDX, IDC_FREEZE, m_freezeBtn);
	DDX_Control(pDX, IDC_SEL_ALL, m_selAllBtn);
	DDX_Control(pDX, IDC_SEL_NONE, m_selNoneBtn);
	DDX_Control(pDX, IDC_SEL_INV, m_selInvBtn);
	DDX_Control(pDX, IDC_SELECT_PROPERTY_NAME, m_selByPropertyName);
	DDX_Control(pDX, IDC_SELECT_PROPERTY_VALUE, m_selByPropertyValue);
	DDX_Check(pDX, IDC_LIST_ENTITY, m_bEntity);
	DDX_Check(pDX, IDC_LIST_PREFABS, m_bPrefabs);
	DDX_Check(pDX, IDC_LIST_OTHER, m_bOther);
	DDX_Check(pDX, IDC_LIST_TAGPOINT, m_bTagPoint);
	DDX_Check(pDX, IDC_LIST_GROUPS, m_bGroups);
	DDX_Check(pDX, IDC_LIST_VOLUMES, m_bVolumes);
	DDX_Check(pDX, IDC_LIST_AIPOINTS, m_bAIPoint);
	DDX_Check(pDX, IDC_LIST_PATHS, m_bShapes);
	DDX_Check(pDX, IDC_LIST_BRUSHES, m_bBrushes);
	DDX_Check(pDX, IDC_AUTOSELECT, m_bAutoselect);
	DDX_Check(pDX, IDC_TREE, m_bTree);
	DDX_Control( pDX,IDC_PROPERTY,m_propertyFilterCtrl );
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSelectObjectDlg, CXTResizeDialog)
	//{{AFX_MSG_MAP(CSelectObjectDlg)
	ON_WM_DESTROY()
	ON_COMMAND(IDC_SEL_ALL, OnSelAll)
	ON_COMMAND(IDC_SEL_NONE, OnSelNone)
	ON_COMMAND(IDC_SEL_INV, OnSelInv)
	ON_BN_CLICKED(IDC_LIST_ENTITY, OnListType)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_OBJECTS, OnColumnclickObjects)
	ON_EN_UPDATE(IDC_FASTFIND, OnUpdateFastfind)
	ON_NOTIFY(NM_CLICK, IDC_OBJECTS, OnClickObjects)
	ON_BN_CLICKED(IDC_VISIBLE, OnVisible)
	ON_BN_CLICKED(IDC_HIDDEN, OnHidden)
	ON_BN_CLICKED(IDC_FROZEN, OnFrozen)
	ON_BN_CLICKED(IDC_HIDE, OnHide)
	ON_BN_CLICKED(IDC_FREEZE, OnFreeze)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_OBJECTS, OnItemchangedObjects)
	ON_BN_CLICKED(IDC_LIST_BUILDING, OnListType)
	ON_BN_CLICKED(IDC_LIST_TAGPOINT, OnListType)
	ON_BN_CLICKED(IDC_LIST_STATOBJ, OnListType)
	ON_BN_CLICKED(IDC_LIST_SOUND, OnListType)
	ON_BN_CLICKED(IDC_LIST_PATHS, OnListType)
	ON_BN_CLICKED(IDC_LIST_VOLUMES, OnListType)
	ON_BN_CLICKED(IDC_LIST_BRUSHES, OnListType)
	ON_BN_CLICKED(IDC_LIST_OTHER, OnListType)
	ON_BN_CLICKED(IDC_AUTOSELECT, OnListType)
	ON_BN_CLICKED(IDC_TREE, OnListType)
	ON_BN_CLICKED(IDC_SELECT_PROPERTY_NAME, OnMatchPropertyName)
	ON_BN_CLICKED(IDC_SELECT_PROPERTY_VALUE, OnMatchPropertyValue)
	ON_NOTIFY(NM_DBLCLK, IDC_OBJECTS, OnDblclkObjects)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static int __stdcall CompareItems( LPARAM p1,LPARAM p2,LPARAM sort )
{
	CBaseObject *o1 = (CBaseObject*)p1;
	CBaseObject *o2 = (CBaseObject*)p2;
	int order = sort >> 8;
	int column = sort & 0xF;
	if (column == 0)
	{
		// Sort by name.
		if (order == 0)
			return stricmp( o1->GetName(),o2->GetName() );
		else
			return -stricmp( o1->GetName(),o2->GetName() );
	}
	else if (column == 1)
	{
		// Sort by type
		if (order == 0)
			return stricmp( o1->GetTypeDescription(),o2->GetTypeDescription() );
		else
			return -stricmp( o1->GetTypeDescription(),o2->GetTypeDescription() );
	}
	else if (column == 2)
	{
		// Sort by type
		if (order == 0)
			return stricmp( o1->GetLayer()->GetName(),o2->GetLayer()->GetName() );
		else
			return -stricmp( o1->GetLayer()->GetName(),o2->GetLayer()->GetName() );
	}
	else if (column == 3)
	{
		const char *mtl1 = "";
		const char *mtl2 = "";
		if (o1->GetMaterial())
			mtl1 = o1->GetMaterial()->GetName();
		if (o2->GetMaterial())
			mtl2 = o2->GetMaterial()->GetName();
		// Sort by type
		if (order == 0)
		{
			return stricmp( mtl1,mtl2 );
		}
		else
			return -stricmp( mtl1,mtl2 );
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSelectObjectDlg message handlers

BOOL CSelectObjectDlg::OnInitDialog() 
{
	CXTResizeDialog::OnInitDialog();

	m_pObjMan = GetIEditor()->GetObjectManager();
	m_displayMode = DISPLAY_VISIBLE;
	m_picking = GetIEditor()->IsPicking();

	if (GetIEditor()->IsPicking())
	{
		SetWindowText( "Pick Object" );
		m_list.ModifyStyle( 0,LVS_SINGLESEL );
	}

	m_nameFilter = "";
	m_propertyFilter = "";
	m_bMatchPropertyName = true;

	CheckDlgButton( IDC_VISIBLE,BST_CHECKED );

	CRect rc;
	GetDlgItem(IDC_OBJECTS)->GetClientRect( rc );
	int w1 = 2*rc.right/4-20;
	int w2 = rc.right/3 + 10;
	int w3 = rc.right - w1 - w2 - 25;

	//m_imageList.Create(IDB_SELECT_OBJECT_LIST, 16, 1, RGB (255, 255, 255));
	CMFCUtils::LoadTrueColorImageList( m_imageList,IDB_SELECT_OBJECT_LIST,16,RGB(255,255,255) );

	LVCOLUMN lvc;
	ZeroStruct(lvc);

	m_list.SetExtendedStyle( LVS_EX_FLATSB|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP );
	m_list.InsertColumn( COLUMN_NAME+1,"Name",LVCFMT_LEFT,w1,0 );
	m_list.InsertColumn( COLUMN_TYPE+1,"Type",LVCFMT_LEFT,w2,1 );
	m_list.InsertColumn( COLUMN_LAYER+1,"Layer",LVCFMT_LEFT,w2,2 );
	m_list.InsertColumn( COLUMN_MATERIAL+1,"Material",LVCFMT_LEFT,80,2 );

	//m_list.InsertColumn( COLUMN_NAME,"Name",LVCFMT_LEFT,w1,0 );
	//m_list.InsertColumn( COLUMN_TYPE,"Type",LVCFMT_LEFT,w2,1 );
	//m_list.InsertColumn( COLUMN_LAYER,"Layer",LVCFMT_LEFT,w2,2 );
	//m_list.InsertColumn( COLUMN_MATERIAL,"Material",LVCFMT_LEFT,80,2 );

	FillList();

	// Define resize anchors.
	SetResize( IDC_OBJECTS,SZ_RESIZE(1) );
	SetResize( IDC_SEL_NONE,SZ_HORREPOS(1) );
	SetResize( IDC_SEL_ALL,SZ_HORREPOS(1) );
	SetResize( IDC_SEL_INV,SZ_HORREPOS(1) );
	
	SetResize( IDC_STATIC,SZ_VERREPOS(1) );
	SetResize( IDC_STATIC1,SZ_HORREPOS(1) );
	SetResize( IDC_STATIC2,SZ_HORREPOS(1) );
	
	SetResize( IDC_FASTFIND,CXTResizeRect(0,1,1,1) );
	SetResize( IDC_STATIC3,CXTResizeRect(0,1,1,1) );
	
	SetResize( IDC_LIST_ENTITY,SZ_HORREPOS(1) );
	SetResize( IDC_LIST_PREFABS,SZ_HORREPOS(1) );
	SetResize( IDC_LIST_AIPOINTS,SZ_HORREPOS(1) );
	SetResize( IDC_LIST_TAGPOINT,SZ_HORREPOS(1) );
	SetResize( IDC_LIST_OTHER,SZ_HORREPOS(1) );
	SetResize( IDC_LIST_GROUPS,SZ_HORREPOS(1) );
	SetResize( IDC_LIST_VOLUMES,SZ_HORREPOS(1) );
	SetResize( IDC_LIST_PATHS,SZ_HORREPOS(1) );
	SetResize( IDC_LIST_BRUSHES,SZ_HORREPOS(1) );

	SetResize( IDC_AUTOSELECT,SZ_HORREPOS(1) );
	SetResize( IDC_TREE,SZ_HORREPOS(1) );

	SetResize( IDC_VISIBLE,SZ_HORREPOS(1) );
	SetResize( IDC_HIDDEN,SZ_HORREPOS(1) );
	SetResize( IDC_FROZEN,SZ_HORREPOS(1) );
	
	SetResize( IDC_HIDE,SZ_HORREPOS(1) );
	SetResize( IDC_FREEZE,SZ_HORREPOS(1) );

	SetResize( IDC_PROPERTY_STATIC,SZ_HORREPOS(1) );
	SetResize( IDC_SELECT_PROPERTY_NAME,SZ_HORREPOS(1) );
	SetResize( IDC_SELECT_PROPERTY_VALUE,SZ_HORREPOS(1) );
	SetResize( IDC_PROPERTY,SZ_HORREPOS(1) );
	
	SetResize( IDOK,SZ_REPOS(1) );
	SetResize( IDCANCEL,SZ_REPOS(1) );

	AutoLoadPlacement( "Dialogs\\SelectObject" );
	EnableControls();

	// Add event listener.
	GetIEditor()->GetObjectManager()->AddObjectEventListener( functor(*this, &CSelectObjectDlg::OnObjectEvent) );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CSelectObjectDlg::OnDestroy()
{
	// Remove event listener.
	GetIEditor()->GetObjectManager()->RemoveObjectEventListener( functor(*this, &CSelectObjectDlg::OnObjectEvent) );
	CXTResizeDialog::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////
void CSelectObjectDlg::PostNcDestroy()
{
	CXTResizeDialog::PostNcDestroy();
	delete this;
	m_instance = 0;
}

//////////////////////////////////////////////////////////////////////////
// Simply sub string searching case insensitive.
//////////////////////////////////////////////////////////////////////////
inline const char* strstri( const char *s1, const char *s2) 
{
	int i,j,k; 
	for(i=0;s1[i];i++) 
		for(j=i,k=0;tolower(s1[j])==tolower(s2[k]);j++,k++) 
			if(!s2[k+1]) 
				return (s1+i); 

	return NULL; 
} 

void CSelectObjectDlg::AddObject( CBaseObject *obj,int level )
{
	int objType = obj->GetType();
	if (gSettings.objectHideMask & objType)
		return;

	if (obj->GetGroup())
	{
		if (!obj->GetGroup()->IsOpen())
			return;
	}

	if (m_displayMode == DISPLAY_VISIBLE)
	{
		if (obj->IsHidden()||obj->IsFrozen())
			return;
	}
	else if (m_displayMode == DISPLAY_HIDDEN)
	{
		if (!obj->IsHidden())
			return;
	}
	else if (m_displayMode == DISPLAY_FROZEN)
	{
		if (!obj->IsFrozen())
			return;
	}

	CString typeName = obj->GetTypeDescription();

	if (!(objType & m_listMask))
		return;

	CObjectLayer *layer = obj->GetLayer();
	if (layer->IsFrozen())
		return;

	if (!layer->IsVisible())
		return;

	/*
	if (type == OBJTYPE_ENTITY && obj->GetRuntimeClass()->IsDerivedFrom(RUNTIME_CLASS(CEntity)))
	{
	typeName = ((CEntity*)obj)->GetEntityClass();
	}
	*/
	CString objName = obj->GetName();
	if (obj->GetGroup())
	{
		objName = CString("[") + obj->GetGroup()->GetName() + "] " + obj->GetName();
	}
	else
	{
		if (obj->GetType() == OBJTYPE_GROUP)
			objName = CString("[") + obj->GetName() + "]";
	}

	// Check if have name filter.
	if (!m_nameFilter.IsEmpty())
	{
		if (strstri(objName,m_nameFilter) == 0)
		{
			return;
		}
	}

	if (!m_propertyFilter.IsEmpty())
	{
		if (!IsPropertyMatch( obj ))
			return;
	}

	int iImage = LIST_BITMAP_ANY;
	switch (objType)
	{
	case OBJTYPE_GROUP:
		iImage = LIST_BITMAP_GROUP;
		break;
	case OBJTYPE_TAGPOINT:
		iImage = LIST_BITMAP_TAGPOINT;
		break;
	case OBJTYPE_ENTITY:
		iImage = LIST_BITMAP_ENTITY;
		break;
	case OBJTYPE_SHAPE:
		iImage = LIST_BITMAP_PATH;
		break;
	case OBJTYPE_VOLUME:
		iImage = LIST_BITMAP_VOLUME;
		break;
	case OBJTYPE_BRUSH:
		iImage = LIST_BITMAP_BRUSH;
		break;
	}

	int last = m_list.GetItemCount();
	//int id = m_list.InsertItem( last,(const char*)objName );

	LVITEM lvi;
	lvi.mask = LVIF_PARAM|LVIF_INDENT|LVIF_IMAGE|LVIF_TEXT;
	lvi.iItem = last;
	lvi.iIndent = level;
	lvi.iSubItem = COLUMN_NAME;
	lvi.stateMask = 0;
	lvi.state = 0;
	lvi.pszText = const_cast<char*>((const char*)objName);
	lvi.iImage = iImage;
	lvi.lParam = (LPARAM)obj;

	int id = m_list.InsertItem( &lvi );

		/*
#if _MFC_VER >= 0x0700 //MFC7.0
	m_list.SetItem( id,COLUMN_NAME,LVIF_PARAM|LVIF_INDENT|LVIF_IMAGE,0,iImage,0,0,(LPARAM)obj,level );
#else //MFC7.0
	m_list.SetItem( id,COLUMN_NAME,LVIF_PARAM|LVIF_INDENT|LVIF_IMAGE,0,iImage,0,0,(LPARAM)obj );
#endif
	*/

	m_list.SetItem( id,COLUMN_TYPE,LVIF_TEXT|LVIF_STATE,(const char*)typeName,0,0,0,0 );
	m_list.SetItem( id,COLUMN_LAYER,LVIF_TEXT|LVIF_STATE,(const char*)layer->GetName(),0,0,0,0 );

	CMaterial *pMtl = obj->GetMaterial();
	if (pMtl)
	{
		m_list.SetItem( id,COLUMN_MATERIAL,LVIF_TEXT|LVIF_STATE,(const char*)pMtl->GetName(),0,0,0,0 );
	}
	//m_list.SetItem( id,2,LVIF_TEXT,"S",0,0,0,0 );

	if (obj->IsSelected())
	{
		m_list.SetItem( id,0,LVIF_STATE,0,0,LVIS_SELECTED,LVIS_SELECTED,0 );
	}
}

void CSelectObjectDlg::AddObjectRecursively( CBaseObject *obj,int level )
{
	AddObject( obj,level );
	for (int i = 0; i < obj->GetChildCount(); i++)
	{
		AddObjectRecursively( obj->GetChild(i),level+1 );
	}
}

//////////////////////////////////////////////////////////////////////////
void CSelectObjectDlg::FillList()
{
	if (m_bTree)
		m_list.SetImageList( &m_imageList,LVSIL_SMALL );
	else
		m_list.SetImageList( NULL,LVSIL_SMALL );

	m_list.SetRedraw(FALSE);
	//m_list.LockWindowUpdate();
	m_list.DeleteAllItems();

	int numObjects = GetIEditor()->GetObjectManager()->GetObjectCount();
	std::vector<CBaseObject*> objects;
	objects.clear();
	objects.reserve( numObjects );
	GetIEditor()->GetObjectManager()->GetObjects( objects );

	int mask = 0;
	if (m_bEntity)
		mask |= OBJTYPE_ENTITY;
	if (m_bPrefabs)
		mask |= OBJTYPE_PREFAB;
	if (m_bTagPoint)
		mask |= OBJTYPE_TAGPOINT;
	if (m_bAIPoint)
		mask |= OBJTYPE_AIPOINT;
	if (m_bVolumes)
		mask |= OBJTYPE_VOLUME;
	if (m_bGroups)
		mask |= OBJTYPE_GROUP;
	if (m_bShapes)
		mask |= OBJTYPE_SHAPE;
	if (m_bBrushes)
		mask |= OBJTYPE_BRUSH;
	if (m_bOther)
		mask |= ~(OBJTYPE_ANY_DEFINED);
	m_listMask = mask;
	
	////////////////////////////////////////////////////////////////////////
	// Fill the list box with data from the object
	////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < objects.size(); i++)
	{
		CBaseObject *obj = objects[i];

		if (m_bTree && obj->GetParent())
			continue;

		if (m_bTree)
			AddObjectRecursively( obj,0 );
		else
			AddObject( obj,0 );
	}
	if (!m_bTree)
		m_list.SortItems( CompareItems,m_sortFlags );
	
	// Ensure that first selected item is visible.
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		m_list.EnsureVisible(nItem,FALSE);
	}
	//m_list.UnlockWindowUpdate();
	m_list.SetRedraw(TRUE);
}

afx_msg void CSelectObjectDlg::OnSelAll()
{
	////////////////////////////////////////////////////////////////////////
	// Select all items in the list
	////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		m_list.SetItem( i,0,LVIF_STATE,0,0,LVIS_SELECTED,LVIS_SELECTED,0 );
		SelectItemObject( i );
	}
	m_list.SetFocus();
}

afx_msg void CSelectObjectDlg::OnSelNone()
{
	////////////////////////////////////////////////////////////////////////
	// Remove selection from all items in the list
	////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		m_list.SetItem( i,0,LVIF_STATE,0,0,0,LVIS_SELECTED,0 );
		SelectItemObject( i );
	}
	m_list.SetFocus();
}

afx_msg void CSelectObjectDlg::OnSelInv()
{
	////////////////////////////////////////////////////////////////////////
	// Reverse selection of all items in the list
	////////////////////////////////////////////////////////////////////////
	m_bIgnoreObjectCallback = true;
	bool bChanged = false;
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		LVITEM item;
		ZeroStruct(item);
		item.mask = LVIF_STATE;
		item.stateMask = LVIS_SELECTED;
		item.iItem = i;
		m_list.GetItem( &item );
		item.state = (item.state==LVIS_SELECTED)?0:LVIS_SELECTED;
		m_list.SetItem( &item );

		SelectItemObject( i );
	}
	m_bIgnoreObjectCallback = false;
	m_list.SetFocus();
}

void CSelectObjectDlg::OnOK() 
{
	//////////////////////////////////////////////////////////////////////
	// Write the changes to the selection back to the document
	//////////////////////////////////////////////////////////////////////
	/*
	if (!m_picking)
	{
		GetIEditor()->ClearSelection();
	}

	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		LVITEM item;
		ZeroStruct(item);
		item.mask = LVIF_STATE|LVIF_PARAM;
		item.stateMask = LVIS_SELECTED;
		item.iItem = i;
		m_list.GetItem( &item );
		if (item.state == LVIS_SELECTED)
		{
			CBaseObject *obj = (CBaseObject*)item.lParam;
			GetIEditor()->GetObjectManager()->SelectObject( obj );
			
			if (picking)
				break;
		}
	}
	*/
	m_bIgnoreObjectCallback = true;
	if (m_picking || !m_bAutoselect)
	{
		if (!m_picking)
		{
			GetIEditor()->ClearSelection();
		}
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			LVITEM item;
			ZeroStruct(item);
			item.mask = LVIF_STATE|LVIF_PARAM;
			item.stateMask = LVIS_SELECTED;
			item.iItem = i;
			m_list.GetItem( &item );
			if (item.state == LVIS_SELECTED)
			{
				CBaseObject *obj = (CBaseObject*)item.lParam;
				m_pObjMan->SelectObject( obj );
				if (m_picking)
					break;
			}
		}
	}
	m_bIgnoreObjectCallback = false;
	
	DestroyWindow();
}

void CSelectObjectDlg::OnCancel() 
{
	DestroyWindow();
}

void CSelectObjectDlg::OnListType() 
{
	UpdateData();
	FillList();
}

void CSelectObjectDlg::OnColumnclickObjects(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

	int order = 0;
	int column = pNMListView->iSubItem;
	int bit = 1 << column;
	if ((m_sortFlags&0xF) == column)
	{
		// change order.
	 order = m_sortFlags >> 8;
	 order = (order == 0) ? 1 : 0;
	}
	m_sortFlags = (order << 8) | column;
	// Resort items.
	if (!m_bTree)
		m_list.SortItems( CompareItems,m_sortFlags );
	
	*pResult = 0;
}

void CSelectObjectDlg::OnUpdateFastfind() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CXTResizeDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	CString findText;
	GetDlgItemText( IDC_FASTFIND,findText );
	//if (findText.IsEmpty())
		//return;
	
	m_nameFilter = findText;
	FillList();
	/*
	LVFINDINFO info;
	ZeroStruct(info);
	info.flags = LVFI_PARTIAL|LVFI_STRING;
	info.psz = findText;

	int nIndex=-1;
	POSITION pos = NULL;

	GetIEditor()->ClearSelection();
	// First unselect all.
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		m_list.SetItem( i,0,LVIF_STATE,0,0,0,LVIS_SELECTED,0 );
	}

	// Find string partially from the begining.
	while ((nIndex = m_list.FindItem(&info,nIndex)) != -1)
	{
		m_list.SetItem( nIndex,0,LVIF_STATE,0,0,LVIS_SELECTED,LVIS_SELECTED,0 );
		SelectItemObject( i );
	}

	pos = m_list.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		// Nothing selected.

		// Select all items that much this string.
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			CString text = m_list.GetItemText( i,0 );
			if (strstr(text,findText) != 0)
			{
				m_list.SetItem( i,0,LVIF_STATE,0,0,LVIS_SELECTED,LVIS_SELECTED,0 );
				SelectItemObject( i );
			}
		}
	}

	// Ensure that first selected item is visible.
	pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		m_list.EnsureVisible(nItem,FALSE);
	}
	GetDlgItem(IDC_FASTFIND)->SetFocus();
	*/
}

void CSelectObjectDlg::OnClickObjects(NMHDR* pNMHDR, LRESULT* pResult) 
{
	/*
	CString text;
	GetDlgItemText( IDC_FASTFIND,text );
	if (!text.IsEmpty())
	{
		SetDlgItemText( IDC_FASTFIND,"" );
		m_nameFilter = "";
	}
	*/
	
	*pResult = 0;
}

void CSelectObjectDlg::EnableControls()
{
	BOOL bEnable;
	if (m_list.GetSelectedCount() > 0)
		bEnable = TRUE;
	else
		bEnable = FALSE;
		
	GetDlgItem(IDC_HIDE)->EnableWindow(bEnable);
	GetDlgItem(IDC_FREEZE)->EnableWindow(bEnable);
}

void CSelectObjectDlg::OnVisible() 
{
	m_displayMode = DISPLAY_VISIBLE;
	FillList();
	
	GetDlgItem(IDC_HIDE)->SetWindowText( "Hide" );
	GetDlgItem(IDC_FREEZE)->SetWindowText( "Freeze" );
}

void CSelectObjectDlg::OnHidden() 
{
	m_displayMode = DISPLAY_HIDDEN;
	FillList();

	GetDlgItem(IDC_HIDE)->SetWindowText( "Unhide" );
	GetDlgItem(IDC_FREEZE)->SetWindowText( "Freeze" );
}

void CSelectObjectDlg::OnFrozen() 
{
	m_displayMode = DISPLAY_FROZEN;
	FillList();
	GetDlgItem(IDC_HIDE)->SetWindowText( "Hide" );
	GetDlgItem(IDC_FREEZE)->SetWindowText( "Unfreeze" );
}

void CSelectObjectDlg::OnHide() 
{
	// Hide/Unhide selected objects.
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		LVITEM item;
		ZeroStruct(item);
		item.mask = LVIF_STATE|LVIF_PARAM;
		item.stateMask = LVIS_SELECTED;
		item.iItem = i;
		m_list.GetItem( &item );
		if (item.state == LVIS_SELECTED)
		{
			CBaseObject *obj = (CBaseObject*)item.lParam;
			if (m_displayMode == DISPLAY_HIDDEN)
				GetIEditor()->GetObjectManager()->HideObject(obj,false);
			else
				GetIEditor()->GetObjectManager()->HideObject(obj,true);
		}
	}
	FillList();
}

void CSelectObjectDlg::OnFreeze() 
{
	// Freeze/Unfreeze selected objects.
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		LVITEM item;
		ZeroStruct(item);
		item.mask = LVIF_STATE|LVIF_PARAM;
		item.stateMask = LVIS_SELECTED;
		item.iItem = i;
		m_list.GetItem( &item );
		if (item.state == LVIS_SELECTED)
		{
			CBaseObject *obj = (CBaseObject*)item.lParam;
			if (m_displayMode == DISPLAY_FROZEN)
				GetIEditor()->GetObjectManager()->FreezeObject(obj,false);
			else
				GetIEditor()->GetObjectManager()->FreezeObject(obj,true);
		}
	}
	FillList();
}

void CSelectObjectDlg::OnItemchangedObjects(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	CBaseObject *pObject = (CBaseObject*)m_list.GetItemData( pNMListView->iItem );
	if (pObject)
	{
		bool bWasSelected = (pNMListView->uOldState & LVIS_SELECTED) != 0;
		bool bNowSelected = (pNMListView->uNewState & LVIS_SELECTED) != 0;
		if (bWasSelected != bNowSelected)
		{
			SelectItemObject( pNMListView->iItem );
		}
	}

	EnableControls();

	
	*pResult = 0;
}

void CSelectObjectDlg::OnDblclkObjects(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (m_picking || !m_bAutoselect)
	{
		OnOK();
		*pResult = 0;
		return;
	}

	// Select and Goto object.
	NMITEMACTIVATE *pNM = (NMITEMACTIVATE*)pNMHDR;
	CBaseObject *pObject = (CBaseObject*)m_list.GetItemData( pNM->iItem );
	if (pObject)
	{
		if (m_displayMode == DISPLAY_VISIBLE)
		{
			SelectItemObject( pNM->iItem );
			Vec3 selPos = pObject->GetWorldPos();
			Vec3 pos = selPos;
			pos += Vec3(0,5,1);
			//pos.z = GetIEditor()->GetTerrainElevation(pos.x,pos.y)+5;
			GetIEditor()->SetViewerPos( pos );
			Vec3 dir = selPos - pos;
			dir = ConvertVectorToCameraAngles(dir);
			GetIEditor()->SetViewerAngles( dir );
			GetIEditor()->SetModifiedFlag();
		}
	}
	
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CSelectObjectDlg::OnObjectEvent( CBaseObject *pObject,int event )
{
	if (m_bIgnoreObjectCallback)
		return;

	switch (event)
	{
	case CBaseObject::ON_DELETE:
	case CBaseObject::ON_SELECT:
	case CBaseObject::ON_UNSELECT:
		{
			LVFINDINFO info;
			ZeroStruct(info);
			info.flags = LVFI_PARAM;
			info.lParam = (LPARAM)pObject;

			int iItem = m_list.FindItem( &info );
			if (iItem >= 0)
			{
				if (event == CBaseObject::ON_DELETE)
					m_list.DeleteItem( iItem );
				else if (event == CBaseObject::ON_SELECT)
					m_list.SetItem( iItem,0,LVIF_STATE,0,0,LVIS_SELECTED,LVIS_SELECTED,0 );
				else if (event == CBaseObject::ON_UNSELECT)
					m_list.SetItem( iItem,0,LVIF_STATE,0,0,0,LVIS_SELECTED,0 );
			}
		}
		break;
	}
}

void CSelectObjectDlg::SelectItemObject( int iItem )
{
	if (!m_bAutoselect)
		return;

	if (m_picking || m_displayMode != DISPLAY_VISIBLE)
	{
		return;
	}
	m_bIgnoreObjectCallback = true;
	
	LVITEM item;
	ZeroStruct(item);
	item.mask = LVIF_STATE|LVIF_PARAM;
	item.stateMask = LVIS_SELECTED;
	item.iItem = iItem;
	m_list.GetItem( &item );

	CBaseObject *pObject = (CBaseObject*)item.lParam;
	if (pObject)
	{
		if (item.state & LVIS_SELECTED)
			m_pObjMan->SelectObject(pObject);
		else
			m_pObjMan->UnselectObject(pObject);
	}
	m_bIgnoreObjectCallback = false;
}

//////////////////////////////////////////////////////////////////////////
void CSelectObjectDlg::OnMatchPropertyName()
{
	m_bMatchPropertyName = true;
	CWaitCursor wait;
	m_propertyFilterCtrl.GetWindowText( m_propertyFilter );
	FillList();
}

//////////////////////////////////////////////////////////////////////////
void CSelectObjectDlg::OnMatchPropertyValue()
{
	m_bMatchPropertyName = false;
	CWaitCursor wait;
	m_propertyFilterCtrl.GetWindowText( m_propertyFilter );
	FillList();
}

//////////////////////////////////////////////////////////////////////////
bool CSelectObjectDlg::IsPropertyMatch( CBaseObject *pObject )
{
	//if (strstri(objName,m_nameFilter) == 0)

	CVarBlock *pVars = pObject->GetVarBlock();
	if (pVars)
	{
		for (int i = 0; i < pVars->GetVarsCount(); i++)
		{
			IVariable *pVar = pVars->GetVariable(i);
			if (IsPropertyMatchVariable( pVar ))
				return true;
		}
	}

	if (pObject->IsKindOf( RUNTIME_CLASS(CEntity) ))
	{
		CEntity *pEntity = (CEntity*)pObject;
		{
			CVarBlock *pVars = pEntity->GetProperties();
			if (pVars)
			{
				for (int i = 0; i < pVars->GetVarsCount(); i++)
				{
					IVariable *pVar = pVars->GetVariable(i);
					if (IsPropertyMatchVariable( pVar ))
						return true;
				}
			}
		}
		{
			CVarBlock *pVars = pEntity->GetProperties2();
			if (pVars)
			{
				for (int i = 0; i < pVars->GetVarsCount(); i++)
				{
					IVariable *pVar = pVars->GetVariable(i);
					if (IsPropertyMatchVariable( pVar ))
						return true;
				}
			}
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CSelectObjectDlg::IsPropertyMatchVariable( IVariable *pVar )
{
	if (m_bMatchPropertyName)
	{
		if (strstri(pVar->GetHumanName(),m_propertyFilter) != 0)
			return true;
	}
	else
	{
		CString value;
		pVar->Get( value );
		if (strstri(value,m_propertyFilter) != 0)
			return true;
	}
	if (pVar->NumChildVars() > 0)
	{
		for (int i =  0; i < pVar->NumChildVars(); i++)
		{
			IVariable *pChildVar = pVar->GetChildVar( i );
			if (IsPropertyMatchVariable( pChildVar ))
				return true;
		}
	}
	return false;
}
