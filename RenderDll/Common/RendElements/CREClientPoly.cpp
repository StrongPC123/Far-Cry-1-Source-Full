/*=============================================================================
	CClientPoly.cpp : implementation of Client polygons RE.
	Copyright (c) 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#include "RenderPCH.h"
#include "RendElement.h"


//===============================================================


TArray<CREClientPoly *> CREClientPoly::mPolysStorage[4];

CRendElement *CREClientPoly::mfCopyConstruct(void)
{
  CREClientPoly *cp = new CREClientPoly;
  *cp = *this;
  return cp;
}

float CREClientPoly::mfDistanceToCameraSquared(const CCObject & thisObject)
{
  CRenderer *rd = gRenDev;

  if (m_fDistance >= 0)
    return m_fDistance;

  Vec3d vMid;
  vMid.Set(0,0,0);
  SColorVert *tv = mVerts;
  for (int i=0; i<mNumVerts; i++, tv++)
  {
    vMid += tv->vert;
  }
  vMid /= (float)mNumVerts;

  vMid += thisObject.GetTranslation();

  Vec3d Delta = rd->m_RP.m_ViewOrg - vMid;
  float fDist = GetLengthSquared(Delta);
  m_fDistance = fDist;

  return fDist;
}

void CREClientPoly::mfPrepare(void)
{
  CRenderer *rd = gRenDev;
  SShader *ef = rd->m_RP.m_pShader;
  byte *inds;
  int i, n;
  SColorVert *tv;

  CREClientPoly::mRS.NumRendPolys++;

  {
    //PROFILE_FRAME_TOTAL(Mesh_REPrepare_Flush3DPoly);
    rd->EF_CheckOverflow(mNumVerts, mNumIndices, this);
  }

  {
    //PROFILE_FRAME_TOTAL(Mesh_REPrepare_3DPoly);

    int savev = rd->m_RP.m_RendNumVerts;
    int savei = rd->m_RP.m_RendNumIndices;

    inds = mIndices;
    n = rd->m_RP.m_RendNumVerts;
    ushort *dinds = &rd->m_RP.m_RendIndices[rd->m_RP.m_RendNumIndices];
    for (i=0; i<mNumIndices; i++, dinds++, inds++)
    {
      *dinds = *inds+n;
    }
    rd->m_RP.m_RendNumIndices += i;

    byte *OffsT, *OffsD;
    tv = mVerts;
    UPipeVertex ptr = rd->m_RP.m_NextPtr;
    switch(rd->m_RP.m_CurVFormat)
    {
      case VERTEX_FORMAT_P3F_COL4UB_TEX2F:
        OffsT = rd->m_RP.m_OffsT + ptr.PtrB;
        OffsD = rd->m_RP.m_OffsD + ptr.PtrB;
        for (i=0; i<mNumVerts; i++, tv++, ptr.PtrB+=rd->m_RP.m_Stride, OffsT+=rd->m_RP.m_Stride, OffsD+=rd->m_RP.m_Stride)
        {
          *(Vec3 *)ptr.PtrB = tv->vert;
          *(float *)(OffsT) = tv->dTC[0];
          *(float *)(OffsT+4) = tv->dTC[1];
          *(uint *)OffsD = tv->color.dcolor;
        }
    	  break;
      default:
        assert(false);
    }
    if (rd->m_RP.m_OffsD && gbRgb)
    {
      OffsD = rd->m_RP.m_OffsD + rd->m_RP.m_NextPtr.PtrB;
      for (i=0; i<mNumVerts; i++, OffsD+=rd->m_RP.m_Stride)
      {
        *(uint *)(OffsD) = COLCONV(*(uint *)(OffsD));
      }
    }

    gRenDev->m_RP.m_NextPtr = ptr;
    gRenDev->m_RP.m_RendNumVerts += mNumVerts;

    CREClientPoly::mRS.NumVerts += gRenDev->m_RP.m_RendNumVerts - savev; 
    CREClientPoly::mRS.NumIndices += gRenDev->m_RP.m_RendNumIndices - savei; 
  }
}

bool CREClientPoly::mfCullBox(Vec3d& vmin, Vec3d& vmax)
{
  CREClientPoly::mRS.NumOccPolys++;

  //if(gfCullBox(vmin, vmax))
  //  return true;

  return false;
}


//=======================================================================

SClientPolyStat CREClientPoly::mRS;

void CREClientPoly::mfPrintStat()
{
/*  char str[1024];

  *gpCurPrX = 4;
  sprintf(str, "Num Indices: %i\n", mRS.NumIndices);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Verts: %i\n", mRS.NumVerts);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Render Client Polys: %i\n", mRS.NumRendPolys);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Occluded Client Polys: %i\n", mRS.NumOccPolys);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  gRenDev->mfPrintString ("\nClient Polygons status info:\n", PS_TRANSPARENT | PS_UP);*/
}
