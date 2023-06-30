////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   command.cpp
//  Version:     v1.00
//  Created:     4/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Command.h"

//////////////////////////////////////////////////////////////////////////
CCommand::CCommand( const CString &name,int Id,int flags )
{
	m_name = name;
	m_flags = flags;
	m_id = Id;
}