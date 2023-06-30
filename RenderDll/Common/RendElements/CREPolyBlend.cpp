/*=============================================================================
	CPolyBlend.cpp : implementation of blended polygons RE.
	Copyright (c) 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#include "RenderPCH.h"

//===============================================================

bool CREPolyBlend::mfCull(CCObject *obj)
{
  CREPolyBlend::mRS.NumPolys++;

  if ((obj->m_ObjFlags & FOB_NEAREST))
    return false;

  return false;
}

bool CREPolyBlend_Base::mfPrepareRB(CCObject *obj, Vec3d& orgo, CFColor& col)
{
  if (obj->m_ObjFlags & FOB_DRSUN)
  {
    orgo = gRenDev->m_RP.m_ViewOrg + 2000.0f * gRenDev->m_RP.m_SunDir;
  }
  /*else
  if (obj->m_ObjFlags & FOB_DRPLANET)
  {
    orgo = obj->m_Trans + gRenDev->m_RP.m_ViewOrg;
    orgo[2] += 2000;
  }*/
  else
  {
    orgo = obj->GetTranslation();
  }

  col = obj->m_Color;

  if (col[0]+col[1]+col[2] == 0)
    col = CFColor(1.0f);

  if (obj->m_ObjFlags & FOB_DRSUN)
  {
    CREFlareGeom *fl = (CREFlareGeom *)obj->m_RE;
    fl->mfCheckVis(col, obj);
    if (!col.a)
      return false; // Flare not visible
    col.ScaleCol(col.a);
    col.a = 1;
  }
  else
  if (eColStyle || eAlphaStyle)
  {
    float frac;
    if (eColStyle)
    {
      col = Col_White;
      if (eColStyle == ePBCS_Decay)
      {
        frac = (gRenDev->m_RP.m_RealTime-obj->m_StartTime)/LiveTime;
        col.ScaleCol(frac);
      }
    }
    if (eAlphaStyle)
    {
      if (eAlphaStyle == ePBCS_Decay)
      {
        if (obj->m_StartTime+LiveTimeA < gRenDev->m_RP.m_RealTime-0.001)
          frac = 1;
        else
          frac = (gRenDev->m_RP.m_RealTime-obj->m_StartTime)/LiveTimeA;
        col[3] = ValA0 + frac * (ValA1-ValA0);
      }
    }
  }
  else
  {
    CLightStyle *ls = CLightStyle::m_LStyles[obj->m_LightStyle];
    ls->mfUpdate(gRenDev->m_RP.m_RealTime);
    col = ls->m_Color;
    col[3] = 1.0f;
  }
  return true;
}

void CREPolyBlend_Base::mfSetVerts(CCObject *obj, Vec3d& orgo, uint c, SOrient *ori)
{
  Vec3d VecX, VecY, v;
  Vec3d org;
  float distX, distY;
  float *verts[4];
  Vec3d norm;
  bvec4 *cols;
  int i;
  static int inds[] = {3, 0, 2, 2, 0, 1};
  static float tverts[4][2] = 
  {
    {0, 0},
    {1, 0},
    {1, 1},
    {0, 1}
  };

  int n = gRenDev->m_RP.m_RendNumVerts;
  byte *p = gRenDev->m_RP.m_Ptr.PtrB + n*gRenDev->m_RP.m_Stride;
  verts[0] = (float *)p;
  p += gRenDev->m_RP.m_Stride;
  verts[1] = (float *)p;
  p += gRenDev->m_RP.m_Stride;
  verts[2] = (float *)p;
  p += gRenDev->m_RP.m_Stride;
  verts[3] = (float *)p;

  org = orgo;

  if (ori->m_Flags & FOR_ORTHO)
  {
    VecX = gRenDev->m_RP.m_CamVecs[1];
    VecY = gRenDev->m_RP.m_CamVecs[2];

    org += ori->m_Coord.m_Org;

    v = org - gRenDev->m_RP.m_ViewOrg;
    distX = v.GetLength();
    norm = -gRenDev->m_RP.m_CamVecs[0];
  }
  else
  {
    VecX = ori->m_Coord.m_Vecs[1];
    VecY = ori->m_Coord.m_Vecs[2];

    org += ori->m_Coord.m_Org;

    v = org - gRenDev->m_RP.m_ViewOrg;
    distX = v.GetLength();
    norm = -ori->m_Coord.m_Vecs[0];
  }
  if (gRenDev->m_RP.m_PersFlags & RBPF_DRAWMIRROR)
  {
    VecY = -VecY;
  }

  distY = distX;
  if (ScaleX > 0)
  {
    distX = ScaleX;
    distY = ScaleY;
  }
  else
  if (ScaleX < 0)
  {
    distX *= -ScaleX * 0.1f;
    distY *= -ScaleY * 0.1f;
  }
  else
    distX = distY = 10.0f;

  VecX *= distX;
  VecY *= distY;

  verts[0][0] = org[0]+VecX[0]+VecY[0];
  verts[0][1] = org[1]+VecX[1]+VecY[1];
  verts[0][2] = org[2]+VecX[2]+VecY[2];

  verts[1][0] = org[0]-VecX[0]+VecY[0];
  verts[1][1] = org[1]-VecX[1]+VecY[1];
  verts[1][2] = org[2]-VecX[2]+VecY[2];

  verts[2][0] = org[0]-VecX[0]-VecY[0];
  verts[2][1] = org[1]-VecX[1]-VecY[1];
  verts[2][2] = org[2]-VecX[2]-VecY[2];

  verts[3][0] = org[0]+VecX[0]-VecY[0];
  verts[3][1] = org[1]+VecX[1]-VecY[1];
  verts[3][2] = org[2]+VecX[2]-VecY[2];

  UPipeVertex ptr = gRenDev->m_RP.m_NextPtr;
  byte *OffsT, *OffsD;
  SMRendTexVert *rtvb;
  byte *OffsN;
  switch (gRenDev->m_RP.m_FT)
  {
    case FLT_BASE:
      cols = &gRenDev->m_RP.m_pClientColors[n];
      OffsT = gRenDev->m_RP.m_OffsT + ptr.PtrB;
      for (i=0; i<4; i++, cols++, OffsT+=gRenDev->m_RP.m_Stride)
      {
        *(float *)(OffsT) = tverts[i][0];
        *(float *)(OffsT+4) = tverts[i][1];
        *(uint *)cols = c;
      }
      break;

    case FLT_BASE + FLT_COL:
      OffsD = gRenDev->m_RP.m_OffsD + ptr.PtrB;
      OffsT = gRenDev->m_RP.m_OffsT + ptr.PtrB;
      for (i=0; i<4; i++, OffsT+=gRenDev->m_RP.m_Stride, OffsD+=gRenDev->m_RP.m_Stride)
      {
        *(float *)(OffsT) = tverts[i][0];
        *(float *)(OffsT+4) = tverts[i][1];
        *(uint *)OffsD = c;
      }
      break;

    case FLT_COL:
      OffsD = gRenDev->m_RP.m_OffsD + ptr.PtrB;
      for (i=0; i<4; i++, OffsD+=gRenDev->m_RP.m_Stride)
      {
        *(uint *)OffsD = c;
      }
      break;

    case FLT_COL + FLT_SYSBASE:
      OffsD = gRenDev->m_RP.m_OffsD + ptr.PtrB;
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      for (i=0; i<4; i++, OffsD+=gRenDev->m_RP.m_Stride)
      {
        Vector2Copy(tverts[i], rtvb[i].vert);
        *(uint *)OffsD = c;
      }
      break;

    case 0:
      cols = &gRenDev->m_RP.m_pClientColors[n];
      for (i=0; i<4; i++, cols++)
      {
        *(uint *)cols = c;
      }
      break;

    case FLT_SYSBASE:
      cols = &gRenDev->m_RP.m_pClientColors[n];
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      for (i=0; i<4; i++, cols++)
      {
        Vector2Copy(tverts[i], rtvb[i].vert);
        *(uint *)cols = c;
      }
      break;


    case FLT_BASE + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      cols = &gRenDev->m_RP.m_pClientColors[n];
      OffsT = gRenDev->m_RP.m_OffsT + ptr.PtrB;
      for (i=0; i<4; i++, cols++, OffsT+=gRenDev->m_RP.m_Stride)
      {
        *(float *)(OffsN) = norm.x;
        *(float *)(OffsN+4) = norm.y;
        *(float *)(OffsN+8) = norm.z;
        *(float *)(OffsT) = tverts[i][0];
        *(float *)(OffsT+4) = tverts[i][1];
        *(uint *)cols = c;
      }
      break;

    case FLT_BASE + FLT_COL + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      OffsD = gRenDev->m_RP.m_OffsD + ptr.PtrB;
      OffsT = gRenDev->m_RP.m_OffsT + ptr.PtrB;
      for (i=0; i<4; i++, OffsT+=gRenDev->m_RP.m_Stride, OffsD+=gRenDev->m_RP.m_Stride)
      {
        *(float *)(OffsN) = norm.x;
        *(float *)(OffsN+4) = norm.y;
        *(float *)(OffsN+8) = norm.z;
        *(float *)(OffsT) = tverts[i][0];
        *(float *)(OffsT+4) = tverts[i][1];
        *(uint *)OffsD = c;
      }
      break;

    case FLT_COL + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      OffsD = gRenDev->m_RP.m_OffsD + ptr.PtrB;
      for (i=0; i<4; i++, OffsD+=gRenDev->m_RP.m_Stride)
      {
        *(float *)(OffsN) = norm.x;
        *(float *)(OffsN+4) = norm.y;
        *(float *)(OffsN+8) = norm.z;
        *(uint *)OffsD = c;
      }
      break;

    case FLT_COL + FLT_SYSBASE + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      OffsD = gRenDev->m_RP.m_OffsD + ptr.PtrB;
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      for (i=0; i<4; i++, OffsD+=gRenDev->m_RP.m_Stride)
      {
        *(float *)(OffsN) = norm.x;
        *(float *)(OffsN+4) = norm.y;
        *(float *)(OffsN+8) = norm.z;
        Vector2Copy(tverts[i], rtvb[i].vert);
        *(uint *)OffsD = c;
      }
      break;

    case 0 + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      cols = &gRenDev->m_RP.m_pClientColors[n];
      for (i=0; i<4; i++, cols++)
      {
        *(float *)(OffsN) = norm.x;
        *(float *)(OffsN+4) = norm.y;
        *(float *)(OffsN+8) = norm.z;
        *(uint *)cols = c;
      }
      break;

    case FLT_SYSBASE + FLT_N:
      OffsN = gRenDev->m_RP.m_OffsN + ptr.PtrB;
      cols = &gRenDev->m_RP.m_pClientColors[n];
      rtvb = &gRenDev->m_RP.m_pBaseTexCoordPointer[n];
      for (i=0; i<4; i++, cols++)
      {
        *(float *)(OffsN) = norm.x;
        *(float *)(OffsN+4) = norm.y;
        *(float *)(OffsN+8) = norm.z;
        Vector2Copy(tverts[i], rtvb[i].vert);
        *(uint *)cols = c;
      }
      break;

    default:
      break;
  }
  if ((gRenDev->m_RP.m_FT & FLT_COL) && gbRgb)
  {
    OffsD = gRenDev->m_RP.m_OffsD + gRenDev->m_RP.m_NextPtr.PtrB;
    for (i=0; i<4; i++, OffsD+=gRenDev->m_RP.m_Stride)
    {
      *(uint *)(OffsD) = COLCONV(*(uint *)(OffsD));
    }
  }

  ptr.PtrB += gRenDev->m_RP.m_Stride*4;
  gRenDev->m_RP.m_NextPtr = ptr;

  ushort *dinds = &gRenDev->m_RP.m_RendIndices[gRenDev->m_RP.m_RendNumIndices];
  for (i=0; i<6; i++)
  {
    *dinds++ = inds[i]+n;
  }
}

void CREPolyBlend::mfPrepare(void)
{
  CCObject *obj = gRenDev->m_RP.m_pCurObject;
  if (!obj)
    return;

  int savev = gRenDev->m_RP.m_RendNumVerts;
  int savei = gRenDev->m_RP.m_RendNumIndices;

  CREPolyBlend::mRS.NumRendPolys++;

  SShader *ef = gRenDev->m_RP.m_pShader;
  CFColor col;
  Vec3d orgo;

  if (!mfPrepareRB(obj, orgo, col))
    return;

  uint c = col.GetTrue();

  for (int o=0; o<NumOrients; o++)
  {
    gRenDev->EF_CheckOverflow(4, 6, this);

    mfSetVerts(obj, orgo, c, Orients[o]);

    gRenDev->m_RP.m_RendNumVerts += 4;
    gRenDev->m_RP.m_RendNumIndices += 6;
  }

  CREPolyBlend::mRS.NumVerts += gRenDev->m_RP.m_RendNumVerts - savev; 
  CREPolyBlend::mRS.NumIndices += gRenDev->m_RP.m_RendNumIndices - savei; 
}

//=====================================================================

bool CREAnimPolyBlend::mfCull(CCObject *obj)
{
  CREPolyBlend::mRS.NumAnimPolys++;

  //if(gfCullSphere(obj->m_Trans, 100)==2)
  //  return true;

  return false;
}

bool CREPolyBlend::mfIsValidTime(SShader *ef, CCObject *obj, float curtime)
{
  if (!LiveTime)
    return true;

  if (curtime > LiveTime+obj->m_StartTime)
    return false;
  return true;
}

bool CREAnimPolyBlend::mfIsValidTime(SShader *ef, CCObject *obj, float curtime)
{
  int i, j;
  SShaderTexUnit *shm;
  SShaderPass *sfm;

  for (i=0; i<ef->m_Passes.Num(); i++)
  {
    sfm = &ef->m_Passes[i];

    for (j=0; j<sfm->m_TUnits.Num(); j++)
    {
      shm = &sfm->m_TUnits[j];
      if (!shm->m_AnimInfo)
        continue;
      if (shm->m_AnimInfo->m_Time && shm->m_AnimInfo->m_TexPics.Num())
      {
        float t = curtime - obj->m_StartTime;
        int m = (int)(t / shm->m_AnimInfo->m_Time);
        if (m >= shm->m_AnimInfo->m_TexPics.Num())
          return false;
      }
    }
  }
  return true;
}

void CREAnimPolyBlend::mfPrepare(void)
{
  CCObject *obj = gRenDev->m_RP.m_pCurObject;
  if (!obj)
    return;
  
  int savev = gRenDev->m_RP.m_RendNumVerts;
  int savei = gRenDev->m_RP.m_RendNumIndices;
  
  CREPolyBlend::mRS.NumRendPolys++;
  
  SShader *ef = gRenDev->m_RP.m_pShader;
  CFColor col;
  Vec3d orgo;
  
  if (!mfPrepareRB(obj, orgo, col))
    return;
  
  uint c = col.GetTrue();
  
  for (int o=0; o<NumOrients; o++)
  {
    gRenDev->EF_CheckOverflow(4, 6, this);
    
    mfSetVerts(obj, orgo, c, Orients[o]);
    
    gRenDev->m_RP.m_RendNumVerts += 4;
    gRenDev->m_RP.m_RendNumIndices += 6;
  }
  
  CREPolyBlend::mRS.NumVerts += gRenDev->m_RP.m_RendNumVerts - savev; 
  CREPolyBlend::mRS.NumIndices += gRenDev->m_RP.m_RendNumIndices - savei; 
}


//=======================================================================

SPolyBlendStat CREPolyBlend::mRS;

void CREPolyBlend::mfPrintStat()
{
/*  char str[1024];

  *gpCurPrX = 4;
  sprintf(str, "Num Indices: %i\n", mRS.NumIndices);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Verts: %i\n", mRS.NumVerts);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Render AnimPolys: %i\n", mRS.NumAnimRendPolys);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Occluding AnimPolys: %i\n", mRS.NumAnimPolys);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Render Polys: %i\n", mRS.NumRendPolys);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Occluding Polys: %i\n", mRS.NumPolys);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  gRenDev->mfPrintString ("\nBlended Polygons status info:\n", PS_TRANSPARENT | PS_UP);*/
}

//===================================================================================================

// Parsing

void CREPolyBlend_Base::mfCompileOrients(SShader *ef, int *nums, SOrient *Orients[], char *scr)
{
  if (!scr || !scr[0])
  {
    Warning( 0,0,"Can't declare orient for effector '%s'\n", ef->m_Name.c_str());
    *nums = 1;
    Orients[0] = &gRenDev->m_cEF.m_Orients[0];
  }
  else
  {
    int ors[16];
    *nums = sscanf(scr, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i", &ors[0], &ors[1], &ors[2], &ors[3], &ors[4], &ors[5], &ors[6], &ors[7], &ors[8], &ors[9], &ors[10], &ors[11], &ors[12], &ors[13], &ors[14], &ors[15]);
    if (!(*nums))
    {
      Warning( 0,0,"Can't declare orient for effector '%s'\n", ef->m_Name.c_str());
      *nums = 1;
      Orients[0] = &gRenDev->m_cEF.m_Orients[0];
    }
    else
    {
      for (int i=0; i<*nums; i++)
      {
        if (ors[i] >= gRenDev->m_cEF.m_NumOrients)
        {
          Warning( 0,0,"Can't declare %d orient (Use Ortho mode)\n", ors[i]);
          *nums = 1;
          Orients[0] = &gRenDev->m_cEF.m_Orients[0];
          return;
        }
        Orients[i] = &gRenDev->m_cEF.m_Orients[ors[i]];
      }
    }
  }
}


bool CREPolyBlend_Base::mfCompile(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eTrace=1, eType, eScale, eScalX, eScalY, eOrients, eRGBStyle, eAlphaStyle};
  static tokenDesc commands[] =
  {
    {eTrace, "Trace"},
    {eType, "Type"},
    {eScale, "Scale"},
    {eScalX, "ScalX"},
    {eScalY, "ScalY"},
    {eOrients, "Orients"},
    {eRGBStyle, "RGBStyle"},
    {eAlphaStyle, "AlphaStyle"},

    {0,0}
  };

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;

    switch (cmd)
    {
      case eTrace:
        mfUpdateFlags(FCEFPB_TRACE);
        break;

      case eType:
        if (!stricmp(data, "Beam"))
          this->eType = ePBT_Beam;
        else
          Warning( 0,0,"unknown Type parameter '%s' in Shader '%s' (CREPolyBlend)\n", data, ef->m_Name.c_str());
        break;

      case eScale:
        ScaleX = ScaleY = shGetFloat(data);
        break;

      case eScalX:
        ScaleX = shGetFloat(data);
        break;

      case eScalY:
        ScaleY = shGetFloat(data);
        break;

      case eRGBStyle:
        if (!stricmp(data, "Decay"))
        {
          eColStyle = ePBCS_Decay;
          if (!params || !params[0])
          {
            Warning( 0,0,"missing RgbStyle Decay value in Shader '%s' (skipped) (CREPolyBlend)\n", ef->m_Name.c_str());
            Val0 = 1;
            Val1 = 0;
            LiveTime = 1;
          }
          else
          {
            sscanf(params, "%f %f %f", &Val0, &Val1, &LiveTime);
          }
        }
        else
          Warning( 0,0,"unknown RgbStyle parameter '%s' in Shader '%s' (CREPolyBlend)\n", data, ef->m_Name.c_str());

      case eAlphaStyle:
        if (!stricmp(data, "Decay"))
        {
          this->eAlphaStyle = ePBCS_Decay;
          if (!params || !params[0])
          {
            Warning( 0,0,"missing RgbStyle Decay value in Shader '%s' (skipped) (CREPolyBlend)\n", ef->m_Name.c_str());
            ValA0 = 1;
            ValA1 = 0;
            LiveTimeA = 1;
          }
          else
          {
            sscanf(params, "%f %f %f", &ValA0, &ValA1, &LiveTimeA);
          }
        }
        else
          Warning( 0,0,"unknown RgbStyle parameter '%s' in Shader '%s' (CREPolyBlend)\n", data, ef->m_Name.c_str());


      case eOrients:
        mfCompileOrients(ef, &NumOrients, Orients, params);
        if (Orients[0] == &gRenDev->m_cEF.m_Orients[0])
          mfUpdateFlags(FCEFPB_ORTHO);
        break;
    }
  }

  return true;
}
