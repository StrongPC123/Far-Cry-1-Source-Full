////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   consoletrack.cpp
//  Version:     v1.00
//  Created:     12/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ConsoleTrack.h"

//////////////////////////////////////////////////////////////////////////
void CConsoleTrack::SerializeKey( IConsoleKey &key,XmlNodeRef &keyNode,bool bLoading )
{
	if (bLoading)
	{
		const char *str;
		str = keyNode->getAttr( "command" );
		strncpy( key.command,str,sizeof(key.command) );
		key.command[sizeof(key.command)-1] = 0;
	}
	else
	{
		if (strlen(key.command) > 0)
			keyNode->setAttr( "command",key.command );
	}
}

//////////////////////////////////////////////////////////////////////////
void CConsoleTrack::GetKeyInfo( int key,const char* &description,float &duration )
{
	assert( key >= 0 && key < (int)m_keys.size() );
	CheckValid();
	description = 0;
	duration = 0;
	if (strlen(m_keys[key].command) > 0)
		description = m_keys[key].command;
}
