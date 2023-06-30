////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ShaderEnum.cpp
//  Version:     v1.00
//  Created:     7/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Enumerate Installed Shaders.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ShaderEnum.h"

//////////////////////////////////////////////////////////////////////////
CShaderEnum::CShaderEnum()
{
	m_bEnumerated = false;
}

CShaderEnum::~CShaderEnum()
{
}


inline bool ShaderLess( const CShaderEnum::ShaderDesc &s1,const CShaderEnum::ShaderDesc &s2 )
{
	return stricmp( s1.name,s2.name ) < 0;
}
/*
struct StringLess {
	bool operator()( const CString &s1,const CString &s2 )
	{
		return stricmp( s1,s2 ) < 0;
	}
};
*/

//! Enum shaders.
int CShaderEnum::EnumShaders()
{
	IRenderer *renderer = GetIEditor()->GetSystem()->GetIRenderer();
	if (!renderer)
		return 0;

	if (m_bEnumerated)
		return m_shaders.size();

	m_bEnumerated = true;

	int numShaders = 0;

	m_shaders.reserve( 100 );
	//! Enumerate Shaders.
	for (int type = 0; type < 2; type++)
	{
		int numFiles = (INT_PTR)renderer->EF_Query( EFQ_NUMEFFILES0+type );
		string *files = (string *)renderer->EF_Query( EFQ_EFFILENAMES0+type );
		for (int i = 0; i < numFiles; i++)
		{
			// for each effect file, get list of shaders inside.
			char **EfNames = renderer->EF_GetShadersForFile( files[i].c_str(),type );
			if (EfNames)
			{
				int j = 0;
				while (EfNames[j])
				{
					ShaderDesc sd;
					sd.name = EfNames[j];
					sd.file = files[i].c_str();
					m_shaders.push_back( sd );
					j++;
				}
			}
		}
	}
	std::sort( m_shaders.begin(),m_shaders.end(),ShaderLess );
	return m_shaders.size();
}