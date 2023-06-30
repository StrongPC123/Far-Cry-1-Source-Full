//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code (c) 2002-2004
// 
//	File: SystemCfg.h	
// 
//	History:
//	-Jan 22,2004:Created 
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <math.h>
#include <string>
#include <map>

typedef string SysConfigKey;
typedef string SysConfigValue;

//////////////////////////////////////////////////////////////////////////
class CSystemConfiguration
{
public:
	CSystemConfiguration( const string& strSysConfigFilePath,CSystem *pSystem);
	~CSystemConfiguration();

	string RemoveWhiteSpaces( string& s )
	{
		string s1;
		for (int i = 0; i < (int)(s.size()); i++)
			if (s[i] != ' ') s1.push_back(s[i]);
		s = s1;
		return s;
		//return( string( s.begin(), std::remove( s.begin(), s.end(), ' ' ) ) );	
	}

private:

	void ParseSystemConfig();

	CSystem	*m_pSystem;
	string m_strSysConfigFilePath;	
};

