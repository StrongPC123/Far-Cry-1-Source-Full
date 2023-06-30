/*=============================================================================
	CClientPoly2D.cpp : implementation of 2D Client polygons RE.
	Copyright (c) 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#include "RenderPCH.h"


//===============================================================


TArray<CREClientPoly2D *> CREClientPoly2D::mPolysStorage;

CRendElement *CREClientPoly2D::mfCopyConstruct(void)
{
  CREClientPoly2D *cp = new CREClientPoly2D;
  *cp = *this;
  return cp;
}

void CREClientPoly2D::mfPrepare(void)
{
  CRenderer *rd = gRenDev;
  SShader *ef = rd->m_RP.m_pShader;
  byte *inds;
  int i, n;
  SColorVert2D *tv;

  CREClientPoly2D::mRS.NumRendPolys++;

  int savev = rd->m_RP.m_RendNumVerts;
  int savei = rd->m_RP.m_RendNumIndices;

  rd->EF_CheckOverflow(mNumVerts, mNumIndices/3, this);
  
  rd->m_RP.m_FlagsPerFlush |= RBSI_DRAWAS2D;

  inds = mIndices;
  n = rd->m_RP.m_RendNumVerts;
  ushort *dinds = &rd->m_RP.m_RendIndices[gRenDev->m_RP.m_RendNumIndices];
  for (i=0; i<mNumIndices; i++, dinds++, inds++)
  {
    *dinds = *inds+n;
  }
  rd->m_RP.m_RendNumIndices += i;

  tv = mVerts;
  UPipeVertex ptr = rd->m_RP.m_NextPtr;
  byte *OffsT, *OffsD;
  switch(rd->m_RP.m_CurVFormat)
  {
    case VERTEX_FORMAT_TRP3F_COL4UB_TEX2F:
      OffsT = rd->m_RP.m_OffsT + ptr.PtrB;
      OffsD = rd->m_RP.m_OffsD + ptr.PtrB;
      for (i=0; i<mNumVerts; i++, tv++, ptr.PtrB+=rd->m_RP.m_Stride, OffsT+=rd->m_RP.m_Stride, OffsD+=rd->m_RP.m_Stride)
      {
        *(float *)(ptr.PtrB+0) = tv->vert[0];
        *(float *)(ptr.PtrB+4) = tv->vert[1];
        *(float *)(ptr.PtrB+8) = 0.0f;
        *(float *)(ptr.PtrB+12) = 1.0f;
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

  rd->m_RP.m_NextPtr = ptr;
  rd->m_RP.m_RendNumVerts += mNumVerts;

  CREClientPoly2D::mRS.NumVerts += rd->m_RP.m_RendNumVerts - savev; 
  CREClientPoly2D::mRS.NumIndices += rd->m_RP.m_RendNumIndices - savei; 
}


//=======================================================================

SClientPolyStat2D CREClientPoly2D::mRS;

void CREClientPoly2D::mfPrintStat()
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
