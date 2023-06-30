// BuildingPanel.cpp : implementation file
//

#include "stdafx.h"
#include "BuildingPanel.h"

#include "Objects\ObjectManager.h"
#include "Objects\Building.h"

#include "ObjectCreateTool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBuildingPanel dialog


CBuildingPanel::CBuildingPanel(CWnd* pParent /*=NULL*/)
: CDialog( CBuildingPanel::IDD,pParent)
{
	//{{AFX_DATA_INIT(CBuildingPanel)
	//}}AFX_DATA_INIT

	m_building = 0;
	m_picking = false;
	m_currHelper = 0;
}

CBuildingPanel::~CBuildingPanel()
{
	if (GetIEditor()->GetEditTool() == m_createTool)
	{
		GetIEditor()->SetEditTool(0);
	}
}

void CBuildingPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBuildingPanel)
	DDX_Control(pDX, IDC_UNBIND, m_unbindBuilding);
	DDX_Control(pDX, IDC_BIND, m_bindButton);
	DDX_Control(pDX, IDC_WIREFRAME, m_wireframe);
	DDX_Control(pDX, IDC_PORTALS, m_portals);
	DDX_Control(pDX, IDC_HIDE_NONE, m_hideNone);
	DDX_Control(pDX, IDC_HIDE_INVERT, m_hideInvert);
	DDX_Control(pDX, IDC_HIDE_ALL, m_hideAll);
	DDX_Control(pDX, IDC_HIDDEN_SECTORS, m_hiddenSectors);
	DDX_Control(pDX, IDC_HELPERS, m_helpers);
	DDX_Control(pDX, IDC_CHANGE, m_browseButton);
	DDX_Control(pDX, IDC_SPAWN, m_spawnButton);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBuildingPanel, CDialog)
	//{{AFX_MSG_MAP(CBuildingPanel)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_CHANGE, OnChange)
	ON_BN_CLICKED(IDC_SPAWN, OnSpawn)
	ON_LBN_SELCHANGE(IDC_HIDDEN_SECTORS, OnSelchangeHiddenSectors)
	ON_BN_CLICKED(IDC_HIDE_ALL, OnHideAll)
	ON_BN_CLICKED(IDC_HIDE_NONE, OnHideNone)
	ON_BN_CLICKED(IDC_HIDE_INVERT, OnHideInvert)
	ON_BN_CLICKED(IDC_BIND, OnBind)
	ON_BN_CLICKED(IDC_UNBIND, OnUnbind)
	ON_BN_CLICKED(IDC_WIREFRAME, OnWireframe)
	ON_BN_CLICKED(IDC_PORTALS, OnBnClickedPortals)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBuildingPanel message handlers

BOOL CBuildingPanel::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CRect rc;
	m_helpers.GetClientRect( rc );
	int w1 = rc.right/2;
	int w2 = rc.right/2-2;

	m_helpers.SetBkColor( RGB(0xE0,0xE0,0xE0) );
	m_helpers.SetTextBkColor( RGB(0xE0,0xE0,0xE0) );

	m_hiddenSectors.SetBkColor( RGB(0xE0,0xE0,0xE0) );

	// Init helpers.
	//m_helpers.SetExtendedStyle( LVS_EX_FLATSB|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP );
	m_helpers.SetExtendedStyle( LVS_EX_FLATSB|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_GRIDLINES );
	m_helpers.InsertColumn( 1,"Helper",LVCFMT_LEFT,w1,0 );
	m_helpers.InsertColumn( 2,"Object",LVCFMT_LEFT,w2,1 );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBuildingPanel::OnAdd() 
{
	assert( m_building != 0 );
	// TODO: Add your control notification handler code here
	// Select Cgf file.
	/*
	CString file,relFile;
	if (GetIEditor()->SelectFile( "*.cgf","Objects\\Buildings",file,relFile ))
	{
		m_building->AddObject( relFile );
		RefreshList();
	}
	*/

	/*
	CString filter = "Building Files (*.bld)|*.bld||";
	char files[4096];
	memset( files,0,sizeof(files) );

	CFileDialog dlg(TRUE, NULL,NULL, OFN_ALLOWMULTISELECT|OFN_ENABLESIZING|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR, filter );
	dlg.m_ofn.lpstrInitialDir = "Objects\\Buildings";
	dlg.m_ofn.lpstrFile = files;
	dlg.m_ofn.nMaxFile = sizeof(files);

	if (dlg.DoModal() == IDOK)
	{
		POSITION pos = dlg.GetStartPosition();
		while (pos != NULL)
		{
			CString fileName = dlg.GetNextPathName(pos);
			CString	relativeFileName = GetIEditor()->GetRelativePath( fileName );
			m_building->AddObject( relativeFileName );
		}
		RefreshList();
	}
	*/
}

void CBuildingPanel::OnRemove() 
{
	assert( m_building != 0 );

	/*
	std::vector<int> items;
	items.resize( m_objects.GetSelCount() );
	if (items.size() > 0)
	{
		std::vector<CString> names;
		names.resize( items.size() );
		m_objects.GetSelItems( items.size(),&items[0] );
		for (int i = 0; i < items.size(); i++)
		{
			names[i] = m_building->GetObjectName(items[i]);
		}
		for (i = 0; i < names.size(); i++)
		{
			m_building->RemoveObject( names[i] );
		}
		RefreshList();
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CBuildingPanel::SetBuilding( CBuilding *obj )
{
	m_building = obj;
	assert( m_building != 0 );
	m_wireframe.SetCheck( (m_building->IsWireframe())?BST_CHECKED:BST_UNCHECKED );
	m_portals.SetCheck( (m_building->IsPortals())?BST_CHECKED:BST_UNCHECKED );
	RefreshList();
};

//////////////////////////////////////////////////////////////////////////
void CBuildingPanel::RefreshList()
{
	if (!m_building)
		return;

	CString str = m_building->GetObjectName();
	str.MakeLower();
	str.Replace( "objects\\buildings\\","" );
	SetDlgItemText( IDC_BUILDING,str );
	/*
	assert( m_building != 0 );
	m_objects.ResetContent();
	for (int i = 0; i < m_building->GetObjectCount(); i++)
	{
		CString name = m_building->GetObjectName(i);
		char file[_MAX_FNAME];
		char ext[_MAX_EXT];
		_splitpath(name,NULL,NULL,file,ext );
		m_objects.InsertString(i,CString(file) );
	}
	*/

	/*
	m_helpers.DeleteAllItems();
	for (int i = 0; i < m_building->GetChildCount(); i++)
	{
		CBaseObject *obj = m_building->GetChild(i);
		int id = m_helpers.InsertItem( i,obj->GetName() );
		m_helpers.SetItem( id,1,LVIF_TEXT|LVIF_STATE,obj->GetTypeDescription(),0,0,0,0 );
	}
	*/
	int i;

	m_helpers.DeleteAllItems();
	std::vector<CBuilding::ObjectHelper> &helpers = m_building->GetHelpers();
	for (i = 0; i < helpers.size(); i++)
	{
		CBaseObject *obj = helpers[i].object;
		int id = m_helpers.InsertItem( i,helpers[i].name );
		if (obj)
			m_helpers.SetItem( id,1,LVIF_TEXT|LVIF_STATE,obj->GetName(),0,0,0,0 );
	}

	m_hiddenSectors.ResetContent();
	for (i = 0; i < m_building->GetNumSectors(); i++)
	{
		CString str;
		str.Format( "Sector %d",i );
		m_hiddenSectors.AddString( str );
		if (m_building->IsSectorHidden(i))
			m_hiddenSectors.SetSel(i);
	}
}

void CBuildingPanel::OnChange() 
{
	CString filter = "Building Files (*.bld)|*.bld||";

	CFileDialog dlg(TRUE, NULL,NULL, OFN_ENABLESIZING|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR, filter );
	dlg.m_ofn.lpstrInitialDir = "Objects\\Buildings";

	if (dlg.DoModal() == IDOK)
	{
		CString	relativeFileName = GetIEditor()->GetRelativePath( dlg.GetPathName() );
		m_building->LoadBuilding( relativeFileName );
		RefreshList();
	}		
}

void CBuildingPanel::OnSpawn() 
{
	int sel = m_helpers.GetNextItem(-1,LVNI_SELECTED);
	if (sel < 0)
		return;
	m_currHelper = sel;

	assert( m_building );
	//m_building->SpawnEntities();
	//RefreshList();
	m_createTool = new CObjectCreateTool( functor(*this,OnCreateCallback) );
	GetIEditor()->SetEditTool( m_createTool );
	m_createTool->StartCreation( "StdEntity" );
}

void CBuildingPanel::OnCreateCallback( class CObjectCreateTool *tool,class CBaseObject *object )
{
	assert( m_building );
	tool->AcceptCreation();
	if (m_currHelper >= 0 && m_currHelper < m_building->GetHelpers().size())
	{
		m_building->BindToHelper( m_currHelper,object );
		RefreshList();
	}
}

void CBuildingPanel::OnSelchangeHiddenSectors() 
{
	for (int i = 0; i < m_hiddenSectors.GetCount(); i++)
	{
		if (m_hiddenSectors.GetSel(i))
			m_building->HideSector(i,true);
		else
			m_building->HideSector(i,false);
	}
}

void CBuildingPanel::OnHideAll() 
{
	for (int i = 0; i < m_building->GetNumSectors(); i++)
		m_building->HideSector(i,true);
	RefreshList();
}

void CBuildingPanel::OnHideNone() 
{
	for (int i = 0; i < m_building->GetNumSectors(); i++)
		m_building->HideSector(i,false);
	RefreshList();
}

void CBuildingPanel::OnHideInvert() 
{
	for (int i = 0; i < m_building->GetNumSectors(); i++)
		m_building->HideSector(i,m_building->IsSectorHidden(i));
	RefreshList();
}

void CBuildingPanel::OnBind() 
{
	if (m_picking)
	{
		GetIEditor()->SetEditTool(0);
		m_picking = false;
		return;
	}

	int sel = m_helpers.GetNextItem(-1,LVNI_SELECTED);
	if (sel < 0)
		return;

	m_currHelper = sel;

	// Bind picked object to helper.
	GetIEditor()->PickObject( this,0,"Pick object to bind" );
	m_bindButton.SetCheck( BST_CHECKED );
	m_picking = true;
}

void CBuildingPanel::OnUnbind() 
{
	int sel = m_helpers.GetNextItem(-1,LVNI_SELECTED);
	if (sel < 0)
		return;

	if (m_picking)
		return;

	// Unbind picked object from helper.
	std::vector<CBuilding::ObjectHelper> &helpers = m_building->GetHelpers();
	if (sel < helpers.size())
	{
		m_building->UnbindHelper( sel );
		RefreshList();
	}
}


//////////////////////////////////////////////////////////////////////////
void CBuildingPanel::OnPick( CBaseObject *picked )
{
	m_bindButton.SetCheck( BST_UNCHECKED );
	m_picking = false;

	assert( m_building != 0 );

	if (!picked)
		return;

	std::vector<CBuilding::ObjectHelper> &helpers = m_building->GetHelpers();
	if (m_currHelper >= 0 && m_currHelper < helpers.size())
	{
		m_building->BindToHelper( m_currHelper,picked );
		RefreshList();
	}
}

void CBuildingPanel::OnCancelPick()
{
	m_picking = false;
	m_bindButton.SetCheck( BST_UNCHECKED );
	m_currHelper = -1;
}

void CBuildingPanel::OnWireframe() 
{
	if (m_wireframe.GetCheck() == BST_CHECKED)
		m_building->SetWireframe(true);
	else
		m_building->SetWireframe(false);
}

void CBuildingPanel::OnBnClickedPortals()
{
	if (m_portals.GetCheck() == BST_CHECKED)
		m_building->SetPortals(true);
	else
		m_building->SetPortals(false);
}
