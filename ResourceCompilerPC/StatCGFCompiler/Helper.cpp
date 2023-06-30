////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   helper.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: loading helper from cgf
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Helper.h"
#include "file.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHelper::CHelper() : CBaseObj()
{
	memset(&m_Chunk,0,sizeof(m_Chunk));
}

CHelper::~CHelper()
{
}

bool CHelper::Load(CXFile *f, int pos)
{
	if(f->FSeek(pos,SEEK_SET)) return true;

	int res=f->FRead(&m_Chunk,sizeof(m_Chunk),1);
	if(res!=1) return true;
	
	if(m_Chunk.chdr.ChunkType != ChunkType_Helper || m_Chunk.chdr.ChunkVersion != HELPER_CHUNK_DESC_VERSION)
	{
		memset(&m_Chunk,0,sizeof(m_Chunk));
		return true;
	}

	m_ChunkHeader=m_Chunk.chdr;

	return false;
}
