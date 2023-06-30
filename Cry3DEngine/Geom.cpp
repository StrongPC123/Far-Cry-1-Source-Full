////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   geom.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: loading geometry from cgf
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Geom.h"
#include "file.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGeom::CGeom() : CBaseObj()
{
	memset(&m_Chunk,0,sizeof(m_Chunk));
	m_pVertices	=NULL;
	m_pFaces		=NULL;
//	m_pLinks		=NULL;
	m_pUVs			=NULL;
	m_sPropstr	=NULL;
//	bones		=NULL;
	m_pTexFaces	=NULL;
//	m_pNumLinks		=NULL;
//	links		=NULL;
	m_pVcols		=NULL;
//	BoneNames	=NULL;
}

CGeom::~CGeom()
{
/*	if(links)		
	{
		for(int i=0;i<m_Chunk.nVerts;i++) if(links[i]) free(links[i]);
		free(links);
	}*/

//	if(bones)		free(bones);
	if(m_pFaces)		free(m_pFaces);
	if(m_pVertices)free(m_pVertices);
	if(m_pUVs)			free(m_pUVs);
	if(m_sPropstr)	free(m_sPropstr);
//	if(nLinks)	free(nLinks);
	if(m_pVcols)		free(m_pVcols);
  if(m_pTexFaces)free(m_pTexFaces);
}

bool CGeom::Load(CXFile *f, int pos)
{
	if(f->FSeek(pos,SEEK_SET)) return true;

	int res=f->FRead(&m_Chunk,sizeof(m_Chunk),1);
	if(res!=1) return true;

	m_ChunkHeader=m_Chunk.chdr;

	if(m_ChunkHeader.ChunkType != ChunkType_Mesh || m_ChunkHeader.ChunkVersion != MESH_CHUNK_DESC_VERSION)
	{
		memset(&m_Chunk,0,sizeof(m_Chunk));
		return true;
	}

	//read verts
	m_pVertices=(CryVertex*)malloc(sizeof(CryVertex)*m_Chunk.nVerts);
	assert(m_pVertices);
	res=f->FRead(m_pVertices,sizeof(CryVertex),m_Chunk.nVerts);
	if(res!=m_Chunk.nVerts) return true;

	//read m_pFaces
	m_pFaces=(CryFace*)malloc(sizeof(CryFace)*m_Chunk.nFaces);
	assert(m_pFaces);
	res=f->FRead(m_pFaces,sizeof(CryFace),m_Chunk.nFaces);
	if(res!=m_Chunk.nFaces) return true;

	//read tverts
	if(m_Chunk.nTVerts)
	{
		m_pUVs=(CryUV*)malloc(sizeof(CryUV)*m_Chunk.nTVerts);
		assert(m_pUVs);
		res=f->FRead(m_pUVs,sizeof(CryUV),m_Chunk.nTVerts);
		if(res!=m_Chunk.nTVerts) return true;

    // flip tex coords (since it was flipped in max?)
    for(int t=0; t<m_Chunk.nTVerts; t++)
      m_pUVs[t].v = 1.f-m_pUVs[t].v;

		//read Tfaces
		//if(m_Chunk.nVerts != m_Chunk.nTVerts)
    if(m_Chunk.nVerts != 12 || pos != 692)//hack to make old grass objects working
		{
			m_pTexFaces=(CryTexFace*)malloc(sizeof(CryTexFace)*m_Chunk.nFaces);
			assert(m_pTexFaces);

			res=f->FRead(m_pTexFaces,sizeof(CryTexFace),m_Chunk.nFaces);
			if(res!=m_Chunk.nFaces) return true;
		}
	}

	//read Vertex Colors
	if(m_Chunk.HasVertexCol)
	{
		m_pVcols=(CryIRGB *)malloc(m_Chunk.nVerts*sizeof(CryIRGB));
		assert(m_pVcols);

		int res=f->FRead(m_pVcols, sizeof(CryIRGB), m_Chunk.nVerts);

/*				
		for (int k=0;k<m_Chunk.nVerts;k++)
		{
			m_pVcols[k].r=255;
			m_pVcols[k].g=0;
			m_pVcols[k].b=0;
		} //k
*/		
		

		if (res!=m_Chunk.nVerts) 
			return (true);		
	}
  else
	{
    m_pVcols=0;
		/*
		m_pVcols=(CryIRGB *)malloc(m_Chunk.nVerts*sizeof(CryIRGB));
		assert(m_pVcols);

		for (int k=0;k<m_Chunk.nVerts;k++)
		{
			m_pVcols[k].r=255;
			m_pVcols[k].g=0;
			m_pVcols[k].b=0;
		} //k
		*/
	}


	return (false);
}
