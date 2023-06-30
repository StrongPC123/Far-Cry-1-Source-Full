/*=============================================================================
  EvalFuncs_RE.cpp : RE specific effectors pipeline implementation.
  Copyright 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "shadow_renderer.h"
#include <IEntityRenderState.h>

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#ifdef WIN64
#pragma warning( push )							//AMD Port
#pragma warning( disable : 4311 )				// 'type cast' : pointer truncation from 'byte *' to 'int'
#endif

//============================================================================


_inline void Deform(float val, float* vrt, float *norm)
{
  vrt[0] += val * norm[0];
  vrt[1] += val * norm[1];
  vrt[2] += val * norm[2];
}


void SEvalFuncs_RE::WaveDeform(SDeform *df)
{
  int i, val;
  float f;

  int Str, StrNRM;
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &Str, GL_FLOAT, eSrcPointer_Vert, FGP_REAL | FGP_WAIT);
  byte *norms = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrNRM, GL_FLOAT, eSrcPointer_TNormal, FGP_SRC | FGP_REAL);
  if ((int)verts < 256)
    return;
  gRenDev->m_RP.m_pRE->mfUpdateFlags(FCEF_MODIF_VERT);
  int nv = gRenDev->m_RP.m_RendNumVerts;
  float *WaveTable;
  
  switch (df->m_DeformGen.m_eWFType)
  {
    case eWF_Sin:
    default:
      WaveTable = gRenDev->m_RP.m_tSinTable;
      break;
    case eWF_Triangle:
      WaveTable = gRenDev->m_RP.m_tTriTable;
      break;
    case eWF_Square:
      WaveTable = gRenDev->m_RP.m_tSquareTable;
      break;
    case eWF_SawTooth:
      WaveTable = gRenDev->m_RP.m_tSawtoothTable;
      break;
    case eWF_InvSawTooth:
      WaveTable = gRenDev->m_RP.m_tInvSawtoothTable;
      break;
    case eWF_Hill:
      WaveTable = gRenDev->m_RP.m_tHillTable;
      break;
  }
  for (i=0; i<nv; i++, verts+=Str, norms+=StrNRM)
  {
    float *v = (float *)verts;
    float *vnrm = (float *)norms;
    f = v[0] + v[1] + v[2];
    f = f*df->m_ScaleVerts + gRenDev->m_RP.m_RealTime*df->m_DeformGen.m_Freq + df->m_DeformGen.m_Phase;
    f *= 1024.0;
    val = QRound(f);

    f = df->m_DeformGen.m_Amp * CRenderer::CV_r_wavescale * WaveTable[val&0x3ff] + df->m_DeformGen.m_Level;

    Deform(f, v, vnrm);
  }
}

static int sTrisIndsToPolyInds(ushort *indsSrc, int nInds, int nVerts, ushort *indsDst)
{
  int i, j, ii, jj, n;
  indsDst[0] = indsSrc[0];
  int nTris = nInds / 3;
  int nDstVerts = 1;
  do
  {
    for (i=0; i<nTris; i++)
    {
      int iInd0 = indsDst[nDstVerts-1];
      int iInd1;
      for (j=0; j<3; j++)
      {
        if (indsSrc[i*3+j] != iInd0)
          continue;
        iInd1 = indsSrc[i*3+(j+1)%3];
        if (nDstVerts == 1)
        {
          if (iInd1 == indsDst[0])
            continue;
        }
        else
        {
          for (n=1; n<nDstVerts; n++)
          {
            if (indsDst[n] == iInd1)
              break;
          }
          if (n != nDstVerts)
            continue;
        }
        for (ii=0; ii<nTris; ii++)
        {
          if (ii == i)
            continue;
          for (jj=0; jj<3; jj++)
          {
            if (indsSrc[ii*3+jj] != iInd1)
              continue;
            if (indsSrc[ii*3+((jj+1)%3)] == iInd0)
              break;
          }
          if (jj != 3)
            break;
        }
        if (ii == nTris)
        {
          indsDst[nDstVerts] = iInd1;
          nDstVerts++;
          break;
        }
      }
      if (j != 3)
        break;
    }
    if (nDstVerts == nVerts)
      break;
  } while (i != nTris);

  return nDstVerts;
}

static ushort sFlareIndices[18][3] = 
{
  {0, 4, 5},
  {0, 5, 6},
  {0, 6, 7},
  {0, 7, 1},
  {1, 7, 8},
  {1, 8, 9},
  {15, 4, 0},
  {15, 0, 3},
  {3, 0, 1},
  {3, 1, 2},
  {2, 1, 9},
  {2, 9, 10},
  {14, 15, 3},
  {14, 3, 13},
  {13, 3, 2},
  {13, 2, 12},
  {12, 2, 11},
  {11, 2, 10},
};

void SEvalFuncs_RE::FlareDeform(SDeform *df)
{
  int Str, i;
  CRenderer *rd = gRenDev;
  byte *vbSrc = (byte *)rd->EF_GetPointer(eSrcPointer_Vert, &Str, GL_FLOAT, eSrcPointer_Vert, FGP_SRC);
	// [Sergiy] Probably because the characters don't have system buffers,
	//          this is NULL for C:\MASTERCD\Objects\glm\ww2_indust_set1\lights\light_indust3\light_indust3.cgf 
	if (!vbSrc)
		return; 
  int nv = rd->m_RP.m_RendNumVerts;
  if (rd->m_RP.m_pRE->mfGetType() != eDATA_OcLeaf)
    return;
  if (!CRenderer::CV_r_procflares)
  {
    rd->m_RP.m_pRE = NULL;
    rd->m_RP.m_RendNumIndices = 0;
    rd->m_RP.m_RendNumVerts = 0;
    return;
  }

  CREOcLeaf *re = (CREOcLeaf *)rd->m_RP.m_pRE;
  if ((nv != 4 && nv != 6) || re->m_pChunk->nNumIndices != 6)
    return;
  Vec3d vbSrcData[4];
  ushort *indsS = &re->m_pBuffer->GetIndices(NULL)[re->m_pChunk->nFirstIndexId];
  if (nv == 6)
  {
    for (i=0; i<3; i++)
    {
      vbSrcData[i] = *(Vec3d *)(&vbSrc[(i+re->m_pChunk->nFirstVertId)*Str]);
    }
    vbSrcData[i] = *(Vec3d *)(&vbSrc[(3+re->m_pChunk->nFirstVertId)*Str]);
  }
  else
  {
    for (i=0; i<4; i++)
    {
      vbSrcData[i] = *(Vec3d *)(&vbSrc[(i+re->m_pChunk->nFirstVertId)*Str]);
    }
  }

  int n = rd->m_RP.m_CurTempMeshes->Num();
  rd->m_RP.m_CurTempMeshes->GrowReset(1);
  CRETempMesh *tm = rd->m_RP.m_CurTempMeshes->Get(n);
  if (!tm)
  {
    tm = new CRETempMesh;
    rd->m_RP.m_CurTempMeshes->Get(n) = tm;
  }
  assert (!tm->m_VBuffer);
  tm->m_VBuffer = rd->CreateBuffer(16, VERTEX_FORMAT_P3F_COL4UB_TEX2F, "CRETempMesh", true);
  rd->CreateIndexBuffer(&tm->m_Inds, &sFlareIndices[0][0], 54);
  rd->m_RP.m_pRE = tm;
  ushort indsSrc[6];
  if (nv == 6)
  {
    indsSrc[0] = 0;
    indsSrc[1] = 1;
    indsSrc[2] = 2;
    indsSrc[3] = 3;
    indsSrc[4] = 2;
    indsSrc[5] = 1;
    nv = 4;
  }
  else
  {
    for (i=0; i<6; i++)
    {
      indsSrc[i] = indsS[i] - re->m_pChunk->nFirstVertId;
    }
  }
  /*for (i=0; i<2; i++)
  {
    Exchange(indsSrc[i*3+0], indsSrc[i*3+2]);
  }*/
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vbDstData = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F[16];
  Vec3d v0 = vbSrcData[indsSrc[2]] - vbSrcData[indsSrc[1]];
  Vec3d v1 = vbSrcData[indsSrc[0]] - vbSrcData[indsSrc[1]];
  Vec3d vCross = v0.Cross(v1);
  float fDot = vCross.Dot(vCross);
  float fLen = cry_sqrtf(fDot);
  vCross *= 1.0f / fLen;
  if (vCross.x == 0 && vCross.y == 0)
  {
    if (vCross.z <= 0)
      vCross.z = -1.0f;
    else
      vCross.z = 1.0f;
  }
  else
  if (vCross.x == 0 && vCross.z == 0)
  {
    if (vCross.y <= 0)
      vCross.y = -1.0f;
    else
      vCross.y = 1.0f;
  }
  else
  if (vCross.y == 0 && vCross.z == 0)
  {
    if (vCross.x <= 0)
      vCross.x = -1.0f;
    else
      vCross.x = 1.0f;
  }
  else
  {
    if (fabs(vCross.x) == 1.0f)
    {
      vCross.y = 0;
      vCross.z = 0;
    }
    else
    if (fabs(vCross.y) == 1.0f)
    {
      vCross.x = 0;
      vCross.z = 0;
    }
    else
    if (fabs(vCross.z) == 1.0f)
    {
      vCross.x = 0;
      vCross.y = 0;
    }
  }
  float f = -vCross.Dot(vbSrcData[indsSrc[1]]);
  Vec3d vOrg;
  SCoord crd;
  Matrix44 mat = rd->m_RP.m_pCurObject->GetInvMatrix();
  TransformPosition(vOrg, rd->m_RP.m_ViewOrg, mat);
  fDot = vOrg.Dot(vCross) + f;
  if (fDot <= 0)
  {
    gRenDev->ReleaseIndexBuffer(&tm->m_Inds);
    delete [] vbDstData;
    rd->m_RP.m_RendNumIndices = 0;
    rd->m_RP.m_RendNumVerts = 0;
    return;
  }
  Vec3d v = vbSrcData[0];
  for (i=1; i<nv; i++)
  {
    v += vbSrcData[i];
  }
  Vec3d vMiddle = v * (1.0f / (float)nv);
  Vec3d vDir = vOrg - vMiddle;
  vDir.Normalize();
  float fIntens = vDir.Dot(vCross) * 8.0f;
  if (fIntens > 1.0f)
    fIntens = 1.0f;
  SShader *lsh = rd->m_RP.m_pShader;
  float r, g, b;
  r = g = b = fIntens;
  if (lsh && lsh->m_EvalLights)
  {
    SLightEval *le = lsh->m_EvalLights;
    if (le->m_LightStyle && le->m_LightStyle<CLightStyle::m_LStyles.Num())
    {
      float fTime = iTimer->GetCurrTime();
      CLightStyle *ls = CLightStyle::m_LStyles[le->m_LightStyle];
      ls->mfUpdate(fTime);
      switch (le->m_EStyleType)
      {
        case eLS_Intensity:
        default:
          r = fIntens * ls->m_fIntensity;
          g = fIntens * ls->m_fIntensity;
          b = fIntens * ls->m_fIntensity;
          break;
        case eLS_RGB:
          r = ls->m_Color.r * fIntens;
          g = ls->m_Color.g * fIntens;
          b = ls->m_Color.b * fIntens;
          break;
      }
    }
  }
  if (gRenDev->m_RP.m_pShaderResources && gRenDev->m_RP.m_pShaderResources->m_LMaterial)
  {
    r *= gRenDev->m_RP.m_pShaderResources->m_LMaterial->Front.m_Ambient.r;
    g *= gRenDev->m_RP.m_pShaderResources->m_LMaterial->Front.m_Ambient.g;
    b *= gRenDev->m_RP.m_pShaderResources->m_LMaterial->Front.m_Ambient.b;
  }

  r = CLAMP(r, 0.0f, 1.0f);
  g = CLAMP(g, 0.0f, 1.0f);
  b = CLAMP(b, 0.0f, 1.0f);
  byte br = (byte)(r*255.0f);
  byte bg = (byte)(g*255.0f);
  byte bb = (byte)(b*255.0f);
  for (i=0; i<tm->m_VBuffer->m_NumVerts; i++)
  {
    if (!gRenDev->m_TexMan->m_bRGBA)
    {
      vbDstData[i].color.bcolor[0] = bb;
      vbDstData[i].color.bcolor[1] = bg;
      vbDstData[i].color.bcolor[2] = br;
      vbDstData[i].color.bcolor[3] = 255;
    }
    else
    {
      vbDstData[i].color.bcolor[2] = br;
      vbDstData[i].color.bcolor[1] = bg;
      vbDstData[i].color.bcolor[0] = bb;
      vbDstData[i].color.bcolor[3] = 255;
    }
  }
  float flareSize = df->m_fFlareSize * CRenderer::CV_r_flaresize;
  ushort iPolyInds[16];
  int m = sTrisIndsToPolyInds(indsSrc, re->m_pChunk->nNumIndices, nv, iPolyInds);
  if (m != 4)
  {
    gRenDev->ReleaseIndexBuffer(&tm->m_Inds);
    delete [] vbDstData;
    rd->m_RP.m_RendNumIndices = 0;
    rd->m_RP.m_RendNumVerts = 0;
    return;
  }
  Vec3d vFlareVecs[4][3];
  for (i=0; i<4; i++)
  {
    int n = 3-i;
    vbDstData[i].xyz = vbSrcData[iPolyInds[i]];
    vbDstData[i].st[0] = 0.5f;
    vbDstData[i].st[1] = 0.5f;

    Vec3d v0 = vbSrcData[iPolyInds[i]] - vOrg;
    v0.Normalize();
    Vec3d v1 = vbSrcData[iPolyInds[(i+1)%4]] - vOrg;
    v1.Normalize();
    vFlareVecs[i][1] = v1.Cross(v0);
    vFlareVecs[i][1].Normalize();
    vFlareVecs[i][1].Flip();
    Vec3d v2 = vbSrcData[iPolyInds[(i+3)%4]] - vOrg;
    v2.Normalize();
    vFlareVecs[i][0] = v2.Cross(v0);
    vFlareVecs[i][0].Normalize();
    vFlareVecs[i][2] = vFlareVecs[i][0] + vFlareVecs[i][1];
    vFlareVecs[i][2].Normalize();
  }
  vbDstData[4].xyz = vFlareVecs[0][0] * flareSize + vbSrcData[iPolyInds[0]];
  vbDstData[5].xyz = vFlareVecs[0][2] * flareSize + vbSrcData[iPolyInds[0]];
  vbDstData[6].xyz = vFlareVecs[0][1] * flareSize + vbSrcData[iPolyInds[0]];

  vbDstData[7].xyz = vFlareVecs[1][0] * flareSize + vbSrcData[iPolyInds[1]];
  vbDstData[8].xyz = vFlareVecs[1][2] * flareSize + vbSrcData[iPolyInds[1]];
  vbDstData[9].xyz = vFlareVecs[1][1] * flareSize + vbSrcData[iPolyInds[1]];

  vbDstData[10].xyz = vFlareVecs[2][0] * flareSize + vbSrcData[iPolyInds[2]];
  vbDstData[11].xyz = vFlareVecs[2][2] * flareSize + vbSrcData[iPolyInds[2]];
  vbDstData[12].xyz = vFlareVecs[2][1] * flareSize + vbSrcData[iPolyInds[2]];

  vbDstData[13].xyz = vFlareVecs[3][0] * flareSize + vbSrcData[iPolyInds[3]];
  vbDstData[14].xyz = vFlareVecs[3][2] * flareSize + vbSrcData[iPolyInds[3]];
  vbDstData[15].xyz = vFlareVecs[3][1] * flareSize + vbSrcData[iPolyInds[3]];

  for (i=0; i<12; i++)
  {
    vDir = vbDstData[i+4].xyz - vOrg;
    float fLength = vDir.Length();
    vDir *= 1.0f / fLength;
    float f = -(fDot / vDir.Dot(vCross));
    if (f > 0 && fLength > f)
    {
      v = vOrg + vDir * f;
      vbDstData[i+4].xyz = v;
    }
    vbDstData[i+4].st[0] = 0;
    vbDstData[i+4].st[1] = 0.5f;
  }
  rd->UpdateBuffer(tm->m_VBuffer, vbDstData, 16, true);
  delete [] vbDstData;
  rd->m_RP.m_FirstVertex = 0;
  rd->m_RP.m_FirstIndex = 0;
  rd->m_RP.m_RendNumIndices = 54;
  rd->m_RP.m_RendNumVerts = 16;

#if defined (DIRECT3D8) || defined (DIRECT3D9)
  rd->m_RP.m_CurVFormat = VERTEX_FORMAT_P3F_COL4UB_TEX2F;
#endif
}

void SEvalFuncs_RE::BeamDeform(SDeform *df)
{
  int StrV, StrN, StrT, i;
  CRenderer *rd = gRenDev;
  byte *vbV = (byte *)rd->EF_GetPointer(eSrcPointer_Vert, &StrV, GL_FLOAT, eSrcPointer_Vert, FGP_SRC);
  byte *vbN = (byte *)rd->EF_GetPointer(eSrcPointer_Normal, &StrN, GL_FLOAT, eSrcPointer_Normal, FGP_SRC);
  byte *vbT = (byte *)rd->EF_GetPointer(eSrcPointer_Tex, &StrT, GL_FLOAT, eSrcPointer_Tex, FGP_SRC);
	// [Sergiy] Probably because the characters don't have system buffers,
	//          this is NULL for C:\MASTERCD\Objects\glm\ww2_indust_set1\lights\light_indust3\light_indust3.cgf 
	if (!vbV || !vbN || !vbT || !CRenderer::CV_r_beams || !rd->m_RP.m_pRE || rd->m_RP.m_pRE->mfGetType() != eDATA_OcLeaf)
  {
    rd->m_RP.m_pRE = NULL;
    rd->m_RP.m_RendNumIndices = 0;
    rd->m_RP.m_RendNumVerts = 0;
		return; 
  }
  int nv = rd->m_RP.m_RendNumVerts;
  int ni = rd->m_RP.m_RendNumIndices;
  CREOcLeaf *pRE = (CREOcLeaf *)rd->m_RP.m_pRE;
  CLeafBuffer *pLB = pRE->m_pBuffer;

  int n = rd->m_RP.m_CurTempMeshes->Num();
  rd->m_RP.m_CurTempMeshes->GrowReset(1);
  CRETempMesh *tm = rd->m_RP.m_CurTempMeshes->Get(n);
  if (!tm)
  {
    tm = new CRETempMesh;
    rd->m_RP.m_CurTempMeshes->Get(n) = tm;
  }
  assert (!tm->m_VBuffer);
  tm->m_VBuffer = rd->CreateBuffer(nv, VERTEX_FORMAT_P3F_COL4UB_TEX2F, "CRETempMesh", true);
  rd->CreateIndexBuffer(&tm->m_Inds, &pLB->m_SecIndices[0], ni);
  rd->m_RP.m_pRE = tm;
  CFColor StartColor, EndColor;
  Vec3d CameraPos;
  SParamComp_User pu;
  pu.m_Name = "startcolor[0]";
  StartColor[0] = pu.mfGet();
  pu.m_Name = "startcolor[1]";
  StartColor[1] = pu.mfGet();
  pu.m_Name = "startcolor[2]";
  StartColor[2] = pu.mfGet();
  pu.m_Name = "startcolor[3]";
  StartColor[3] = pu.mfGet();

  pu.m_Name = "endcolor[0]";
  EndColor[0] = pu.mfGet();
  pu.m_Name = "endcolor[1]";
  EndColor[1] = pu.mfGet();
  pu.m_Name = "endcolor[2]";
  EndColor[2] = pu.mfGet();
  pu.m_Name = "endcolor[3]";
  EndColor[3] = pu.mfGet();

  Matrix44& im = rd->m_RP.m_pCurObject->GetInvMatrix();
  TransformPosition(CameraPos, rd->m_RP.m_ViewOrg, im);
  pu.m_Name = "origlength";
  float OrigLength = pu.mfGet();
  pu.m_Name = "origwidth";
  float OrigWidth = pu.mfGet();
  pu.m_Name = "startradius";
  float StartRadius = pu.mfGet();
  pu.m_Name = "endradius";
  float EndRadius = pu.mfGet();
  pu.m_Name = "length";
  float Length = pu.mfGet();
  float fiOrigLength = 1.0f/OrigLength;
  float fiOrigWidth = 1.0f/OrigWidth;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vbDst = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F [nv];
  
  for (i=0; i<nv; i++, vbV+=StrV, vbN+=StrN, vbT+=StrT)
  {
    Vec3d *vPos = (Vec3 *)vbV;
    Vec3d *vNormal = (Vec3 *)vbN;

    // fLerp = vrt->x/OrigLength
    float fLerp = vPos->x * fiOrigLength;

    vbDst[i].xyz.x = fLerp * Length;
    float fCurRadius = LERP(StartRadius, EndRadius, fLerp);
    vbDst[i].xyz.y = vPos->y * fiOrigWidth * fCurRadius;
    vbDst[i].xyz.z = vPos->z * fiOrigWidth * fCurRadius;

    CFColor color = LERP(StartColor, EndColor, fLerp);
    color.Clamp();

    Vec3 camVec = CameraPos - *vPos;
    camVec.Normalize();

    float d = camVec.Dot(*vNormal);
    d = d * d;
    d *= min(fLerp*10.0f, 1.0f);

    color.a = color.a * d;
    if (gbRgb)
    {
      vbDst[i].color.bcolor[0] = (byte)(color.r*255.0f);
      vbDst[i].color.bcolor[1] = (byte)(color.g*255.0f);
      vbDst[i].color.bcolor[2] = (byte)(color.b*255.0f);
      vbDst[i].color.bcolor[3] = (byte)(color.a*255.0f);
    }
    else
    {
      vbDst[i].color.bcolor[2] = (byte)(color.r*255.0f);
      vbDst[i].color.bcolor[1] = (byte)(color.g*255.0f);
      vbDst[i].color.bcolor[0] = (byte)(color.b*255.0f);
      vbDst[i].color.bcolor[3] = (byte)(color.a*255.0f);
    }
    vbDst[i].st[0] = *(float *)(vbT+0);
    vbDst[i].st[1] = *(float *)(vbT+4);
  }
  rd->UpdateBuffer(tm->m_VBuffer, vbDst, nv, true);
  delete [] vbDst;
  rd->m_RP.m_FirstVertex = 0;
  rd->m_RP.m_FirstIndex = 0;

#if defined (DIRECT3D8) || defined (DIRECT3D9)
  rd->m_RP.m_CurVFormat = VERTEX_FORMAT_P3F_COL4UB_TEX2F;
#endif
}

void SEvalFuncs_RE::VerticalWaveDeform(SDeform *df)
{
  int i, val;
  float f;

  int Str;
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &Str, GL_FLOAT, eSrcPointer_Vert, FGP_REAL | FGP_WAIT);
  if ((int)verts < 256)
    return;
  gRenDev->m_RP.m_pRE->mfUpdateFlags(FCEF_MODIF_VERT);
  int nv = gRenDev->m_RP.m_RendNumVerts;
  float *WaveTable;
  
  switch (df->m_DeformGen.m_eWFType)
  {
    case eWF_Sin:
    default:
      WaveTable = gRenDev->m_RP.m_tSinTable;
      break;
    case eWF_Triangle:
      WaveTable = gRenDev->m_RP.m_tTriTable;
      break;
    case eWF_Square:
      WaveTable = gRenDev->m_RP.m_tSquareTable;
      break;
    case eWF_SawTooth:
      WaveTable = gRenDev->m_RP.m_tSawtoothTable;
      break;
    case eWF_InvSawTooth:
      WaveTable = gRenDev->m_RP.m_tInvSawtoothTable;
      break;
    case eWF_Hill:
      WaveTable = gRenDev->m_RP.m_tHillTable;
      break;
  }
  for (i=0; i<nv; i++, verts+=Str)
  {
    float *v = (float *)verts;
    float vnrm[3] = {0, 0, 1};
    f = v[0] + v[1] + v[2];
    f = f*df->m_ScaleVerts + gRenDev->m_RP.m_RealTime*df->m_DeformGen.m_Freq + df->m_DeformGen.m_Phase;
    f *= 1024.0;
    val = QRound(f);

    f = df->m_DeformGen.m_Amp * CRenderer::CV_r_wavescale * WaveTable[val&0x3ff] + df->m_DeformGen.m_Level;

    Deform(f, v, vnrm);
  }
}

void SEvalFuncs_RE::SqueezeDeform(SDeform *df)
{
  int i;
  float f;

  int Str, StrNRM;
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &Str, GL_FLOAT, eSrcPointer_Vert, FGP_REAL | FGP_WAIT);
  if ((int)verts < 256)
    return;
  byte *norms = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrNRM, GL_FLOAT, eSrcPointer_TNormal, FGP_SRC | FGP_REAL);
  gRenDev->m_RP.m_pRE->mfUpdateFlags(FCEF_MODIF_VERT);
  int nv = gRenDev->m_RP.m_RendNumVerts;

  f = EvalWaveForm(&df->m_DeformGen);

  for (i=0; i<nv; i++, verts+=Str,norms+=StrNRM)
  {
    float *v = (float *)verts;
    float *n = (float *)norms;
    v[0] += f * n[0];
    v[1] += f * n[1];
    v[2] += f * n[2];
  }
}

void SEvalFuncs_RE::BulgeDeform(SDeform *df)
{
  int i;
  float f;
  int val;
  int Str, StrNRM, StrTC;
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &Str, GL_FLOAT, eSrcPointer_Vert, FGP_REAL | FGP_WAIT);
  if ((int)verts < 256)
    return;
  byte *norms = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrNRM, GL_FLOAT, eSrcPointer_TNormal, FGP_SRC | FGP_REAL);
  byte *tc = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Tex, &StrTC, GL_FLOAT, eSrcPointer_Tex, FGP_SRC | FGP_REAL);
  gRenDev->m_RP.m_pRE->mfUpdateFlags(FCEF_MODIF_VERT);
  int nv = gRenDev->m_RP.m_RendNumVerts;
  for (i=0; i<nv; i++, verts+=Str,norms+=StrNRM,tc+=StrTC)
  {
    float *t = (float *)tc;
    float *v = (float *)verts;
    float *n = (float *)norms;
    f = t[0] + t[1] + v[0] + v[1] + v[2];
    f = f * df->m_ScaleVerts + df->m_DeformGen.m_Phase + gRenDev->m_RP.m_RealTime*df->m_DeformGen.m_Freq;
    f *= 1024;
    val = QRound(f);
    val &= 0x3ff;

    f = df->m_DeformGen.m_Amp * CRenderer::CV_r_wavescale * gRenDev->m_RP.m_tSinTable[val] + df->m_DeformGen.m_Level;
    v[0] += f * n[0];
    v[1] += f * n[1];
    v[2] += f * n[2];
  }
}

//==================================================================================================
// Texture coords generate

void SEvalFuncs_RE::ETC_Environment(int ns)
{
  int i;
  float f, d;
  Vec3d v;

  int StrTC;
  int Str, StrNRM;
  byte *ptr = (byte *)gRenDev->EF_GetPointer((ESrcPointer)(eSrcPointer_Tex+ns), &StrTC, GL_FLOAT, (ESrcPointer)(eSrcPointer_Tex+ns), FGP_REAL | FGP_WAIT);
  if ((int)ptr < 256)
    return;
  byte *norms = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrNRM, GL_FLOAT, eSrcPointer_TNormal, FGP_SRC | FGP_REAL);
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &Str, GL_FLOAT, eSrcPointer_Vert, FGP_SRC | FGP_REAL);
  int nv = gRenDev->m_RP.m_RendNumVerts;
  gRenDev->m_RP.m_pRE->mfUpdateFlags(FCEF_MODIF_TC);
  Vec3d oTr = gRenDev->m_RP.m_pCurObject->GetTranslation();
  for (i=0; i<nv; i++, norms+=StrNRM,verts+=Str,ptr+=StrTC)
  {
    float *vrt = (float *)verts;
    v.x = oTr.x - vrt[0];
    v.y = oTr.y - vrt[1];
    v.z = oTr.z - vrt[2];
    v.Normalize();

		float* fptr=(float*)norms;
		//Vec3d nrm; nrm.Set( (float*)norms);
		Vec3d nrm; nrm.Set(fptr[0],fptr[1],fptr[2]);

		d = v | nrm;
    f = d * nrm.y;
    f += f;
    *(float *)(ptr) = ((f - v[1]) + 1.0f) * 0.5f;

    f = d * nrm.z;
    f += f;
    *(float *)(ptr+4) = 0.5f - ((f - v[2]) * 0.5f);
  }
}

void SEvalFuncs_RE::ETC_SphereMap(int ns)
{
  int i;
  CCObject *obj = gRenDev->m_RP.m_pCurObject;

  float r00 = gRenDev->m_ViewMatrix(0,0), r01 = gRenDev->m_ViewMatrix(0,1), r02 = gRenDev->m_ViewMatrix(0,2);
  float r10 = gRenDev->m_ViewMatrix(1,0), r11 = gRenDev->m_ViewMatrix(1,1), r12 = gRenDev->m_ViewMatrix(1,2);
  float r20 = gRenDev->m_ViewMatrix(2,0), r21 = gRenDev->m_ViewMatrix(2,1), r22 = gRenDev->m_ViewMatrix(2,2);

  // Loop through the vertices, transforming each one and calculating
  // the correct texture coordinates.
  int StrTC, StrNRM;
  byte *ptr = (byte *)gRenDev->EF_GetPointer((ESrcPointer)(eSrcPointer_Tex+ns), &StrTC, GL_FLOAT, (ESrcPointer)(eSrcPointer_Tex+ns), FGP_REAL | FGP_WAIT);
  if ((int)ptr < 256)
    return;
  byte *norms = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrNRM, GL_FLOAT, eSrcPointer_TNormal, FGP_SRC | FGP_REAL);
  int nv = gRenDev->m_RP.m_RendNumVerts;
  gRenDev->m_RP.m_pRE->mfUpdateFlags(FCEF_MODIF_TC);
  for( i=0; i<nv; i++, ptr+=StrTC,norms+=StrNRM )
  {
    float *nrm = (float *)norms;
    float nx = nrm[0];
    float ny = nrm[1];
    float nz = nrm[2];

    // Check the z-component, to skip any vertices that face backwards
    //if( nx*m13 + ny*m23 + nz*m33 > 0.0f )
    //  continue;

    // Assign the spheremap's texture coordinates
    *(float *)(ptr) = 0.5f * ( 1.0f + ( nx*r00 + ny*r10 + nz*r20 ) );
    *(float *)(ptr+4) = 0.5f * ( 1.0f - ( nx*r01 + ny*r11 + nz*r21 ) );
  }
}

void SEvalFuncs_RE::ETC_SphereMapEnvironment(int ns)
{
  int i;
  SCoord c; 

  float r00 = gRenDev->m_RP.m_pCurObject->m_Matrix(0,0), r01 = gRenDev->m_RP.m_pCurObject->m_Matrix(0,1), r02 = gRenDev->m_RP.m_pCurObject->m_Matrix(0,2);
  float r10 = gRenDev->m_RP.m_pCurObject->m_Matrix(1,0), r11 = gRenDev->m_RP.m_pCurObject->m_Matrix(1,1), r12 = gRenDev->m_RP.m_pCurObject->m_Matrix(1,2);
  float r20 = gRenDev->m_RP.m_pCurObject->m_Matrix(2,0), r21 = gRenDev->m_RP.m_pCurObject->m_Matrix(2,1), r22 = gRenDev->m_RP.m_pCurObject->m_Matrix(2,2);

  // Loop through the vertices, transforming each one and calculating
  // the correct texture coordinates.
  int StrTC, StrNRM;
  byte *ptr = (byte *)gRenDev->EF_GetPointer((ESrcPointer)(eSrcPointer_Tex+ns), &StrTC, GL_FLOAT, (ESrcPointer)(eSrcPointer_Tex+ns), FGP_REAL | FGP_WAIT);
  if ((int)ptr < 256)
    return;
  byte *norms = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrNRM, GL_FLOAT, eSrcPointer_TNormal, FGP_SRC | FGP_REAL);
  int nv = gRenDev->m_RP.m_RendNumVerts;
  gRenDev->m_RP.m_pRE->mfUpdateFlags(FCEF_MODIF_TC);
  for( i=0; i<nv; i++, ptr+=StrTC,norms+=StrNRM )
  {
    float *nrm = (float *)norms;
    float nx = nrm[0];
    float ny = nrm[1];
    float nz = nrm[2];

    // Check the z-component, to skip any vertices that face backwards
    //if( nx*m13 + ny*m23 + nz*m33 > 0.0f )
    //  continue;

    // Assign the spheremap's texture coordinates
    *(float *)(ptr) = 0.5f * ( 1.0f + ( nx*r00 + ny*r10 + nz*r20 ) );
    *(float *)(ptr+4) = 0.5f * ( 1.0f - ( nx*r01 + ny*r11 + nz*r21 ) );
  }
}

void SEvalFuncs_RE::ETC_ShadowMap(int ns)
{
  CRenderer *rd = gRenDev;

  rd->m_RP.m_pRE->m_CustomTexBind[ns] = -1;
  if (!rd->m_RP.m_pCurObject)
    return;
  
  assert(rd->m_RP.m_FlagsPerFlush & RBSI_SHADOWPASS);
  int nsFrust;
  if (rd->m_RP.m_pShader->m_eSort == eS_TerrainShadowPass)
    nsFrust = 0;
  else
    nsFrust = ns + rd->m_RP.m_nCurStartCaster;

  list2<ShadowMapLightSourceInstance> * lsources = (list2<ShadowMapLightSourceInstance>*)rd->m_RP.m_pCurObject->m_pShadowCasters;

//	bool bActiveShadowReceiving = false;

	// skip this stage if this entity was used to create this shadow map
  if(!lsources || nsFrust>=lsources->Count())
	{
		if(!lsources)
			Warning( 0,0,"Warning: SEvalFuncs_RE::ETC_ShadowMap: !lsources");
		else if(nsFrust<lsources->Count())
			Warning( 0,0,"Warning: SEvalFuncs_RE::ETC_ShadowMap: nsFrust<lsources->Count()");

    if ((rd->GetFeatures() & RFT_SHADOWMAP_SELFSHADOW) && !(rd->GetFeatures() & RFT_DEPTHMAPS))
      rd->m_RP.m_pRE->m_CustomTexBind[ns] = rd->m_TexMan->m_Text_Depth->m_Bind;
    else
      rd->m_RP.m_pRE->m_CustomTexBind[ns] = rd->m_TexMan->m_Text_WhiteShadow->m_Bind;

    if ((rd->GetFeatures() & RFT_SHADOWMAP_SELFSHADOW) && !(rd->GetFeatures() & RFT_DEPTHMAPS))
    {
      Matrix44 *mt = &rd->m_cEF.m_TempMatrices[ns][7];
      mt->SetIdentity();
    }

		rd->SelectTMU(0);
		return; // cancel this stage
	}
    
  if(!(*lsources)[nsFrust].m_pLS)
    return;

	// get projection frustum
  ShadowMapFrustum * pShadowMapFrustum = (*lsources)[nsFrust].m_pLS->GetShadowMapFrustum(0);
//	if(bActiveShadowReceiving)
	//	pShadowMapFrustum = (*lsources)[nsFrust].m_pLS->GetShadowMapFrustumPassiveCasters(0);
	if(!pShadowMapFrustum)
		return;

  // detect usage of same lsource second time -> use penumbra frustum
  if( nsFrust>0 && (*lsources)[nsFrust].m_pLS == (*lsources)[nsFrust-1].m_pLS && pShadowMapFrustum->pPenumbra)
    pShadowMapFrustum = pShadowMapFrustum->pPenumbra;

  Matrix44 *m = NULL;
  Vec3d vObjTrans;
  if (rd->m_RP.m_ObjFlags & FOB_TRANS_MASK)
  {
    m = &gRenDev->m_RP.m_pCurObject->m_Matrix;
    vObjTrans = rd->m_RP.m_pCurObject->GetTranslation();
  }
  else
    vObjTrans = Vec3(0,0,0);

  // setup projection
  gRenDev->SetupShadowOnlyPass(ns,    
    pShadowMapFrustum, 
    &((*lsources)[nsFrust].m_vProjTranslation), 
    (*lsources)[nsFrust].m_fProjScale,
    vObjTrans,
    1.f, 
    Vec3d(0,0,0),
    m);
}

void SEvalFuncs_RE::ETC_Projection(int ns, float *Mat, float wdt, float hgt)
{
  int i;
  int Str, StrTC;
  byte *ptr = (byte *)gRenDev->EF_GetPointer((ESrcPointer)(eSrcPointer_Tex+ns), &StrTC, GL_FLOAT, (ESrcPointer)(eSrcPointer_Tex+ns), FGP_REAL | FGP_WAIT);
  if ((int)ptr < 256)
    return;
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &Str, GL_FLOAT, eSrcPointer_Vert, FGP_SRC | FGP_REAL);
  int nv = gRenDev->m_RP.m_RendNumVerts;

  Vec3d Pos = gRenDev->m_RP.m_pCurObject->GetTranslation();
  
  for (i=0; i<nv; i++, verts+=Str, ptr+=StrTC)
  {
    float *v = (float *)verts;
    float tx = v[0]*Mat[0] + v[1]*Mat[4] + v[2]*Mat[8]  + Mat[12];
    float ty = v[0]*Mat[1] + v[1]*Mat[5] + v[2]*Mat[9]  + Mat[13];
    float tw = v[0]*Mat[3] + v[1]*Mat[7] + v[2]*Mat[11] + Mat[15];

    float s = tx / tw;
    float t = ty / tw;
    *(float *)(ptr+0) = s;
    *(float *)(ptr+4) = t;
  }
}


void SRendItem::mfCalcProjectVectors(int type, float *Mat, float RefractIndex, byte *Dst, int StrDst)
{
  int StrV, StrN;
  
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &StrV, GL_FLOAT, eSrcPointer_Vert, FGP_NOCALC | FGP_SRC | FGP_REAL);
  if ((int)verts < 256)
    return;
  byte *norms = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrN, GL_FLOAT, eSrcPointer_TNormal, FGP_NOCALC | FGP_SRC | FGP_REAL);

  if (RefractIndex < -1000.0f || RefractIndex > 1000.0f)
    RefractIndex = 0;
  
  int numVerts = gRenDev->m_RP.m_RendNumVerts;
  
  RefractIndex = CLAMP(RefractIndex, -5.0f, 5.0f);
  
  if (type == GL_UNSIGNED_BYTE)
  {
  }
  else
  if (type == GL_FLOAT)
  {
    for (int i=0; i<numVerts; i++, Dst+=StrDst, verts+=StrV, norms+=StrN)
    {
      float *v = (float *)verts;
      float *n = (float *)norms;
      Vec3d vv;

      vv.x = v[0]+n[0]*RefractIndex;
      vv.y = v[1]+n[1]*RefractIndex;
      vv.z = v[2]+n[2]*RefractIndex;

      float tx = v[0]*Mat[0] + v[1]*Mat[4] + v[2]*Mat[8] + Mat[12];
      float ty = v[0]*Mat[1] + v[1]*Mat[5] + v[2]*Mat[9] + Mat[13];

      float s = CLAMP(tx, -1.0f, 1.0f);
      float t = CLAMP(ty, -1.0f, 1.0f);
      *(float *)(Dst+0) = s;
      *(float *)(Dst+4) = t;
    }
  }
}


//=========================================================================================
//Calculate Lights and per-vertex color generation

void SEvalFuncs_RE::EALPHA_Beam()
{
  int i;
  Vec3d v;
  CCObject *obj = gRenDev->m_RP.m_pCurObject;
  if (!obj)
    return;
  CDLight *dl;

  for (i=0; i<gRenDev->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); i++)
  {
    dl = gRenDev->m_RP.m_DLights[SRendItem::m_RecurseLevel][i];
    if (dl->m_pObject[0][0] == obj)
      break;
  }
  if (i == gRenDev->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num())
    return;

  Vec3d Pos;
  CCamera cam = gRenDev->GetCamera();
  Vec3d p = cam.GetPos();
  TransformPosition(Pos, p, gRenDev->m_RP.m_pCurObject->GetInvMatrix());

  Vec3d lightForWard = Vec3d(1,0,0); //dl->m_Orientation.m_vForward;

  CREBeam *rb = (CREBeam *)obj->m_RE;
  if (!rb)
    return;

  int StrRGBA;
  int Str, StrVD, StrNRM;
  byte *ptr = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Color, &StrRGBA, GL_BYTE, eSrcPointer_Color, FGP_REAL | FGP_WAIT);
  if ((int)ptr < 256)
    return;
  byte *norms = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrNRM, GL_FLOAT, eSrcPointer_TNormal, FGP_SRC | FGP_REAL);
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &Str, GL_FLOAT, eSrcPointer_Vert, FGP_SRC | FGP_REAL);
  byte *vertsDst = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &StrVD, GL_FLOAT, eSrcPointer_Vert, FGP_REAL);
  int nv = gRenDev->m_RP.m_RendNumVerts;
  gRenDev->m_RP.m_pRE->mfUpdateFlags(FCEF_MODIF_COL | FCEF_MODIF_VERT);
  for (i=0; i<nv; i++, norms+=StrNRM,verts+=Str,vertsDst+=StrVD,ptr+=StrRGBA)
  {
    Vec3d *vrt = (Vec3d *)verts;
    Vec3d *vrtD = (Vec3d *)vertsDst;
    Vec3d v;
    v.x = vrt->x / rb->m_fLengthScale * rb->m_fLength;
    float fLerp = vrt->x / rb->m_fLengthScale;
    CFColor col = LERP(rb->m_StartColor, rb->m_EndColor, fLerp);
    float fCurRadius = LERP(rb->m_fStartRadius, rb->m_fEndRadius, fLerp);
    v.y = vrt->y / rb->m_fWidthScale * fCurRadius; 
    v.z = vrt->z / rb->m_fWidthScale * fCurRadius; 
    *vrtD = v;
    
    Vec3d *nrm = (Vec3d *)norms;
    Vec3d p = Pos - v;
    p.Normalize();
    float fd = p.Dot(lightForWard);
    if (fd < 0)
      fd = -fd;
    float d = p.Dot(*nrm);
    if (d < 0)
      d = -d;
    fd *= fd;
    d *= d;
    d += fd;
    d *= col.a;
    d = CLAMP(d, 0.0f, 1.0f);
    ptr[0] = (byte)(col.r * 255.0f); 
    ptr[1] = (byte)(col.g * 255.0f); 
    ptr[2] = (byte)(col.b * 255.0f); 
    ptr[3] = (byte)(d * 255.0f);
  }
}

//=======================================================================================

#ifdef WIN64
#pragma warning( pop )							//AMD Port
#endif