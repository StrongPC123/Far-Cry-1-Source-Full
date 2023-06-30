////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   commandmanager.h
//  Version:     v1.00
//  Created:     4/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __commandmanager_h__
#define __commandmanager_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "Command.h"

/** Implement this interface and register it to command manager to intercept commands
		before they executed.
*/
class	ICommandHandler
{
public:
	/** Called before this command is executed.
			@return Return false to stop command from executing.
	*/
	virtual	bool OnCommand( CCommand *command ) = 0;
};

/** Command manager stores all commands in the Editor.
*/
class	CCommandManager
{
public:
	typedef Functor0 CommandCallback;

	CCommandManager();	// Ctor.
	~CCommandManager();

	//! Execute command by name.
	void Execute( const char *command );

	//! Execute command by id.
	void ExecuteId( int commandId );

	void GetSortedCmdList( std::vector<CString> &cmds );
	CString	AutoComplete( const char* substr );
	CString	AutoCompletePrev( const char* substr );

	void	RegisterCommand( CCommand *cmd );
	void	UnregisterCommand( CCommand *cmd );
	void	AddCommandHandler( ICommandHandler *handler );
	void	RemoveCommandHandler( ICommandHandler *handler );

	//! Regster callback command.
	void RegisterCommand( const CString &sCommand,CommandCallback callback );
	//! Unregister command by name.
	void UnregisterCommand( const CString &sCommand );

private:
	CCommand*	FindCommand( const CString &sCommand ) const;

	//typedef string_hash_multimap<CommandOp*> Commands;
	typedef std::map<CString,CCommandPtr> Commands;
	Commands m_commands;
	
	typedef	std::vector<ICommandHandler*>	CmdHandlers;
	CmdHandlers	m_cmdHandlers;
};

#endif // __commandmanager_h__
