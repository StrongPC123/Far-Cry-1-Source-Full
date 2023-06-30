/*=============================================================================
  D3DTexturesStreaming.cpp : Direct3D9 vertex/index buffers management.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"

//===============================================================================

#ifdef _DEBUG
TArray<SDynVB> gDVB;
#endif

//===============================================================================

void *CVertexBuffer::GetStream(int nStream, int *nOffs)
{
  if (m_VS[nStream].m_pPool)
  {
    if (nOffs)
      *nOffs = m_VS[nStream].m_nBufOffset;
    return m_VS[nStream].m_pPool->m_pVB;
  }
  else
  {
    if (nOffs)
      *nOffs = 0;
    return m_VS[nStream].m_VertBuf.m_pPtr;
  }
}

void CD3D9Renderer::DrawDynVB(int nOffs, int Pool, int nVerts)
{
	PROFILE_FRAME(Draw_IndexMesh);

  if (!m_SceneRecurseCount)
  {
    iLog->Log("Error: CD3D9Renderer::DrawDynVB without BeginScene\n");
    return;
  }
  if (CV_d3d9_forcesoftware)
    return;
  if (!nVerts)
    return;
  if (m_bDeviceLost)
    return;

  HRESULT h;

  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pDst = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(nVerts, nOffs);
  //assert(pDst);
  if (!pDst)
    return;

  if (m_TempDynVB)
  {
    cryMemcpy(pDst, m_TempDynVB, nVerts*sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    SAFE_DELETE_ARRAY(m_TempDynVB);
  }
  else
    cryMemcpy(pDst, m_DynVB, nVerts*sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));

  m_pVB3D[0]->Unlock();

  // Bind our vertex as the first data stream of our device
  h = m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);
  // Render the two triangles from the data stream
  h = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, nOffs, nVerts/3);
}


void CD3D9Renderer::DrawDynVB(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pBuf, ushort *pInds, int nVerts, int nInds, int nPrimType)
{
	PROFILE_FRAME(Draw_IndexMesh);

  if (!pBuf)
    return;

  if (!m_SceneRecurseCount)
  {
    iLog->Log("Error: CD3D9Renderer::DrawDynVB without BeginScene\n");
    return;
  }
  if (CV_d3d9_forcesoftware)
    return;
  if (!nVerts || !nInds)
    return;
  if (FAILED(m_pd3dDevice->TestCooperativeLevel()))
    return;

  HRESULT h;

  int nOffs, nIOffs;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pDst = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(nVerts, nOffs);
  assert(pDst);
  if (!pDst)
    return;

  ushort *pDstInds = NULL;
  if (pInds)
    pDstInds = GetIBPtr(nInds, nIOffs);
  cryMemcpy(pDst, pBuf, nVerts*sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  if (pDstInds)
    cryMemcpy(pDstInds, pInds, nInds*sizeof(short));

  m_pVB3D[0]->Unlock();
  if (pDstInds)
    m_pIB->Unlock();

  // Bind our vertex as the first data stream of our device
  h = m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);
  // Render triangles from the data stream
  if (pDstInds)
  {
    h = m_pd3dDevice->SetIndices((IDirect3DIndexBuffer9 *)m_pIB);
    h = m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, nOffs, 0, nVerts, nIOffs, nInds/3);
  }
  else
  {
    h = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, nOffs, nVerts/3);
  }
}

//===============================================================================

void CD3D9Renderer::CreateIndexBuffer(SVertexStream *dest,const void *src,int indexcount)
{
  WaitForDevice();

  if (dest->m_VertBuf.m_pPtr)
  {
    IDirect3DIndexBuffer9 *pIndBuf = (IDirect3DIndexBuffer9 *)dest->m_VertBuf.m_pPtr;
#ifdef _DEBUG
    sRemoveVB(pIndBuf, dest);
#endif
    SAFE_RELEASE(pIndBuf);
  }

  dest->m_nItems = 0;
  if (indexcount)
  {
    IDirect3DIndexBuffer9 *ibuf=NULL;
    int size = indexcount*sizeof(ushort);
    int flags = D3DUSAGE_WRITEONLY;
    D3DPOOL Pool = D3DPOOL_MANAGED;
    if (dest->m_bDynamic)
    {
      flags |= D3DUSAGE_DYNAMIC;
      Pool = D3DPOOL_DEFAULT;
    }
    HRESULT hReturn = m_pd3dDevice->CreateIndexBuffer(size, flags, D3DFMT_INDEX16, Pool, &ibuf, NULL);

#ifdef _DEBUG
    sAddVB(ibuf, dest, NULL, "Index buf");
#endif

    if (FAILED(hReturn))
    {
      iLog->Log("Failed to create index buffer\n");
      return;
    }
    dest->m_VertBuf.m_pPtr = ibuf;
    dest->m_nItems = indexcount;
  }
  if (src && indexcount)
    UpdateIndexBuffer(dest, src, indexcount, true);
}
void CD3D9Renderer::UpdateIndexBuffer(SVertexStream *dest,const void *src,int indexcount, bool bUnLock)
{
  PROFILE_FRAME(Mesh_UpdateIBuffers);

  if (m_bDeviceLost)
    return;

  HRESULT hReturn;
  IDirect3DIndexBuffer9 *ibuf;
  if (src && indexcount)
  {
    if (dest->m_nItems < indexcount)
    {
      if (dest->m_nItems)
        ReleaseIndexBuffer(dest);
      CreateIndexBuffer(dest, NULL, indexcount);
    }
    ushort *dst;
    ibuf = (IDirect3DIndexBuffer9 *)dest->m_VertBuf.m_pPtr;
    {
      PROFILE_FRAME(Mesh_UpdateIBuffersLock);
      hReturn = ibuf->Lock(0, 0, (void **) &dst, dest->m_bDynamic ? D3DLOCK_DISCARD : 0);
    }
    int size = indexcount*sizeof(ushort);
    {
      PROFILE_FRAME(Mesh_UpdateIBuffersCopy);
      cryMemcpy(dst, src, size);
    }
    dest->m_VData = dst;
    //hReturn = m_pd3dDevice->TestCooperativeLevel();

    if (bUnLock)
    {
      hReturn = ibuf->Unlock();
      dest->m_bLocked = false;
    }
    else
      dest->m_bLocked = true;
  }
  else
  if (dest->m_VertBuf.m_pPtr)
  {
    if (bUnLock && dest->m_bLocked)
    {
      ibuf = (IDirect3DIndexBuffer9 *)dest->m_VertBuf.m_pPtr;
      hReturn = ibuf->Unlock();
      dest->m_bLocked = false;
    }
    else
    if (!bUnLock && !dest->m_bLocked)
    {
      PROFILE_FRAME(Mesh_UpdateIBuffersLock);
      ibuf = (IDirect3DIndexBuffer9 *)dest->m_VertBuf.m_pPtr;
      ushort *dst;
      hReturn = ibuf->Lock(0, 0, (void **) &dst, dest->m_bDynamic ? D3DLOCK_DISCARD : 0);
      dest->m_bLocked = true;
      dest->m_VData = dst;
    }
  }
}
void CD3D9Renderer::ReleaseIndexBuffer(SVertexStream *dest)
{
  IDirect3DIndexBuffer9 *ibuf = (IDirect3DIndexBuffer9 *)dest->m_VertBuf.m_pPtr;
#ifdef _DEBUG
  if (ibuf)
    sRemoveVB(ibuf, dest);
#endif
  SAFE_RELEASE(ibuf);
  dest->Reset();
}

void *CD3D9Renderer::GetDynVBPtr(int nVerts, int &nOffs, int Pool)
{
  void *vBuf = NULL;
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
      vBuf = GetVBPtr3D(nVerts, nOffs, Pool);
  	  break;
  }
  return vBuf;
}

#ifdef _DEBUG
static char *szDescBuf[] = 
{
  "Base Mesh",
  "Mesh Tangents",
  "Mesh LM",
};
#endif

bool CD3D9Renderer::AllocateVBChunk(int bytes_count, TVertPool *Ptr, SVertexStream *pVB, const char *szSource)
{
  assert(bytes_count);

  int best_i = -1;
  int min_size = 10000000;

  // find best chunk
  for(int i=0; i<Ptr->m_alloc_info.Count(); i++)
  {
    if(!Ptr->m_alloc_info[i].busy)
    {
      if(Ptr->m_alloc_info[i].bytes_num >= bytes_count)
      {
        if(Ptr->m_alloc_info[i].bytes_num < min_size)
        {
          best_i = i;
          min_size = Ptr->m_alloc_info[i].bytes_num;
        }
      }
    }
  }

  if(best_i>=0)
  { // use best free chunk
    Ptr->m_alloc_info[best_i].busy = true;
    Ptr->m_alloc_info[best_i].szSource = szSource;

    int bytes_free = Ptr->m_alloc_info[best_i].bytes_num - bytes_count;
    if(bytes_free>0)
    { 
      // modify reused shunk
      Ptr->m_alloc_info[best_i].bytes_num = bytes_count;

      // insert another free shunk
      alloc_info_struct new_shunk;
      new_shunk.bytes_num = bytes_free;
      new_shunk.ptr = Ptr->m_alloc_info[best_i].ptr + Ptr->m_alloc_info[best_i].bytes_num;
      new_shunk.busy = false;

      if(best_i < Ptr->m_alloc_info.Count()-1) // if not last
        Ptr->m_alloc_info.InsertBefore(new_shunk, best_i+1);
      else
        Ptr->m_alloc_info.Add(new_shunk);
    }

    pVB->m_nBufOffset = Ptr->m_alloc_info[best_i].ptr;
    pVB->m_pPool = (SVertPool *)Ptr;

    return true;
  }

  int res_ptr = 0;

  int piplevel = Ptr->m_alloc_info.Count() ? (Ptr->m_alloc_info.Last().ptr - Ptr->m_alloc_info[0].ptr) + Ptr->m_alloc_info.Last().bytes_num : 0;
  if(piplevel + bytes_count + 100 >= Ptr->m_nBufSize)
  { 
    return false;
  }
  else
  {
    res_ptr = piplevel;
  }

  // register new chunk
  alloc_info_struct ai;
  ai.ptr = res_ptr;
  ai.szSource = szSource;
  ai.bytes_num  = bytes_count;
  ai.busy = true;
  Ptr->m_alloc_info.Add(ai);

  pVB->m_nBufOffset = res_ptr;
  pVB->m_pPool = (SVertPool *)Ptr;

  return true;
}

bool CD3D9Renderer::ReleaseVBChunk(TVertPool *Ptr, SVertexStream *pVB)
{
  int p = pVB->m_nBufOffset;
  for(int i=0; i<Ptr->m_alloc_info.Count(); i++)
  {
    if(Ptr->m_alloc_info[i].ptr == p)
    {
      Ptr->m_alloc_info[i].busy = false;

      // delete info about last unused shunks
      while(Ptr->m_alloc_info.Count() && Ptr->m_alloc_info.Last().busy == false)
        Ptr->m_alloc_info.Delete(Ptr->m_alloc_info.Count()-1);

      // merge unused shunks
      for(int s=0; s<Ptr->m_alloc_info.Count()-1; s++)
      {
        assert(Ptr->m_alloc_info[s].ptr < Ptr->m_alloc_info[s+1].ptr);

        if(Ptr->m_alloc_info[s].busy == false)
        {
          if(Ptr->m_alloc_info[s+1].busy == false)
          {
            Ptr->m_alloc_info[s].bytes_num += Ptr->m_alloc_info[s+1].bytes_num;
            Ptr->m_alloc_info.Delete(s+1);
            s--;
          }
        }
      }

      return Ptr->m_alloc_info.Count() ? false : true;
    }
  }
  iLog->Log("Error: CD3D9Renderer::ReleaseVBChunk::ReleasePointer: pointer not found");

  return false;
}

void CD3D9Renderer::AllocVBInPool(int size, int nVFormat, SVertexStream *pVB)
{
  CD3D9Renderer *rd = gcpRendD3D;

  assert(nVFormat>=0 && nVFormat<VERTEX_FORMAT_NUMS);

  int Flags = D3DUSAGE_WRITEONLY;
  D3DPOOL Pool = D3DPOOL_MANAGED;
  int fvf = 0;
  int VBsize = CV_d3d9_vbpoolsize;

  TVertPool* Ptr;
  for(Ptr=sVertPools; Ptr; Ptr=Ptr->Next)
  {
    if (AllocateVBChunk(size, Ptr, pVB, NULL))
      break;
  }
  if (!Ptr)
  {
    Ptr = new TVertPool;
    Ptr->m_nBufSize = VBsize;
    Ptr->m_pVB = NULL;
    Ptr->Next = sVertPools;
    Ptr->PrevLink = &sVertPools;
    if (sVertPools)
    {
      assert(sVertPools->PrevLink == &sVertPools);
      sVertPools->PrevLink = &Ptr->Next;
    }
    sVertPools = Ptr;
    AllocateVBChunk(size, Ptr, pVB, NULL);
  }
  if (!Ptr->m_pVB)
    rd->m_pd3dDevice->CreateVertexBuffer(VBsize, Flags, fvf, Pool, &Ptr->m_pVB, NULL);
}

void CD3D9Renderer::CreateBuffer(int size, int vertexformat, CVertexBuffer *buf, int Type, const char *szSource)
{
  PROFILE_FRAME(Mesh_CreateVBuffers);

  WaitForDevice();

  if (CV_d3d9_vbpools && size < CV_d3d9_vbpoolsize && !buf->m_bDynamic)
  {
    AllocVBInPool(size, vertexformat, &buf->m_VS[Type]);
    return;
  }

  IDirect3DVertexBuffer9 *vptr = NULL;
  int fvf = m_RP.m_D3DFixedPipeline[Type][vertexformat].m_Handle;

  int Flags = D3DUSAGE_WRITEONLY;
  D3DPOOL Pool = D3DPOOL_MANAGED;
  if (buf->m_bDynamic)
  {
    Flags |= D3DUSAGE_DYNAMIC;
    Pool = D3DPOOL_DEFAULT;
  }

  HRESULT hReturn = m_pd3dDevice->CreateVertexBuffer(size, Flags, fvf, Pool, &vptr, NULL);

  if (FAILED(hReturn))
    return;

  buf->m_VS[Type].m_bDynamic = buf->m_bDynamic;

#ifdef _DEBUG
    sAddVB(vptr, &buf->m_VS[Type], buf, szDescBuf[Type]);
#endif

  void *dst;
  buf->m_VS[Type].m_VertBuf.m_pPtr = vptr;
  hReturn = vptr->Lock(0, 0, (void **) &dst, buf->m_bDynamic ? D3DLOCK_NOOVERWRITE : 0);
  buf->m_VS[Type].m_VData = dst;
  hReturn = vptr->Unlock();

  m_CurVertBufferSize += size;
}

CVertexBuffer *CD3D9Renderer::CreateBuffer(int vertexcount,int vertexformat, const char *szSource, bool bDynamic)
{
  PROFILE_FRAME(Mesh_CreateVBuffers);

  WaitForDevice();

  IDirect3DVertexBuffer9 *vptr = NULL;
  int fvf = m_RP.m_D3DFixedPipeline[0][vertexformat].m_Handle;

  int Flags = D3DUSAGE_WRITEONLY;
  D3DPOOL Pool = D3DPOOL_MANAGED;
  if (bDynamic)
  {
    Flags |= D3DUSAGE_DYNAMIC;
    Pool = D3DPOOL_DEFAULT;
  }
  int size = m_VertexSize[vertexformat]*vertexcount;
  if (size+m_CurVertBufferSize > m_MaxVertBufferSize)
  {
    CLeafBuffer *pLB = CLeafBuffer::m_Root.m_Prev;
    while (size+m_CurVertBufferSize > m_MaxVertBufferSize)
    {
      if (pLB == &CLeafBuffer::m_Root)
        iConsole->Exit("Error: Pipeline buffer overflow. Current geometry cannot fit in video memory (%s)", gRenDev->GetStatusText(eRS_VidBuffer));
      
      CLeafBuffer *Next = pLB->m_Prev;
      pLB->Unload();
      pLB = Next;
    }
  }

  m_CurVertBufferSize += m_VertexSize[vertexformat]*vertexcount;

  CVertexBuffer *newbuf = new CVertexBuffer;
  newbuf->m_bDynamic = bDynamic;
  newbuf->m_VS[VSF_GENERAL].m_bDynamic = bDynamic;
  newbuf->m_VS[VSF_GENERAL].m_bLocked = false;
  newbuf->m_fence=0;
  newbuf->m_NumVerts = vertexcount;
  newbuf->m_vertexformat = vertexformat;

  if (CV_d3d9_vbpools && m_VertexSize[vertexformat]*vertexcount < CV_d3d9_vbpoolsize && !newbuf->m_bDynamic)
  {
    AllocVBInPool(size, vertexformat, &newbuf->m_VS[VSF_GENERAL]);
    return newbuf;
  }

  HRESULT hReturn = m_pd3dDevice->CreateVertexBuffer(m_VertexSize[vertexformat]*vertexcount, Flags, fvf, Pool, &vptr, NULL);
  if (FAILED(hReturn))
    return (NULL);
  newbuf->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr = vptr;

#ifdef _DEBUG
    sAddVB(vptr, &newbuf->m_VS[VSF_GENERAL], newbuf, szDescBuf[VSF_GENERAL]);
#endif

  return(newbuf);
}

///////////////////////////////////////////
void CD3D9Renderer::UpdateBuffer(CVertexBuffer *dest, const void *src, int vertexcount, bool bUnLock, int offs, int Type)
{
  PROFILE_FRAME(Mesh_UpdateVBuffers);

  if (m_bDeviceLost)
    return;

  VOID *pVertices;

  HRESULT hr = 0;
  IDirect3DVertexBuffer9 *tvert;
  int size;
  int nOffs = 0;
  if (!src)
  {
    if (!Type)
    {
      tvert=(IDirect3DVertexBuffer9 *)dest->GetStream(VSF_GENERAL, &nOffs);
      size = m_VertexSize[dest->m_vertexformat];
      if (bUnLock)
      {
        // video buffer update
        if (dest->m_VS[VSF_GENERAL].m_bLocked)
        {
          dest->m_VS[VSF_GENERAL].m_bLocked = false;
          hr = tvert->Unlock();
        }
      }
      else
      {
        // video buffer update
        if (!dest->m_VS[VSF_GENERAL].m_bLocked)
        {
          PROFILE_FRAME(Mesh_UpdateVBuffersLock);
          dest->m_VS[VSF_GENERAL].m_bLocked = true;
          hr=tvert->Lock(0, 0, (void **) &pVertices, dest->m_bDynamic ? D3DLOCK_DISCARD : 0);
          byte *pData = (byte *)pVertices;
          dest->m_VS[VSF_GENERAL].m_VData = &pData[nOffs];
        }
      }
    }
    else
    {
      for (int i=0; i<VSF_NUM; i++)
      {
        tvert = (IDirect3DVertexBuffer9 *)dest->GetStream(i, &nOffs);
        if (!tvert)
          continue;
        if (!((1<<i) & Type))
          continue;
        if (bUnLock)
        {
          if (dest->m_VS[i].m_bLocked)
          {
            dest->m_VS[i].m_bLocked = false;
            hr = tvert->Unlock();
          }
        }
        else
        {
          if (!dest->m_VS[i].m_bLocked)
          {
            PROFILE_FRAME(Mesh_UpdateVBuffersLock);
            dest->m_VS[i].m_bLocked = true;
            hr=tvert->Lock(0, 0, (void **) &pVertices, dest->m_bDynamic ? D3DLOCK_DISCARD : 0);
            byte *pData = (byte *)pVertices;
            dest->m_VS[VSF_GENERAL].m_VData = &pData[nOffs];
          }
        }
      }
    }
    return;
  }

  if (Type == VSF_GENERAL)
  {
    if (dest->m_VS[VSF_GENERAL].m_pPool)
    {
      tvert = dest->m_VS[VSF_GENERAL].m_pPool->m_pVB;
      nOffs = dest->m_VS[VSF_GENERAL].m_nBufOffset;
    }
    else
      tvert = (IDirect3DVertexBuffer9 *)dest->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr;
    size = m_VertexSize[dest->m_vertexformat];
  }
  else
  if (Type == VSF_TANGENTS)
  {
    if (dest->m_VS[VSF_TANGENTS].m_pPool)
    {
      tvert = dest->m_VS[VSF_TANGENTS].m_pPool->m_pVB;
      nOffs = dest->m_VS[VSF_TANGENTS].m_nBufOffset;
    }
    else
      tvert = (IDirect3DVertexBuffer9 *)dest->m_VS[VSF_TANGENTS].m_VertBuf.m_pPtr;
    size = sizeof(SPipTangents);
  }

  if (!tvert)  // system buffer update
  {
    PROFILE_FRAME(Mesh_UpdateVBuffersCopy);
    if (dest->m_bFenceSet)
      cryMemcpy(dest->m_VS[Type].m_VData, src, size*vertexcount);
    else
    if (Type == VSF_GENERAL && dest->m_VS[VSF_GENERAL].m_VData)
      cryMemcpy(dest->m_VS[VSF_GENERAL].m_VData, src, size*vertexcount);
    return;
  }

  // video buffer update
  if (!dest->m_VS[Type].m_bLocked)
  {
    PROFILE_FRAME(Mesh_UpdateVBuffersLock);
    dest->m_VS[Type].m_bLocked = true;
    hr=tvert->Lock(nOffs, size*vertexcount, (void **) &pVertices, dest->m_bDynamic ? D3DLOCK_DISCARD : 0);
    assert(!hr);
    dest->m_VS[Type].m_VData = pVertices;
  }

  if (SUCCEEDED(hr) && src)
  {
    PROFILE_FRAME(Mesh_UpdateVBuffersCopy);
    cryMemcpy(dest->m_VS[Type].m_VData, src, size*vertexcount);
    tvert->Unlock();
    dest->m_VS[Type].m_bLocked = false;
    m_RP.m_PS.m_MeshUpdateBytes += size*vertexcount;
  }
  else
  if (dest->m_VS[Type].m_bLocked && bUnLock)
  {
    tvert->Unlock();
    dest->m_VS[Type].m_bLocked = false;
  }
}

void CD3D9Renderer::UnlockBuffer(CVertexBuffer *buf, int Type)
{
  if (!buf->m_VS[Type].m_bLocked)
    return;
  if (m_bDeviceLost)
    return;

  if (buf->m_bFenceSet)
  {
    UnlockVB3D(Type+1);
    buf->m_VS[Type].m_bLocked = false;
    return;
  }

  IDirect3DVertexBuffer9 *tvert;
  tvert = (IDirect3DVertexBuffer9 *)buf->GetStream(Type, NULL);

  HRESULT hr = tvert->Unlock();
  buf->m_VS[Type].m_bLocked = false;
}


///////////////////////////////////////////
void CD3D9Renderer::ReleaseBuffer(CVertexBuffer *bufptr)
{
  if (bufptr)
  {
    m_CurVertBufferSize -= m_VertexSize[bufptr->m_vertexformat]*bufptr->m_NumVerts;
    if (bufptr->m_VS[VSF_TANGENTS].m_VertBuf.m_pPtr)
      m_CurVertBufferSize -= sizeof(SPipTangents)*bufptr->m_NumVerts;

    IDirect3DVertexBuffer9 *vtemp;
    if (bufptr->m_VS[VSF_GENERAL].m_pPool)
    {
      TVertPool *pPool = (TVertPool *)bufptr->m_VS[VSF_GENERAL].m_pPool;
      if (ReleaseVBChunk(pPool, &bufptr->m_VS[VSF_GENERAL]))
      {
        IDirect3DVertexBuffer9 *vtemp = pPool->m_pVB;
        SAFE_RELEASE(vtemp);
        pPool->Unlink();
        delete pPool;
      }
    }
    else
    {
      vtemp = (IDirect3DVertexBuffer9 *)bufptr->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr;
#ifdef _DEBUG
      if (vtemp)
        sRemoveVB(vtemp, &bufptr->m_VS[VSF_GENERAL]);
#endif
      SAFE_RELEASE(vtemp);
      bufptr->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr = NULL;
    }

    if (bufptr->m_VS[VSF_TANGENTS].m_pPool)
    {
      TVertPool *pPool = (TVertPool *)bufptr->m_VS[VSF_TANGENTS].m_pPool;
      if (ReleaseVBChunk(pPool, &bufptr->m_VS[VSF_TANGENTS]))
      {
        IDirect3DVertexBuffer9 *vtemp = pPool->m_pVB;
        SAFE_RELEASE(vtemp);
        pPool->Unlink();
        delete pPool;
      }
    }
    else
    {
      vtemp = (IDirect3DVertexBuffer9 *)bufptr->m_VS[VSF_TANGENTS].m_VertBuf.m_pPtr;
#ifdef _DEBUG
      if (vtemp)
        sRemoveVB(vtemp, &bufptr->m_VS[VSF_TANGENTS]);
#endif
      SAFE_RELEASE(vtemp);
      bufptr->m_VS[VSF_TANGENTS].m_VertBuf.m_pPtr = NULL;
    }
     
    delete bufptr;
  }
}


#include "../Common/NvTriStrip/NVTriStrip.h"

///////////////////////////////////////////
void CD3D9Renderer::DrawBuffer(CVertexBuffer *src,SVertexStream *indicies,int numindices,int offsindex,int prmode,int vert_start,int vert_stop, CMatInfo *mi)
{
  if (m_bDeviceLost)
    return;

  if (CV_d3d9_forcesoftware)
    return;

  if (!m_SceneRecurseCount)
  {
    iLog->Log("ERROR: CD3D9Renderer::DrawBuffer before BeginScene");
    return;
  }
  if (!indicies->m_VertBuf.m_pPtr || !src)
    return;

  PROFILE_FRAME(Draw_IndexMesh);

  int size = numindices * sizeof(short);

  if (src->m_VS[VSF_GENERAL].m_bLocked)
  {
    IDirect3DVertexBuffer9 *tvert =  (IDirect3DVertexBuffer9 *)src->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr;
    tvert->Unlock();
    src->m_VS[VSF_GENERAL].m_bLocked = false;
  }
  IDirect3DIndexBuffer9 *ibuf = (IDirect3DIndexBuffer9 *)indicies->m_VertBuf.m_pPtr;
  HRESULT h = EF_SetVertexDeclaration(0, src->m_vertexformat);
  int nOffs;
  IDirect3DVertexBuffer9 *pBuf = (IDirect3DVertexBuffer9 *)src->GetStream(VSF_GENERAL, &nOffs);
  h = m_pd3dDevice->SetStreamSource( 0, pBuf, nOffs, m_VertexSize[src->m_vertexformat]);
  h = m_pd3dDevice->SetIndices(ibuf);

  int NumVerts = src->m_NumVerts;
  if (vert_stop)
    NumVerts = vert_stop;

  switch(prmode)
  {
    case R_PRIMV_TRIANGLES:
      h = m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,NumVerts,offsindex,numindices/3);
      m_nPolygons+=numindices/3;
      break;

    case R_PRIMV_TRIANGLE_STRIP:
      h = m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,0,0,NumVerts,offsindex,numindices-2);
      m_nPolygons+=numindices-2;
      break;

    case R_PRIMV_MULTI_GROUPS:
      {
        if (mi)
        {
          int offs = mi->nFirstIndexId;
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
                  if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, mi->nNumVerts, g->offsIndex+offs, g->numIndices - 2)))
                  {
                    Error("CD3D9Renderer::DrawBuffer: DrawIndexedPrimitive error", h);
                    return;
                  }
                  m_nPolygons += (g->numIndices - 2);
                  break;
                  
                case PT_LIST:
                  if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mi->nNumVerts, g->offsIndex+offs, g->numIndices / 3)))
                  {
                    Error("CD3D9Renderer::DrawBuffer: DrawIndexedPrimitive error", h);
                    return;
                  }
                  m_nPolygons += (g->numIndices / 3);
                  break;
                  
                case PT_FAN:
                  if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, mi->nNumVerts, g->offsIndex+offs, g->numIndices - 2)))
                  {
                    Error("CD3D9Renderer::DrawBuffer: DrawIndexedPrimitive error", h);
                    return;
                  }
                  m_nPolygons += (g->numIndices - 2);
                  break;
              }
            }
          }
        }
      }
      break;
  }
}

