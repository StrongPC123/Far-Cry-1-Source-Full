// EntityPanel.cpp : implementation file
//

#include "stdafx.h"
#include "EntityPanel.h"
#include "Objects\Entity.h"
#include "Objects\ObjectManager.h"
#include "StringDlg.h"

#include "CryEditDoc.h"
#include "Mission.h"
#include "MissionScript.h"
#include "EntityPrototype.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEntityPanel dialog


CEntityPanel::CEntityPanel(CWnd* pParent /*=NULL*/)
	: CXTResizeDialog(CEntityPanel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEntityPanel)
	m_selectedMethod = _T("");
	//}}AFX_DATA_INIT

	m_entity = 0;

	m_grayBrush.CreateSolidBrush( RGB(0xE0,0xE0,0xE0) );
	m_pickTool = 0;
}


void CEntityPanel::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEntityPanel)
	DDX_Control(pDX, IDC_EVENT_SEND, m_sendEvent);
	DDX_Control(pDX, IDC_RUN_METHOD, m_runButton);
	DDX_Control(pDX, IDC_GOTO_METHOD, m_gotoMethodBtn);
	DDX_Control(pDX, IDC_EDITSCRIPT, m_editScriptBtn);
	DDX_Control(pDX, IDC_ADD_METHOD, m_addMethodBtn);
	DDX_Control(pDX, IDC_RELOADSCRIPT, m_reloadScriptBtn);
	DDX_Control(pDX, IDC_EVENT_REMOVE, m_removeButton);
	DDX_Control(pDX, IDC_EVENTTREE, m_eventTree);
	DDX_Control(pDX, IDC_EVENT_ADD, m_pickButton);
	DDX_Control(pDX, IDC_PROTOTYPE, m_prototypeButton);
	DDX_Control(pDX, IDC_EVENT_ADDMISSION, m_addMissionBtn);
	DDX_Control(pDX, IDC_METHODS, m_methods);
	DDX_LBString(pDX, IDC_METHODS, m_selectedMethod);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEntityPanel, CXTResizeDialog)
	//{{AFX_MSG_MAP(CEntityPanel)
	ON_BN_CLICKED(IDC_EDITSCRIPT, OnEditScript)
	ON_BN_CLICKED(IDC_RELOADSCRIPT, OnReloadScript)
	ON_WM_CTLCOLOR()
	ON_LBN_DBLCLK(IDC_METHODS, OnDblclkMethods)
	ON_BN_CLICKED(IDC_GOTO_METHOD, OnGotoMethod)
	ON_BN_CLICKED(IDC_ADD_METHOD, OnAddMethod)
	//ON_BN_CLICKED(IDC_EVENT_ADD, OnEventAdd)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_EVENT_REMOVE, OnEventRemove)
	ON_NOTIFY(TVN_SELCHANGED, IDC_EVENTTREE, OnSelChangedEventTree)
	ON_NOTIFY(NM_RCLICK, IDC_EVENTTREE, OnRclickEventTree)
	ON_NOTIFY(NM_DBLCLK, IDC_EVENTTREE, OnDblClickEventTree)
	ON_BN_CLICKED(IDC_RUN_METHOD, OnRunMethod)
	ON_BN_CLICKED(IDC_EVENT_SEND, OnEventSend)
	ON_BN_CLICKED(IDC_EVENT_ADDMISSION, OnBnAddMission)
	ON_BN_CLICKED(IDC_PROTOTYPE, OnPrototype)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_GETPHYSICS, OnBnClickedGetphysics)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEntityPanel message handlers
void CEntityPanel::SetEntity( CEntity *entity )
{
	assert( entity );

	m_entity = entity;
	ReloadMethods();
	ReloadEvents();

	if (!entity->GetPrototype())
	{
		m_prototypeButton.EnableWindow( FALSE );
		m_prototypeButton.SetWindowText( _T("Entity Archetype") );
	}
	else
	{
		m_prototypeButton.EnableWindow( TRUE );
		m_prototypeButton.SetWindowText( entity->GetPrototype()->GetName() );
	}
}


void CEntityPanel::OnEditScript() 
{
	assert( m_entity != 0 );
	CEntityScript *script = m_entity->GetScript();
	CFileUtil::EditTextFile( script->GetFile() );
}

void CEntityPanel::ReloadMethods()
{
	assert( m_entity != 0 );

	// Parse entity lua file.
	CEntityScript *script = m_entity->GetScript();

	m_methods.ResetContent();
	///if (script->Load( m_entity->GetEntityClass() ))
	{
		for (int i = 0; i < script->GetMethodCount(); i++)
		{
			m_methods.AddString( script->GetMethod(i) );
		}
	}
}

void CEntityPanel::OnReloadScript() 
{
	assert( m_entity != 0 );

	CEntityScript *script = m_entity->GetScript();
	script->Reload();
	m_entity->Reload(true);
	ReloadMethods();
}

HBRUSH CEntityPanel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CXTResizeDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
   // CWnd::GetDlgCtrlID() to perform the most efficient test.
   if (pWnd->GetDlgCtrlID() == IDC_METHODS)
   {
      // Set the text color to red
      //pDC->SetTextColor(RGB(255, 0, 0));

      // Set the background mode for text to transparent 
      // so background will show thru.
      pDC->SetBkMode(TRANSPARENT);

      // Return handle to our CBrush object
      //hbr = (HBRUSH)GetStockObject( LTGRAY_BRUSH );
			hbr = m_grayBrush;
   }

	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CEntityPanel::OnDblclkMethods() 
{
	UpdateData(TRUE);
	GotoMethod( m_selectedMethod );
}

void CEntityPanel::OnRunMethod() 
{
	assert( m_entity != 0 );

	UpdateData(TRUE);
	CEntityScript *script = m_entity->GetScript();
	if (m_entity->GetIEntity())
		script->RunMethod( m_entity->GetIEntity(),m_selectedMethod );
}

void CEntityPanel::GotoMethod( const CString &method )
{
	assert( m_entity != 0 );
	CEntityScript *script = m_entity->GetScript();
	script->GotoMethod( method );
}

void CEntityPanel::OnGotoMethod() 
{
	UpdateData(TRUE);
	GotoMethod( m_selectedMethod );
}

void CEntityPanel::OnAddMethod() 
{
	assert( m_entity != 0 );
	CStringDlg dlg( "Enter Method Name" );
	if (dlg.DoModal() == IDOK)
	{
		CString method = dlg.m_strString;
		if (m_methods.FindString(-1,method) == LB_ERR)
		{
			CEntityScript *script = m_entity->GetScript();
			script->AddMethod( method );
			script->GotoMethod( method );
			OnReloadScript();
		}
	}
}

BOOL CEntityPanel::OnInitDialog() 
{
	CXTResizeDialog::OnInitDialog();
	
	m_treeImageList.Create(IDB_ENTITY_EVENTS_TREE, 12, 1, RGB (255,255,255));
	m_eventTree.SetImageList(&m_treeImageList, TVSIL_NORMAL);
	m_eventTree.SetIndent( 0 );
	m_eventTree.SetBkColor( RGB(0xE0,0xE0,0xE0) );
	//m_eventTree.ModifyStyle( 0,TVS_NOSCROLL );

	//m_pickButton.SetPushedBkColor( RGB(255,255,0) );
	m_pickButton.SetPickCallback( this,_T("Pick Target Entity for Event"),RUNTIME_CLASS(CEntity) );

	SetResize( IDC_PROTOTYPE,SZ_HORRESIZE(1) );
	SetResize( IDC_EVENTTREE,SZ_HORRESIZE(1) );
	SetResize( IDC_METHODS,SZ_HORRESIZE(1) );
	SetResize( IDC_METHODS_FRAME,SZ_RESIZE(1) );
	SetResize( IDC_RUN_METHOD,SZ_REPOS(1) );
	SetResize( IDC_GOTO_METHOD,SZ_REPOS(1) );
	SetResize( IDC_ADD_METHOD,SZ_REPOS(1) );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEntityPanel::ReloadEvents()
{
	assert( m_entity != 0 );
	CEntityScript *script = m_entity->GetScript();

	int i;

	m_sourceEventsMap.Clear();
	m_targetEventsMap.Clear();

	// Reload events tree.
	m_eventTree.DeleteAllItems();
	for (i = 0; i < script->GetEventCount(); i++)
	{
		CString sourceEvent = script->GetEvent(i);
		HTREEITEM hRootItem = m_eventTree.InsertItem( CString("On ")+sourceEvent, 0, 1, TVI_ROOT );
		m_sourceEventsMap.Insert( hRootItem,sourceEvent );
		
		bool haveEvents = false;
		for (int j = 0; j < m_entity->GetEventTargetCount(); j++)
		{
			CString targetName;
			CEntityEventTarget &et = m_entity->GetEventTarget(j);
			if (stricmp(et.sourceEvent,sourceEvent) != 0)
				continue;

			if (et.target)
			{
				targetName = et.target->GetName();
			}
			else
				targetName = "Mission";

			targetName += CString(" [") + et.event + "]";
			HTREEITEM hEventItem = m_eventTree.InsertItem( targetName, 2, 2, hRootItem );
			m_targetEventsMap.Insert( hEventItem,j );

			haveEvents = true;
		}
		if (haveEvents)
		{
			m_eventTree.Expand( hRootItem,TVE_EXPAND );
			m_eventTree.SetItemState(hRootItem, TVIS_BOLD, TVIS_BOLD);
		}
	}
	m_pickButton.EnableWindow(FALSE);
	m_removeButton.EnableWindow(FALSE);
	m_sendEvent.EnableWindow(FALSE);
	m_addMissionBtn.EnableWindow(FALSE);
	m_currentTrgEventId = -1;
	m_currentSourceEvent = "";
}

void CEntityPanel::OnEventAdd() 
{
	/*
	assert( m_entity != 0 );
	//m_entity->PickEntity();	
	
	if (m_pickTool)
	{
		// If pick tool already enabled, disable it.
		OnCancelPick();
	}

	GetIEditor()->PickObject( this,RUNTIME_CLASS(CEntity),"Pick Target Entity for Event" );
	m_pickButton.SetCheck( BST_CHECKED );
	*/
}

//////////////////////////////////////////////////////////////////////////
void CEntityPanel::OnEventRemove() 
{
	if (m_currentTrgEventId >= 0)
	{
		{
			CUndo undo("Remove Event Target");
			m_entity->RemoveEventTarget( m_currentTrgEventId );
		}
		ReloadEvents();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityPanel::OnBnAddMission()
{
	assert( m_entity );

	if (!m_currentSourceEvent.IsEmpty())
	{
		CMissionScript *script = GetIEditor()->GetDocument()->GetCurrentMission()->GetScript();
		if (!script)
			return;

		if (script->GetEventCount() < 1)
			return;

		// Popup Menu with Event selection.
		CMenu menu;
		menu.CreatePopupMenu();
		for (int i = 0; i < script->GetEventCount(); i++)
		{
			menu.AppendMenu( MF_STRING,i+1,script->GetEvent(i) );
		}

		CPoint p;
		::GetCursorPos(&p);
		//CRect rc;
		//m_addMissionBtn.GetWindowRect( rc );
		int res = ::TrackPopupMenuEx( menu.GetSafeHmenu(),TPM_LEFTBUTTON|TPM_RETURNCMD,p.x,p.y,GetSafeHwnd(),NULL );
		if (res > 0 && res < script->GetEventCount()+1)
		{
			CUndo undo( "Change Event" );

			CString event = script->GetEvent(res-1);
			m_entity->AddEventTarget( 0,event,m_currentSourceEvent );
			// Update script event table.
			if (m_entity->GetScript())
				m_entity->GetScript()->SetEventsTable( m_entity );
			ReloadEvents();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityPanel::OnPick( CBaseObject *picked )
{
	m_pickTool = 0;

	CEntity *pickedEntity = (CEntity*)picked;
	if (!pickedEntity)
		return;

	m_pickButton.SetCheck( BST_UNCHECKED );
	if (pickedEntity->GetScript()->GetEventCount() > 0)
	{
		if (!m_currentSourceEvent.IsEmpty())
		{
			CUndo undo( "Add Event Target" );
			m_entity->AddEventTarget( pickedEntity,pickedEntity->GetScript()->GetEvent(0),m_currentSourceEvent );
			if (m_entity->GetScript())
				m_entity->GetScript()->SetEventsTable( m_entity );
			ReloadEvents();
		}
	}
}

void CEntityPanel::OnCancelPick()
{
	m_pickButton.SetCheck( BST_UNCHECKED );
	m_pickTool = 0;
}

void CEntityPanel::OnDestroy() 
{
	if (m_pickTool == GetIEditor()->GetEditTool())
		GetIEditor()->SetEditTool( 0 );
	m_pickTool = 0;

	CXTResizeDialog::OnDestroy();
}

void CEntityPanel::OnSelChangedEventTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	assert( m_entity != 0 );
	
	HTREEITEM selectedItem = pNMTreeView->itemNew.hItem;
	CString str;
	if (m_sourceEventsMap.Find(selectedItem,str))
	{
		m_currentSourceEvent = str;
		m_currentTrgEventId = -1;
		m_pickButton.EnableWindow(TRUE);
		m_removeButton.EnableWindow(FALSE);
		m_sendEvent.EnableWindow(TRUE);
		m_addMissionBtn.EnableWindow(TRUE);
	}

	int id;
	if (m_targetEventsMap.Find(selectedItem,id))
	{
		m_currentSourceEvent = m_entity->GetEventTarget(id).sourceEvent;
		m_currentTrgEventId = id;
		m_pickButton.EnableWindow(FALSE);
		m_removeButton.EnableWindow(TRUE);
		m_sendEvent.EnableWindow(TRUE);
		m_addMissionBtn.EnableWindow(FALSE);
	}
	*pResult = 0;
}

void CEntityPanel::OnRclickEventTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	if (m_currentTrgEventId >= 0)
	{
		CEntityScript *script = 0;
		CMissionScript *missionScript = 0;
		int eventCount = 0;

		// Popup Menu with Event selection.
		CMenu menu;
		menu.CreatePopupMenu();

		CBaseObject *trgObject = m_entity->GetEventTarget(m_currentTrgEventId).target;
		if (trgObject != 0)
		{
			CEntity* targetEntity = (CEntity*)trgObject;
			if (!targetEntity)
				return;

			script = targetEntity->GetScript();
			if (!script)
				return;

			eventCount = script->GetEventCount();

			for (int i = 0; i < eventCount; i++)
			{
				menu.AppendMenu( MF_STRING,i+1,script->GetEvent(i) );
			}
		}
		else
		{
			missionScript = GetIEditor()->GetDocument()->GetCurrentMission()->GetScript();
			if (!missionScript)
				return;
			eventCount = missionScript->GetEventCount();
			for (int i = 0; i < eventCount; i++)
			{
				menu.AppendMenu( MF_STRING,i+1,missionScript->GetEvent(i) );
			}
		}
		CPoint p;
		::GetCursorPos( &p );
		int res = ::TrackPopupMenuEx( menu.GetSafeHmenu(),TPM_LEFTBUTTON|TPM_RETURNCMD,p.x,p.y,GetSafeHwnd(),NULL );
		if (res > 0 && res < eventCount+1)
		{
			CUndo undo( "Change Event" );
			CString event;
			if (script)
				event = script->GetEvent(res-1);
			else if (missionScript)
				event = missionScript->GetEvent(res-1);

			m_entity->GetEventTarget(m_currentTrgEventId).event = event;
			// Update script event table.
			if (m_entity->GetScript())
				m_entity->GetScript()->SetEventsTable( m_entity );
			ReloadEvents();
		}
	}

	
	*pResult = TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CEntityPanel::OnDblClickEventTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	if (m_currentTrgEventId >= 0)
	{
		CBaseObject *trgObject = m_entity->GetEventTarget(m_currentTrgEventId).target;
		if (trgObject != 0)
		{
			CUndo undo( "Select Object" );
			GetIEditor()->ClearSelection();
			GetIEditor()->SelectObject( trgObject );
		}
	}

	*pResult = TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CEntityPanel::OnEventSend() 
{
	if (!m_currentSourceEvent.IsEmpty())
	{
		CEntityScript *script = m_entity->GetScript();
		if (m_entity->GetIEntity())
			script->SendEvent( m_entity->GetIEntity(),m_currentSourceEvent );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityPanel::OnPrototype()
{
	// Go to the entity prototype.
	// Open corresponding prototype.
	if (m_entity)
	{
		if (m_entity->GetPrototype())
		{
			GetIEditor()->OpenDataBaseLibrary( EDB_ARCHETYPE_LIBRARY,m_entity->GetPrototype() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityPanel::OnBnClickedGetphysics()
{
	if (m_entity)
	{
		CUndo undo( "Accept Physics State" );
		m_entity->AcceptPhysicsState();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityPanel::OnBnClickedResetphysics()
{
	if (m_entity)
	{
		CUndo undo( "Reset Physics State" );
		m_entity->ResetPhysicsState();
	}
}
