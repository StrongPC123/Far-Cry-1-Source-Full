////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   IconManager.cpp
//  Version:     v1.00
//  Created:     24/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "IconManager.h"

#include <I3DEngine.h>

namespace
{
	// Object names in this array must correspond to EObject enumeration.
	const char *ObjectNames[STATOBJECT_LAST] =
	{
		"Objects\\Editor\\Arrow.cgf",
		"Objects\\Editor\\Axis.cgf",
		"Objects\\Editor\\Sphere.cgf",
		"Objects\\Editor\\Anchor.cgf",
		"Objects\\Editor\\entrypoint.cgf",
		"Objects\\Editor\\hidepoint.cgf",
	};

	const char *IconNames[ICON_LAST] =
	{
		"Editor\\Icons\\quad.tga",
	};
};

//////////////////////////////////////////////////////////////////////////
CIconManager::CIconManager()
{
	ZeroStruct( m_icons );
	ZeroStruct( m_objects );
}

//////////////////////////////////////////////////////////////////////////
CIconManager::~CIconManager()
{
	/*
	std::vector<int> ids;
	m_textures.GetAsVector( ids );
	for (int i = 0; i < ids.size(); i++)
	{
	}
	*/
}


//////////////////////////////////////////////////////////////////////////
void CIconManager::Init()
{
}

//////////////////////////////////////////////////////////////////////////
void CIconManager::Done()
{
}

//////////////////////////////////////////////////////////////////////////
void CIconManager::Reset()
{
	I3DEngine *pEngine = GetIEditor()->Get3DEngine();
	// Do not unload objects. but clears them.
	int i;
	for (i = 0; i < sizeof(m_objects)/sizeof(m_objects[0]); i++)
	{
		if (m_objects[i] && pEngine)
			pEngine->ReleaseObject( m_objects[i] );
		m_objects[i] = 0;
	}
	for (i = 0; i < ICON_LAST; i++)
	{
		m_icons[i] = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
int CIconManager::GetIconTexture( const CString &iconName )
{
	int id = 0;
	if (m_textures.Find( iconName,id ))
	{
		return id;
	}

	CImage image;
	// Load icon.
	if (CImageUtil::LoadImage( iconName,image ))
	{
		id = GetIEditor()->GetRenderer()->DownLoadToVideoMemory( (unsigned char*)image.GetData(),image.GetWidth(),image.GetHeight(),eTF_8888,eTF_8888,0,0,0 );
	}
	return id;
}

//////////////////////////////////////////////////////////////////////////
IStatObj*	CIconManager::GetObject( EObject object )
{
	assert( object >= 0 && object < STATOBJECT_LAST);

	if (m_objects[object])
		return m_objects[object];

	// Try to load this object.
	m_objects[object] = GetIEditor()->Get3DEngine()->MakeObject( ObjectNames[object] );
	if (!m_objects[object])
	{
		CLogFile::FormatLine( "Error: Load Failed: %s",ObjectNames[object] );
	}
	else
	{
		m_objects[object]->SetShaderTemplate( EFT_USER_FIRST+1,"s_ObjectColor",NULL );
	}
	return m_objects[object];
}

//////////////////////////////////////////////////////////////////////////
int CIconManager::GetIcon( EIcon icon )
{
	assert( icon >= 0 && icon < ICON_LAST );
	if (m_icons[icon])
		return m_icons[icon];

	int id = 0;
	// Try to load this Icon.
	ITexPic *pPic = GetIEditor()->GetRenderer()->EF_LoadTexture( IconNames[icon],FT_NOREMOVE,0,eTT_Base, -1 );

	m_icons[icon] = pPic->GetTextureID();
	if (!m_icons[icon])
	{
		CLogFile::FormatLine( "Error: Load Failed: %s",IconNames[icon] );
	}
	return m_icons[icon];
}