////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   config.cpp
//  Version:     v1.00
//  Created:     4/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "config.h"

Config::Config()
{
}

Config::~Config()
{
}

//////////////////////////////////////////////////////////////////////////
IConfig* Config::Clone() const
{
	Config *cfg = new Config;
	cfg->m_map = m_map;
	return cfg;
}

//////////////////////////////////////////////////////////////////////////
bool Config::HasKey( const char *key ) const
{
	if (m_map.find(key) != m_map.end())
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool Config::Get( const char *key,float &value ) const
{
	Map::const_iterator it = m_map.find(key);
	if (it != m_map.end())
	{
		const char* szValue = it->second.GetString();
		if (1 == sscanf (szValue, "%f", &value))
			return true;
		else
			return false;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool Config::Get( const char *key,int &value ) const
{
	Map::const_iterator it = m_map.find(key);
	if (it != m_map.end())
	{
		const char* szValue = it->second.GetString();
		if (1 == sscanf (szValue, "%d", &value))
			return true;
		else
			return false;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool Config::Get( const char *key,bool &value ) const
{
	Map::const_iterator it = m_map.find(key);
	if (it != m_map.end())
	{
		const char* szValue = it->second.GetString();
		int nTryInt;
		if (1 == sscanf (szValue, "%d", &nTryInt))
		{
			value = nTryInt != 0;
			return true;
		}

		if (!stricmp(szValue, "true")
			||!stricmp(szValue, "yes")
			||!stricmp(szValue, "enable")
			||!stricmp(szValue, "y")
			||!stricmp(szValue, "t"))
		{
			value = true;
			return true;
		}

		if (!stricmp(szValue, "false")
			||!stricmp(szValue, "no")
			||!stricmp(szValue, "disable")
			||!stricmp(szValue, "n")
			||!stricmp(szValue, "f"))
		{
			value = false;
			return true;
		}
	}
	return false;
}

void Config::Merge( IConfig *inpConfig )
{
	Config *pConfig=inpConfig->GetInternalRepresentation();

	assert(pConfig);if(!pConfig)return;

	Map::iterator it;

	for(it=pConfig->m_map.begin();it!=pConfig->m_map.end();++it)
	{
		CString key=(*it).first;
		CString value=(*it).second;

		Set(key.GetString(),value.GetString());
	}
}



Config *Config::GetInternalRepresentation( void )
{
	return(this);	
}


//////////////////////////////////////////////////////////////////////////
bool Config::Get( const char *key,CString &value ) const
{
	Map::const_iterator it = m_map.find(key);
	if (it != m_map.end())
	{
		value = it->second;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void Config::Set( const char *key,const char* value )
{
	m_map[key] = value;
}