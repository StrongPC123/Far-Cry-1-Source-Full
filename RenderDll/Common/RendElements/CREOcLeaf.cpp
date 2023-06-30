/*=============================================================================
  CREOcLeaf.cpp : implementation of OcLeaf Render Element.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#include "RenderPCH.h"
#include "../NvTriStrip/NVTriStrip.h"
#include <ICryAnimation.h>

CREOcLeaf *CREOcLeaf::m_pLastRE;

void CREOcLeaf::mfCenter(Vec3d& Pos, CCObject*pObj)
{
  if (m_Flags & FCEF_CALCCENTER)
  {
    Pos = m_Center;
    if (pObj)
      Pos += pObj->GetTranslation();
    return;
  }
  m_Flags |= FCEF_CALCCENTER;
  int i;
  CRenderer *rd = gRenDev;
  CLeafBuffer *lb = m_pBuffer;

  Pos = Vec3d(0,0,0);
  byte *pD = lb->m_pSecVertBuffer ? (byte *)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData : 0;
  if (!pD)
  {
    Vec3d Mins = m_pBuffer->m_vBoxMin;
    Vec3d Maxs = m_pBuffer->m_vBoxMax;
    Pos = (Mins + Maxs) * 0.5f;
    m_Center = Pos;
    if (pObj)
      Pos += pObj->GetTranslation();
    return;
  }
  int Stride = m_VertexSize[lb->m_pSecVertBuffer->m_vertexformat];
  pD += m_pChunk->nFirstVertId * Stride;
  for (i=0; i<m_pChunk->nNumVerts; i++, pD+=Stride)
  {
    Vec3d *p = (Vec3d *)pD;
    Pos += *p;
  }
  float f = 1.0f / (float)m_pChunk->nNumVerts;
  Pos *= f;
  m_Center = Pos;
  if (pObj)
    Pos += pObj->GetTranslation();
}

void CREOcLeaf::mfGetBBox(Vec3d& vMins, Vec3d& vMaxs)
{
  vMins = m_pBuffer->GetVertexContainer()->m_vBoxMin;
  vMaxs = m_pBuffer->GetVertexContainer()->m_vBoxMax;
}

float CREOcLeaf::mfDistanceToCameraSquared(const CCObject & thisObject)
{
  if (thisObject.m_fDistanceToCam >= 0)
    return thisObject.m_fDistanceToCam;

  CRenderer *rd = gRenDev;

  Vec3d Mins = m_pBuffer->m_vBoxMin;
  Vec3d Maxs = m_pBuffer->m_vBoxMax;
  Vec3d Center = (Mins + Maxs) * 0.5f;
  Center += thisObject.GetTranslation();
  Vec3d Delta = rd->m_RP.m_ViewOrg - Center;
  return GetLengthSquared(Delta);
}


bool CREOcLeaf::mfCullByClipPlane(CCObject *pObj)
{
  CRenderer *rd = gRenDev;

  if (rd->m_RP.m_ClipPlaneEnabled && CRenderer::CV_r_cullbyclipplanes)
  {
    Vec3d Mins = m_pBuffer->m_vBoxMin;
    Vec3d Maxs = m_pBuffer->m_vBoxMax;
    if (!(pObj->m_ObjFlags & FOB_TRANS_ROTATE))
    {
      // Non rotated bounding-box. Fast solution
      if (!(pObj->m_ObjFlags & FOB_TRANS_SCALE))
      {
        Mins += pObj->GetTranslation();
        Maxs += pObj->GetTranslation();
      }
      else
      {
        Matrix44& m = pObj->GetMatrix();
        Mins = m.TransformPointOLD(Mins);
        Maxs = m.TransformPointOLD(Maxs);
      }
    }
    else
    { 
      // General slow solution (separatelly transform all 8 points of
      // non transformed cube and calculate axis aligned bounding box)
      Vec3d v[8];
      int i;
      Matrix44& m = pObj->GetMatrix();
      for (i=0; i<8; i++)
      {
        if (i & 1)
          v[i].x = Mins.x;
        else
          v[i].x = Maxs.x;

        if (i & 2)
          v[i].y = Mins.y;
        else
          v[i].y = Maxs.y;

        if (i & 4)
          v[i].z = Mins.z;
        else
          v[i].z = Maxs.z;
        v[i] = m.TransformPointOLD(v[i]);
      }
      Mins=SetMaxBB();
      Maxs=SetMinBB();
      for (i=0; i<8; i++)
      {
        AddToBounds(v[i], Mins, Maxs);
      }
    }
    if (CullBoxByPlane(&Mins.x, &Maxs.x, &rd->m_RP.m_CurClipPlaneCull) == 2)
      return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////

//#undef DO_ASM

_inline byte *sCopyTransf_P_T(struct_VERTEX_FORMAT_P3F_TEX2F *dst, Matrix44 *mat, int nNumVerts, byte *OffsP, int nP, byte *OffsT, int nT)
{
#ifdef DO_ASM
  _asm
  {
    mov         eax, mat
    mov         ecx, nNumVerts;
    movaps      xmm2,xmmword ptr [eax]
    mov         esi, OffsP
    movaps      xmm4,xmmword ptr [eax+10h]
    mov         ebx, OffsT
    movaps      xmm6,xmmword ptr [eax+20h]
    mov         edi, dst
    movaps      xmm5,xmmword ptr [eax+30h]
align 16
_Loop2:
    prefetchT0  [esi+20]
    movlps      xmm1,qword ptr [esi]
    movss       xmm0,dword ptr [esi+8]
    shufps      xmm0,xmm0,0
    movaps      xmm3,xmm1
    mulps       xmm0,xmm6
    add         edi, 20
    shufps      xmm3,xmm1,55h
    mov         eax, [ebx]
    mulps       xmm3,xmm4
    shufps      xmm1,xmm1,0
    mov         [edi+12-20], eax
    mulps       xmm1,xmm2
    addps       xmm3,xmm1
    mov         eax, [ebx+4]
    addps       xmm3,xmm0
    add         ebx, nT
    addps       xmm3,xmm5
    add         esi, nP
    movhlps     xmm1,xmm3
    movlps      qword ptr [edi-20],xmm3
    dec         ecx
    mov         [edi+16-20], eax
    movss       dword ptr [edi+8-20],xmm1
    jne         _Loop2
    mov         dst, edi
  }
#else
  for (int i=0; i<nNumVerts; i++, OffsP+=nP, OffsT+=nT)
  {
    dst->xyz = mat->TransformPointOLD(*(Vec3d *)OffsP);
    dst->st[0] = *(float *)OffsT;
    dst->st[1] = *(float *)(OffsT+4);
    dst++;
  }
#endif
  return (byte *)dst;
}

_inline byte *sCopyTransf_P_C_T(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *dst, Matrix44 *mat, int nNumVerts, byte *OffsP, int nP, byte *OffsD, int nD, byte *OffsT, int nT)
{
#ifdef DO_ASM
  _asm
  {
    mov         eax, mat
    mov         ecx, nNumVerts;
    movaps      xmm2,xmmword ptr [eax]
    mov         esi, OffsP
    movaps      xmm4,xmmword ptr [eax+10h]
    mov         ebx, OffsT
    movaps      xmm6,xmmword ptr [eax+20h]
    mov         edi, dst
    movaps      xmm5,xmmword ptr [eax+30h]
    mov         edx, OffsD
align 16
_Loop1:
    prefetchT0  [esi+24]
    movlps      xmm1,qword ptr [esi]
    movss       xmm0,dword ptr [esi+8]
    shufps      xmm0,xmm0,0
    add         edi, 24
    movaps      xmm3,xmm1
    mulps       xmm0,xmm6
    mov         eax, [edx]
    shufps      xmm3,xmm1,55h
    mulps       xmm3,xmm4
    mov         [edi+12-24], eax
    shufps      xmm1,xmm1,0
    mulps       xmm1,xmm2
    mov         eax, [ebx]
    addps       xmm3,xmm1
    mov         [edi+16-24], eax
    addps       xmm3,xmm0
    mov         eax, [ebx+4]
    add         ebx, nT
    addps       xmm3,xmm5
    add         esi, nP
    movhlps     xmm1,xmm3
    add         edx, nD
    movlps      qword ptr [edi-24],xmm3
    dec         ecx
    mov         [edi+20-24], eax
    movss       dword ptr [edi+8-24],xmm1
    jne         _Loop1
    mov         dst, edi
  }
#else
  for (int i=0; i<nNumVerts; i++, OffsP+=nP, OffsD+=nD, OffsT+=nT)
  {
    dst->xyz = mat->TransformPointOLD(*(Vec3 *)OffsP);
    dst->color.dcolor = *(DWORD *)OffsD;
    dst->st[0] = *(float *)OffsT;
    dst->st[1] = *(float *)(OffsT+4);
    dst++;
  }
#endif
  return (byte *)dst;
}

_inline byte *sCopyTransf_P_C_C1_T(struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F *dst, Matrix44 *mat, int nNumVerts, byte *OffsP, int nP, byte *OffsD, int nD, byte *OffsT, int nT)
{
#ifdef DO_ASM
  _asm
  {
    mov         eax, mat
    mov         ecx, nNumVerts;
    movaps      xmm2,xmmword ptr [eax]
    mov         esi, OffsP
    movaps      xmm4,xmmword ptr [eax+10h]
    mov         ebx, OffsT
    movaps      xmm6,xmmword ptr [eax+20h]
    mov         edi, dst
    movaps      xmm5,xmmword ptr [eax+30h]
    mov         edx, OffsD
align 16
_Loop1:
    prefetchT0  [esi+28]
    movlps      xmm1,qword ptr [esi]
    movss       xmm0,dword ptr [esi+8]
    shufps      xmm0,xmm0,0
    add         edi, 28
    movaps      xmm3,xmm1
    mov         eax, [edx]
    mulps       xmm0,xmm6
    shufps      xmm3,xmm1,55h
    mov         [edi+12-28], eax
    mulps       xmm3,xmm4
    mov         eax, [esi+16]
    shufps      xmm1,xmm1,0
    mov         [edi+16-28], eax
    mulps       xmm1,xmm2
    mov         eax, [ebx]
    addps       xmm3,xmm1
    mov         [edi+20-28], eax
    addps       xmm3,xmm0
    mov         eax, [ebx+4]
    add         ebx, nT
    addps       xmm3,xmm5
    add         esi, nP
    movhlps     xmm1,xmm3
    add         edx, nD
    movlps      qword ptr [edi-28],xmm3
    dec         ecx
    mov         [edi+24-28], eax
    movss       dword ptr [edi+8-28],xmm1
    jne         _Loop1
    mov         dst, edi
  }
#else
  for (int i=0; i<nNumVerts; i++, OffsP+=nP, OffsD+=nD, OffsT+=nT)
  {
    dst->xyz = mat->TransformPointOLD(*(Vec3 *)OffsP);
    dst->color.dcolor = *(DWORD *)OffsD;
    dst->seccolor.dcolor = *(DWORD *)(OffsD+4);
    dst->st[0] = *(float *)OffsT;
    dst->st[1] = *(float *)(OffsT+4);
    dst++;
  }
#endif
  return (byte *)dst;
}

_inline void sCopyInds8(uint *dinds, uint *inds, int nInds8, int n)
{
  if (!nInds8)
    return;
#ifdef DO_ASM
  _asm
  {
    mov        edi, dinds
    mov        esi, inds
    mov        ecx, nInds8
    mov        eax, n
align 4
_Loop:
    prefetchT0  [esi+10h]
    mov        edx, [esi]
    add        edx, eax
    mov        [edi], edx
    mov        ebx, [esi+4]
    add        edi, 16
    add        ebx, eax
    mov        [edi+4-16], ebx
    mov        edx, [esi+8]
    add        edx, eax
    add        esi, 16
    mov        [edi+8-16], edx
    mov        ebx, [esi+12-16]
    add        ebx, eax
    dec        ecx
    mov        [edi+12-16], ebx
    jne        _Loop
  }
#else
  for (int i=0; i<nInds8; i++, dinds+=4, inds+=4)
  {
    dinds[0] = inds[0] + n;
    dinds[1] = inds[1] + n;
    dinds[2] = inds[2] + n;
    dinds[3] = inds[3] + n;
  }
#endif
}

void CREOcLeaf::mfFillRB(CCObject *pObj)
{
  int i;
  CRenderer *rd = gRenDev;
  CLeafBuffer *lb = m_pBuffer;
  CMatInfo *mi = m_pChunk;

  rd->EF_CheckOverflow(mi->nNumVerts, mi->nNumIndices/3, this);

  SShader *ef = rd->m_RP.m_pShader;
  SRenderShaderResources *Res = rd->m_RP.m_pShaderResources;
  SShaderTechnique *hs = rd->m_RP.m_pCurTechnique;
  int nFlags = hs ? hs->m_Flags : 0;

  int nDstV = rd->m_RP.m_RendNumVerts;
  int nDstI = rd->m_RP.m_RendNumIndices;

  int n = nDstV - mi->nFirstVertId;
  ushort *inds = &lb->m_SecIndices[mi->nFirstIndexId];
  ushort *dinds = &rd->m_RP.m_SysRendIndices[nDstI];
  int nInds = mi->nNumIndices;
  rd->m_RP.m_RendNumIndices += nInds;
  if (n >= 0)
  {
    int m = n | n<<16;
    int nInds8 = nInds>>3;
    sCopyInds8((uint *)dinds, (uint *)inds, nInds8, m);
    nInds &= 7;
    nInds8 <<= 3;
    dinds += nInds8;
    inds += nInds8;
  }
  for (i=0; i<nInds; i++)
  {
    dinds[i] = inds[i] + n;
  }

  UPipeVertex ptr = rd->m_RP.m_NextPtr;
  int nVFormat = rd->m_RP.m_CurVFormat;
  Matrix44 *mat = &pObj->m_Matrix;
  /*Vec3d v = pObj->GetTranslation();
  if ((v-Vec3d(810.5,1819,43.5)).Length() <= 0.9f)
  {
    int nnn = 0;
  }*/
  /*if (pObj->m_ObjFlags & FOB_SELECTED)
  {
    int nnn = 0;
  }*/
  int nNumVerts = mi->nNumVerts;
  byte *OffsD, *OffsT, *OffsN, *OffsP;
  int nD, nT, nN, nP;
  byte *pData = (byte *)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
  SBufInfoTable *pOffs = &gBufInfoTable[lb->m_pSecVertBuffer->m_vertexformat];
  int Stride = m_VertexSize[lb->m_pSecVertBuffer->m_vertexformat];
  nP = Stride;
  OffsP = &pData[mi->nFirstVertId*Stride];
  switch (nVFormat)
  {
    case VERTEX_FORMAT_P3F_COL4UB_TEX2F:
      {
        // Used mostly for vegetations
        if (pOffs->OffsColor)
        {
          OffsD = &OffsP[pOffs->OffsColor];
          nD = Stride;
        }
        else
        {
          OffsD = (byte *)&lb->m_TempColors[mi->nFirstVertId];
          nD = sizeof(UCol);
        }
        if (pOffs->OffsTC)
        {
          OffsT = &OffsP[pOffs->OffsTC];
          nT = Stride;
        }
        else
        {
          OffsT = (byte *)&lb->m_TempTexCoords[mi->nFirstVertId];
          nT = sizeof(SMRendTexVert);
        }
        struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *dst = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)ptr.Ptr;
        rd->m_RP.m_NextPtr.Ptr = sCopyTransf_P_C_T(dst, mat, nNumVerts, OffsP, nP, OffsD, nD, OffsT, nT);
      }
  	  break;
    case VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F:
      {
        // Used mostly for vegetations
        assert (pOffs->OffsColor);
        if (pOffs->OffsColor)
        {
          OffsD = &OffsP[pOffs->OffsColor];
          nD = Stride;
        }
        else
        {
          OffsD = (byte *)&lb->m_TempColors[mi->nFirstVertId];
          nD = sizeof(UCol);
        }
        if (pOffs->OffsTC)
        {
          OffsT = &OffsP[pOffs->OffsTC];
          nT = Stride;
        }
        else
        {
          OffsT = (byte *)&lb->m_TempTexCoords[mi->nFirstVertId];
          nT = sizeof(SMRendTexVert);
        }
        struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F *dst = (struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F *)ptr.Ptr;
        rd->m_RP.m_NextPtr.Ptr = sCopyTransf_P_C_C1_T(dst, mat, nNumVerts, OffsP, nP, OffsD, nD, OffsT, nT);
      }
      break;
    case VERTEX_FORMAT_P3F_TEX2F:
      {
        // Used mostly for brushes/entities
        if (pOffs->OffsTC)
        {
          OffsT = &OffsP[pOffs->OffsTC];
          nT = Stride;
        }
        else
        {
          OffsT = (byte *)&lb->m_TempTexCoords[mi->nFirstVertId];
          nT = sizeof(SMRendTexVert);
        }
        struct_VERTEX_FORMAT_P3F_TEX2F *dst = (struct_VERTEX_FORMAT_P3F_TEX2F *)ptr.Ptr;
        rd->m_RP.m_NextPtr.Ptr = sCopyTransf_P_T(dst, mat, nNumVerts, OffsP, nP, OffsT, nT);
      }
      break;
    case VERTEX_FORMAT_P3F_N:
      {
        if (pOffs->OffsNormal)
        {
          OffsN = &pData[mi->nFirstVertId*Stride+pOffs->OffsNormal];
          nN = Stride;
        }
        else
        {
          OffsN = (byte *)&lb->m_TempNormals[mi->nFirstVertId];
          nN = sizeof(Vec3);
        }
        struct_VERTEX_FORMAT_P3F_N *dst = (struct_VERTEX_FORMAT_P3F_N *)ptr.Ptr;
        for (i=0; i<mi->nNumVerts; i++, OffsP+=nP, OffsN+=nN)
        {
          dst->xyz = mat->TransformPointOLD(*(Vec3 *)OffsP);
          dst->normal = mat->TransformVectorOLD(*(Vec3 *)OffsN);
          dst++;
        }
        rd->m_RP.m_NextPtr.Ptr = dst;
      }
      break;
    case VERTEX_FORMAT_P3F_N_TEX2F:
      {
        if (pOffs->OffsTC)
        {
          OffsT = &pData[mi->nFirstVertId*Stride+pOffs->OffsTC];
          nT = Stride;
        }
        else
        {
          OffsT = (byte *)&lb->m_TempTexCoords[mi->nFirstVertId];
          nT = sizeof(SMRendTexVert);
        }
        if (pOffs->OffsNormal)
        {
          OffsN = &pData[mi->nFirstVertId*Stride+pOffs->OffsNormal];
          nN = Stride;
        }
        else
        {
          OffsN = (byte *)&lb->m_TempNormals[mi->nFirstVertId];
          nN = sizeof(Vec3);
        }
        struct_VERTEX_FORMAT_P3F_N_TEX2F *dst = (struct_VERTEX_FORMAT_P3F_N_TEX2F *)ptr.Ptr;
        for (i=0; i<mi->nNumVerts; i++, OffsP+=nP, OffsT+=nT, OffsN+=nN)
        {
          dst->xyz = mat->TransformPointOLD(*(Vec3 *)OffsP);
          dst->normal = mat->TransformVectorOLD(*(Vec3 *)OffsN);
          dst->st[0] = *(float *)OffsT;
          dst->st[1] = *(float *)(OffsT+4);
          dst++;
        }
        rd->m_RP.m_NextPtr.Ptr = dst;
      }
      break;
    case VERTEX_FORMAT_P3F_N_COL4UB_TEX2F:
      {
        if (pOffs->OffsTC)
        {
          OffsT = &pData[mi->nFirstVertId*Stride+pOffs->OffsTC];
          nT = Stride;
        }
        else
        {
          OffsT = (byte *)&lb->m_TempTexCoords[mi->nFirstVertId];
          nT = sizeof(SMRendTexVert);
        }
        if (pOffs->OffsNormal)
        {
          OffsN = &pData[mi->nFirstVertId*Stride+pOffs->OffsNormal];
          nN = Stride;
        }
        else
        {
          OffsN = (byte *)&lb->m_TempNormals[mi->nFirstVertId];
          nN = sizeof(Vec3);
        }
        if (pOffs->OffsColor)
        {
          OffsD = &OffsP[pOffs->OffsColor];
          nD = Stride;
        }
        else
        {
          OffsD = (byte *)&lb->m_TempColors[mi->nFirstVertId];
          nD = sizeof(UCol);
        }
        struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *dst = (struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *)ptr.Ptr;
        for (i=0; i<mi->nNumVerts; i++, OffsP+=nP, OffsT+=nT, OffsN+=nN, OffsD+=nD)
        {
          dst->xyz = mat->TransformPointOLD(*(Vec3 *)OffsP);
          dst->color.dcolor = *(DWORD *)OffsD;
          dst->normal = mat->TransformVectorOLD(*(Vec3 *)OffsN);
          dst->st[0] = *(float *)OffsT;
          dst->st[1] = *(float *)(OffsT+4);
          dst++;
        }
        rd->m_RP.m_NextPtr.Ptr = dst;
      }
      break;
    case VERTEX_FORMAT_P3F:
      {
        struct_VERTEX_FORMAT_P3F *dst = (struct_VERTEX_FORMAT_P3F *)ptr.Ptr;
        for (i=0; i<mi->nNumVerts; i++, OffsP+=nP)
        {
          dst->xyz = mat->TransformPointOLD(*(Vec3 *)OffsP);
          dst++;
        }
        rd->m_RP.m_NextPtr.Ptr = dst;
      }
      break;
    default:
      assert(false);
  }
  if (nFlags & (FHF_TANGENTS | FHF_LMTC))
  {
    rd->m_RP.m_MergedREs.AddElem(this);
    rd->m_RP.m_MergedObjs.AddElem(pObj);
  }

  rd->m_RP.m_ObjFlags &= ~FOB_TRANS_MASK;
  rd->m_RP.m_RendNumVerts += mi->nNumVerts;
}

void CREOcLeaf::mfPrepare()
{
  CRenderer *rd = gRenDev;

  {
    //PROFILE_FRAME_TOTAL(Mesh_REPrepare_Ocleaf);

    if (rd->m_RP.m_ClipPlaneEnabled && CRenderer::CV_r_cullbyclipplanes)
    {
      if (mfCullByClipPlane(rd->m_RP.m_pCurObject))
      {
        rd->EF_CheckOverflow(0, 0, this);
        // Completly culled by plane (nothing to do with it)
        rd->m_RP.m_pRE = NULL;
        rd->m_RP.m_RendNumIndices = 0;
        rd->m_RP.m_RendNumVerts = 0;
        return;
      }
    }

    CLeafBuffer *lb = m_pBuffer;
    CMatInfo *mi = m_pChunk;
    if (m_Flags & FCEF_MODIF_MASK)
    {
      m_Flags &= ~FCEF_MODIF_MASK;
      lb->UpdateVidVertices(lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData, lb->m_SecVertCount);
    }

    if (CRenderer::CV_r_rb_merge && !lb->m_bOnlyVideoBuffer)
    {
      if ((m_Flags & FCEF_MERGABLE) && mi->nNumVerts < abs(CRenderer::CV_r_rb_merge))
      {
        if (rd->m_RP.m_pCurObject->IsMergable())
        {
          SShader *ef = rd->m_RP.m_pShader;
          SShaderTechnique *hs = rd->m_RP.m_pCurTechnique;
          if (!hs || !(hs->m_Flags & FHF_NOMERGE))
          {
            rd->m_RP.m_FlagsPerFlush |= RBSI_MERGED;
            mfFillRB(rd->m_RP.m_pCurObject);
            return;
          }
        }
      }
    }
    rd->EF_CheckOverflow(0, 0, this);

    rd->m_RP.m_pRE = this;

    if (rd->m_RP.m_pShader->m_Flags2 & EF2_REDEPEND)
    {
      // Choose appropriate shader technique depend on some input parameters
      SShader *ef = rd->m_RP.m_pShader;
      if (ef->m_HWTechniques.Num())
      {
        int nHW = rd->EF_SelectHWTechnique(ef);
        if (nHW >= 0)
          rd->m_RP.m_pCurTechnique = ef->m_HWTechniques[nHW];
        else
          rd->m_RP.m_pCurTechnique = NULL;
      }
      else
        rd->m_RP.m_pCurTechnique = NULL;
    }

    if (lb->m_nPrimetiveType == R_PRIMV_TRIANGLE_STRIP)
    {
      rd->m_RP.m_FirstVertex = 0;
      rd->m_RP.m_FirstIndex = 0;
      rd->m_RP.m_RendNumIndices = mi->nNumIndices;
      rd->m_RP.m_RendNumVerts = mi->nNumVerts;
    }
    else
    {
      rd->m_RP.m_FirstVertex = mi->nFirstVertId;
      rd->m_RP.m_FirstIndex = mi->nFirstIndexId;
      rd->m_RP.m_RendNumIndices = mi->nNumIndices;
      rd->m_RP.m_RendNumVerts = mi->nNumVerts;
      if (lb->m_pVertexBuffer)
        rd->m_RP.m_BaseVertex = lb->m_pVertexBuffer->m_fence;
    }      
  }
}

list2<CMatInfo> *CREOcLeaf::mfGetMatInfoList()
{
  return m_pBuffer->m_pMats;
}

int CREOcLeaf::mfGetMatId()
{
  return m_pChunk->m_Id;
}

CMatInfo *CREOcLeaf::mfGetMatInfo()
{
  return m_pChunk;
}

void CREOcLeaf::mfGenerateIndicesInsideFrustrum(SLightIndicies *li, CDLight *pDLight)
{
  CCamera cam;
  int i;
  Vec3d mins, maxs;
  CLeafBuffer *lb = m_pBuffer;
  byte *pD = (byte *)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
  int Stride = m_VertexSize[lb->m_pVertexBuffer->m_vertexformat];
  Vec3d *v0, *v1, *v2;

  Vec3d objectSpaceLightPosition;
  TransformPosition(objectSpaceLightPosition, pDLight->m_Origin, gRenDev->m_RP.m_pCurObject->GetInvMatrix());
  cam.SetPos(objectSpaceLightPosition);
  Vec3d Angles(pDLight->m_ProjAngles[1], 0, pDLight->m_ProjAngles[2]+90.0f);
  cam.SetAngle(Angles);
  cam.Init(1, 1, (pDLight->m_fLightFrustumAngle*2)/180.0f*PI, 1024.0f, 1.0f, 0.0f);
  cam.Update();
  li->m_pIndiciesAttenFrustr.Free();
  if (!cam.IsAABBVisibleFast( AABB(m_pBuffer->m_vBoxMin,m_pBuffer->m_vBoxMax) ))
    return;
  for (i=0; i<li->m_pIndicies.Num(); i+=3)
  {
    mins=SetMaxBB();
    maxs=SetMinBB();
    int i0 = li->m_pIndicies[i+0];
    int i1 = li->m_pIndicies[i+1];
    int i2 = li->m_pIndicies[i+2];
    v0 = (Vec3d *)&pD[i0*Stride];
    v1 = (Vec3d *)&pD[i1*Stride];
    v2 = (Vec3d *)&pD[i2*Stride];
    AddToBounds( (*v0), mins, maxs);
    AddToBounds( (*v1), mins, maxs);
    AddToBounds( (*v2), mins, maxs);
    if (cam.IsAABBVisibleFast( AABB(mins,maxs) ))
    {
      li->m_pIndiciesAttenFrustr.AddElem(i0);
      li->m_pIndiciesAttenFrustr.AddElem(i1);
      li->m_pIndiciesAttenFrustr.AddElem(i2);
    }
  }
}

float sDistPointToSegment(Vec3d& P, Vec3d& SP0, Vec3d& SP1)
{
  Vec3d v = SP1 - SP0;
  Vec3d w = P - SP0;

  float c1 = w.Dot(v);
  if (c1 <= 0)
    return GetDistance(P,SP0);

  float c2 = v.Dot(v);
  if (c2 <= c1)
    return GetDistance(P,SP1);

  float b = c1 / c2;
  Vec3d Pb = SP0 + b * v;
  return GetDistance(P,Pb);
}

void CREOcLeaf::mfGenerateIndicesForAttenuation(SLightIndicies *li, CDLight *pDLight)
{
  TArray<unsigned short> NewInds;
  int i;
  CLeafBuffer *lb = m_pBuffer;
  byte *pD = (byte *)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
  int Stride = m_VertexSize[lb->m_pVertexBuffer->m_vertexformat];
  Vec3d *v0, *v1, *v2;
  Vec3d objectSpaceLightPosition;
  TransformPosition(objectSpaceLightPosition, pDLight->m_Origin, gRenDev->m_RP.m_pCurObject->GetInvMatrix());
  li->m_pIndiciesAttenFrustr.Free();

  for (i=0; i<li->m_pIndicies.Num(); i+=3)
  {
    int i0 = li->m_pIndicies[i+0];
    int i1 = li->m_pIndicies[i+1];
    int i2 = li->m_pIndicies[i+2];
    v0 = (Vec3d *)&pD[i0*Stride];
    v1 = (Vec3d *)&pD[i1*Stride];
    v2 = (Vec3d *)&pD[i2*Stride];
    Plane pl;
    pl.CalcPlane(*v0, *v1, *v2);
    float fDist = pl.DistFromPlane(objectSpaceLightPosition);
    Vec3d vProjPoint = objectSpaceLightPosition + pl.n * -fDist;
    if (!Overlap::PointInTriangle(vProjPoint, *v0, *v1, *v2, pl.n))
    {
      float fDist0 = sDistPointToSegment(objectSpaceLightPosition, *v0, *v1);
      float fDist1 = sDistPointToSegment(objectSpaceLightPosition, *v0, *v2);
      float fDist2 = sDistPointToSegment(objectSpaceLightPosition, *v1, *v2);
      fDist = min(fDist0, fDist1);
      fDist = min(fDist, fDist2);
    }

    if (fDist <= pDLight->m_fRadius)
    {
      li->m_pIndiciesAttenFrustr.AddElem(i0);
      li->m_pIndiciesAttenFrustr.AddElem(i1);
      li->m_pIndiciesAttenFrustr.AddElem(i2);
    }
  }
}

SLightIndicies *CREOcLeaf::mfGetIndiciesForLight(CDLight *pDLight)
{
  CLeafBuffer *lb = m_pBuffer;
  CMatInfo *mi = m_pChunk;
  int i, j;
  
  ushort *pInds = lb->GetIndices(NULL);

  if (!m_Faces || (m_Flags & FCEF_DYNAMIC))
    return &gRenDev->m_RP.m_FakeLightIndices;

  if (gRenDev->m_FS.m_bEnable)
  {
    float fDistToCam = mfMinDistanceToCamera(gRenDev->m_RP.m_pCurObject);
    if (fDistToCam > gRenDev->m_FS.m_FogStart)
      return &gRenDev->m_RP.m_FakeLightIndices;
  }

  if (!m_LIndicies)
    m_LIndicies = new TArray <SLightIndicies*>;

  Vec3d objPos, objAng;
  float fObjScale;
  objPos = gRenDev->m_RP.m_pCurObject ? gRenDev->m_RP.m_pCurObject->GetTranslation() : Vec3d(0,0,0);
  objAng = Vec3d(0,0,0);
  fObjScale = gRenDev->m_RP.m_pCurObject->GetScaleX();

  SLightIndicies *li = NULL;
  int Best = -1;
  for (i=0; i<m_LIndicies->Num(); i++)
  {
    li = m_LIndicies->Get(i);
  //  if ((li->m_AssociatedLight.m_Flags & DLF_LIGHTTYPE_MASK) == (pDLight->m_Flags & DLF_LIGHTTYPE_MASK) && li->m_AssociatedLight.m_Origin == pDLight->m_Origin && li->m_AssociatedLight.m_Orientation.m_vForward == objPos && li->m_AssociatedLight.m_Orientation.m_vRight == objAng && li->m_AssociatedLight.m_Orientation.m_vUp[0] == fObjScale)
    if ((li->m_AssociatedLight.m_Flags & DLF_LIGHTTYPE_MASK) == (pDLight->m_Flags & DLF_LIGHTTYPE_MASK) &&    IsEquivalent(li->m_AssociatedLight.m_Origin,pDLight->m_Origin)  &&    IsEquivalent(li->m_AssociatedLight.m_Orientation.m_vForward,objPos) && IsEquivalent(li->m_AssociatedLight.m_Orientation.m_vRight,objAng) && li->m_AssociatedLight.m_Orientation.m_vUp[0] == fObjScale)
    {
      if (li->m_AssociatedLight.m_NumCM)
      {
        li->m_AssociatedLight.m_Id = gRenDev->GetFrameID();
        if (pDLight->m_Flags & DLF_PROJECT)
        {
          bool bNeedList = false;

          //if (li->m_AssociatedLight.m_ProjAngles != pDLight->m_ProjAngles || li->m_AssociatedLight.m_fLightFrustumAngle != pDLight->m_fLightFrustumAngle)
          if ( !IsEquivalent(li->m_AssociatedLight.m_ProjAngles,pDLight->m_ProjAngles) || li->m_AssociatedLight.m_fLightFrustumAngle != pDLight->m_fLightFrustumAngle)
          {
            li->m_AssociatedLight.m_ProjAngles = pDLight->m_ProjAngles;
            li->m_AssociatedLight.m_fLightFrustumAngle = pDLight->m_fLightFrustumAngle;
            li->m_AssociatedLight.m_fLifeTime = gRenDev->m_RP.m_RealTime;
            if (CRenderer::CV_r_cullgeometryforlights != 2)
            {
              li->m_AssociatedLight.m_NumCM &= ~2;
              Best = i;
              break;
            }
          }
          // if static already
          if (li->m_AssociatedLight.m_fLifeTime+1.0f < gRenDev->m_RP.m_RealTime)
          {
            if (!(li->m_AssociatedLight.m_NumCM & 2))
              bNeedList = true;
          }
          else
          if (CRenderer::CV_r_cullgeometryforlights == 2)
            bNeedList = true;
          if (bNeedList)
          {
            STexPic *pic = (STexPic *)((ITexPic*)pDLight->m_pLightImage);
            if (pic && (pic->m_Flags2 & FT2_CUBEASSINGLETEXTURE))
            {
              li->m_AssociatedLight.m_fLastTime = gRenDev->m_RP.m_RealTime; 
              li->m_AssociatedLight.m_ProjAngles = pDLight->m_ProjAngles;
              li->m_AssociatedLight.m_fLightFrustumAngle = pDLight->m_fLightFrustumAngle;
              li->m_AssociatedLight.m_NumCM |= 2;
              mfGenerateIndicesInsideFrustrum(li, pDLight);
            }
          }
        }
        else
        if (pDLight->m_Flags & DLF_POINT)
        {
          bool bNeedList = false;
          if (li->m_AssociatedLight.m_fRadius != pDLight->m_fRadius)
          {
            li->m_AssociatedLight.m_fRadius = pDLight->m_fRadius;
            li->m_AssociatedLight.m_fLifeTime = gRenDev->m_RP.m_RealTime;
            if (CRenderer::CV_r_cullgeometryforlights != 2)
            {
              Best = i;
              li->m_AssociatedLight.m_NumCM &= ~2;
              break;
            }
          }
          // if static already
          if (li->m_AssociatedLight.m_fLifeTime+1.0f < gRenDev->m_RP.m_RealTime)
          {
            if (!(li->m_AssociatedLight.m_NumCM & 2))
              bNeedList = true;
          }
          else
          if (CRenderer::CV_r_cullgeometryforlights == 2)
            bNeedList = true;
          if (bNeedList)
          {
            li->m_AssociatedLight.m_fRadius = pDLight->m_fRadius;
            li->m_AssociatedLight.m_NumCM |= 2;
            mfGenerateIndicesForAttenuation(li, pDLight);
          }
        }
        li->m_AssociatedLight.m_fLastTime = gRenDev->m_RP.m_RealTime; 
        if (li->m_AssociatedLight.m_NumCM & 2)
        {
          int nInds = li->m_pIndiciesAttenFrustr.Num();
          if (nInds)
          {
            return li;
          }
          return NULL;
        }
        else
        {
          int nInds = li->m_pIndicies.Num();
          if (nInds)
          {
            return li;
          }
          return NULL;
        }
      }
      else
      {
        Best = i;
        break;
      }
    }
  }
  
  if (Best < 0)
  {
    for (i=0; i<m_LIndicies->Num(); i++)
    {
      li = m_LIndicies->Get(i);
      if ((li->m_AssociatedLight.m_Name[0] && !strcmp(li->m_AssociatedLight.m_Name, pDLight->m_Name)) || li->m_AssociatedLight.m_fLastTime+8 < gRenDev->m_RP.m_RealTime)
      {
        Best = i;
        li->m_AssociatedLight.m_NumCM = 0;
        break;
      }
    }
    if (Best < 0)
    {
      li = new SLightIndicies;
      li->m_AssociatedLight.m_NumCM = 0;
      m_LIndicies->AddElem(li);
      memset(li, 0, sizeof(SLightIndicies));
    }
  }
  else
    li = m_LIndicies->Get(Best);
  li->m_AssociatedLight.m_Flags = pDLight->m_Flags;
  li->m_AssociatedLight.m_Orientation.m_vForward = objPos;
  li->m_AssociatedLight.m_Orientation.m_vRight = objAng;

  li->m_AssociatedLight.m_Orientation.m_vUp.Set(fObjScale,fObjScale,fObjScale);

  li->m_AssociatedLight.m_Origin = pDLight->m_Origin;
  li->m_AssociatedLight.m_ProjAngles = pDLight->m_ProjAngles;
  li->m_AssociatedLight.m_fLightFrustumAngle = pDLight->m_fLightFrustumAngle;
  strcpy(li->m_AssociatedLight.m_Name, pDLight->m_Name);

  li->m_AssociatedLight.m_fLastTime = gRenDev->m_RP.m_RealTime;
  li->m_AssociatedLight.m_fLifeTime = gRenDev->m_RP.m_RealTime;
  if (!(li->m_AssociatedLight.m_NumCM & 1))
  {
    li->m_AssociatedLight.m_NumCM = 1;
    li->m_pIndicies.Free();

    Vec3d pos;
    TransformPosition(pos, li->m_AssociatedLight.m_Origin, gRenDev->m_RP.m_pCurObject->GetInvMatrix());

    switch(lb->m_nPrimetiveType)
    {
      case R_PRIMV_TRIANGLES:
        {
          int n = 0;
          int nOffs = mi->nFirstIndexId;
          int nNumInds = mi->nNumIndices;
          for(i=0; i<nNumInds-2; i+=3, n++)
          {
            SMeshFace *mf = &m_Faces->Get(n);
            Vec3d vDelt = pos - mf->m_Middle;
            vDelt.NormalizeFast();
            if ((vDelt | mf->m_Normal) > -0.5f)
            {
              unsigned short * face = &pInds[i+nOffs];
              li->m_pIndicies.AddElem(face[0]);
              li->m_pIndicies.AddElem(face[1]);
              li->m_pIndicies.AddElem(face[2]);
            }
          }
        }
        break;

      case R_PRIMV_MULTI_GROUPS:
        {
          int nOffs = mi->nFirstIndexId;
          unsigned int n;
          for (j=0; j<mi->m_dwNumSections; j++)
          {
            SPrimitiveGroup *g = &mi->m_pPrimitiveGroups[j];
            int incr;
            switch (g->type)
            {
              case PT_LIST:
                incr = 3;
                break;
              case PT_STRIP:
              case PT_FAN:
                incr = 1;
                break;
            }
            int nf = g->nFirstFace;
            int offs = g->offsIndex + nOffs;
            for (n=0; n<g->numIndices-2; n+=incr)
            {
              int i0, i1, i2;
              switch (g->type)
              {
                case PT_LIST:
                  i0 = pInds[offs+n];
                  i1 = pInds[offs+n+1];
                  i2 = pInds[offs+n+2];
                  break;
                case PT_STRIP:
                  i0 = pInds[offs+n];
                  i1 = pInds[offs+n+1];
                  i2 = pInds[offs+n+2];
                  break;
                case PT_FAN:
                  i0 = pInds[offs+0];
                  i1 = pInds[offs+n+1];
                  i2 = pInds[offs+n+2];
                  break;
              }
              // ignore degenerate triangle
              if (i0==i1 || i0==i2 || i1==i2)
                continue;

              SMeshFace *mf = &m_Faces->Get(nf);
              Vec3d vDelt = pos - mf->m_Middle;
              vDelt.NormalizeFast();
              if ((vDelt | mf->m_Normal) > -0.5f)
              {
                li->m_pIndicies.AddElem(i0);
                li->m_pIndicies.AddElem(i1);
                li->m_pIndicies.AddElem(i2);
              }
              nf++;
            }
          }
        }
        break;

      default:
        assert(0);
    }
    li->m_pIndicies.Shrink();
  }
  if (li->m_AssociatedLight.m_NumCM & 2)
  {
    int nInds = li->m_pIndiciesAttenFrustr.Num();
    if (nInds)
    {
#if defined (DIRECT3D8) || defined (DIRECT3D9)
      gRenDev->UpdateIndexBuffer(&li->m_IndexBuffer, &li->m_pIndiciesAttenFrustr[0], nInds);
#endif
      return li;
    }
    return NULL;
  }
  else
  {
    int nInds = li->m_pIndicies.Num();
    if (nInds)
    {
#if defined (DIRECT3D8) || defined (DIRECT3D9)
      gRenDev->UpdateIndexBuffer(&li->m_IndexBuffer, &li->m_pIndicies[0], nInds);
#endif
      return li;
    }
    return NULL;
  }
}

float CREOcLeaf::mfMinDistanceToCamera(CCObject *pObj)
{
  Vec3d pos = gRenDev->GetCamera().GetPos();
  CMatInfo *mi = m_pChunk;
  float fDist = 0;
  if (!mi->m_fRadius)
  {
    Vec3d vMins, vMaxs, vCenterRE;
    mfGetBBox(vMins, vMaxs);
    vCenterRE = (vMins + vMaxs) * 0.5f;
    vCenterRE += pObj->GetTranslation();
    float fRadRE = (vMaxs - vMins).GetLength() * 0.5f;
    float fScale = pObj->m_Matrix(0,0)*pObj->m_Matrix(0,0) + pObj->m_Matrix(0,1)*pObj->m_Matrix(0,1) + pObj->m_Matrix(0,2)*pObj->m_Matrix(0,2);
    fScale = cryISqrtf(fScale);
    fDist = (pos - vCenterRE).GetLength();
    fDist = fDist - fRadRE / fScale;
  }
  else
  {
    Vec3d vMid = mi->m_vCenter;

    vMid += pObj->GetTranslation();
    float fScale = pObj->m_Matrix(0,0)*pObj->m_Matrix(0,0) + pObj->m_Matrix(0,1)*pObj->m_Matrix(0,1) + pObj->m_Matrix(0,2)*pObj->m_Matrix(0,2);
    fScale = cryISqrtf(fScale);
    fDist = (pos - vMid).GetLength();
    fDist -= mi->m_fRadius / fScale * 2.0f;
  }
  if (fDist < 0.25f)
    fDist = 0.25f;

  return fDist;
}

bool CREOcLeaf::mfCheckUpdate(int nVertFormat, int Flags)
{
  //PROFILE_FRAME(Mesh_CheckUpdate);

  CLeafBuffer *lb = m_pBuffer->GetVertexContainer();

  bool bNeedAddNormals = false;
  if ((Flags & SHPF_NORMALS) && g_VertFormatNormalOffsets[lb->m_pSecVertBuffer->m_vertexformat] < 0)
    bNeedAddNormals = true;

  if (gRenDev->m_RP.m_pShader && (gRenDev->m_RP.m_pShader->m_Flags2 & EF2_DEFAULTVERTEXFORMAT))
    nVertFormat = lb->m_nVertexFormat;
  bool bWasReleased = m_pBuffer->CheckUpdate(nVertFormat, Flags, bNeedAddNormals);

  if (!lb->m_pVertexBuffer)
    return false;

#if defined (DIRECT3D8) || defined (DIRECT3D9)
  gRenDev->m_RP.m_CurVFormat = lb->m_pVertexBuffer->m_vertexformat;
#endif

#ifndef PIPE_USE_INSTANCING
  if((Flags & SHPF_LMTC) && gRenDev->m_RP.m_pCurObject->m_pLMTCBufferO)
  {
    lb = gRenDev->m_RP.m_pCurObject->m_pLMTCBufferO;
    if (!lb->m_pVertexBuffer)
      lb->UpdateVidVertices(lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData, lb->m_SecVertCount);
    if (!lb->m_pVertexBuffer)
      return false;
  }
#else
  if((Flags & SHPF_LMTC))
  {
    CCObject *pObj = gRenDev->m_RP.m_pCurObject;
    int nObj = 0;
    while (true)
    {
      if (pObj->m_pLMTCBufferO)
      {
        lb = pObj->m_pLMTCBufferO;
        lb->Unlink();
        lb->Link(&CLeafBuffer::m_Root);
        if (!lb->m_pVertexBuffer)
          lb->UpdateVidVertices(lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData, lb->m_SecVertCount);
        if (!lb->m_pVertexBuffer)
          return false;
      }
      nObj++;
      if (nObj >= gRenDev->m_RP.m_MergedObjects.Num())
        break;
      pObj = gRenDev->m_RP.m_MergedObjects[nObj];
    }
  }
#endif
  if (bWasReleased && !lb->m_pSecVertBuffer && !(Flags & FHF_FORANIM) && lb->m_pVertexBuffer)
  {
    // Callback function can change buffer sizes
    gRenDev->m_RP.m_FirstVertex = m_pChunk->nFirstVertId;
    gRenDev->m_RP.m_FirstIndex = m_pChunk->nFirstIndexId;
    gRenDev->m_RP.m_RendNumIndices = m_pChunk->nNumIndices;
    gRenDev->m_RP.m_RendNumVerts = m_pChunk->nNumVerts;
    if (gRenDev->m_RP.m_pCurObject && gRenDev->m_RP.m_pCurObject->m_pCharInstance)
    {
      PROFILE_FRAME(Mesh_CheckUpdateSkinning);
      double time0 = 0;
      ticks(time0);
      gRenDev->m_RP.m_pCurObject->m_pCharInstance->ProcessSkinning(Vec3(zero),gRenDev->m_RP.m_pCurObject->m_Matrix, gRenDev->m_RP.m_pCurObject->m_nTemplId, gRenDev->m_RP.m_pCurObject->m_nLod, true);
      unticks(time0);
      gRenDev->m_RP.m_PS.m_fSkinningTime += (float)(time0*1000.0*g_SecondsPerCycle);
    }
  }
#ifndef OPENGL
  if (!m_pBuffer->m_Indices.m_VData)
  {
    //assert(!m_pChunk->nNumIndices);
    return false;
  }
#else
  if (!m_pBuffer->m_SecIndices.Num())
  {
#ifndef WIN64
    //assert(!m_pChunk->nNumIndices);
#endif //WIN64
    return false;
  }
#endif

  return true;
}

static _inline byte *sGetBuf(CLeafBuffer *lb, int *Stride, int Stream, int Flags)
{
  byte *pD;
  if (Flags & FGP_WAIT)
    gRenDev->UpdateBuffer(lb->m_pVertexBuffer, NULL, 0, false, 0, Stream);
  if (!(Flags & FGP_SRC))
  {
    pD = (byte*)lb->m_pVertexBuffer->m_VS[Stream].m_VData;
    gRenDev->m_RP.m_nCurBufferID = lb->m_pVertexBuffer->m_VS[Stream].m_VertBuf.m_nID;
  }
  else
  {
    if (!lb->m_pSecVertBuffer)
      pD = NULL;
    else
      pD = (byte *)lb->m_pSecVertBuffer->m_VS[Stream].m_VData;
  }
  if (!pD && !gRenDev->m_RP.m_nCurBufferID)
    return pD;
  if (Stream == VSF_GENERAL)
    *Stride = m_VertexSize[lb->m_nVertexFormat];
  else
  if (Stream == VSF_TANGENTS)
    *Stride = sizeof(SPipTangents);
  else
    assert(0);
  int Offs = gRenDev->m_RP.m_FirstVertex * (*Stride);
  if (Flags & FGP_REAL)
    pD = &pD[Offs];

  return pD;
}

void *CREOcLeaf::mfGetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags)
{
  SBufInfoTable *pOffs;
  CLeafBuffer *lb = m_pBuffer->GetVertexContainer();
  byte *pD;

  switch(ePT) 
  {
    case eSrcPointer_Vert:
      pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
      gRenDev->m_RP.m_nCurBufferOffset = 0;
      return pD;
    case eSrcPointer_Tex:
      pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
      pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
      if (pOffs->OffsTC)
      {
        gRenDev->m_RP.m_nCurBufferOffset = pOffs->OffsTC;
        return &pD[pOffs->OffsTC];
      }
      else
      {
        if (!(Flags & FGP_SRC))
        {
          Warning( 0,0,"Error: Missed texcoord pointer for shader '%s'", gRenDev->m_RP.m_pShader->m_Name.c_str());
          return NULL;
        }
        *Stride = sizeof(SMRendTexVert);
        if (Flags & FGP_REAL)
          return &lb->m_TempTexCoords[gRenDev->m_RP.m_FirstVertex];
        else
          return lb->m_TempTexCoords;
      }
      break;
    case eSrcPointer_TexLM:     
      if(gRenDev->m_RP.m_pCurObject && gRenDev->m_RP.m_pCurObject->m_pLMTCBufferO)
      { // separate stream for lightmaps
        CVertexBuffer * pVideoBuffer = gRenDev->m_RP.m_pCurObject->m_pLMTCBufferO->m_pVertexBuffer;
        assert(pVideoBuffer->m_vertexformat==VERTEX_FORMAT_TEX2F);    // M.M.
        *Stride = m_VertexSize[pVideoBuffer->m_vertexformat];
        gRenDev->m_RP.m_nCurBufferOffset = 0;
        gRenDev->m_RP.m_nCurBufferID = pVideoBuffer->m_VS[VSF_GENERAL].m_VertBuf.m_nID;
        return(pVideoBuffer->m_VS[VSF_GENERAL].m_VData);
      }
      pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
      pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
      gRenDev->m_RP.m_nCurBufferOffset = 0;
      return pD;
    case eSrcPointer_Normal:
      pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
      pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
      if (pOffs->OffsNormal)
      {
        gRenDev->m_RP.m_nCurBufferOffset = pOffs->OffsNormal;
        return &pD[pOffs->OffsNormal];
      }
      else
      {
        if (!(Flags & FGP_SRC))
        {
          Warning( 0,0,"Error: Missed normal pointer for shader '%s'", gRenDev->m_RP.m_pShader->m_Name.c_str());
          return NULL;
        }
        *Stride = sizeof(Vec3d);
        if (Flags & FGP_REAL)
          return &lb->m_TempNormals[gRenDev->m_RP.m_FirstVertex];
        else
          return lb->m_TempNormals;
      }
      return &pD[0];
    case eSrcPointer_Binormal:
      pD = sGetBuf(lb, Stride, VSF_TANGENTS, Flags);
      gRenDev->m_RP.m_nCurBufferOffset = 12;
      return &pD[12];
    case eSrcPointer_TNormal:
      pD = sGetBuf(lb, Stride, VSF_TANGENTS, Flags);
      gRenDev->m_RP.m_nCurBufferOffset = 24;
      return &pD[24];
    case eSrcPointer_Tangent:
      pD = sGetBuf(lb, Stride, VSF_TANGENTS, Flags);
      gRenDev->m_RP.m_nCurBufferOffset = 0;
      return &pD[0];
    case eSrcPointer_Color:
      pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
      pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
      gRenDev->m_RP.m_nCurBufferOffset = pOffs->OffsColor;
      return &pD[pOffs->OffsColor];
    case eSrcPointer_SecColor:
      pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
      pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
      gRenDev->m_RP.m_nCurBufferOffset = pOffs->OffsSecColor;
      return &pD[pOffs->OffsSecColor];


    case eSrcPointer_LightVector:
      {
        byte *dst;
        switch (Dst)
        {
          case eSrcPointer_LightVector:
            dst = (byte *)&gRenDev->m_RP.m_pLightVectors[gRenDev->m_RP.m_nCurLight][0];
            *Stride = sizeof(Vec3d);
            break;
          case eSrcPointer_Tex:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (!(Flags & (FGP_SRC | FGP_WAIT)))
            gRenDev->UpdateBuffer(lb->m_pVertexBuffer, NULL, 0, false, 0);

          int Offs = gRenDev->m_RP.m_FirstVertex * (*Stride);
          if ((gRenDev->m_RP.m_pShader->m_Flags3 & EF3_PREPARELV) && Dst != eSrcPointer_LightVector)
          {
            byte *Dst = dst;
            int str = *Stride;
            Vec3d *lv = gRenDev->m_RP.m_pLightVectors[gRenDev->m_RP.m_nCurLight];
            if (!(Flags & FGP_REAL))
              Dst = &Dst[Offs];
            for (int i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, Dst+=str, lv++)
            {
              float *fd = (float *)Dst;
              fd[0] = lv->x;
              fd[1] = lv->y;
              fd[2] = lv->z;
            }
          }
          else
          {
            if (gRenDev->m_RP.m_Frame != mFrameCalcLight)
            {
              mFrameCalcLight = gRenDev->m_RP.m_Frame;
              mMaskLight0 = 0;
            }
            if (!(mMaskLight0 & (1<<gRenDev->m_RP.m_nCurLight)))
            {
              mMaskLight0 |= (1<<gRenDev->m_RP.m_nCurLight);
              byte *Dst;
              if ((Flags & FGP_REAL) || dst == (byte *)gRenDev->m_RP.m_pLightVectors[gRenDev->m_RP.m_nCurLight])
                Dst = dst;
              else
                Dst = &dst[Offs];
              SRendItem::mfCalcLightVectors(Dst, *Stride);
            }
          }
        }
        return dst;
      }
      break;

    case eSrcPointer_LightVector_Terrain:
      {
        byte *dst;
        switch (Dst)
        {
          case eSrcPointer_LightVector:
            dst = (byte *)&gRenDev->m_RP.m_pLightVectors[gRenDev->m_RP.m_nCurLight][0];
            *Stride = sizeof(Vec3d);
            break;
          case eSrcPointer_Tex:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
          case eSrcPointer_Color:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (!(Flags & (FGP_SRC | FGP_WAIT)))
            gRenDev->UpdateBuffer(lb->m_pVertexBuffer, NULL, 0, false, 0);

          int Offs = gRenDev->m_RP.m_FirstVertex * (*Stride);
          if ((gRenDev->m_RP.m_pShader->m_Flags3 & EF3_PREPARELV) && Dst != eSrcPointer_LightVector_Terrain)
          {
            byte *Dst = dst;
            int str = *Stride;
            Vec3d *lv = gRenDev->m_RP.m_pLightVectors[gRenDev->m_RP.m_nCurLight];
            if (!(Flags & FGP_REAL))
              Dst = &Dst[Offs];
            for (int i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, Dst+=str, lv++)
            {
              float *fd = (float *)Dst;
              fd[0] = lv->x;
              fd[1] = lv->y;
              fd[2] = lv->z;
            }
          }
          else
          {
            if (gRenDev->m_RP.m_Frame != mFrameCalcLight)
            {
              mFrameCalcLight = gRenDev->m_RP.m_Frame;
              mMaskLight0 = 0;
            }
            if (!(mMaskLight0 & (1<<gRenDev->m_RP.m_nCurLight)))
            {
              mMaskLight0 |= (1<<gRenDev->m_RP.m_nCurLight);
              byte *Dst;
              if ((Flags & FGP_REAL) || dst == (byte *)gRenDev->m_RP.m_pLightVectors[gRenDev->m_RP.m_nCurLight])
                Dst = dst;
              else
                Dst = &dst[Offs];
              SRendItem::mfCalcLightVectors_Terrain(Dst, *Stride);
            }
          }
        }
        return dst;
      }
      break;

    case eSrcPointer_NormLightVector:
      {
        byte *dst;
        switch (Dst)
        {
          case eSrcPointer_LightVector:
            dst = (byte *)&gRenDev->m_RP.m_pLightVectors[gRenDev->m_RP.m_nCurLight][0];
            *Stride = sizeof(Vec3d);
            break;
          case eSrcPointer_Tex:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
          case eSrcPointer_Color:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (!(Flags & (FGP_SRC | FGP_WAIT)))
            gRenDev->UpdateBuffer(lb->m_pVertexBuffer, NULL, 0, false, 0);
          int Offs = gRenDev->m_RP.m_FirstVertex * (*Stride);
          if ((gRenDev->m_RP.m_pShader->m_Flags3 & EF3_PREPARELV) && Dst != eSrcPointer_NormLightVector)
          {
            byte *Dst = dst;
            int str = *Stride;
            Vec3d *lv = gRenDev->m_RP.m_pLightVectors[gRenDev->m_RP.m_nCurLight];
            if (!(Flags & FGP_REAL))
              Dst = &Dst[Offs];
            if (Type == GL_FLOAT)
            {
              for (int i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, Dst+=str, lv++)
              {
                float *fd = (float *)Dst;
                float fInvLen=1.0f/cry_sqrtf(lv->x*lv->x+lv->y*lv->y+lv->z*lv->z); 
                fd[0] = lv->x * fInvLen;
                fd[1] = lv->y * fInvLen;
                fd[2] = lv->z * fInvLen;
              }
            }
            else
            {
              for (int i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, Dst+=str, lv++)
              {
                byte *fd = (byte *)Dst;
                float fInvLen=1.0f/cry_sqrtf(lv->x*lv->x+lv->y*lv->y+lv->z*lv->z); 
                fd[0] = (byte)(lv->x * fInvLen * 128.0f + 128.0f);
                fd[1] = (byte)(lv->y * fInvLen * 128.0f + 128.0f);
                fd[2] = (byte)(lv->z * fInvLen * 128.0f + 128.0f);
              }
            }
          }
          else
          {
            if (gRenDev->m_RP.m_Frame != mFrameCalcLight)
            {
              mFrameCalcLight = gRenDev->m_RP.m_Frame;
              mMaskLight0 = 0;
            }
            if (!(mMaskLight0 & (1<<gRenDev->m_RP.m_nCurLight)))
            {
              mMaskLight0 |= (1<<gRenDev->m_RP.m_nCurLight);
              byte *Dst;
              if ((Flags & FGP_REAL) || dst == (byte *)gRenDev->m_RP.m_pLightVectors[gRenDev->m_RP.m_nCurLight])
                Dst = dst;
              else
                Dst = &dst[Offs];
              SRendItem::mfCalcNormLightVectors(Dst, *Stride, Type);
            }
          }
        }
        return dst;
      }
      break;
      
    case eSrcPointer_HalfAngleVector:
      {
        byte *dst;
        pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
        pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
        switch (Dst)
        {
          case eSrcPointer_HalfAngleVector:
            dst = (byte *)&gRenDev->m_RP.m_pHalfAngleVectors[gRenDev->m_RP.m_nCurLight][0];
            *Stride = sizeof(Vec3d);
            break;
          case eSrcPointer_Tex:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
          case eSrcPointer_Color:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (!(Flags & (FGP_SRC | FGP_WAIT)))
            gRenDev->UpdateBuffer(lb->m_pVertexBuffer, NULL, 0, false, 0);
          int Offs = gRenDev->m_RP.m_FirstVertex * (*Stride);
          if ((gRenDev->m_RP.m_pShader->m_Flags3 & EF3_PREPAREHAV) && Dst != eSrcPointer_HalfAngleVector)
          {
            byte *Dst = dst;
            int str = *Stride;
            if (!(Flags & FGP_REAL))
              Dst = &Dst[Offs];
            Vec3d *lv = gRenDev->m_RP.m_pHalfAngleVectors[gRenDev->m_RP.m_nCurLight];
            for (int i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, Dst+=str, lv++)
            {
              float *fd = (float *)Dst;
              fd[0] = lv->x;
              fd[1] = lv->y;
              fd[2] = lv->z;
            }
          }
          else
          {
            if (gRenDev->m_RP.m_Frame != mFrameCalcHalfAngle)
            {
              mFrameCalcHalfAngle = gRenDev->m_RP.m_Frame;
              mMaskLight1 = 0;
            }
            if (!(mMaskLight1 & (1<<gRenDev->m_RP.m_nCurLight)))
            {
              mMaskLight1 |= (1<<gRenDev->m_RP.m_nCurLight);
              byte *Dst;
              if ((Flags & FGP_REAL) || dst == (byte *)gRenDev->m_RP.m_pHalfAngleVectors[gRenDev->m_RP.m_nCurLight])
                Dst = dst;
              else
                Dst = &dst[Offs];
              SRendItem::mfCalcHalfAngles(Type, Dst, *Stride);
            }
          }
        }
        return dst;
      }
      break;

    case eSrcPointer_HalfAngleVector_Terrain:
      {
        byte *dst;
        pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
        pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
        switch (Dst)
        {
          case eSrcPointer_HalfAngleVector:
            dst = (byte *)&gRenDev->m_RP.m_pHalfAngleVectors[gRenDev->m_RP.m_nCurLight][0];
            *Stride = sizeof(Vec3d);
            break;
          case eSrcPointer_Tex:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
          case eSrcPointer_Color:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (!(Flags & (FGP_SRC | FGP_WAIT)))
            gRenDev->UpdateBuffer(lb->m_pVertexBuffer, NULL, 0, false, 0);
          int Offs = gRenDev->m_RP.m_FirstVertex * (*Stride);
          if ((gRenDev->m_RP.m_pShader->m_Flags3 & EF3_PREPAREHAV) && Dst != eSrcPointer_HalfAngleVector_Terrain)
          {
            byte *Dst = dst;
            int str = *Stride;
            if (!(Flags & FGP_REAL))
              Dst = &Dst[Offs];
            Vec3d *lv = gRenDev->m_RP.m_pHalfAngleVectors[gRenDev->m_RP.m_nCurLight];
            for (int i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, Dst+=str, lv++)
            {
              float *fd = (float *)Dst;
              fd[0] = lv->x;
              fd[1] = lv->y;
              fd[2] = lv->z;
            }
          }
          else
          {
            if (gRenDev->m_RP.m_Frame != mFrameCalcHalfAngle)
            {
              mFrameCalcHalfAngle = gRenDev->m_RP.m_Frame;
              mMaskLight1 = 0;
            }
            if (!(mMaskLight1 & (1<<gRenDev->m_RP.m_nCurLight)))
            {
              mMaskLight1 |= (1<<gRenDev->m_RP.m_nCurLight);
              byte *Dst;
              if ((Flags & FGP_REAL) || dst == (byte *)gRenDev->m_RP.m_pHalfAngleVectors[gRenDev->m_RP.m_nCurLight])
                Dst = dst;
              else
                Dst = &dst[Offs];
              SRendItem::mfCalcHalfAngles_Terrain(Type, Dst, *Stride);
            }
          }
        }
        return dst;
      }
      break;

    case eSrcPointer_LAttenuationSpec0:
      {
        byte *dst;
        switch (Dst)
        {
          case eSrcPointer_LAttenuationSpec0:
            dst = (byte *)&gRenDev->m_RP.m_pLAttenSpec0[0];
            *Stride = sizeof(Vec3d);
            break;
          case eSrcPointer_Tex:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
          case eSrcPointer_Color:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (!(Flags & (FGP_SRC | FGP_WAIT)))
            gRenDev->UpdateBuffer(lb->m_pVertexBuffer, NULL, 0, false, 0);
          int Offs = gRenDev->m_RP.m_FirstVertex * (*Stride);
          if ((gRenDev->m_RP.m_pShader->m_Flags3 & EF3_PREPARELAS0) && Dst != eSrcPointer_LAttenuationSpec0)
          {
            byte *Dst = dst;
            int str = *Stride;
            if (!(Flags & FGP_REAL))
              Dst = &Dst[Offs];
            Vec3d *lv = gRenDev->m_RP.m_pLAttenSpec0;
            if (Type == GL_FLOAT)
            {
              for (int i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, Dst+=str, lv++)
              {
                float *fd = (float *)Dst;
                fd[0] = lv->x;
                fd[1] = lv->y;
                fd[2] = lv->z;
              }
            }
            else
            {
              for (int i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, Dst+=str, lv++)
              {
                byte *fd = Dst;
                fd[0] = (byte)(lv->x * 255.0f);
                fd[1] = (byte)(lv->y * 255.0f);
                fd[2] = (byte)(lv->z * 255.0f);
              }
            }
          }
          else
          {
            if (gRenDev->m_RP.m_Frame != mFrameCalcLAttenSpec0)
            {
              mFrameCalcLAttenSpec0 = gRenDev->m_RP.m_Frame;
              mMaskLight2 = 0;
            }
            if (!(mMaskLight2 & (1<<gRenDev->m_RP.m_nCurLight)))
            {
              mMaskLight2 |= (1<<gRenDev->m_RP.m_nCurLight);
              byte *Dst;
              if ((Flags & FGP_REAL) || dst == (byte *)gRenDev->m_RP.m_pLAttenSpec0)
                Dst = dst;
              else
                Dst = &dst[Offs];
              int StrLV;
              byte *lv = (byte *)SRendItem::mfGetPointerCommon(eSrcPointer_LightVector, &StrLV, GL_FLOAT, eSrcPointer_LightVector, FGP_SRC | FGP_REAL);
              if (dst)
                SRendItem::mfCalcLAttenuationSpec0(Dst, *Stride, lv, StrLV, Type);
            }
          }
        }
        return dst;
      }
      break;

    case eSrcPointer_LAttenuationSpec1:
      {
        byte *dst;
        switch (Dst)
        {
          case eSrcPointer_LAttenuationSpec1:
            dst = (byte *)&gRenDev->m_RP.m_pLAttenSpec1[0];
            *Stride = sizeof(Vec3d);
            break;
          case eSrcPointer_HalfAngleVector:
            dst = (byte *)&gRenDev->m_RP.m_pHalfAngleVectors[gRenDev->m_RP.m_nCurLight][0];
            *Stride = sizeof(Vec3d);
            break;
          case eSrcPointer_Tex:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
          case eSrcPointer_Color:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (!(Flags & (FGP_SRC | FGP_WAIT)))
            gRenDev->UpdateBuffer(lb->m_pVertexBuffer, NULL, 0, false, 0);
          int Offs = gRenDev->m_RP.m_FirstVertex * (*Stride);
          if ((gRenDev->m_RP.m_pShader->m_Flags3 & EF3_PREPARELAS1) && Dst != eSrcPointer_LAttenuationSpec1)
          {
            byte *Dst = dst;
            int str = *Stride;
            if (!(Flags & FGP_REAL))
              Dst = &Dst[Offs];
            Vec3d *lv = gRenDev->m_RP.m_pLAttenSpec1;
            if (Type == GL_FLOAT)
            {
              for (int i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, Dst+=str, lv++)
              {
                float *fd = (float *)Dst;
                fd[0] = lv->x;
                fd[1] = lv->y;
                fd[2] = lv->z;
              }
            }
            else
            {
              for (int i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, Dst+=str, lv++)
              {
                byte *fd = Dst;
                fd[0] = uchar(lv->x * 255.0f);
                fd[1] = uchar(lv->y * 255.0f);
                fd[2] = uchar(lv->z * 255.0f);
              }
            }
          }
          else
          {
            if (gRenDev->m_RP.m_Frame != mFrameCalcLAttenSpec1)
            {
              mFrameCalcLAttenSpec1 = gRenDev->m_RP.m_Frame;
              mMaskLight3 = 0;
            }
            if (!(mMaskLight3 & (1<<gRenDev->m_RP.m_nCurLight)))
            {
              mMaskLight3 |= (1<<gRenDev->m_RP.m_nCurLight);
              byte *Dst;
              if ((Flags & FGP_REAL) || dst == (byte *)gRenDev->m_RP.m_pLAttenSpec1)
                Dst = dst;
              else
                Dst = &dst[Offs];
              int StrLV;
              byte *lv = (byte *)SRendItem::mfGetPointerCommon(eSrcPointer_LightVector, &StrLV, GL_FLOAT, eSrcPointer_LightVector, FGP_SRC | FGP_REAL);
              if (dst)
                SRendItem::mfCalcLAttenuationSpec1(Dst, *Stride, lv, StrLV, Type);
            }
          }
        }
        return dst;
      }
      break;
      
    case eSrcPointer_Refract:
      {
        byte *dst;
        pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
        pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
        switch (Dst)
        {
          case eSrcPointer_Tex:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
          case eSrcPointer_Color:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (!(Flags & (FGP_SRC | FGP_WAIT)))
            gRenDev->UpdateBuffer(lb->m_pVertexBuffer, NULL, 0, false, 0);
          int Offs = gRenDev->m_RP.m_FirstVertex * (*Stride);
          if (gRenDev->m_RP.m_Frame != mFrameCalcRefract)
          {
            mFrameCalcRefract = gRenDev->m_RP.m_Frame;
            byte *Dst;
            if (Flags & FGP_REAL)
              Dst = dst;
            else
              Dst = &dst[Offs];
            SRendItem::mfCalcRefractVectors(Type, Dst, *Stride);
          }
        }
        return dst;
      }
      break;

    case eSrcPointer_Project:
      {
        byte *dst;
        pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
        pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
        switch (Dst)
        {
          case eSrcPointer_Tex:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
          case eSrcPointer_Color:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          static float Smat[16] = 
          {
            0.5f, 0,    0,    0,
            0,    0.5f, 0,    0,
            0,    0,    0.5f, 0,
            0.5f, 0.5f, 0.5f, 1.0f
          };
          if (!(Flags & (FGP_SRC | FGP_WAIT)))
            gRenDev->UpdateBuffer(lb->m_pVertexBuffer, NULL, 0, false, 0);
          int Offs = gRenDev->m_RP.m_FirstVertex * (*Stride);
          if (gRenDev->m_RP.m_Frame != mFrameCalcProject)
          {
            mFrameCalcProject = gRenDev->m_RP.m_Frame;
            float fRefract = -1000.0f;
            if (gRenDev->m_RP.m_pCurObject)
              fRefract = gRenDev->m_RP.m_pCurObject->m_fRefract;
            byte *Dst;
            if (Flags & FGP_REAL)
              Dst = dst;
            else
              Dst = &dst[Offs];
            float matProj[16], matView[16], m1[16], m2[16];
            gRenDev->GetProjectionMatrix(matProj);
            gRenDev->GetModelViewMatrix(matView);
            multMatrices(m2, Smat, matProj);
            multMatrices(m1, m2, matView);
            SRendItem::mfCalcProjectVectors(Type, m1, fRefract, Dst, *Stride);
          }
        }
        return dst;
      }
      break;

    case eSrcPointer_ProjectTexture:
      {
        byte *dst;
        switch (Dst)
        {
          case eSrcPointer_Tex:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
          case eSrcPointer_Color:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (!(Flags & (FGP_SRC | FGP_WAIT)))
            gRenDev->UpdateBuffer(lb->m_pVertexBuffer, NULL, 0, false, 0);
          int Offs = gRenDev->m_RP.m_FirstVertex * (*Stride);
          if (gRenDev->m_RP.m_Frame != mFrameCalcProject)
          {
            mFrameCalcProject = gRenDev->m_RP.m_Frame;
            byte *Dst;
            if (Flags & FGP_REAL)
              Dst = dst;
            else
              Dst = &dst[Offs];
            SShaderPass *sl = gRenDev->m_RP.m_CurrPass;
            int nl = Flags >> FGP_STAGE_SHIFT;
            if (nl < sl->m_TUnits.Num())
            {
              SShaderTexUnit *tl = &sl->m_TUnits[nl];
              float *matr = tl->m_TexPic->m_Matrix;
              if (matr)
                SRendItem::mfCalcProjectVectors(Type, matr, 0.5f, Dst, *Stride);
            }
          }
        }
        return dst;
      }
      break;

    case eSrcPointer_Attenuation:
      {
        byte *dst;
        switch (Dst)
        {
          case eSrcPointer_Tex:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
          case eSrcPointer_Color:
            pD = sGetBuf(lb, Stride, VSF_GENERAL, Flags);
            pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
            dst = &pD[pOffs->OffsTC];
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (!(Flags & (FGP_SRC | FGP_WAIT)))
            gRenDev->UpdateBuffer(lb->m_pVertexBuffer, NULL, 0, false, 0);
          int Offs = gRenDev->m_RP.m_FirstVertex * (*Stride);
          if (gRenDev->m_RP.m_Frame != mFrameCalcAtten)
          {
            mFrameCalcAtten = gRenDev->m_RP.m_Frame;
            mMaskLight4 = 0;
          }
          if (!(mMaskLight4 & (1<<gRenDev->m_RP.m_nCurLight)))
          {
            mMaskLight4 |= (1<<gRenDev->m_RP.m_nCurLight);
            byte *Dst;
            if (Flags & FGP_REAL)
              Dst = dst;
            else
              Dst = &dst[Offs];
            SRendItem::mfCalcLightAttenuation(Type, Dst, *Stride);
          }
        }
        return dst;
      }
      break;

    default:
      assert(false);
      break;
  }
  return NULL;
}

#include "../NvTriStrip/NVTriStrip.h"

void CREOcLeaf::mfGetPlane(Plane& pl)
{
  CLeafBuffer *lb = m_pBuffer->GetVertexContainer();
  
  byte *p = (byte *)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
  int size = m_VertexSize[lb->m_pSecVertBuffer->m_vertexformat];

  ushort *inds;
  inds = &lb->GetIndices(NULL)[0]+m_pChunk->nFirstIndexId;
  float *f0 = (float *)&p[inds[0]*size];
  float *f1 = (float *)&p[inds[1]*size];
  float *f2 = (float *)&p[inds[2]*size];
  Vec3d p0 = Vec3d(f0[0], f0[1], f0[2]);
  Vec3d p1 = Vec3d(f1[0], f1[1], f1[2]);
  Vec3d p2 = Vec3d(f2[0], f2[1], f2[2]);
  pl.CalcPlane(p2, p1, p0);
}
