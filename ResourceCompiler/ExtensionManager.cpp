////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   extensionmanager.cpp
//  Version:     v1.00
//  Created:     5/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include <time.h>
#include "ExtensionManager.h"
#include "IConvertor.h"
#include "IResCompiler.h"				// IResourceCompiler
#include "IRCLog.h"							// IRCLog

//////////////////////////////////////////////////////////////////////////
ExtensionManager::ExtensionManager()
{
}

//////////////////////////////////////////////////////////////////////////
ExtensionManager::~ExtensionManager()
{
	for (int i = 0; i < (int)m_convertors.size(); i++)
	{
		IConvertor *conv = m_convertors[i];
		conv->Release();
	}
}

//////////////////////////////////////////////////////////////////////////
IConvertor* ExtensionManager::FindConvertor( Platform platform,const char *ext ) const
{
	const ExtMap *extMap = &(m_extMap[platform]);


	ExtMap::const_iterator it;
	for (it=extMap->begin(); it!=extMap->end(); it++)
	{
		CString strExtName = (* it).first;
		assert((* it).second != NULL);
	}


	IConvertor *foundConvertor = 0;
	ExtMap::const_iterator low = extMap->find(ext);
	if (low != extMap->end())
	{
		IConvertor* conv = low->second;
		return conv;
		/*
		ExtMap::const_iterator up = extMap->upper_bound(ext);
		for (ExtMap::const_iterator it = low; it != up; ++it)
		{
			IConvertor* conv = *it;
			// Find convertor that matches platform.
			int numPlatforms = conv->GetNumPlatforms();
			for (int i = 0; i < numPlatforms; i++)
			{
				if (conv->GetPlatform(i) == platform)
				{
					// Matching Convertor found.
					return conv;
				}
			}
		}
		*/
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void ExtensionManager::RegisterConvertor( IConvertor *conv, IResourceCompiler *rc )
{
	assert(conv);
	assert(rc);

	IRCLog *log=rc->GetIRCLog();				assert(log);

	m_convertors.push_back( conv );

	string strExt;
	for (int i = 0; i < conv->GetNumExt(); ++i)
	{
		if (i)
			strExt += ", ";
		strExt += conv->GetExt(i);
	}

	time_t nTime = conv->GetTimestamp();
	char* szTime = "unknown";
	if (nTime)
	{
		szTime = asctime(localtime(&nTime));
		szTime[strlen(szTime)-1] = '\0';
	}

//	Info ("timestamp %s", szTime);
	log->Log("    Registered convertor for %s", strExt.c_str());

	assert( conv );
	for (int k = 0; k < conv->GetNumPlatforms(); k++)
	{
		Platform platform = conv->GetPlatform(k);
		for (int i = 0; i < conv->GetNumExt(); i++)
		{
			const char *ext = conv->GetExt(i);
			assert(ext);
			m_extMap[platform].insert( ExtMap::value_type(ext,conv) );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void ExtensionManager::UnregisterAll()
{
	for (int i = 0; i < PLATFORM_LAST; i++)
	{
		m_extMap[i].clear();
	}
	m_convertors.clear();
}