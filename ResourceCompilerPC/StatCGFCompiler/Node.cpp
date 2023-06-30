////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   node.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: loading node info from cgf
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Node.h"
#include "Geom.h"
#include "file.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNodeCGF::CNodeCGF() : CBaseObj()
{
	m_pObj			= NULL;
//	m_sPropStr		= NULL;
//	m_ppChildren	= NULL;
//	m_pnChildrenIds= NULL;
	m_pParent		= NULL;
//	m_pMtl			= NULL;
//	m_pConRot		= NULL;
//	m_pConPos		= NULL;
//	m_pConScl		= NULL;
	memset(&m_Chunk,0,sizeof(m_Chunk));
}

CNodeCGF::~CNodeCGF()
{
//	if(m_ppChildren)		free(m_ppChildren);
//	if(m_pnChildrenIds)	free(m_pnChildrenIds);
//	if(m_sPropStr)			free(m_sPropStr);
}

bool CNodeCGF::Load(CXFile *f, int pos)
{
	if(f->FSeek(pos,SEEK_SET)) return true;

	int res=f->FRead(&m_Chunk,sizeof(m_Chunk),1);
	if(res!=1) return true;
	
	if(m_Chunk.chdr.ChunkType != ChunkType_Node || m_Chunk.chdr.ChunkVersion != NODE_CHUNK_DESC_VERSION)
	{
		memset(&m_Chunk,0,sizeof(m_Chunk));
		return true;
	}

	m_ChunkHeader=m_Chunk.chdr;
//	m_NodeMatrix = m_Chunk.tm;

	//read propstr
/*	if(m_Chunk.PropStrLen)
	{
		m_sPropStr=(char*)malloc(m_Chunk.PropStrLen+1);
		assert(m_sPropStr);
		res=f->FRead(m_sPropStr,m_Chunk.PropStrLen,1);
    m_sPropStr[m_Chunk.PropStrLen]=0;
		if(res!=1) return true;
	}*/

	//read m_ppChildren
/*	if(m_Chunk.nChildren)
	{
		m_pnChildrenIds = (int*) malloc(m_Chunk.nChildren*sizeof(int));
		assert(m_pnChildrenIds);
		
    m_ppChildren = (CNodeCGF **) malloc(m_Chunk.nChildren*sizeof(CNodeCGF*));
		assert(m_pnChildrenIds);
    memset(m_ppChildren,0,m_Chunk.nChildren*sizeof(CNodeCGF*));
		
		int res=f->FRead(m_pnChildrenIds,sizeof(int),m_Chunk.nChildren);
		if(res!=m_Chunk.nChildren) return true;
	}*/

	return false;
}

void CNodeCGF::Bind(CBaseObj ** all_objects, int n_obj)
{
  if(m_bBinded) 
    return;

  for(int i=0;i<n_obj;i++)
  {
    CBaseObj * o = all_objects[i];
    if(!o || o->m_ChunkHeader.ChunkID == -1) 
      continue;

    if(o->m_ChunkHeader.ChunkID == m_Chunk.ObjectID)	
    {
      m_pObj = o;
      o->m_nUsers++;
    }
    else if(o->m_ChunkHeader.ChunkID == m_Chunk.ParentID)	
      m_pParent=(CNodeCGF *)o;		/*
    else if(o->m_ChunkHeader.ChunkID == m_Chunk.MatID)		
      m_pMtl=o;		
    else if(o->m_ChunkHeader.ChunkID == m_Chunk.pos_cont_id) 
      m_pConPos=(Controller *)o;		
    else if(o->m_ChunkHeader.ChunkID == m_Chunk.rot_cont_id) 
      m_pConRot=(Controller *)o;		
    else if(o->m_ChunkHeader.ChunkID == m_Chunk.scl_cont_id) 
      m_pConScl=(Controller *)o;

    for(int j=0;j<m_Chunk.nChildren;j++)
    {
      if(o->m_ChunkHeader.ChunkID == m_pnChildrenIds[j]) 
        m_ppChildren[j]=(CNodeCGF *)o;
    }*/
  }

  m_bBinded = true;
}
