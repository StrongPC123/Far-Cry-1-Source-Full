////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2004.
// -------------------------------------------------------------------------
//  File name:   ApplicationHelper.h
//  Version:     v1.00
//  Created:     02/12/2004 by MartinM
//  Compilers:   Visual Studio.NET
//  Description: Helper class for simple application creation
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once


class CApplicationHelper
{
public:
	//! destructor
	CApplicationHelper();
	//! constructor
	virtual ~CApplicationHelper();

	// ----------------------------------------------------------------------

	//! Sink for ParseArguments()
	struct ICmdlineArgumentSink
	{
		//! used for early command e.g. "-DEVMODE", "-IP:123.22.23.1", "-MOD:CS"
		//! or for console command e.g. "map mp_airstrip", "name test" 
		//! \param inszArgument must not be 0
		virtual void ReturnArgument( const char *inszArgument )=0;
	};

	// ----------------------------------------------------------------------

	//! to parse the command-line in a consistent way (you have to implement CApplicationHelper::ICommandlineArgumentSink)
	//! arguments bigger than 1024 chars are not handles correctly
	//! \param inszCommandLine must not be 0, e.g. -DEVMODE -IP:123.22.23.1 -MOD:CS "g_gametype ASSAULT" "start_server mp_airstrip"
	//! \param pEarlyCommands 0 or pointer to the sink for early commands e.g. "-DEVMODE", "-IP:123.22.23.1", "-MOD:CS"
	//! \param pConsoleCommands 0 or pointer to the sink for console commands e.g. "map mp_airstrip", "name test" 
	static void ParseArguments( const char *inszCommandLine, ICmdlineArgumentSink *pEarlyCommands, 
		ICmdlineArgumentSink *pConsoleCommands );

private: // ---------------------------------------------------------------
};

