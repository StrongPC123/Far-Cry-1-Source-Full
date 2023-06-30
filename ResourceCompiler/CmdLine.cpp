////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cmdline.cpp
//  Version:     v1.00
//  Created:     5/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "CmdLine.h"
#include "Config.h"

//////////////////////////////////////////////////////////////////////////
CmdLine::CmdLine()
{
	m_bHelp = false;
	m_platform = PLATFORM_UNKNOWN;
	m_config = 0;
}

//////////////////////////////////////////////////////////////////////////
void CmdLine::Parse( int argc, char **argv,Config *config )
{
	m_config = config;
	for (int i = 1; i < argc; i++)
	{
		const char *param = argv[i];
		bool bFlag = false;
		bool bLast = ((i + 1) == argc);
		if (param[0] == '-' || param[0] == '/')
		{
			// remove flag specifier
			bFlag = true;
			++param;
		}
		ParseParam(param,bFlag,bLast);
	}
}

//////////////////////////////////////////////////////////////////////////
void CmdLine::ParseParam( const char* param, bool bFlag,bool bLast )
{
	if (!bFlag)
	{
		if (m_fileSpec.IsEmpty())
			m_fileSpec = param;
	}
	else
	{
		// Flag.
		// Split on key/value pair.
		if (stricmp("h",param) == 0)
			m_bHelp = true;
		else
		{
			CString prm = param;
			int splitter = prm.Find('=');
			if (splitter >= 0)
			{
				CString key = prm.Mid(0,splitter);
				CString value = prm.Mid(splitter+1);
				// Put key/value pair to config.
				if (!key.IsEmpty() && m_config)
					m_config->Set( key.GetString(),value.GetString() );
			}
			else
			{
				m_config->Set (prm.GetString(), "");
			}
		}
	}
}
