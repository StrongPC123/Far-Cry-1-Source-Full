////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushpanel.cpp
//  Version:     v1.00
//  Created:     2/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BrushPanel.h"

#include "..\Objects\ObjectManager.h"
#include "..\Objects\BrushObject.h"

// CBrushPanel dialog

IMPLEMENT_DYNAMIC(CBrushPanel, CXTResizeDialog)
CBrushPanel::CBrushPanel(CWnd* pParent /*=NULL*/)
	: CXTResizeDialog(CBrushPanel::IDD, pParent)
{
	m_brushObj = 0;
	Create( IDD,pParent );
}

//////////////////////////////////////////////////////////////////////////
CBrushPanel::~CBrushPanel()
{
}

//////////////////////////////////////////////////////////////////////////
void CBrushPanel::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_SIDES, m_sides);
	DDX_Control(pDX, IDC_RESETSIZE, m_resetSizeBtn);
	DDX_Control(pDX, IDC_REFRESH, m_reloadBtn);
}

//////////////////////////////////////////////////////////////////////////
void CBrushPanel::SetBrush( CBrushObject *obj )
{
	m_brushObj = obj;
}

BEGIN_MESSAGE_MAP(CBrushPanel, CXTResizeDialog)
	//ON_CBN_SELENDOK(IDC_SIDES, OnCbnSelendokSides)
	ON_BN_CLICKED(IDC_RESETSIZE, OnBnClickedResetsize)
	ON_BN_CLICKED(IDC_REFRESH, OnBnClickedReload)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
BOOL CBrushPanel::OnInitDialog()
{
	CXTResizeDialog::OnInitDialog();

	/*
	CString str;
	for (int i = 0; i < 10; i++)
	{
		str.Format( "%d",i );
		m_sides.AddString( str );
	}
	*/

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CBrushPanel::OnCbnSelendokSides()
{
	
}

//////////////////////////////////////////////////////////////////////////
void CBrushPanel::OnBnClickedResetsize()
{
	if (m_brushObj)
	{
		m_brushObj->ResetToPrefabSize();
	}
	else
	{
		// Reset all selected brushes.
		CSelectionGroup *selection = GetIEditor()->GetSelection();
		for (int i = 0; i < selection->GetCount(); i++)
		{
			CBaseObject *pBaseObj = selection->GetObject(i);
			if (pBaseObj->IsKindOf(RUNTIME_CLASS(CBrushObject)))
				((CBrushObject*)pBaseObj)->ResetToPrefabSize();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushPanel::OnBnClickedReload()
{
	if (m_brushObj)
	{
		m_brushObj->ReloadPrefabGeometry();
	}
	else
	{
		// Reset all selected brushes.
		CSelectionGroup *selection = GetIEditor()->GetSelection();
		for (int i = 0; i < selection->GetCount(); i++)
		{
			CBaseObject *pBaseObj = selection->GetObject(i);
			if (pBaseObj->IsKindOf(RUNTIME_CLASS(CBrushObject)))
				((CBrushObject*)pBaseObj)->ReloadPrefabGeometry();
		}
	}
}