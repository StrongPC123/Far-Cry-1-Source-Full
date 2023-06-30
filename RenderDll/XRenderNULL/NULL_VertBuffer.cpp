//////////////////////////////////////////////////////////////////////
//
//  Crytek CryENGINE Source code
//  
//  File: PS2_VertBuffer.cpp
//  Description: Implementation of the vertex buffer management
//
//  History:
//  -Jan 31,2001:Created by Vladimir Kajalin
//
//////////////////////////////////////////////////////////////////////

#include "RenderPCH.h"
#include "NULL_Renderer.h"

void *CVertexBuffer::GetStream(int nStream, int *nOffs)
{
  return NULL;
}

void *CNULLRenderer::GetDynVBPtr(int nVerts, int &nOffs, int Pool)
{
  return m_DynVB;
}

void CNULLRenderer::DrawDynVB(int nOffs, int Pool, int nVerts)
{
}

void CNULLRenderer::DrawDynVB(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pBuf, ushort *pInds, int nVerts, int nInds, int nPrimType)
{
}

// allocates vertex buffer
void CNULLRenderer::CreateBuffer(int size, int vertexformat, CVertexBuffer *buf, int Type, const char *szSource)
{
  assert(Type >= 0 && Type <= 3);

  void *data;

  // System buffer 
  data=new unsigned char [size];
  buf->m_VS[Type].m_VData = data;
}

CVertexBuffer *CNULLRenderer::CreateBuffer(int vertexcount,int vertexformat, const char *szSource, bool bDynamic)
{
  CVertexBuffer * vtemp = new CVertexBuffer;

  vtemp->m_bDynamic = bDynamic;

  int size = m_VertexSize[vertexformat]*vertexcount;
  // System buffer 
  vtemp->m_VS[VSF_GENERAL].m_VData = new unsigned char [size];
  vtemp->m_fence = 0;

  vtemp->m_vertexformat=vertexformat;
  vtemp->m_NumVerts = vertexcount;

  return (vtemp);
}


///////////////////////////////////////////
void CNULLRenderer::DrawBuffer(CVertexBuffer *src,SVertexStream *indicies,int numindices, int offsindex, int prmode,int vert_start,int vert_stop, CMatInfo *mi)
{
}


void CNULLRenderer::SetFenceCompleted(CVertexBuffer * buffer)
{ 
}

///////////////////////////////////////////
// Updates the vertex buffer dest with the data from src
// NOTE: src may be NULL, in which case the data will not be copied
void CNULLRenderer::UpdateBuffer(CVertexBuffer *dest,const void *src,int vertexcount, bool bUnlock, int offs, int Type)
{
  assert (Type >= 0 && Type <= 2);

  // NOTE: some subsystems need to initialize the system buffer without actually intializing its values;
	// for that purpose, src may sometimes be NULL
  if(src && vertexcount)
  {
    assert(vertexcount<=dest->m_NumVerts);
    if(vertexcount>dest->m_NumVerts)
    {
      iLog->Log("CNULLRenderer::UpdateBuffer: vertexcount>dest->m_NumVerts");
      return;
    }

    byte *dst = (byte *)dest->m_VS[Type].m_VData;
    if (dst)
    {
      if (Type == VSF_GENERAL)
        memcpy(&dst[m_VertexSize[dest->m_vertexformat]*offs],src,m_VertexSize[dest->m_vertexformat]*vertexcount);
      else
      if (Type == VSF_TANGENTS)
        memcpy(&dst[sizeof(SPipTangents)*offs],src,sizeof(SPipTangents)*vertexcount);
    }
  }
}

///////////////////////////////////////////
void CNULLRenderer::ReleaseBuffer(CVertexBuffer *bufptr)
{
  if (bufptr)
  {
    SAFE_DELETE_ARRAY(bufptr->m_VS[VSF_GENERAL].m_VData);
    SAFE_DELETE_ARRAY(bufptr->m_VS[VSF_TANGENTS].m_VData);

    delete bufptr;
  } 
}

///////////////////////////////////////////
void CNULLRenderer::DrawTriStrip(CVertexBuffer *src, int vert_num)
{ 
}


void CLeafBuffer::DrawImmediately()
{						
}

void CNULLRenderer::CreateIndexBuffer(SVertexStream *dest,const void *src,int indexcount)
{
  SAFE_DELETE_ARRAY(dest->m_VData);
  dest->m_nItems = 0;
  if (indexcount)
  {
    dest->m_VData = new ushort[indexcount];
    dest->m_nItems = indexcount;
  }
  if (src && indexcount)
  {
    cryMemcpy(dest->m_VData, src, indexcount*sizeof(ushort));
    m_RP.m_PS.m_MeshUpdateBytes += indexcount*sizeof(ushort);
  }
}
void CNULLRenderer::UpdateIndexBuffer(SVertexStream *dest,const void *src,int indexcount, bool bUnLock)
{
  PROFILE_FRAME(Mesh_UpdateIBuffers);
  if (src && indexcount)
  {
    if (dest->m_nItems < indexcount)
    {
      delete [] dest->m_VData;
      dest->m_VData = new ushort[indexcount];
      dest->m_nItems = indexcount;
    }
    cryMemcpy(dest->m_VData, src, indexcount*sizeof(ushort));
    m_RP.m_PS.m_MeshUpdateBytes += indexcount*sizeof(ushort);
  }
}
void CNULLRenderer::ReleaseIndexBuffer(SVertexStream *dest)
{
  SAFE_DELETE_ARRAY(dest->m_VData);
  dest->Reset();
}
