////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   light.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: loading light source from cgf
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Light.h"
#include "file.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLight::CLight()
{
	memset(&m_Chunk,0,sizeof(m_Chunk));
}

CLight::~CLight()
{
}

bool CLight::Load(CXFile *f, int pos)
{
	if(f->FSeek(pos,SEEK_SET)) return true;

	int res=f->FRead(&m_Chunk,sizeof(m_Chunk),1);
	if(res!=1) return true;
	
	if(m_Chunk.chdr.ChunkType != ChunkType_Light || m_Chunk.chdr.ChunkVersion != LIGHT_CHUNK_DESC_VERSION)
	{
		memset(&m_Chunk,0,sizeof(m_Chunk));
		return true;
	}

	m_ChunkHeader=m_Chunk.chdr;
	
	return false;
}
