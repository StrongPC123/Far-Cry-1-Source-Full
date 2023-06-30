// AIPointPanel.cpp : implementation file
//

#include "stdafx.h"
#include "AIPointPanel.h"

#include "Objects\AIPoint.h"

// CAIPointPanel dialog

IMPLEMENT_DYNCREATE(CAIPointPanel, CDialog)

CAIPointPanel::CAIPointPanel(CWnd* pParent /*=NULL*/)
	: CDialog(CAIPointPanel::IDD, pParent)
	, m_type(0)
{
	m_object = 0;
	Create( IDD,pParent );
}

CAIPointPanel::~CAIPointPanel()
{
}

void CAIPointPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NODES, m_links);
	DDX_Control(pDX, IDC_PICK, m_pickBtn);
	DDX_Control(pDX, IDC_SELECT, m_selectBtn);
	DDX_Control(pDX, IDC_REMOVE, m_removeBtn);
	
	DDX_Radio(pDX, IDC_WAYPOINT, m_type);
}

BOOL CAIPointPanel::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_links.SetBkColor( RGB(0xE0,0xE0,0xE0) );
	
	m_pickBtn.SetPickCallback( this,"Pick AIPoint to Link",0 );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CAIPointPanel, CDialog)
	ON_BN_CLICKED(IDC_SELECT, OnBnClickedSelect)
	ON_BN_CLICKED(IDC_REMOVE, OnBnClickedRemove)
	ON_LBN_DBLCLK(IDC_NODES, OnLbnDblclkLinks)
	ON_LBN_SELCHANGE(IDC_NODES, OnLbnLinksSelChange)
	ON_BN_CLICKED(IDC_WAYPOINT, OnBnClickedWaypoint)
	ON_BN_CLICKED(IDC_HIDEPOINT, OnBnClickedHidepoint)
	ON_BN_CLICKED(IDC_ENTRYPOINT, OnBnClickedEntrypoint)
	ON_BN_CLICKED(IDC_EXITPOINT, OnBnClickedExitpoint)
END_MESSAGE_MAP()


void CAIPointPanel::SetObject( CAIPoint *object )
{
	if (m_object)
	{
		for (int i = 0; i < m_object->GetLinkCount(); i++)
			m_object->SelectLink(i,false);
	}

	assert( object );
	m_object = object;
	switch (object->GetAIType())
	{
	case EAIPOINT_WAYPOINT:
		m_type = 0;
		break;
	case EAIPOINT_HIDE:
		m_type = 1;
		break;
	case EAIPOINT_ENTRY:
		m_type = 2;
		break;
	case EAIPOINT_EXIT:
		m_type = 3;
		break;
	}
	UpdateData(FALSE);
	ReloadLinks();
}

//////////////////////////////////////////////////////////////////////////
void CAIPointPanel::StartPick()
{
	// Simulate click on pick button.
	m_pickBtn.OnClicked();
}

void CAIPointPanel::ReloadLinks()
{
	m_links.ResetContent();
	for (int i = 0; i < m_object->GetLinkCount(); i++)
	{
		CAIPoint *obj = m_object->GetLink(i);
		if (obj)
			m_links.AddString( obj->GetName() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAIPointPanel::OnBnClickedSelect()
{
	assert( m_object );
	int sel = m_links.GetCurSel();
	if (sel != LB_ERR)
	{
		CBaseObject *obj = m_object->GetLink(sel);
		if (obj)
		{
			GetIEditor()->ClearSelection();
			GetIEditor()->SelectObject( obj );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAIPointPanel::OnBnClickedRemove()
{
	assert( m_object );
	int sel = m_links.GetCurSel();
	if (sel != LB_ERR)
	{
		CUndo undo( "Unlink AIPoint" );
		CAIPoint *obj = m_object->GetLink(sel);
		if (obj)
			m_object->RemoveLink( obj );
		ReloadLinks();
	}
}

//////////////////////////////////////////////////////////////////////////
void CAIPointPanel::OnLbnDblclkLinks()
{
	// Select current entity.
	OnBnClickedSelect();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CAIPointPanel::OnPick( CBaseObject *picked )
{
	assert( m_object );
	CUndo undo( "Link AIPoint" );
	m_object->AddLink( (CAIPoint*)picked );
	ReloadLinks();

	// 
//	m_entityName.SetWindowText( picked->GetName() );
}

//////////////////////////////////////////////////////////////////////////
bool CAIPointPanel::OnPickFilter( CBaseObject *picked )
{
	assert( picked != 0 );
	return picked != m_object && picked->IsKindOf( RUNTIME_CLASS(CAIPoint) );
}

//////////////////////////////////////////////////////////////////////////
void CAIPointPanel::OnCancelPick()
{
}

//////////////////////////////////////////////////////////////////////////
void CAIPointPanel::OnBnClickedWaypoint()
{
	assert( m_object );
	m_object->SetAIType( EAIPOINT_WAYPOINT );
}

//////////////////////////////////////////////////////////////////////////
void CAIPointPanel::OnBnClickedHidepoint()
{
	assert( m_object );
	m_object->SetAIType( EAIPOINT_HIDE );
}

//////////////////////////////////////////////////////////////////////////
void CAIPointPanel::OnBnClickedEntrypoint()
{
	assert( m_object );
	m_object->SetAIType( EAIPOINT_ENTRY );
}

//////////////////////////////////////////////////////////////////////////
void CAIPointPanel::OnBnClickedExitpoint()
{
	assert( m_object );
	m_object->SetAIType( EAIPOINT_EXIT );
}

//////////////////////////////////////////////////////////////////////////
void CAIPointPanel::OnLbnLinksSelChange()
{
	assert( m_object );
	int sel = m_links.GetCurSel();
	if (sel != LB_ERR)
	{
		// Unselect all others.
		for (int i = 0; i < m_object->GetLinkCount(); i++)
		{
			if (sel == i)
				m_object->SelectLink(i,true);
			else
				m_object->SelectLink(i,false);
		}
	}
}