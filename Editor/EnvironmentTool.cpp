////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   EnvironmentTool.cpp
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Places Environment on terrain.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EnvironmentTool.h"
#include "EnvironmentPanel.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CEnvironmentTool,CEditTool)

//////////////////////////////////////////////////////////////////////////
CEnvironmentTool::CEnvironmentTool()
{
	SetStatusText( _T("Click Apply to accept changes") );
	m_panelId = 0;
	m_panel = 0;
}

//////////////////////////////////////////////////////////////////////////
CEnvironmentTool::~CEnvironmentTool()
{
}

//////////////////////////////////////////////////////////////////////////
void CEnvironmentTool::BeginEditParams( IEditor *ie,int flags )
{
	m_ie = ie;
	if (!m_panelId)
	{
		m_panel = new CEnvironmentPanel(AfxGetMainWnd());
		m_panelId = m_ie->AddRollUpPage( ROLLUP_TERRAIN,"Environment",m_panel );
		AfxGetMainWnd()->SetFocus();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEnvironmentTool::EndEditParams()
{
	if (m_panelId)
	{
		m_ie->RemoveRollUpPage(ROLLUP_TERRAIN,m_panelId);
		m_panel = 0;
	}
}
