////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   CommandManager.cpp
//  Version:     v1.00
//  Created:     4/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "CommandManager.h"

//////////////////////////////////////////////////////////////////////////
// Functor command.
//////////////////////////////////////////////////////////////////////////
class CFunctorCommand : public CCommand
{
public:
	explicit CFunctorCommand( const char *sName,Functor0 func ) : CCommand( sName,0 )
	{
		m_func = func;
	}
	virtual void Execute()
	{
		if (m_func)
			m_func();
	}
private:
	Functor0 m_func;
};

///////////////////////////////////////////////////////////////////////////////
//
// CCommandManager.
//
///////////////////////////////////////////////////////////////////////////////
CCommandManager::CCommandManager()
{
	// Make big enough hash table for faster searches.
//	m_commands.resize( 1024 );
}

CCommandManager::~CCommandManager()
{
}

//////////////////////////////////////////////////////////////////////////
void CCommandManager::RegisterCommand( const CString &sCommand,CommandCallback callback )
{
	assert( callback != 0 );
	if (FindCommand( sCommand ) != 0)
	{
		CString err;
		err.Format( "Error: Command %s already registered",(const char*)sCommand );
		CLogFile::WriteLine( err );
		AfxMessageBox( err );
	}
	CFunctorCommand *cmd = new CFunctorCommand(sCommand,callback);
	m_commands.insert( Commands::value_type( sCommand,cmd ) );
}

//////////////////////////////////////////////////////////////////////////
void CCommandManager::UnregisterCommand( const CString &sCommand )
{
	CCommand *pCmd = FindCommand( sCommand );
	if (pCmd)
		UnregisterCommand( pCmd );
}

//////////////////////////////////////////////////////////////////////////
void CCommandManager::RegisterCommand( CCommand *cmd )
{
	assert( cmd != 0 );
	if (FindCommand( cmd->GetName() ) != 0)
	{
		CString err;
		err.Format( "Error: Command %s already registered",cmd->GetName() );
		CLogFile::WriteLine( err );
		AfxMessageBox( err );
	}
	m_commands.insert( Commands::value_type( cmd->GetName(),cmd ) );
}

void CCommandManager::UnregisterCommand( CCommand *cmd )
{
	for (Commands::iterator it = m_commands.begin(); it != m_commands.end(); ++it)
	{
		if (it->second == cmd)
		{
			m_commands.erase( it );
			break;
		}
	}
	//error( "Cannot unregister command %s",cmd->name() );
}

CCommand*	CCommandManager::FindCommand( const CString &sCommand ) const
{
	Commands::const_iterator it = m_commands.find( sCommand );
	if (it != m_commands.end())
	{
		return it->second;
	}
	return 0;
}

void	CCommandManager::Execute( const char *command )
{
	CCommand *cmd = FindCommand( command );
	if (cmd)
	{
		for (CmdHandlers::iterator it = m_cmdHandlers.begin(); it != m_cmdHandlers.end(); ++it)
		{
			// if OnCommand handler return false ignore this commmand.
			if (!(*it)->OnCommand( cmd ))
				return;
		}
		cmd->Execute();
	}
	else
	{
		CString err;
		err.Format( "Error: Trying to execute unknown Command: %s",command );
		CLogFile::WriteLine( err );
	}
}

//////////////////////////////////////////////////////////////////////////
void	CCommandManager::ExecuteId( int commandId )
{
	CCommand *cmd = 0;
	for (Commands::iterator it = m_commands.begin(); it != m_commands.end(); it++)
	{
		CCommand *c = it->second;
		if (c->GetId() == commandId)
		{
			cmd = c;
			break;
		}
	}

	if (cmd)
	{
		for (CmdHandlers::iterator it = m_cmdHandlers.begin(); it != m_cmdHandlers.end(); ++it)
		{
			// if OnCommand handler return false ignore this commmand.
			if (!(*it)->OnCommand( cmd ))
				return;
		}
		cmd->Execute();
	}
	/*
	else
	{
		CString err;
		err.Format( "Error: Tring to execute unknown Command: %s",command );
		CLogFile::WriteLine( err );
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void	CCommandManager::AddCommandHandler( ICommandHandler *handler )
{
	m_cmdHandlers.push_back( handler );
}

void	CCommandManager::RemoveCommandHandler( ICommandHandler *handler )
{
	CmdHandlers::iterator it = std::find( m_cmdHandlers.begin(),m_cmdHandlers.end(),handler );
	if (it != m_cmdHandlers.end()) {
		m_cmdHandlers.erase( it );
	}
}

//////////////////////////////////////////////////////////////////////////
void CCommandManager::GetSortedCmdList( std::vector<CString> &cmds )
{
	cmds.clear();
	cmds.reserve( m_commands.size() );
	for (Commands::iterator it = m_commands.begin(); it != m_commands.end(); it++)
	{
		if (!cmds.empty())
		{
			// Ignore duplicate strings.
			if (stricmp(cmds.back(),it->first) == 0) continue;
		}
		cmds.push_back( it->first );
	}
	std::sort( cmds.begin(),cmds.end() );
}

//////////////////////////////////////////////////////////////////////////
CString CCommandManager::AutoComplete( const char* substr )
{
	std::vector<CString> cmds;
	GetSortedCmdList( cmds );

	int substrLen = strlen(substr);
	
	// If substring is empty return first command.
	if (substrLen==0 && cmds.size()>0)
		return cmds[0];

	for (int i = 0; i < cmds.size(); i++)
	{
		int cmdlen = strlen(cmds[i]);
		if (cmdlen >= substrLen && memicmp(cmds[i],substr,substrLen) == 0)
		{
			if (substrLen == cmdlen)
			{
				i++;
				if (i < cmds.size()) return cmds[i];
				return cmds[i-1];
			}
			return cmds[i];
		}
	}
	// Not found.
	return "";
}

//////////////////////////////////////////////////////////////////////////
CString CCommandManager::AutoCompletePrev( const char* substr )
{
	std::vector<CString> cmds;
	GetSortedCmdList( cmds );

	// If substring is empty return last command.
	if (strlen(substr)==0 && cmds.size()>0)
		return cmds[cmds.size()-1];

	for (int i = 0; i < cmds.size(); i++)
	{
		if (stricmp(substr,cmds[i])==0)
		{
			if (i > 0) 
				return cmds[i-1];
			else
				return cmds[0];
		}
	}
	return AutoComplete( substr );
}