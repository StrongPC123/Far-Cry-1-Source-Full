////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2004.
// -------------------------------------------------------------------------
//  File name:   ApplicationHelper.cpp
//  Version:     v1.00
//  Created:     02/12/2004 by MartinM
//  Compilers:   Visual Studio.NET
//  Description: Helper class for simple application creation
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ApplicationHelper.h"					// CApplicationHelper


CApplicationHelper::CApplicationHelper()
{
}

CApplicationHelper::~CApplicationHelper()
{
}



void CApplicationHelper::ParseArguments( const char *inszCommandLine, ICmdlineArgumentSink *pEarlyCommands, ICmdlineArgumentSink *pConsoleCommands )
{
	assert(inszCommandLine);

//	int iArgNo=0;
	char *src=(char *)inszCommandLine;
	char Arg[1024];

	while(*src)
	{
		char *dst=Arg;

		while(*src<=' ' && *src!=0)
			src++;		// jump over whitespace

		if(*src=='\"')
		{
			src++;

			while(*src!='\"' && *src!=0)
				*dst++=*src++;

			if(*src=='\"')
				src++;
		}
		else
		{
			while(*src!=' ' && *src!=0)
				*dst++=*src++;
		}

		*dst=0;

		if(*Arg!=0)
		{
//			if(iArgNo!=0)		// ignore .exe name
			{
				if(Arg[0]=='-')
				{
					if(pEarlyCommands)
						pEarlyCommands->ReturnArgument(&Arg[1]);
				}
				else
				{
					if(pConsoleCommands)
						pConsoleCommands->ReturnArgument(Arg);
				}
			}

//			iArgNo++;
		}
	}
}
