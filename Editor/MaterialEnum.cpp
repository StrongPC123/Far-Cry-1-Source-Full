////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   MaterialEnum.cpp
//  Version:     v1.00
//  Created:     7/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Enumerate Installed Materials.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MaterialEnum.h"
#include "I3DEngine.h"

//////////////////////////////////////////////////////////////////////////
CMaterialEnum::CMaterialEnum()
{
	m_bEnumerated = false;
}

CMaterialEnum::~CMaterialEnum()
{
}

inline bool StringLess( const CString &s1,const CString &s2 )
{
	return stricmp( s1,s2 ) < 0;
}

//! Enum Materials.
int CMaterialEnum::EnumMaterials()
{
	m_materials.clear();
	I3DEngine *p3DEngine = GetIEditor()->Get3DEngine();
	if (!p3DEngine)
		return 0;

	IPhysMaterialEnumerator *pMtls = p3DEngine->GetPhysMaterialEnumerator();
	if (!pMtls)
		return 0;

	for (int i = 0; i < pMtls->GetMaterialCount(); i++)
	{
		const char *sMaterial = pMtls->GetMaterialNameByIndex(i);
		if (sMaterial && strlen(sMaterial) > 0)
			m_materials.push_back(sMaterial);
	}

	std::sort( m_materials.begin(),m_materials.end(),StringLess );
	return m_materials.size();
}