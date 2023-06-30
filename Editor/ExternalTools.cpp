////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   externaltools.cpp
//  Version:     v1.00
//  Created:     27/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ExternalTools.h"

//////////////////////////////////////////////////////////////////////////
CExternalToolsManager::CExternalToolsManager()
{
	// Load from registry.
	Load();
}

//////////////////////////////////////////////////////////////////////////
CExternalToolsManager::~CExternalToolsManager()
{
	// Save to registry.
	Save();

	// delete tools.
	for (int i = 0; i < m_tools.size(); i++)
	{
		delete m_tools[i];
	}
	m_tools.clear();
}

//////////////////////////////////////////////////////////////////////////
int CExternalToolsManager::GetToolsCount() const
{
	return m_tools.size();
}

//////////////////////////////////////////////////////////////////////////
CExternalTool* CExternalToolsManager::GetTool( int iIndex ) const
{
	return m_tools[iIndex];
}

//////////////////////////////////////////////////////////////////////////
void CExternalToolsManager::AddTool( CExternalTool *tool )
{
	m_tools.push_back(tool);
}

//////////////////////////////////////////////////////////////////////////
void CExternalToolsManager::DeleteTool( CExternalTool* tool )
{
	for (int i = 0; i < m_tools.size(); i++)
	{
		if (m_tools[i] == tool)
		{
			m_tools.erase( m_tools.begin() + i );
			delete tool;
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CExternalToolsManager::Load()
{
	CWinApp *pApp = AfxGetApp();
	int i = 0;
	bool finished = false;
	do
	{
		CString tool;
		tool.Format( "Tool%d",i );

		CString title = pApp->GetProfileString( "Tools",tool+"Title" );
		if (title.IsEmpty())
			break;

		CExternalTool *pTool = new CExternalTool;
		m_tools.push_back(pTool);
		pTool->m_title = title;
		pTool->m_command = pApp->GetProfileString( "Tools",tool+"Cmd" );
		pTool->m_variableToggle = pApp->GetProfileInt( "Tools",tool+"ToggleVar",0 ) != 0;
		i++;
	}
	while (!finished);
}

//////////////////////////////////////////////////////////////////////////
void CExternalToolsManager::Save()
{
	CWinApp *pApp = AfxGetApp();
	for (int i = 0; i < m_tools.size(); i++)
	{
		if (m_tools[i])
		{
			CString tool;
			tool.Format( "Tool%d",i );
			pApp->WriteProfileString( "Tools",tool+"Title",m_tools[i]->m_title );
			pApp->WriteProfileString( "Tools",tool+"Cmd",m_tools[i]->m_command );
			pApp->WriteProfileInt( "Tools",tool+"ToggleVar",m_tools[i]->m_variableToggle );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CExternalToolsManager::ClearTools()
{
	m_tools.clear();
}

//////////////////////////////////////////////////////////////////////////
void CExternalToolsManager::ExecuteTool( int iIndex )
{
	if (iIndex < 0 || iIndex >= m_tools.size())
		return;

	if (!m_tools[iIndex]->m_variableToggle)
	{
		GetIEditor()->GetSystem()->GetIConsole()->ExecuteString( m_tools[iIndex]->m_command );
	}
	else
	{
		// Toggle variable.
		float val = GetIEditor()->GetConsoleVar(m_tools[iIndex]->m_command);
		bool bOn = val != 0;
		GetIEditor()->SetConsoleVar( m_tools[iIndex]->m_command,(bOn)?0:1 );
	}
}