////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   externaltools.h
//  Version:     v1.00
//  Created:     27/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __externaltools_h__
#define __externaltools_h__
#pragma once

/** Represents external tool, added to main menu.
*/
class CExternalTool
{
public:
	CString m_title;	// Command title.
	CString m_command; // Console command.
	bool m_variableToggle;
};

/** Manages user defined external tools.
*/
class CExternalToolsManager
{
public:
	CExternalToolsManager();
	~CExternalToolsManager();

	// Save tools configuration to registry.
	void Save();

	// Load tools configuration from registry.
	void Load();

	//! Get number of managed tools.
	int GetToolsCount() const;
	//! Get tool by index.
	CExternalTool* GetTool( int iIndex ) const;
	//! Adds new tool to manager.
	void AddTool( CExternalTool *tool );
	//! Deletes tool from manager.
	void DeleteTool( CExternalTool* tool );
	//! Delete all tools.
	void ClearTools();

	//! Execute tool with specified id.
	void ExecuteTool( int iIndex );

private:
	std::vector<CExternalTool*> m_tools;
};

#endif // __externaltools_h__
