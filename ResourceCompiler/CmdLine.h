////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cmdline.h
//  Version:     v1.00
//  Created:     5/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __cmdline_h__
#define __cmdline_h__
#pragma once

class Config;

/** Command Line parser.
*/
class CmdLine
{
public:
	CmdLine();

	void Parse ( int argc, char **argv,Config *config );
	void ParseParam ( const char *param,bool bFlag,bool bLast );

	CString m_fileSpec;
	bool m_bHelp;
	Platform m_platform;
	Config* m_config;
};

#endif // __cmdline_h__
