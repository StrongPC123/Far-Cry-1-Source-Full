/*=============================================================================
	CREPolyMesh.cpp : implementation of mesh polygons RE.
	Copyright (c) 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#include "RenderPCH.h"
#include <CREPolyMesh.h>


//===============================================================

SPolyStat CREPolyMesh::mRS;

CREPolyMesh::~CREPolyMesh()
{
  if (TriVerts)
    delete [] TriVerts;
  if (Indices)
    delete [] Indices;
  if (bNoDeform)
    delete [] bNoDeform;
}

void CREPolyMesh::mfGetPlane(Plane& pl)
{
  pl.n = m_Plane.n;
  pl.d = m_Plane.d;
}

CRendElement *CREPolyMesh::mfCopyConstruct(void)
{
  CREPolyMesh *pm = new CREPolyMesh;
  *pm = *this;
  return pm;
}

bool CREPolyMesh::mfCullFace(ECull cl)
{
  CREPolyMesh::mRS.NumOccPolys++;

  if (cl != eCULL_None)
  {
    float d = m_Plane.n * gRenDev->m_RP.m_ViewOrg;
    if (cl == eCULL_Front)
    {
      if (d < m_Plane.d-8.0f)
        return true;
    }
    else
    {
      if (d > m_Plane.d+8.0f)
        return true;
    }
  }

  CREPolyMesh::mRS.NumRendPolys++;

  return false;
}

void CREPolyMesh::mfCenter(Vec3d& centr, CCObject *pObj)
{
  int i;

  centr(0,0,0);
  for (i=0; i<NumVerts; i++)
  {
    centr += TriVerts[i].vert;
  }
  float s = 1.0f / NumVerts;
  centr *= s;
  if (pObj)
    centr += pObj->GetTranslation();
}

int CREPolyMesh::mfTransform(Matrix44& ViewMatr, Matrix44& ProjMatr, vec4_t *verts, vec4_t *vertsp, int Num)
{
  int i, j;

  for (i=0; i<NumVerts; i++)
  {
    if (i == Num)
      break;
    for (j=0; j<4; j++)
    {
      verts[i][j] = TriVerts[i].vert[0]*ViewMatr(0,j) + TriVerts[i].vert[1]*ViewMatr(1,j) + TriVerts[i].vert[2]*ViewMatr(2,j) + ViewMatr(3,j);
    }
  }
  
  for (i=0; i<NumVerts; i++)
  {
    if (i == Num)
      break;
    for (j=0; j<4; j++)
    {
      vertsp[i][j] = verts[i][0]*ProjMatr(0,j) + verts[i][1]*ProjMatr(1,j) + verts[i][2]*ProjMatr(2,j) + verts[i][3]*ProjMatr(3,j);
    }
  }
  return NumVerts;
}

void CREPolyMesh::mfPrepare(void)
{
  ushort *inds;
  int i, n;
  SShader *ef = gRenDev->m_RP.m_pShader;

  CREPolyMesh::mRS.NumVerts += NumVerts;
  CREPolyMesh::mRS.NumIndices += NumIndices;

  // Check overflow
  gRenDev->EF_CheckOverflow(NumVerts, NumIndices, this);

  inds = Indices;
  n = gRenDev->m_RP.m_RendNumVerts;
#ifdef OPENGL
  ushort *dinds = &gRenDev->m_RP.m_RendIndices[gRenDev->m_RP.m_RendNumIndices];
#else
  ushort *dinds = &gRenDev->m_RP.m_RendIndices[gRenDev->m_RP.m_RendNumIndices];
#endif
  i = NumIndices;
  gRenDev->m_RP.m_RendNumIndices += i;
  while(i--)
  {
    *dinds++ = *inds++ + n;
  }

  UPipeVertex ptr = gRenDev->m_RP.m_NextPtr;
  //SMRendTexVert *rtvb, *rtvl;
  SMTriVert *tv = TriVerts;
  //byte *OffsN;
  int m = NumVerts;

  /*switch (gRenDev->m_RP.mFT)
  {
    case FLT_BASE:
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        Vector2Copy(tv[i].dTC, ptr.Ptr_D_1T->st);
      }
      break;

    case FLT_BASE + FLT_COL:
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_1T->color.dcolor = -1;
        Vector2Copy(tv[i].dTC, ptr.Ptr_D_1T->st);
      }
      break;

    case FLT_SYSBASE:
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride, rtvb++)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        Vector2Copy(tv[i].dTC, rtvb->vert);
      }
      break;

    case FLT_SYSBASE + FLT_COL:
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride, rtvb++)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_1T->color.dcolor = -1;
        Vector2Copy(tv[i].dTC, rtvb->vert);
      }
      break;

    case FLT_BASE + FLT_LM:
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        Vector2Copy(tv[i].dTC, ptr.Ptr_D_2T->st[0]);
        Vector2Copy(tv[i].lmTC, ptr.Ptr_D_2T->st[1]);
      }
      break;

    case FLT_BASE + FLT_LM + FLT_COL:
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_2T->color.dcolor = -1;
        Vector2Copy(tv[i].dTC, ptr.Ptr_D_2T->st[0]);
        Vector2Copy(tv[i].lmTC, ptr.Ptr_D_2T->st[1]);
      }
      break;

    case FLT_SYSBASE + FLT_LM:
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride, rtvb++)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        Vector2Copy(tv[i].lmTC, ptr.Ptr_D_2T->st[1]);
        Vector2Copy(tv[i].dTC, rtvb->vert);
      }
      break;

    case FLT_SYSBASE + FLT_LM + FLT_COL:
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride, rtvb++)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_2T->color.dcolor = -1;
        Vector2Copy(tv[i].lmTC, ptr.Ptr_D_2T->st[1]);
        Vector2Copy(tv[i].dTC, rtvb->vert);
      }
      break;

    case FLT_SYSBASE + FLT_SYSLM:
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      rtvl = &gRenDev->m_RP.m_pLMTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride, rtvb++, rtvl++)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        Vector2Copy(tv[i].lmTC, rtvl->vert);
        Vector2Copy(tv[i].dTC, rtvb->vert);
      }
      break;

    case FLT_SYSBASE + FLT_SYSLM + FLT_COL:
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      rtvl = &gRenDev->m_RP.m_pLMTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride, rtvb++, rtvl++)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_2T->color.dcolor = -1;
        Vector2Copy(tv[i].lmTC, rtvl->vert);
        Vector2Copy(tv[i].dTC, rtvb->vert);
      }
      break;


    case FLT_BASE + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        Vector2Copy(tv[i].dTC, ptr.Ptr_D_1T->st);
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_BASE + FLT_COL + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_1T->color.dcolor = -1;
        Vector2Copy(tv[i].dTC, ptr.Ptr_D_1T->st);
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_SYSBASE + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        Vector2Copy(tv[i].dTC, rtvb[i].vert);
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_SYSBASE + FLT_COL + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_1T->color.dcolor = -1;
        Vector2Copy(tv[i].dTC, rtvb[i].vert);
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_BASE + FLT_LM + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        Vector2Copy(tv[i].dTC, ptr.Ptr_D_2T->st[0]);
        Vector2Copy(tv[i].lmTC, ptr.Ptr_D_2T->st[1]);
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_BASE + FLT_LM + FLT_COL + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_2T->color.dcolor = -1;
        Vector2Copy(tv[i].dTC, ptr.Ptr_D_2T->st[0]);
        Vector2Copy(tv[i].lmTC, ptr.Ptr_D_2T->st[1]);
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_SYSBASE + FLT_LM + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        Vector2Copy(tv[i].lmTC, ptr.Ptr_D_2T->st[1]);
        Vector2Copy(tv[i].dTC, rtvb[i].vert);
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_SYSBASE + FLT_LM + FLT_COL + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_2T->color.dcolor = -1;
        Vector2Copy(tv[i].lmTC, ptr.Ptr_D_2T->st[1]);
        Vector2Copy(tv[i].dTC, rtvb[i].vert);
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_SYSBASE + FLT_SYSLM + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      rtvl = &gRenDev->m_RP.m_pLMTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        Vector2Copy(tv[i].lmTC, rtvl[i].vert);
        Vector2Copy(tv[i].dTC, rtvb[i].vert);
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_SYSBASE + FLT_SYSLM + FLT_COL + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      rtvl = &gRenDev->m_RP.m_pLMTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_2T->color.dcolor = -1;
        Vector2Copy(tv[i].lmTC, rtvl[i].vert);
        Vector2Copy(tv[i].dTC, rtvb[i].vert);
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_COL + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_2T->color.dcolor = -1;
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_LM + FLT_COL + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_2T->color.dcolor = -1;
        Vector2Copy(tv[i].lmTC, ptr.Ptr_D_2T->st[1]);
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_SYSLM + FLT_COL + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      rtvl = &gRenDev->m_RP.m_pLMTexCoordPointer[n];
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_2T->color.dcolor = -1;
        Vector2Copy(tv[i].lmTC, rtvl[i].vert);
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case FLT_COL:
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        ptr.Ptr_D_2T->color.dcolor = -1;
      }
      break;

    case FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
        *(float *)(OffsN) = m_Plane.n.x;
        *(float *)(OffsN+4) = m_Plane.n.y;
        *(float *)(OffsN+8) = m_Plane.n.z;
      }
      break;

    case 0:
      for (i=0; i<m; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
      {
        ptr.Ptr_D_1T->xyz = tv[i].vert;
      }
      break;

    default:
      break;
  }*/

  gRenDev->m_RP.m_NextPtr = ptr;
  gRenDev->m_RP.m_RendNumVerts += NumVerts;
}

void CREPolyMesh::mfPrintStat()
{
//  char str[1024];

/*  *gpCurPrX = 4;
  sprintf(str, "Num Indices: %i\n", mRS.NumIndices);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Verts: %i\n", mRS.NumVerts);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Rend. Details: %i\n", mRS.NumRendDetails);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Calc. Details: %i\n", mRS.NumDetails);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Rend Polys: %i\n", mRS.NumRendPolys);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num BSP Occluded Polys: %i\n", mRS.NumOccPolys);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  gRenDev->mfPrintString ("\nPolys status info:\n", PS_TRANSPARENT | PS_UP);*/
}
