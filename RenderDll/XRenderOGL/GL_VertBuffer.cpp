//////////////////////////////////////////////////////////////////////
//
//  Crytek CryENGINE Source code
//  
//  File:GlRenderer.cpp
//  Description: Implementation of the vertex buffer management
//
//  History:
//  -Jan 31,2001:Created by Vladimir Kajalin
//
//////////////////////////////////////////////////////////////////////

#include "RenderPCH.h"
#include "GL_Renderer.h"

void *CVertexBuffer::GetStream(int nStream, int *nOffs)
{
  if (nOffs)
    *nOffs = 0;
  return m_VS[nStream].m_VData;
}

void *CGLRenderer::GetDynVBPtr(int nVerts, int &nOffs, int Pool)
{
  nOffs = 0;
  switch(Pool)
  {
    case 0:
    default:
      if (nVerts <= 2048)
      {
        SAFE_DELETE_ARRAY(m_TempDynVB);
        return m_DynVB;
      }
      else
      {
        m_TempDynVB = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F [nVerts];
        return m_TempDynVB;
      }
      break;

    case 1:
    case 2:
      assert(0);
      break;
  }
  return NULL;
}

void CGLRenderer::DrawDynVB(int nOffs, int Pool, int nVerts)
{
  if (!m_DynVBId)
  {
    int size = sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F)*2048;
    /*if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      glGenBuffersARB(1, &m_DynVBId);
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_DynVBId);
      glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, GL_DYNAMIC_DRAW_ARB);
    }
    else
    if(IsVarPresent())
    {
      m_DynVBId = (uint)AllocateVarShunk(size, "Font buf");
      glGenFencesNV(1, &m_DynVBFence);
    }*/
  }
  /*if (SUPPORTS_GL_ARB_vertex_buffer_object)
  {
    int nVFormat = VERTEX_FORMAT_P3F_COL4UB_TEX2F;
    int sizeV = m_VertexSize[nVFormat];
    if (nVerts + m_nOffsDMesh3D > 2048)
    {
      m_nOffsDMesh3D = 0;
    }

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_DynVBId);
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sizeV*m_nOffsDMesh3D, sizeV*nVerts, m_DynVB);
    m_RP.m_PS.m_MeshUpdateBytes += sizeV*nVerts;

    SBufInfoTable *pOffs = &gBufInfoTable[nVFormat];
    //glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeV, BUFFER_OFFSET(0));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeV, BUFFER_OFFSET(pOffs->OffsColor));
    glTexCoordPointer(2, GL_FLOAT, sizeV, BUFFER_OFFSET(pOffs->OffsTC));

    glDrawArrays(GL_TRIANGLES, m_nOffsDMesh3D, nVerts); 

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    m_nOffsDMesh3D += nVerts;
  }*/
  /*else
  if(IsVarPresent())
  {
    int nVFormat = VERTEX_FORMAT_P3F_COL4UB_TEX2F;
    int sizeV = m_VertexSize[nVFormat];
    if (nVerts + m_nOffsDMesh3D > 2048)
    {
      if(m_bDynVBFenceSet && !glTestFenceNV(m_DynVBFence))
        glFinishFenceNV(m_DynVBFence);
      m_bDynVBFenceSet = false;
      m_nOffsDMesh3D = 0;
    }
    SBufInfoTable *pOffs = &gBufInfoTable[nVFormat];
    //glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    byte *pDst = (byte *)m_DynVBId;
    memcpy(&pDst[m_nOffsDMesh3D*sizeV], m_DynVB, sizeV*nVerts);

    glVertexPointer(3, GL_FLOAT, sizeV, (byte *)m_DynVBId);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeV, (byte *)m_DynVBId+pOffs->OffsColor);
    glTexCoordPointer(2, GL_FLOAT, sizeV, (byte *)m_DynVBId+pOffs->OffsTC);

    glDrawArrays(GL_TRIANGLES, m_nOffsDMesh3D, nVerts); 
    glSetFenceNV(m_DynVBFence, GL_ALL_COMPLETED_NV);
    m_bDynVBFenceSet = true;

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    m_nOffsDMesh3D += nVerts;
  }
  else*/
  {
    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pVB = m_DynVB;
    if (m_TempDynVB)
      pVB = m_TempDynVB;

    glBegin(GL_TRIANGLES);
    int nTris = nVerts/3;
    
    for (int k=0; k<nTris; k++)    
    {
      int nInd = k*3+0;
      glTexCoord2fv(&pVB[nInd].st[0]);
      glColor4ubv(&pVB[nInd].color.bcolor[0]);
      glVertex3fv(&pVB[nInd].xyz.x);     

      nInd = k*3+1;
      glTexCoord2fv(&pVB[nInd].st[0]);
      glColor4ubv(&pVB[nInd].color.bcolor[0]);
      glVertex3fv(&pVB[nInd].xyz.x);     

      nInd = k*3+2;
      glTexCoord2fv(&pVB[nInd].st[0]);
      glColor4ubv(&pVB[nInd].color.bcolor[0]);
      glVertex3fv(&pVB[nInd].xyz.x);     
    }
    
    glEnd();
  }
}

void CGLRenderer::DrawDynVB(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pBuf, ushort *pInds, int nVerts, int nInds, int nPrimType)
{
  if (!m_DynVBId)
  {
    int size = sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F)*4096;
    /*if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      glGenBuffersARB(1, &m_DynVBId);
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_DynVBId);
      glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, GL_DYNAMIC_DRAW_ARB);
    }
    else
    if(IsVarPresent())
    {
      m_DynVBId = (uint)AllocateVarShunk(size, "Dyn buf");
      glGenFencesNV(1, &m_DynVBFence);
    }*/
  }
  /*if (SUPPORTS_GL_ARB_vertex_buffer_object)
  {
  }
  else
  if(IsVarPresent())
  {
    int nVFormat = VERTEX_FORMAT_P3F_COL4UB_TEX2F;
    int sizeV = m_VertexSize[nVFormat];
    if (nVerts + m_nOffsDMesh3D > 2048)
    {
      if(m_bDynVBFenceSet && !glTestFenceNV(m_DynVBFence))
        glFinishFenceNV(m_DynVBFence);
      m_bDynVBFenceSet = false;
      m_nOffsDMesh3D = 0;
    }
    SBufInfoTable *pOffs = &gBufInfoTable[nVFormat];
    //glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    byte *pDst = (byte *)m_DynVBId;
    int nOffs = m_nOffsDMesh3D*sizeV;
    memcpy(&pDst[nOffs], m_DynVB, sizeV*nVerts);

    glVertexPointer(3, GL_FLOAT, sizeV, (byte *)m_DynVBId+nOffs);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeV, (byte *)m_DynVBId+nOffs+pOffs->OffsColor);
    glTexCoordPointer(2, GL_FLOAT, sizeV, (byte *)m_DynVBId+nOffs+pOffs->OffsTC);

    glDrawElements(GL_TRIANGLES,nInds,GL_UNSIGNED_SHORT,pInds); 
    glSetFenceNV(m_DynVBFence, GL_ALL_COMPLETED_NV);
    m_bDynVBFenceSet = true;

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    m_nOffsDMesh3D += nVerts;
  }
  else*/
  {
    glBegin(GL_TRIANGLES);
    int nTris = nInds/3;
    
    for (int k=0; k<nTris; k++)    
    {
      int nInd = pInds[k*3+0];
      glTexCoord2fv(&pBuf[nInd].st[0]);
      glColor4ubv(&pBuf[nInd].color.bcolor[0]);
      glVertex3fv(&pBuf[nInd].xyz.x);     

      nInd = pInds[k*3+1];
      glTexCoord2fv(&pBuf[nInd].st[0]);
      glColor4ubv(&pBuf[nInd].color.bcolor[0]);
      glVertex3fv(&pBuf[nInd].xyz.x);     

      nInd = pInds[k*3+2];
      glTexCoord2fv(&pBuf[nInd].st[0]);
      glColor4ubv(&pBuf[nInd].color.bcolor[0]);
      glVertex3fv(&pBuf[nInd].xyz.x);     
    }
    
    glEnd();
  }
}

// return true if vertex array range extension in use
BOOL CGLRenderer::IsVarPresent()
{
  return m_AGPbuf && m_var_valid && wglAllocateMemoryNV;
}

// allocates vertex buffer
void CGLRenderer::CreateBuffer(int size, int vertexformat, CVertexBuffer *buf, int Type, const char *szSource)
{
  PROFILE_FRAME(Mesh_CreateVBuffers);

  assert(Type >= 0 && Type < VSF_NUM);

  void *data;
  if(IsVarPresent())
  {
    data = AllocateVarShunk(size, szSource);

    // We couldn't allocate video biffer chunk :( Well. try to deallocate LeafBuffer's ones.
    if (!data)
    {
      CLeafBuffer *pLB = CLeafBuffer::m_Root.m_Prev;
      while (!data)
      {
        if (pLB == &CLeafBuffer::m_Root)
        {
          GenerateVBLog("LogVBuffers.txt");
          iConsole->Exit("Error: Pipeline buffer overflow. Current geometry is too-oo-oo big (%s) (See LogVBuffers.txt for more info)", gRenDev->GetStatusText(eRS_VidBuffer));
        }
        
        CLeafBuffer *Next = pLB->m_Prev;
        if (pLB->m_pVertexBuffer != buf)
          pLB->Unload();
        pLB = Next;
        data = AllocateVarShunk(size, szSource);
      }
    }
  }
  else
  if (SUPPORTS_GL_ARB_vertex_buffer_object)
  {
    glGenBuffersARB(1, &buf->m_VS[Type].m_VertBuf.m_nID);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, buf->m_VS[Type].m_VertBuf.m_nID);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, buf->m_VS[Type].m_bDynamic ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB);
    buf->m_VS[Type].m_VData = (void *)1;
    //buf->m_VS[Type].m_VData = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
    //buf->m_VS[Type].m_bLocked = true;
    m_CurVertBufferSize += size;
    return;
  }
  else
  { // System buffer 
    data=new unsigned char [size];
  }
  buf->m_VS[Type].m_VData = data;
}

void CGLRenderer::CreateIndexBuffer(SVertexStream *dest,const void *src,int indexcount)
{
  ReleaseIndexBuffer(dest);
  if (indexcount)
  {
    if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      glGenBuffersARB(1, &dest->m_VertBuf.m_nID);
      glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, dest->m_VertBuf.m_nID);
      glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexcount*sizeof(ushort), NULL, dest->m_bDynamic ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB);
    }
    else
      dest->m_VData = new ushort[indexcount];
    dest->m_nItems = indexcount;
  }
  if (src && indexcount)
    UpdateIndexBuffer(dest, src, indexcount, true);
}

void CGLRenderer::UpdateIndexBuffer(SVertexStream *dest,const void *src,int indexcount, bool bUnLock)
{
  PROFILE_FRAME(Mesh_UpdateIBuffers);
  if (src && indexcount)
  {
    void *dst = NULL;
    if (dest->m_nItems < indexcount)
    {
      if (dest->m_nItems)
        ReleaseIndexBuffer(dest);
      CreateIndexBuffer(dest, NULL, indexcount);
    }
    int size = indexcount*sizeof(ushort);
    if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, dest->m_VertBuf.m_nID);
      dst = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
      dest->m_VData = dst;
      dest->m_bLocked = true;
    }
    else
      dst = dest->m_VData;
    if (dst)
      cryMemcpy(dst, src, size);
    m_RP.m_PS.m_MeshUpdateBytes += size;
    if (bUnLock)
    {
      if (SUPPORTS_GL_ARB_vertex_buffer_object)
      {
        glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
        dest->m_bLocked = false;
      }
    }
  }
}

void CGLRenderer::ReleaseIndexBuffer(SVertexStream *dest)
{
  if (SUPPORTS_GL_ARB_vertex_buffer_object)
  {
    if (dest->m_VertBuf.m_nID)
      glDeleteBuffersARB(1, &dest->m_VertBuf.m_nID);
  }
  else
    SAFE_DELETE_ARRAY(dest->m_VData);
  dest->Reset();
}


CVertexBuffer *CGLRenderer::CreateBuffer(int vertexcount,int vertexformat, const char *szSource, bool bDynamic)
{
  PROFILE_FRAME(Mesh_CreateVBuffers);

  CVertexBuffer * vtemp = new CVertexBuffer;

  vtemp->m_bDynamic = bDynamic;

  int size = m_VertexSize[vertexformat]*vertexcount;
  if(IsVarPresent())
  {
    vtemp->m_VS[VSF_GENERAL].m_VData = AllocateVarShunk(size, szSource);
    glGenFencesNV(1, &vtemp->m_fence);

    // We couldn't allocate video biffer chunk :( Well. try to deallocate LeafBuffer's ones.
    if (!vtemp->m_VS[VSF_GENERAL].m_VData)
    {
      CLeafBuffer *pLB = CLeafBuffer::m_Root.m_Prev;
      while (!vtemp->m_VS[VSF_GENERAL].m_VData)
      {
        if (pLB == &CLeafBuffer::m_Root)
        {
          GenerateVBLog("LogVBuffers.txt");
          iConsole->Exit("Error: Pipeline buffer overflow. Current geometry cannot fit in video memory (%s) (See LogVBuffers.txt for more info)", gRenDev->GetStatusText(eRS_VidBuffer));
        }
        
        CLeafBuffer *Next = pLB->m_Prev;
        pLB->Unload();
        pLB = Next;
        vtemp->m_VS[VSF_GENERAL].m_VData = AllocateVarShunk(size, szSource);
      }
    }
  }
  else
  if (SUPPORTS_GL_ARB_vertex_buffer_object)
  {
    if (size+m_CurVertBufferSize > m_MaxVertBufferSize)
    {
      CLeafBuffer *pLB = CLeafBuffer::m_Root.m_Prev;
      while (size+m_CurVertBufferSize > m_MaxVertBufferSize)
      {
        if (pLB == &CLeafBuffer::m_Root)
          iConsole->Exit("Error: Pipeline buffer overflow. Current geometry is too-oo-oo big (%s)", gRenDev->GetStatusText(eRS_VidBuffer));
        
        CLeafBuffer *Next = pLB->m_Prev;
        pLB->Unload();
        pLB = Next;
      }
    }
    // create the vertex object
    glGenBuffersARB(1, &vtemp->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vtemp->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, bDynamic ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB);
    vtemp->m_VS[VSF_GENERAL].m_VData = (void *)1;
    //vtemp->m_VS[VSF_GENERAL].m_VData = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
    //vtemp->m_VS[VSF_GENERAL].m_bLocked = true;

    m_CurVertBufferSize += size;
  }
  else
  { // System buffer 
    vtemp->m_VS[VSF_GENERAL].m_VData = new unsigned char [size];
    vtemp->m_fence = 0;
  }

  vtemp->m_vertexformat=vertexformat;
  vtemp->m_NumVerts = vertexcount;

  return (vtemp);
}

///////////////////////////////////////////
// Updates the vertex buffer dest with the data from src
// NOTE: src may be NULL, in which case the data will not be copied
void CGLRenderer::UpdateBuffer(CVertexBuffer *dest,const void *src,int vertexcount, bool bUnlock, int offs, int Type)
{
  void *pVertices;

  if(IsVarPresent() && dest && !Type && dest->m_fence && CRenderer::CV_r_syncvbuf)
  {
    if(dest->m_bFenceSet && !glTestFenceNV(dest->m_fence))
      glFinishFenceNV(dest->m_fence);
    dest->m_bFenceSet = false;
  }

  if (!src)
  {
    assert (Type >= 0 && Type <= 3);

    if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      if (!Type)
      {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, dest->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
        if (bUnlock)
        {
          // video buffer update
          if (dest->m_VS[VSF_GENERAL].m_bLocked)
          {
            dest->m_VS[VSF_GENERAL].m_bLocked = false;
            glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
          }
        }
        else
        {
          // video buffer update
          if (!dest->m_VS[VSF_GENERAL].m_bLocked)
          {
            PROFILE_FRAME(Mesh_UpdateVBuffersLock);
            dest->m_VS[VSF_GENERAL].m_bLocked = true;
            pVertices = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
            dest->m_VS[VSF_GENERAL].m_VData = pVertices;
          }
        }
      }
      else
      {
        for (int i=0; i<VSF_NUM; i++)
        {
          if (!((1<<i) & Type))
            continue;
          if (!dest->m_VS[i].m_VertBuf.m_nID)
            continue;
          glBindBufferARB(GL_ARRAY_BUFFER_ARB, dest->m_VS[i].m_VertBuf.m_nID);
          if (bUnlock)
          {
            if (dest->m_VS[i].m_bLocked)
            {
              dest->m_VS[i].m_bLocked = false;
              glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
            }
          }
          else
          {
            if (!dest->m_VS[i].m_bLocked)
            {
              PROFILE_FRAME(Mesh_UpdateVBuffersLock);
              dest->m_VS[i].m_bLocked = true;
              pVertices = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
              dest->m_VS[i].m_VData = pVertices;
            }
          }
        }
      }
    }
    return;
  }

  {
    PROFILE_FRAME(Mesh_UpdateVBuffers);

	  // NOTE: some subsystems need to initialize the system buffer without actually intializing its values;
	  // for that purpose, src may sometimes be NULL
    if(src && vertexcount)
    {
      assert(vertexcount<=dest->m_NumVerts);
      if(vertexcount>dest->m_NumVerts)
      {
        iLog->Log("CGLRenderer::UpdateBuffer: vertexcount>dest->m_NumVerts, %s", GetBufferComment(dest));
        return;
      }

      if (SUPPORTS_GL_ARB_vertex_buffer_object && dest->m_VS[Type].m_VertBuf.m_nID)
      {
        uint IdBuf = dest->m_VS[Type].m_VertBuf.m_nID;
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, IdBuf);
        /*if (dest->m_VS[Type].m_bLocked)
        {
          glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
          dest->m_VS[Type].m_bLocked = false;
        }*/
        if (Type == VSF_GENERAL)
        {
          glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, m_VertexSize[dest->m_vertexformat]*offs, m_VertexSize[dest->m_vertexformat]*vertexcount, src);
          m_RP.m_PS.m_MeshUpdateBytes += m_VertexSize[dest->m_vertexformat]*vertexcount;
        }
        else
        if (Type == VSF_TANGENTS)
        {
          glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sizeof(SPipTangents)*offs, sizeof(SPipTangents)*vertexcount, src);
          m_RP.m_PS.m_MeshUpdateBytes += sizeof(SPipTangents)*vertexcount;
        }
      }
      else
      {
        byte *dst = (byte *)dest->m_VS[Type].m_VData;
        if (dst)
        {
          if (Type == VSF_GENERAL)
          {
            cryMemcpy(&dst[m_VertexSize[dest->m_vertexformat]*offs],src,m_VertexSize[dest->m_vertexformat]*vertexcount);
            m_RP.m_PS.m_MeshUpdateBytes += m_VertexSize[dest->m_vertexformat]*vertexcount;
          }
          else
          if (Type == VSF_TANGENTS)
          {
            cryMemcpy(&dst[sizeof(SPipTangents)*offs],src,sizeof(SPipTangents)*vertexcount);
            m_RP.m_PS.m_MeshUpdateBytes += sizeof(SPipTangents)*vertexcount;
          }
        }
      }
    }
  }
}


#include "../Common/NvTriStrip/NVTriStrip.h"

///////////////////////////////////////////
void CGLRenderer::DrawBuffer(CVertexBuffer * src, SVertexStream *indicies,int numindices, int offsindex, int prmode,int vert_start,int vert_num, CMatInfo *mi)
{
  PROFILE_FRAME(Draw_IndexMesh);

  if (!numindices && !mi)
    return;

  assert(indicies || mi);
  ushort *pInds;
  if (SUPPORTS_GL_ARB_vertex_buffer_object)
    pInds = NULL;
  else
    pInds = (ushort *)indicies->m_VData;
  pInds += offsindex;

#ifdef _DEBUG
  if (!SUPPORTS_GL_ARB_vertex_buffer_object)
  {
	  assert(src);
    if (indicies)
    {
      for(int i=0; i<numindices; i++)
      {
        int id = pInds[i];
        assert(id < src->m_NumVerts);
      }
    }
  }
#endif // _DEBUG

	if(!src || src->m_vertexformat<0 || src->m_vertexformat>=VERTEX_FORMAT_NUMS)
	{
		iLog->Log("Error: CGLRenderer::DrawBuffer: VertexBuffer is NULL (!src || src->m_vertexformat<0 || src->m_vertexformat>5)");
		return;
	}
  
  assert(numindices>0);
  SBufInfoTable *pOffs = &gBufInfoTable[src->m_vertexformat];

  switch(src->m_vertexformat)
  {
    case VERTEX_FORMAT_TRP3F_COL4UB_TEX2F:
      { 
        struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F * pData = (struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F*)src->m_VS[VSF_GENERAL].m_VData;

        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        if(!(m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
          glEnableClientState(GL_COLOR_ARRAY);
        if (SUPPORTS_GL_ARB_vertex_buffer_object)
        {
          glBindBufferARB(GL_ARRAY_BUFFER_ARB, src->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
          glVertexPointer(3, GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(0));

          if(!(m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
            glColorPointer(4, GL_UNSIGNED_BYTE, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(pOffs->OffsColor));

          glTexCoordPointer(2, GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(pOffs->OffsTC));
          glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indicies->m_VertBuf.m_nID);
        }
        else
        {
          glVertexPointer(3,GL_FLOAT,m_VertexSize[src->m_vertexformat],&pData->x);          

          if(!(m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
            glColorPointer(4,GL_UNSIGNED_BYTE,m_VertexSize[src->m_vertexformat],&pData->color.bcolor[0]);          

          glTexCoordPointer(2,GL_FLOAT,m_VertexSize[src->m_vertexformat],&pData->st[0]);         
        }
      }
      break;
    case VERTEX_FORMAT_P3F:
      { 
        if (SUPPORTS_GL_ARB_vertex_buffer_object)
        {
          glBindBufferARB(GL_ARRAY_BUFFER_ARB, src->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
          glVertexPointer(3, GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(0));          
          glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indicies->m_VertBuf.m_nID);
        }
        else
          glVertexPointer(3, GL_FLOAT, m_VertexSize[src->m_vertexformat], src->m_VS[VSF_GENERAL].m_VData);    
        glEnableClientState(GL_VERTEX_ARRAY);
      }   
      break;
    case VERTEX_FORMAT_P3F_TEX2F:
      { 
        struct_VERTEX_FORMAT_P3F_TEX2F * pData = (struct_VERTEX_FORMAT_P3F_TEX2F*)src->m_VS[VSF_GENERAL].m_VData;

        glClientActiveTextureARB(GL_TEXTURE0_ARB);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        if (SUPPORTS_GL_ARB_vertex_buffer_object)
        {
          glBindBufferARB(GL_ARRAY_BUFFER_ARB, src->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
          glVertexPointer(3, GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(0));          
          glTexCoordPointer(2, GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(pOffs->OffsTC));
          glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indicies->m_VertBuf.m_nID);
        }
        else
        {
          glVertexPointer(3,GL_FLOAT,m_VertexSize[src->m_vertexformat],&pData->xyz.x);    
          glTexCoordPointer(2,GL_FLOAT,m_VertexSize[src->m_vertexformat],&pData->st[0]);
        }
      }   
      break;
    case VERTEX_FORMAT_P3F_N_TEX2F:
      { 
        struct_VERTEX_FORMAT_P3F_N_TEX2F * pData = (struct_VERTEX_FORMAT_P3F_N_TEX2F*)src->m_VS[VSF_GENERAL].m_VData;

        glClientActiveTextureARB(GL_TEXTURE0_ARB);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        if (SUPPORTS_GL_ARB_vertex_buffer_object)
        {
          glBindBufferARB(GL_ARRAY_BUFFER_ARB, src->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
          glVertexPointer(3, GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(0));          
          glNormalPointer(GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(pOffs->OffsNormal));          
          glTexCoordPointer(2, GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(pOffs->OffsTC));
          glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indicies->m_VertBuf.m_nID);
        }
        else
        {
          glVertexPointer(3,GL_FLOAT,m_VertexSize[src->m_vertexformat],&pData->xyz.x);    
          glTexCoordPointer(2,GL_FLOAT,m_VertexSize[src->m_vertexformat],&pData->st[0]);
          glNormalPointer(GL_FLOAT, m_VertexSize[src->m_vertexformat], &pData->normal.x);          
        }
      }   
      break;
    case VERTEX_FORMAT_P3F_COL4UB_TEX2F:
      { 
        struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pData = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F*)src->m_VS[VSF_GENERAL].m_VData;

        glClientActiveTextureARB(GL_TEXTURE0_ARB);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        if(!(m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
          glEnableClientState(GL_COLOR_ARRAY);
        if (SUPPORTS_GL_ARB_vertex_buffer_object)
        {
          glBindBufferARB(GL_ARRAY_BUFFER_ARB, src->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
          glVertexPointer(3, GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(0));          
          if(!(m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
            glColorPointer(4, GL_UNSIGNED_BYTE, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(pOffs->OffsColor));
          glTexCoordPointer(2, GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(pOffs->OffsTC));
          glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indicies->m_VertBuf.m_nID);
        }
        else
        {
          glVertexPointer(3,GL_FLOAT,m_VertexSize[src->m_vertexformat],&pData->xyz.x);          
          if(!(m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
            glColorPointer(4,GL_UNSIGNED_BYTE,m_VertexSize[src->m_vertexformat],&pData->color.bcolor[0]);          
          glTexCoordPointer(2,GL_FLOAT,m_VertexSize[src->m_vertexformat],&pData->st[0]);         
        }
      }   
      break;
    case VERTEX_FORMAT_P3F_N_COL4UB_TEX2F:
      { 
        struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F * pData = (struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F*)src->m_VS[VSF_GENERAL].m_VData;

        glClientActiveTextureARB(GL_TEXTURE0_ARB);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        if(!(m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
          glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        if (SUPPORTS_GL_ARB_vertex_buffer_object)
        {
          glBindBufferARB(GL_ARRAY_BUFFER_ARB, src->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
          glVertexPointer(3, GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(0));          
          glNormalPointer(GL_FLOAT,m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(pOffs->OffsNormal));          
          if(!(m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
            glColorPointer(4, GL_UNSIGNED_BYTE, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(pOffs->OffsColor));
          glTexCoordPointer(2, GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(pOffs->OffsTC));
          glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indicies->m_VertBuf.m_nID);
        }
        else
        {
          glVertexPointer(3,GL_FLOAT,m_VertexSize[src->m_vertexformat],&pData->xyz.x);          
          glNormalPointer(GL_FLOAT,m_VertexSize[src->m_vertexformat],&pData->normal.x);          
          if(!(m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
            glColorPointer(4,GL_UNSIGNED_BYTE,m_VertexSize[src->m_vertexformat],&pData->color.bcolor[0]);          
          glTexCoordPointer(2,GL_FLOAT,m_VertexSize[src->m_vertexformat],&pData->st[0]);         
        }
      }   
      break;
    case VERTEX_FORMAT_P3F_COL4UB:
      { 
        struct_VERTEX_FORMAT_P3F_COL4UB * pData = (struct_VERTEX_FORMAT_P3F_COL4UB*)src->m_VS[VSF_GENERAL].m_VData;

        if(!(m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
          glEnableClientState(GL_COLOR_ARRAY);
        if (SUPPORTS_GL_ARB_vertex_buffer_object)
        {
          glBindBufferARB(GL_ARRAY_BUFFER_ARB, src->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
          glVertexPointer(3, GL_FLOAT, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(0));          
          if(!(m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
            glColorPointer(4, GL_UNSIGNED_BYTE, m_VertexSize[src->m_vertexformat], BUFFER_OFFSET(pOffs->OffsColor));
          glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indicies->m_VertBuf.m_nID);
        }
        else
        {
          glVertexPointer(3,GL_FLOAT,m_VertexSize[src->m_vertexformat],&pData->xyz.x);          
        
          if(!(m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
            glColorPointer(4,GL_UNSIGNED_BYTE,m_VertexSize[src->m_vertexformat],&pData->color.bcolor[0]);
        }
      }   
      break;
    default:
      iLog->Log("Error: CGLRenderer::DrawBuffer: vertex format not supported: %d", src->m_vertexformat);
      assert(0);
      break;
  }
  // draw
  switch (prmode)
  {
    case R_PRIMV_TRIANGLES:
      {
  			glDrawElements(GL_TRIANGLES,numindices,GL_UNSIGNED_SHORT,pInds); 
        
        m_nPolygons+=numindices/3;
      }
      break;

    case R_PRIMV_TRIANGLE_STRIP:
      {		 
  			glDrawElements(GL_TRIANGLE_STRIP,numindices,GL_UNSIGNED_SHORT,pInds); 
        
        m_nPolygons+=numindices-2;
      }
      break;

    case R_PRIMV_MULTI_GROUPS:
      {
        if (mi)
        {
          int nGroups = mi->m_dwNumSections;
          SPrimitiveGroup *gr = mi->m_pPrimitiveGroups;
          if (gr)
          {
            for (int i=0; i<nGroups; i++)
            {
              SPrimitiveGroup *g = &gr[i];
              switch (g->type)
              {
                case PT_STRIP:
                  glDrawElements(GL_TRIANGLE_STRIP, g->numIndices, GL_UNSIGNED_SHORT, &pInds[g->offsIndex]);
                  break;
                  
                case PT_LIST:
                  glDrawElements(GL_TRIANGLES, g->numIndices, GL_UNSIGNED_SHORT, &pInds[g->offsIndex]);
                  break;
                  
                case PT_FAN:
                  glDrawElements(GL_TRIANGLE_FAN, g->numIndices, GL_UNSIGNED_SHORT, &pInds[g->offsIndex]);
                  break;
              }
              m_nPolygons += g->numTris;
            }
          }
        }
      }
      break;
  }
  
  if(!SUPPORTS_GL_ARB_vertex_buffer_object && IsVarPresent() && !src->m_bFenceSet)
  {
    glSetFenceNV(src->m_fence, GL_ALL_COMPLETED_NV);
    src->m_bFenceSet = true;
  }

  // disable arrays
  switch(src->m_vertexformat)
  {
    case VERTEX_FORMAT_P3F_TEX2F:
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      break;
    case VERTEX_FORMAT_P3F_COL4UB_TEX2F:
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);
      break;
    case VERTEX_FORMAT_P3F_N_COL4UB_TEX2F:
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      break;
    case VERTEX_FORMAT_P3F_N_TEX2F:
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      break;
    case VERTEX_FORMAT_P3F_COL4UB:
      glDisableClientState(GL_COLOR_ARRAY);
      break;
  }
}


void CGLRenderer::SetFenceCompleted(CVertexBuffer * buffer)
{ 
  if(IsVarPresent() && buffer)
  {
  //  if(!buffer->m_fence_set)
    {
      glSetFenceNV(buffer->m_fence, GL_ALL_COMPLETED_NV);
      buffer->m_bFenceSet = true;
    }
//    gRenDev->CheckError("SetFenceCompleted fence");
  }
}

const char * CGLRenderer::GetBufferComment(CVertexBuffer * pVertBuff)
{
  for(int i=0; pVertBuff && i<m_alloc_info.Count(); i++)
  {
    if(m_alloc_info[i].ptr == pVertBuff->m_VS[VSF_GENERAL].m_VData)
      return m_alloc_info[i].szSource;
  }

  return "Buffer is not allocated in video memory";
}

///////////////////////////////////////////
void CGLRenderer::ReleaseBuffer(CVertexBuffer *bufptr)
{
  m_nFrameCreateBuf++;
  if (bufptr)
  {
    if(IsVarPresent())
    {
      if (bufptr->m_VS[VSF_GENERAL].m_VData)
      {
        if(bufptr->m_bFenceSet && !glTestFenceNV(bufptr->m_fence))
          glFinishFenceNV(bufptr->m_fence);
        bufptr->m_bFenceSet = false;

        ReleaseVarShunk(bufptr->m_VS[VSF_GENERAL].m_VData);
        if (bufptr->m_VS[VSF_TANGENTS].m_VData)
          ReleaseVarShunk(bufptr->m_VS[VSF_TANGENTS].m_VData);
        if(bufptr->m_fence)
          glDeleteFencesNV(1, &bufptr->m_fence);
        bufptr->m_VS[VSF_GENERAL].m_VData = NULL;
        bufptr->m_VS[VSF_TANGENTS].m_VData = NULL;
        bufptr->m_fence=0;
      }
    }
    else
    if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      if (bufptr->m_VS[VSF_GENERAL].m_VertBuf.m_nID)
      {
        glDeleteBuffersARB(1, &bufptr->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
        bufptr->m_VS[VSF_GENERAL].m_VertBuf.m_nID = 0;
        m_CurVertBufferSize -= m_VertexSize[bufptr->m_vertexformat] * bufptr->m_NumVerts;
      }
      if (bufptr->m_VS[VSF_TANGENTS].m_VertBuf.m_nID)
      {
        glDeleteBuffersARB(1, &bufptr->m_VS[VSF_TANGENTS].m_VertBuf.m_nID);
        bufptr->m_VS[VSF_TANGENTS].m_VertBuf.m_nID = 0;
        m_CurVertBufferSize -= sizeof(SPipTangents) * bufptr->m_NumVerts;
      }
    }
    else
    {
      if (!bufptr->m_fence)
      {
        SAFE_DELETE_ARRAY(bufptr->m_VS[VSF_GENERAL].m_VData);
        SAFE_DELETE_ARRAY(bufptr->m_VS[VSF_TANGENTS].m_VData);
      }
    }

    delete bufptr;
  } 
}

void CGLRenderer::InitVAR()
{
  float bufSize = CLAMP(CV_gl_pip_buff_size, 4.0f, 64.0f);
  m_pip_buffer_size = (int)(bufSize*1024.0f*1024.0f);

  m_MaxVertBufferSize = m_pip_buffer_size;
  m_CurVertBufferSize = 0;
  
  float pip_allow_VAR = CV_gl_pip_allow_var;

  if(pip_allow_VAR==0)
    return;

  if (!SUPPORTS_GL_NV_vertex_array_range)
    return;

  while(1)
  { // if first try fails - try to allocate less memory
    iLog->Log("Allocating %.2f MB in %s memory ... ", 
      (float)(m_pip_buffer_size/1024/1024), 
      (pip_allow_VAR == 1.f) ? "Video" : "AGP");
    
    m_AGPbuf = (uchar*)wglAllocateMemoryNV(m_pip_buffer_size/*+100*/,
                                   0.0f,   // low read frequency
                                   0.0f,   // low write frequency
                                   (pip_allow_VAR == 1.f) ? 1.0f : 0.5f);  // high priority 1.0 = video, 0.5 = agp

    if(m_AGPbuf)
      break; // success

    iLog->Log("wglAllocateMemoryNV error");

    m_pip_buffer_size/=2;

//    if(m_pip_buffer_size<1)
      break; // error
  }

  if(!m_AGPbuf)
    iLog->Log("Error: CGLRenderer::InitVAR: wglAllocateMemoryNV error");

  if(!m_AGPbuf)
    return;

  // crash test
  ((uchar*)m_AGPbuf)[0]=0; 
  ((uchar*)m_AGPbuf)[m_pip_buffer_size-1]=0;

  iLog->LogPlus("ok");

  glVertexArrayRangeNV(m_pip_buffer_size, m_AGPbuf);    
  glEnableClientState (GL_VERTEX_ARRAY_RANGE_NV);

  // check status
  glGetIntegerv(GL_VERTEX_ARRAY_RANGE_VALID_NV, &m_var_valid);
  if(!m_var_valid) 
    iConsole->Exit("Error: CGLRenderer::InitVAR: NV_vertex_array_range has invalid state");
  m_alloc_info.Free();
}

void CGLRenderer::ShutDownVAR()
{
  if(m_AGPbuf)
    wglFreeMemoryNV(m_AGPbuf); 
  m_AGPbuf=0;
}

void * CGLRenderer::AllocateVarShunk(int bytes_count, const char *szSource)
{
  assert(bytes_count);

  int best_i = -1;
  int min_size = 10000000;

  // find best chunk
  for(int i=0; i<m_alloc_info.Count(); i++)
  if(!m_alloc_info[i].busy)
    if(m_alloc_info[i].bytes_num >= bytes_count)
      if(m_alloc_info[i].bytes_num < min_size)
  {
    best_i = i;
    min_size = m_alloc_info[i].bytes_num;
  }

  if(best_i>=0)
  { // use best free chunk
//    iLog->Log("CGLRenderer::InitVAR::AllocateBufferBytes: buffer reused"); 
    m_alloc_info[best_i].busy = true;
    m_alloc_info[best_i].szSource = szSource;

    int bytes_free = m_alloc_info[best_i].bytes_num - bytes_count;
    if(bytes_free>0)
    { 
      // modify reused shunk
      m_alloc_info[best_i].bytes_num = bytes_count;

      // insert another free shunk
      alloc_info_struct new_shunk;
      new_shunk.bytes_num = bytes_free;
      new_shunk.ptr = (uchar*)m_alloc_info[best_i].ptr + m_alloc_info[best_i].bytes_num;
      new_shunk.busy = false;

      if(best_i < m_alloc_info.Count()-1) // if not last
        m_alloc_info.InsertBefore(new_shunk, best_i+1);
      else
        m_alloc_info.Add(new_shunk);
    }

    return m_alloc_info[best_i].ptr;
  }

  void * res_ptr = 0;

  int piplevel = GetPipWaterLevel();
  if(GetPipWaterLevel() + bytes_count + 100 >= m_pip_buffer_size)
  { 
    return NULL;
    iConsole->Exit("Error: Pipline buffer overflow.");// Please increase pip_buffer_size value in engine.ini file and restart");
/*    ptr = (void *)new unsigned char [bytes_count];
    iLog->Log("Error: pip buffer overflow");
    iLog->Log("  Please close app and increase pip_buffer_size value in ini file, sorry :)");
    glDisableClientState (GL_VERTEX_ARRAY_RANGE_NV);*/
  }
  else
  {
    res_ptr = &m_AGPbuf[GetPipWaterLevel()];
  }

  // register new chunk
  alloc_info_struct ai;
  ai.ptr = res_ptr;
  ai.szSource = szSource;
  ai.bytes_num  = bytes_count;
  ai.busy = true;
  m_alloc_info.Add(ai);

//  iLog->LogToFile("New VAR chunk allocated: %d bytes, wl is %d", bytes_count, GetPipWaterLevel());

  return res_ptr;
}

BOOL CGLRenderer::ReleaseVarShunk(void * p)
{
  for(int i=0; i<m_alloc_info.Count(); i++)
  if(m_alloc_info[i].ptr == p)
  {
    m_alloc_info[i].busy = false;

    // delete info about last unused shunks
    while(m_alloc_info.Count() && m_alloc_info.Last().busy == false)
      m_alloc_info.Delete(m_alloc_info.Count()-1);

    // merge unused shunks
    for(int s=0; s<m_alloc_info.Count()-1; s++)
    {
      assert(m_alloc_info[s].ptr < m_alloc_info[s+1].ptr);

      if(m_alloc_info[s].busy == false)
      if(m_alloc_info[s+1].busy == false)
      {
        m_alloc_info[s].bytes_num += m_alloc_info[s+1].bytes_num;
        m_alloc_info.Delete(s+1);
        s--;
      }
    }

    return TRUE;
  }

  iLog->Log("Error: CGLRenderer::ReleaseVarShunk::ReleasePointer: warninig: pointer not found");

  return FALSE;
}

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4311 )						// I think the use of int's below is okay
#endif

int CGLRenderer::GetPipWaterLevel()
{
  if(m_alloc_info.Count())
    return ((int)m_alloc_info.Last().ptr - (int)m_alloc_info[0].ptr) + m_alloc_info.Last().bytes_num;

  return 0;
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

///////////////////////////////////////////
void CGLRenderer::DrawTriStrip(CVertexBuffer *src, int vert_num)
{ 
  switch (src->m_vertexformat)
  {
    case VERTEX_FORMAT_P3F: 
      {       
        struct_VERTEX_FORMAT_P3F * p = (struct_VERTEX_FORMAT_P3F*)src->m_VS[VSF_GENERAL].m_VData;
        
        glBegin(GL_TRIANGLE_STRIP);
        
        for (int k=0;k<vert_num;k++)    
        {
          glVertex3fv(&p[k].xyz.x);     
        }
        
        glEnd();
      }
      break;
  
    case VERTEX_FORMAT_P3F_TEX2F: 
      {       
        struct_VERTEX_FORMAT_P3F_TEX2F * p = (struct_VERTEX_FORMAT_P3F_TEX2F*)src->m_VS[VSF_GENERAL].m_VData;
        
        glBegin(GL_TRIANGLE_STRIP);
        
        for (int k=0;k<vert_num;k++)    
        {
          glTexCoord2fv(&p[k].st[0]);
          glVertex3fv(&p[k].xyz.x);     
        }
        
        glEnd();
      }
      break;

    case VERTEX_FORMAT_P3F_COL4UB:
      {       
        EF_SetVertColor();
        struct_VERTEX_FORMAT_P3F_COL4UB * p = (struct_VERTEX_FORMAT_P3F_COL4UB*)src->m_VS[VSF_GENERAL].m_VData;
        
        glBegin(GL_TRIANGLE_STRIP);
        
        for (int k=0;k<vert_num;k++)    
        {
          glColor4ubv(&p[k].color.bcolor[0]);
          glVertex3fv(&p[k].xyz.x);     
        }
        
        glEnd();
      }
      break;

    case VERTEX_FORMAT_P3F_COL4UB_TEX2F:
      {       
        EF_SetVertColor();
        struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * p = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F*)src->m_VS[VSF_GENERAL].m_VData;
        
        glBegin(GL_TRIANGLE_STRIP);
        
        for (int k=0;k<vert_num;k++)    
        {
          glTexCoord2fv(&p[k].st[0]);
          glColor4ubv(&p[k].color.bcolor[0]);
          glVertex3fv(&p[k].xyz.x);     
        }
        
        glEnd();
      }
      break;

    case VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F:
      {       
        struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F * p = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F*)src->m_VS[VSF_GENERAL].m_VData;

        glBegin(GL_TRIANGLE_STRIP);

        for (int k=0;k<vert_num;k++)    
        {
          glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, &p[k].st0[0]);
          glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, &p[k].st1[0]);
          glColor4ubv(&p[k].color.bcolor[0]);
          glVertex3fv(&p[k].xyz.x);     
        }

        glEnd();
      }
      break;

    case VERTEX_FORMAT_TRP3F_COL4UB_TEX2F:
      {       
        struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F * p = (struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F*)src->m_VS[VSF_GENERAL].m_VData;
        
        glBegin(GL_TRIANGLE_STRIP);
        
        for (int k=0;k<vert_num;k++)    
        {
          glTexCoord2fv(&p[k].st[0]);
          glColor4ubv(&p[k].color.bcolor[0]);
          glVertex3fv(&p[k].x);     
        }
        
        glEnd();
      }
      break;

    default:
      assert(0);
      break;
  }
  m_nPolygons += vert_num-2;
}

//========================================================================

struct SLogVBuf
{
  int nMemory;
  const char *szSource;
};

int LogVCallbackName( const VOID* arg1, const VOID* arg2 )
{
  SLogVBuf *pi1 = (SLogVBuf *)arg1;
  SLogVBuf *pi2 = (SLogVBuf *)arg2;
  int n = strcmp(pi1->szSource, pi2->szSource);
  if (n > 0)
    return -1;
  if (n < 0)
    return 1;
  return 0;
}

int LogVCallbackSize( const VOID* arg1, const VOID* arg2 )
{
  SLogVBuf *pi1 = (SLogVBuf *)arg1;
  SLogVBuf *pi2 = (SLogVBuf *)arg2;
  if (pi1->nMemory > pi2->nMemory)
    return -1;
  if (pi1->nMemory < pi2->nMemory)
    return 1;
  return 0;
}


void CGLRenderer::GenerateVBLog(const char *szName)
{
  SLogVBuf LV;
  TArray<SLogVBuf> LVs;
  for(int i=0; i<m_alloc_info.Count(); i++)
  {
    if(m_alloc_info[i].busy)
    {
      LV.nMemory = m_alloc_info[i].bytes_num;
      LV.szSource = m_alloc_info[i].szSource;
      LVs.AddElem(LV);
    }
  }
  if (CV_r_logVBuffers == 1)
    qsort(&LVs[0], LVs.Num(), sizeof(SLogVBuf), LogVCallbackName );
  else
    qsort(&LVs[0], LVs.Num(), sizeof(SLogVBuf), LogVCallbackSize );

  FILE *fp = fxopen(szName, "w");
  if (!fp)
    return;

  fprintf(fp, "*** All allocated VBuffers: ***\n");
  int Size = 0;
  int SizeLB = 0;
  int SizeGR = 0;
  const char *szLast = NULL;
  for (i=0; i<LVs.Num(); i++)
  {
    if (!szLast)
      szLast = LVs[i].szSource;
    else
    if (strcmp(szLast, LVs[i].szSource))
    {
      fprintf(fp, "%0.3fMb\t\tSource: %s\n", SizeGR/1024.0f/1024.0f, szLast);
      SizeGR = 0;
      szLast = LVs[i].szSource;
    }
    Size += LVs[i].nMemory;
    SizeGR += LVs[i].nMemory;
    if (!strncmp(LVs[i].szSource, "LeafBuffer", 10))
      SizeLB += LVs[i].nMemory;
  }
  fprintf(fp, "*** Total Size: %0.3fMb\n", (float)Size/1024.0f/1024.0f);
  fprintf(fp, "*** Total LB Size: %0.3fMb\n\n", (float)SizeLB/1024.0f/1024.0f);
  fclose(fp);
}

void CLeafBuffer::DrawImmediately()
{						
  // get position offset and stride
	int nPosStride = 0, nTexStride = 0, nColStride = 0;
  byte * pPos  = GetPosPtr	(nPosStride, 0, true);
  byte * pTex  = GetUVPtr		(nTexStride, 0, true);
  byte * pCol  = GetColorPtr(nColStride, 0, true);

  int nInds;
  ushort *pInds = GetIndices(&nInds);
	assert(nInds%3 == 0);

	glBegin(GL_TRIANGLES);
	for(int i=0; i<nInds; i+=3)
	{
		// get tri vertices
		Vec3d v0 = *((Vec3d*)&pPos[nPosStride*pInds[i+0]]);
		Vec3d v1 = *((Vec3d*)&pPos[nPosStride*pInds[i+1]]);
		Vec3d v2 = *((Vec3d*)&pPos[nPosStride*pInds[i+2]]);

		float * t0 = ((float*)&pTex[nTexStride*pInds[i+0]]);
		float * t1 = ((float*)&pTex[nTexStride*pInds[i+1]]);
		float * t2 = ((float*)&pTex[nTexStride*pInds[i+2]]);

		byte * c0 = ((byte*)&pCol[nColStride*pInds[i+0]]);
		byte * c1 = ((byte*)&pCol[nColStride*pInds[i+1]]);
		byte * c2 = ((byte*)&pCol[nColStride*pInds[i+2]]);

		glColor4ubv(c0);
		glTexCoord2fv(t0);
		glVertex3fv((float*)&v0);

		glColor4ubv(c1);
		glTexCoord2fv(t1);
		glVertex3fv((float*)&v1);

		glColor4ubv(c2);
		glTexCoord2fv(t2);
		glVertex3fv((float*)&v2);

		gRenDev->m_nPolygons++;
	}
	glEnd();
}
