////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   PickEntitiesPanel.cpp
//  Version:     v1.00
//  Created:     24/10/2002 by Lennert.
//  Compilers:   Visual C++.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PickEntitiesPanel.h"
#include "objects/BaseObject.h"
#include "objects/entity.h"
#include "Viewport.h"
#include "EditTool.h"

// CShapePanel dialog

IMPLEMENT_DYNAMIC(CPickEntitiesPanel, CDialog)

//////////////////////////////////////////////////////////////////////////
CPickEntitiesPanel::CPickEntitiesPanel( CWnd* pParent /* = NULL */)
	: CDialog(CPickEntitiesPanel::IDD, pParent)
{
}

CPickEntitiesPanel::~CPickEntitiesPanel()
{
}

void CPickEntitiesPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PICK, m_pickButton);
	DDX_Control(pDX, IDC_SELECT, m_selectButton);
	DDX_Control(pDX, IDC_ENTITIES, m_entities);
}


BEGIN_MESSAGE_MAP(CPickEntitiesPanel, CDialog)
	ON_BN_CLICKED(IDC_SELECT, OnBnClickedSelect)
	ON_BN_CLICKED(IDC_REMOVE, OnBnClickedRemove)
	ON_LBN_DBLCLK(IDC_ENTITIES, OnLbnDblclkEntities)
END_MESSAGE_MAP()


// CShapePanel message handlers

BOOL CPickEntitiesPanel::OnInitDialog()
{
	__super::OnInitDialog();

	m_pickButton.SetPickCallback( this,"Pick Entity" );
	m_entities.SetBkColor( RGB(0xE0,0xE0,0xE0) );
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPickEntitiesPanel::SetOwner(IPickEntitesOwner *pOwner)
{
	assert(pOwner);
	m_pOwner=pOwner;
	ReloadEntities();
}

//////////////////////////////////////////////////////////////////////////
void CPickEntitiesPanel::OnPick( CBaseObject *picked )
{
	assert( m_pOwner );
	CUndo undo("[PickEntityOwner] Add Entity");
	m_pOwner->AddEntity( (CEntity*)picked );
	ReloadEntities();
}

//////////////////////////////////////////////////////////////////////////
bool CPickEntitiesPanel::OnPickFilter( CBaseObject *picked )
{
	assert( picked != 0 );
	return picked->GetType() == OBJTYPE_ENTITY;
}

//////////////////////////////////////////////////////////////////////////
void CPickEntitiesPanel::OnCancelPick()
{
}

//////////////////////////////////////////////////////////////////////////
void CPickEntitiesPanel::OnBnClickedSelect()
{
	assert( m_pOwner );
	int sel = m_entities.GetCurSel();
	if (sel != LB_ERR)
	{
		CBaseObject *obj = m_pOwner->GetEntity(sel);
		if (obj)
		{
			CUndo undo( "Select Object" );
			GetIEditor()->ClearSelection();
			GetIEditor()->SelectObject( obj );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CPickEntitiesPanel::ReloadEntities()
{
	if (!m_pOwner)
		return;

	m_entities.ResetContent();
	for (int i = 0; i < m_pOwner->GetEntityCount(); i++)
	{
		CBaseObject *obj = m_pOwner->GetEntity(i);
		if (obj)
			m_entities.AddString( obj->GetName() );
		else
			m_entities.AddString( "<Null>" );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPickEntitiesPanel::OnBnClickedRemove()
{
	assert( m_pOwner );
	int sel = m_entities.GetCurSel();
	if (sel != LB_ERR)
	{
		CUndo undo("[PickEntityOwner] Remove Entity");
		if (sel < m_pOwner->GetEntityCount())
			m_pOwner->RemoveEntity(sel);
		ReloadEntities();
	}
}

void CPickEntitiesPanel::OnLbnDblclkEntities()
{
	// Select current entity.
	OnBnClickedSelect();
}
