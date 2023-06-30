/*=============================================================================
  RendElement.cpp : common RE functions.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"

//TArray<CRendElement *> CRendElement::m_AllREs;

CRendElement CRendElement::m_RootGlobal;

//===============================================================

TArray <CFColor> gCurLightStyles;

TArray<SRendItemPre> SRendItem::m_RendItems[NUMRI_LISTS];  

int SRendItem::m_RecurseLevel;
int SRendItem::m_StartRI[8][NUMRI_LISTS];
int SRendItem::m_EndRI[8][NUMRI_LISTS];

static _inline int Compare(SRendItemPre &a, SRendItemPre &b)
{
#ifndef PIPE_USE_INSTANCING
  if (a.SortVal.SortVal<b.SortVal.SortVal)
    return -1;
  else
  if (a.SortVal.SortVal>b.SortVal.SortVal)
    return 1;
  else
    return 0;
#else
  if (a.SortVal.i.High<b.SortVal.i.High)
    return -1;
  if (a.SortVal.i.High>b.SortVal.i.High)
    return 1;

  if (a.ObjSort < b.ObjSort)
    return -1;
  if (a.ObjSort > b.ObjSort)
    return 1;

  if (a.Item < b.Item)
    return -1;
  if (a.Item > b.Item)
    return 1;

  if (a.DynLMask < b.DynLMask)
    return -1;
  if (a.DynLMask > b.DynLMask)
    return 1;

  int nSa = a.SortVal.i.Low&~(0xfff<<20);
  int nSb = b.SortVal.i.Low&~(0xfff<<20);
  if (nSa < nSb)
    return -1;
  if (nSa > nSb)
    return 1;
  return 0;
#endif
}

static _inline int Compare(SRendItemStenc &a, SRendItemStenc &b)
{
#ifndef PIPE_USE_INSTANCING
  if (a.SortVal.SortVal<b.SortVal.SortVal)
    return -1;
  else
  if (a.SortVal.SortVal>b.SortVal.SortVal)
    return 1;
  else
    return 0;
#else
  if (a.SortVal.i.High<b.SortVal.i.High)
    return -1;
  if (a.SortVal.i.High>b.SortVal.i.High)
    return 1;

  if (a.ObjSort < b.ObjSort)
    return -1;
  if (a.ObjSort > b.ObjSort)
    return 1;

  uint Itema = (uint)a.Item;
  uint Itemb = (uint)b.Item;

  if (a.ObjSort & FOB_IGNOREREPOINTER)
  {
    int nObj = (a.SortVal.i.Low>>20) & 0xfff;
    CCObject *pObj = gRenDev->m_RP.m_VisObjects[nObj];
    Itema = FtoI(pObj->m_SortId);
  }
  if (b.ObjSort & FOB_IGNOREREPOINTER)
  {
    int nObj = (b.SortVal.i.Low>>20) & 0xfff;
    CCObject *pObj = gRenDev->m_RP.m_VisObjects[nObj];
    Itemb = FtoI(pObj->m_SortId);
  }

  if (Itema < Itemb)
    return -1;
  if (Itema > Itemb)
    return 1;

  if (a.DynLMask < b.DynLMask)
    return -1;
  if (a.DynLMask > b.DynLMask)
    return 1;

  int nSa = a.SortVal.i.Low&~(0xfff<<20);
  int nSb = b.SortVal.i.Low&~(0xfff<<20);
  if (nSa<nSb)
    return -1;
  if (nSa>nSb)
    return 1;
  return 0;
#endif
}

static _inline int Compare(SRendItem &a, SRendItem &b)
{
  float fDistA = a.fDist;
  float fDistB = b.fDist;
  if (fDistA+0.01f < fDistB)
    return 1;
  else
  if (fDistA > fDistB+0.01f)
    return -1;
  else
    return 0;
}

/*static _inline int Compare(SRendItemPre &a, SRendItemPre &b)
{
  if (a.SortVal.SortVal<b.SortVal.SortVal)
    return -1;
  else
  if (a.SortVal.SortVal>b.SortVal.SortVal)
    return 1;
  else
    return 0;
}*/


void SRendItem::mfSort(SRendItemPre *First, int Num)
{
  ::Sort(First, Num);
}

void SRendItem::mfSortForStencil(SRendItemPre *First, int Num)
{
  ::Sort((SRendItemStenc *)First, Num);
}

void SRendItem::mfSortByDist(SRendItemPre *First, int Num)
{
  CRenderer *rd = gRenDev;
  int i;
  for (i=0; i<Num; i++)
  {
    SRendItemPre *pRI = &First[i];
#ifndef PIPE_USE_INSTANCING
    int nObject = (pRI->SortVal.i.High>>15) & 0x7ff;
#else
    int nObject = (pRI->SortVal.i.Low>>20) & 0xfff;
#endif
    CCObject *pObj = rd->m_RP.m_VisObjects[nObject];
    float fAddDist = pObj->m_SortId+0.05f*(i&0x1f);
    //SShader *pSH = SShader::m_Shaders_known[(pRI->SortVal.i.High>>14) & 0xfff];
    pRI->fDist = pRI->Item->mfDistanceToCameraSquared(*pObj) + fAddDist + pRI->Item->m_SortId;
  }
  ::Sort((SRendItem *)First, Num);
  /*if (rd->m_LogFile)
    rd->Logv(SRendItem::m_RecurseLevel, "*** Start Dist sort ***\n");
  for (int i=0; i<Num; i++)
  {
    SRendItemPre *pRI = &First[i];
#ifndef PIPE_USE_INSTANCING
    int nObject = (pRI->SortVal.i.High>>15) & 0x7ff;
#else
    int nObject = (pRI->SortVal.i.Low>>20) & 0xfff;
#endif
    CCObject *pObj = rd->m_RP.m_VisObjects[nObject];
    float fAddDist = pObj->m_SortId;
    SShader *pSH = SShader::m_Shaders_known[(pRI->SortVal.i.High>>14) & 0xfff];
    float fDistNew = pRI->Item->mfDistanceToCameraSquared(*pObj);
    Vec3d vObj = pObj->GetTranslation();
    if (rd->m_LogFile)
      rd->Logv(SRendItem::m_RecurseLevel, "Item %d, Dist: %.3f, ObjDist: %.3f, REDist: %d, CamDist: %.3f, Sh: %s, Obj: %d [%.3f, %.3f, %.3f]\n", i, pRI->fDist, fAddDist, pRI->Item->m_SortId, fDistNew, pSH->m_Name.c_str(), nObject, vObj.x, vObj.y, vObj.z);
  }
  if (rd->m_LogFile)
    rd->Logv(SRendItem::m_RecurseLevel, "*** End Dist sort ***\n");*/
}

void SRendItem::mfSortByLight(SRendItemPre *First, int Num)
{
  ::Sort((SRendItemLight *)First, Num);
}

static _inline int Compare(SRendItemPreprocess &a, SRendItemPreprocess &b)
{
  CCObject *obj1 = a.m_Object;
  CCObject *obj2 = b.m_Object;
  int Flags1 = obj1->m_ObjFlags & FOB_CUBE_MASK; 
  int Flags2 = obj2->m_ObjFlags & FOB_CUBE_MASK;
  if (Flags1 < Flags2)
    return -1;
  if (Flags1 > Flags2)
    return 1;
  if (a.SortVal.SortVal < b.SortVal.SortVal)
    return -1;
  else
  if (a.SortVal.SortVal > b.SortVal.SortVal)
    return 1;
  else
    return 0;
}


void SRendItemPreprocess::mfSort(SRendItemPreprocess *First, int Num)
{
  ::Sort(First, Num);
}

//============================================================================

void CRendElement::mfPrepare()
{
}

CMatInfo *CRendElement::mfGetMatInfo() {return NULL;}
list2<CMatInfo> *CRendElement::mfGetMatInfoList() {return NULL;}
int CRendElement::mfGetMatId() {return -1;}
bool CRendElement::mfCull(CCObject *obj) {return false;}
bool CRendElement::mfCull(CCObject *obj, SShader *ef) {return false;}
void CRendElement::mfReset() {}
bool CRendElement::mfCullByClipPlane(CCObject *pObj) { return false; }

const char *CRendElement::mfTypeString()
{
  switch(m_Type)
  {
    case eDATA_Sky:
      return "Sky";
    case eDATA_Beam:
      return "Beam";
    	break;
    case eDATA_Poly:
      return "Poly";
    	break;
    case eDATA_Curve:
      return "Curve";
    	break;
    case eDATA_MotModel:
      return "MotModel";
    	break;
    case eDATA_MeshModel:
      return "MeshModel";
    	break;
    case eDATA_PolyBlend:
      return "PolyBlend";
    	break;
    case eDATA_AnimPolyBlend:
      return "AnimPolyBlend";
    	break;
    case eDATA_ClientPoly:
      return "ClientPoly";
    	break;
    case eDATA_ClientPoly2D:
      return "ClientPoly2D";
    	break;
    case eDATA_ParticleSpray:
      return "ParticleSpray";
    	break;
    case eDATA_TriMesh:
      return "TriMesh";
    	break;
    case eDATA_TriMeshShadow:
      return "TriMeshShadow";
    	break;
    case eDATA_Prefab:
      return "Prefab";
    	break;
    case eDATA_Flare:
      return "Flare";
    	break;
    case eDATA_FlareGeom:
      return "FlareGeom";
    	break;
    case eDATA_FlareProp:
      return "FlareProp";
    	break;
    case eDATA_Tree:
      return "Tree";
    	break;
    case eDATA_Tree_Leaves:
      return "Tree_Leaves";
    	break;
    case eDATA_Tree_Branches:
      return "Tree_Branches";
    	break;
    case eDATA_Terrain:
      return "Terrain";
    	break;
    case eDATA_SkyZone:
      return "SkyZone";
    	break;
    case eDATA_OcLeaf:
      return "OcLeaf";
    	break;
    case eDATA_FlashBang:
      return "FlashBang";
      break;
    case eDATA_TerrainSector:
      return "TerrainSector";
    	break;
    case eDATA_2DQuad:
      return "2DQuad";
    	break;
    case eDATA_FarTreeSprites:
      return "FarTreeSprites";
    	break;
    case eDATA_AnimModel:
      return "AnimModel";
    	break;
    case eDATA_MotionBlur:
      return "MotionBlur";
    	break;
    case eDATA_ShadowMapGen:
      return "ShadowMapGen";
    	break;
    case eDATA_TerrainDetailTextureLayers:
      return "TerrainDetailTextureLayers";
    	break;
    case eDATA_TerrainParticles:
      return "TerrainParticles";
    	break;
    case eDATA_Ocean:
      return "Ocean";
    	break;

    // tiago: added
    case eDATA_ScreenProcess: 
      return "ScreenProcess";
      break;

    default:
      return "Unknown";
    	break;
  }
}

CRendElement *CRendElement::mfCopyConstruct(void)
{
  CRendElement *re = new CRendElement;
  *re = *this;
  return re;
}
void CRendElement::mfCenter(Vec3d& centr, CCObject *pObj)
{
  centr(0,0,0);
}
void CRendElement::mfGetPlane(Plane& pl)
{
  pl.n = Vec3d(0,0,1);
  pl.d = 0;
}

void CRendElement::mfEndFlush() {}
void CRendElement::Release() { delete this;}
int  CRendElement::mfTransform(Matrix44& ViewMatr, Matrix44& ProjMatr, vec4_t *verts, vec4_t *vertsp, int Num) { return 0; }
bool CRendElement::mfIsValidTime(SShader *ef, CCObject *obj, float curtime) {return true;}
void CRendElement::mfBuildGeometry(SShader *ef) {}
bool CRendElement::mfCompile(SShader *ef, char *scr) {return true;};
CRendElement *CRendElement::mfCreateWorldRE(SShader *ef, SInpData *ds) {return NULL;}
bool CRendElement::mfDraw(SShader *ef, SShaderPass *sfm) {return false;}
void *CRendElement::mfGetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags) {return NULL;}
float CRendElement::mfDistanceToCameraSquared(const CCObject & thisObject) {return 0.1f;}

//=============================================================================

void SRendItem::mfCalcRefractVectors(int type, byte *Dst, int StrDst)
{
  int StrV, StrNorm;

  SParamComp_User pr;
  pr.m_Name = "refraction";
  float ri = pr.mfGet();
  
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &StrV, GL_FLOAT, eSrcPointer_Vert, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *normals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrNorm, GL_FLOAT, eSrcPointer_TNormal, FGP_NOCALC | FGP_SRC | FGP_REAL);

  ri = CLAMP(ri, -20.0f, 20.0f);

  Vec3d cur_cam_vec, cam;
  cam = gRenDev->GetCamera().GetPos();
  TransformPosition(cur_cam_vec, cam, gRenDev->m_RP.m_pCurObject->GetInvMatrix());

  int numVerts = gRenDev->m_RP.m_RendNumVerts;
  
  if (type == GL_UNSIGNED_BYTE)
  {
  }
  else
  if (type == GL_FLOAT)
  {
    for (int i=0; i<numVerts; i++, Dst+=StrDst, verts+=StrV, normals+=StrNorm)
    {
      float *v = (float *)verts;
      float *n = (float *)normals;

      float fENX = cur_cam_vec.x - v[0];
      float fENY = cur_cam_vec.y - v[1];
      float fENZ = cur_cam_vec.z - v[2];

      float fNDotE = n[0]*fENX + n[1]*fENY + n[2]*fENZ;

      float fNDotN = n[0]*n[0] + n[1]*n[1] + n[2]*n[2];
      
      fNDotE *= 1.0f/ri;

      Vec3d ref, refrot;
      
      ref.x = n[0]*fNDotE - fENX*fNDotN; 
      ref.y = n[1]*fNDotE - fENY*fNDotN;
      ref.z = n[2]*fNDotE - fENZ*fNDotN;

      TransformVector(refrot, ref, gRenDev->m_RP.m_pCurObject->m_Matrix);

      refrot.Normalize();

      float *d = (float *)Dst;
      d[0] = refrot.x;
      d[1] = refrot.y;
      d[2] = refrot.z;
    }
  }
}

void SRendItem::mfCalcHalfAngles(int type, byte *ha, int StrHA)
{
  int StrLV;
  byte *lv = (byte *)gRenDev->EF_GetPointer(eSrcPointer_LightVector, &StrLV, GL_FLOAT, eSrcPointer_LightVector, FGP_SRC | FGP_REAL);
  
  int StrTX, StrTY, StrVrt, StrTZ;
  byte *tangents = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Tangent, &StrTX, GL_FLOAT, eSrcPointer_Tangent, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *binormals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Binormal, &StrTY, GL_FLOAT, eSrcPointer_Binormal, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *tnormals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrTZ, GL_FLOAT, eSrcPointer_TNormal, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, FGP_NOCALC | FGP_SRC | FGP_REAL);

  Vec3d pos = gRenDev->GetCamera().GetPos();
  Vec3d objectSpaceEyePosition;
  TransformPosition(objectSpaceEyePosition, pos, gRenDev->m_RP.m_pCurObject->GetInvMatrix());

  int numVerts = gRenDev->m_RP.m_RendNumVerts;
  if (type == GL_FLOAT)
  {
    for (int i=0; i<numVerts; i++, verts+=StrVrt,lv+=StrLV,ha+=StrHA,tangents+=StrTX,binormals+=StrTY,tnormals+=StrTZ)
    {
      float *dlv = (float *)lv;

      const float *ty = (float *)tangents;
      const float *tx = (float *)binormals;
      const float *tz = (float *)tnormals;
      const float *v = (float *)verts;

      Vec3d vo;      /* object-space view vector */
      float *hat;    /* texture-space half angle */
      float x, y, z;
      
      vo.x = objectSpaceEyePosition.x - v[0];
      vo.y = objectSpaceEyePosition.y - v[1];
      vo.z = objectSpaceEyePosition.z - v[2];
  //    vo.Normalize();
      
      x = tx[0]*vo[0]+tx[1]*vo[1]+tx[2]*vo[2];
      y = ty[0]*vo[0]+ty[1]*vo[1]+ty[2]*vo[2];
      z = tz[0]*vo[0]+tz[1]*vo[1]+tz[2]*vo[2];
      
      hat = (float*) ha;
      hat[0] = dlv[0] + x;
      hat[1] = dlv[1] + y;
      hat[2] = dlv[2] + z;
      float invlen = 1.0f / cry_sqrtf(hat[0]*hat[0] + hat[1]*hat[1] + hat[2]*hat[2]);
      hat[0] *= invlen;
      hat[1] *= invlen;
      hat[2] *= invlen;
    }
  }
  else
  {
    for (int i=0; i<numVerts; i++, verts+=StrVrt,lv+=StrLV,ha+=StrHA,tangents+=StrTX,binormals+=StrTY,tnormals+=StrTZ)
    {
      float *dlv = (float *)lv;
      
      const float *tz = (float *)tnormals;
      const float *ty = (float *)tangents;
      const float *tx = (float *)binormals;
      const float *v = (float *)verts;
      
      Vec3d vo;      /* object-space view vector */
      byte *hat;    /* texture-space half angle */
      float x, y, z;
      
      vo.x = objectSpaceEyePosition.x - v[0];
      vo.y = objectSpaceEyePosition.y - v[1];
      vo.z = objectSpaceEyePosition.z - v[2];
      //    vo.Normalize();

      x = tx[0]*vo[0]+tx[1]*vo[1]+tx[2]*vo[2];
      y = ty[0]*vo[0]+ty[1]*vo[1]+ty[2]*vo[2];
      z = tz[0]*vo[0]+tz[1]*vo[1]+tz[2]*vo[2];
      
      hat = (byte*) ha;
      Vec3d h;
      h.x = dlv[0] + x;
      h.y = dlv[1] + y;
      h.z = dlv[2] + z;
      float invlen = 1.0f / cry_sqrtf(h.x*h.x + h.y*h.y + h.z*h.z);
      hat[0] = (byte)(h.x * invlen * 128.0f + 128.0f);
      hat[1] = (byte)(h.y * invlen * 128.0f + 128.0f);
      hat[2] = (byte)(h.z * invlen * 128.0f + 128.0f);
    }
  }
}

void SRendItem::mfCalcHalfAngles_Terrain(int type, byte *ha, int StrHA)
{
  int StrLV;
  byte *lv = (byte *)gRenDev->EF_GetPointer(eSrcPointer_LightVector, &StrLV, GL_FLOAT, eSrcPointer_LightVector, FGP_SRC | FGP_REAL);

  int StrN, StrVrt;
  byte *normals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrN, GL_FLOAT, eSrcPointer_TNormal, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, FGP_NOCALC | FGP_SRC | FGP_REAL);

  Vec3d pos = gRenDev->GetCamera().GetPos();
  Vec3d objectSpaceEyePosition;
  TransformPosition(objectSpaceEyePosition, pos, gRenDev->m_RP.m_pCurObject->GetInvMatrix());

  int numVerts = gRenDev->m_RP.m_RendNumVerts;
  float binorm[3] = {0,-1,0};
  if (type == GL_FLOAT)
  {
    for (int i=0; i<numVerts; i++, verts+=StrVrt,lv+=StrLV,ha+=StrHA,normals+=StrN)
    {
      float *dlv = (float *)lv;

      const float *n = (float *)normals;
      const float *v = (float *)verts;

      Vec3d vo;      /* object-space view vector */
      float *hat;    /* texture-space half angle */
      float x, y, z;

      vo.x = objectSpaceEyePosition.x - v[0];
      vo.y = objectSpaceEyePosition.y - v[1];
      vo.z = objectSpaceEyePosition.z - v[2];
      //    vo.Normalize();

      float tang[3];
      tang[0] = binorm[1]*n[2]-binorm[2]*n[1]; tang[1]=binorm[2]*n[0]-binorm[0]*n[2]; tang[2]=binorm[0]*n[1]-binorm[1]*n[0];

      x = binorm[0]*vo[0]+binorm[1]*vo[1]+binorm[2]*vo[2];
      y = tang[0]*vo[0]+tang[1]*vo[1]+tang[2]*vo[2];
      z = n[0]*vo[0]+n[1]*vo[1]+n[2]*vo[2];

      hat = (float*) ha;
      hat[0] = dlv[0] + x;
      hat[1] = dlv[1] + y;
      hat[2] = dlv[2] + z;
      float invlen = 1.0f / cry_sqrtf(hat[0]*hat[0] + hat[1]*hat[1] + hat[2]*hat[2]);
      hat[0] *= invlen;
      hat[1] *= invlen;
      hat[2] *= invlen;
    }
  }
  else
  {
    for (int i=0; i<numVerts; i++, verts+=StrVrt,lv+=StrLV,ha+=StrHA,normals+=StrN)
    {
      float *dlv = (float *)lv;

      const float *n = (float *)normals;
      const float *v = (float *)verts;

      Vec3d vo;      /* object-space view vector */
      byte *hat;    /* texture-space half angle */
      float x, y, z;

      vo.x = objectSpaceEyePosition.x - v[0];
      vo.y = objectSpaceEyePosition.y - v[1];
      vo.z = objectSpaceEyePosition.z - v[2];
      //    vo.Normalize();

      float tang[3];
      tang[0] = binorm[1]*n[2]-binorm[2]*n[1]; tang[1]=binorm[2]*n[0]-binorm[0]*n[2]; tang[2]=binorm[0]*n[1]-binorm[1]*n[0];

      x = binorm[0]*vo[0]+binorm[1]*vo[1]+binorm[2]*vo[2];
      y = tang[0]*vo[0]+tang[1]*vo[1]+tang[2]*vo[2];
      z = n[0]*vo[0]+n[1]*vo[1]+n[2]*vo[2];

      hat = (byte*) ha;
      Vec3d h;
      h.x = dlv[0] + x;
      h.y = dlv[1] + y;
      h.z = dlv[2] + z;
      float invlen = 1.0f / cry_sqrtf(h.x*h.x + h.y*h.y + h.z*h.z);
      hat[0] = (byte)(h.x * invlen * 128.0f + 128.0f);
      hat[1] = (byte)(h.y * invlen * 128.0f + 128.0f);
      hat[2] = (byte)(h.z * invlen * 128.0f + 128.0f);
    }
  }
}

void SRendItem::mfCalcLightVectors(byte *lv, int StrLV)
{
  CDLight *dl = gRenDev->m_RP.m_pCurLight;
  Vec3d objectSpaceLightPosition;
  TransformPosition(objectSpaceLightPosition, dl->m_Origin, gRenDev->m_RP.m_pCurObject->GetInvMatrix());
  
  int StrTX, StrTY, StrVrt,StrTZ;
  byte *tangents = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Tangent, &StrTX, GL_FLOAT, eSrcPointer_Tangent, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *binormals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Binormal, &StrTY, GL_FLOAT, eSrcPointer_Binormal, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *tnormals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrTZ, GL_FLOAT, eSrcPointer_TNormal, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, FGP_NOCALC | FGP_SRC | FGP_REAL);

  int numVerts = gRenDev->m_RP.m_RendNumVerts;
  for (int i=0; i<numVerts; i++, verts+=StrVrt,lv+=StrLV,tangents+=StrTX,binormals+=StrTY,tnormals+=StrTZ)
  {
    float *dlv = (float *)lv;

    const float *ty = (float *)tangents;
    const float *tx = (float *)binormals;
    const float *tz = (float *)tnormals;
    const float *v = (float *)verts;
    Vec3d lo;

    lo.x = objectSpaceLightPosition.x - v[0];
    lo.y = objectSpaceLightPosition.y - v[1];
    lo.z = objectSpaceLightPosition.z - v[2];

    dlv[0] = (tx[0]*lo[0] + tx[1]*lo[1] + tx[2]*lo[2]);
    dlv[1] = (ty[0]*lo[0] + ty[1]*lo[1] + ty[2]*lo[2]);
    dlv[2] = (tz[0]*lo[0] + tz[1]*lo[1] + tz[2]*lo[2]);
  }
}

void SRendItem::mfCalcLightVectors_Terrain(byte *lv, int StrLV)
{
  CDLight *dl = gRenDev->m_RP.m_pCurLight;
  Vec3d objectSpaceLightPosition;
  TransformPosition(objectSpaceLightPosition, dl->m_Origin, gRenDev->m_RP.m_pCurObject->GetInvMatrix());

  int StrN, StrVrt;
  byte *normals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrN, GL_FLOAT, eSrcPointer_TNormal, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, FGP_NOCALC | FGP_SRC | FGP_REAL);

  int numVerts = gRenDev->m_RP.m_RendNumVerts;

  float binorm[3] = {0,-1,0};
  for (int i=0; i<numVerts; i++, verts+=StrVrt,lv+=StrLV,normals+=StrN)
  {
    float *dlv = (float *)lv;

    const float *n = (float *)normals;
    const float *v = (float *)verts;
    Vec3d lo;

    lo.x = objectSpaceLightPosition.x - v[0];
    lo.y = objectSpaceLightPosition.y - v[1];
    lo.z = objectSpaceLightPosition.z - v[2];

    float tang[3];
    tang[0] = binorm[1]*n[2]-binorm[2]*n[1]; tang[1]=binorm[2]*n[0]-binorm[0]*n[2]; tang[2]=binorm[0]*n[1]-binorm[1]*n[0];

    dlv[0] = (binorm[0]*lo[0] + binorm[1]*lo[1] + binorm[2]*lo[2]);
    dlv[1] = (tang[0]*lo[0] + tang[1]*lo[1] + tang[2]*lo[2]);
    dlv[2] = (n[0]*lo[0] + n[1]*lo[1] + n[2]*lo[2]);
  }
}

void SRendItem::mfCalcNormLightVectors(byte *lv, int StrLV, int Type)
{
  CDLight *dl = gRenDev->m_RP.m_pCurLight;
  Vec3d objectSpaceLightPosition;
  TransformPosition(objectSpaceLightPosition, dl->m_Origin, gRenDev->m_RP.m_pCurObject->GetInvMatrix());
  
  int StrTX, StrTY, StrVrt, StrTZ;
  byte *tangents = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Tangent, &StrTX, GL_FLOAT, eSrcPointer_Tangent, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *binormals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Binormal, &StrTY, GL_FLOAT, eSrcPointer_Binormal, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *tnormals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrTZ, GL_FLOAT, eSrcPointer_TNormal, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, FGP_NOCALC | FGP_SRC | FGP_REAL);

  int numVerts = gRenDev->m_RP.m_RendNumVerts;
  if (Type == GL_FLOAT)
  {
    for (int i=0; i<numVerts; i++, verts+=StrVrt,lv+=StrLV,tangents+=StrTX,binormals+=StrTY,tnormals+=StrTZ)
    {
      float *dlv = (float *)lv;

      const float *ty = (float *)tangents;
      const float *tx = (float *)binormals;
      const float *tz = (float *)tnormals;
      const float *v = (float *)verts;
      Vec3d lo;

      lo.x = objectSpaceLightPosition.x - v[0];
      lo.y = objectSpaceLightPosition.y - v[1];
      lo.z = objectSpaceLightPosition.z - v[2];

      lo.Normalize();

      dlv[0] = (tx[0]*lo.x + tx[1]*lo.y + tx[2]*lo.z);
      dlv[1] = (ty[0]*lo.x + ty[1]*lo.y + ty[2]*lo.z);
      dlv[2] = (tz[0]*lo.x + tz[1]*lo.y + tz[2]*lo.z);
    }
  }
  else
  {
    for (int i=0; i<numVerts; i++, verts+=StrVrt,lv+=StrLV,tangents+=StrTX,binormals+=StrTY,tnormals+=StrTZ)
    {
      byte *dlv = (byte *)lv;
      
      const float *ty = (float *)tangents;
      const float *tx = (float *)binormals;
      const float *tz = (float *)tnormals;
      const float *v = (float *)verts;
      Vec3d lo;
      
      lo.x = objectSpaceLightPosition.x - v[0];
      lo.y = objectSpaceLightPosition.y - v[1];
      lo.z = objectSpaceLightPosition.z - v[2];
      
      lo.Normalize();

      dlv[0] = (byte)((tx[0]*lo.x + tx[1]*lo.y + tx[2]*lo.z) * 128.0f + 128.0f);
      dlv[1] = (byte)((ty[0]*lo.x + ty[1]*lo.y + ty[2]*lo.z) * 128.0f + 128.0f);
      dlv[2] = (byte)((tz[0]*lo.x + tz[1]*lo.y + tz[2]*lo.z) * 128.0f + 128.0f);
    }
  }
}


void SRendItem::mfCalcProjectAttenFromCamera(byte *dst, int Str)
{
  Vec3d pos = gRenDev->GetCamera().GetPos();
  Vec3d objectSpaceEyePosition;
  TransformPosition(objectSpaceEyePosition, pos, gRenDev->m_RP.m_pCurObject->GetInvMatrix());

  int StrTX, StrTY, StrVrt, StrTZ;
  byte *tangents = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Tangent, &StrTX, GL_FLOAT, eSrcPointer_Tangent, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *binormals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Binormal, &StrTY, GL_FLOAT, eSrcPointer_Binormal, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *tnormals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrTZ, GL_FLOAT, eSrcPointer_TNormal, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, FGP_NOCALC | FGP_SRC | FGP_REAL);
  
  int numVerts = gRenDev->m_RP.m_RendNumVerts;

  float distance = CRenderer::CV_r_detaildistance;
  if (gRenDev->m_bEditor)
    distance *= 100.0f;

  for (int i=0; i<numVerts; i++, verts+=StrVrt,dst+=Str,tangents+=StrTX,binormals+=StrTY,tnormals+=StrTZ)
  {
    float *d = (float *)dst;
    
    const float *ty = (float *)tangents;
    const float *tx = (float *)binormals;
    const float *tz = (float *)tnormals;
    const float *v = (float *)verts;

    Vec3d lo, dlv;
    
    lo.x = objectSpaceEyePosition.x - v[0];
    lo.y = objectSpaceEyePosition.y - v[1];
    lo.z = objectSpaceEyePosition.z - v[2];

    dlv.x = (tx[0]*lo[0] + tx[1]*lo[1] + tx[2]*lo[2]);
    dlv.y = (ty[0]*lo[0] + ty[1]*lo[1] + ty[2]*lo[2]);
    dlv.z = (tz[0]*lo[0] + tz[1]*lo[1] + tz[2]*lo[2]);

    Vec3d a;
    
    a = dlv / distance;
    d[0] = a.x + 0.5f;
    d[1] = a.y + 0.5f;
    d[2] = a.z + 0.5f;
  }
}

void SRendItem::mfCalcLAttenuationSpec0(byte *dst, int StrDst, byte *lv, int StrLV, int type)
{
  int numVerts = gRenDev->m_RP.m_RendNumVerts;

  CDLight *dl = gRenDev->m_RP.m_pCurLight;
  float brightness = dl->m_fRadius * 2.0f;
  float iBr = 1.0f / brightness;

  if (type == GL_UNSIGNED_BYTE)
  {
    float Ibr1 = iBr * 255.0f;
    for (int v=0; v<numVerts; v++, dst+=StrDst,lv+=StrLV)
    {
      Vec3d *l = (Vec3d *)lv;
      
      dst[0] = (byte)CLAMP(l->x * Ibr1 + 128.0f, 0.0f, 255.0f);
      dst[1] = (byte)CLAMP(l->y * Ibr1 + 128.0f, 0.0f, 255.0f);
      dst[2] = (byte)CLAMP(l->z * Ibr1 + 128.0f, 0.0f, 255.0f);
    }
  }
  else
  if (type == GL_FLOAT)
  {
    for (int v=0; v<numVerts; v++, dst+=StrDst,lv+=StrLV)
    {
      Vec3d *l = (Vec3d *)lv;
      float *d = (float *)dst;
      d[0] = CLAMP(l->x * iBr + 0.5f, 0.0f, 1.0f);
      d[1] = CLAMP(l->y * iBr + 0.5f, 0.0f, 1.0f);
      d[2] = CLAMP(l->z * iBr + 0.5f, 0.0f, 1.0f);
    }
  }
}

void SRendItem::mfCalcLAttenuationSpec1(byte *dst, int StrDst, byte *lv, int StrLV, int type)
{
  int numVerts = gRenDev->m_RP.m_RendNumVerts;

  CDLight *dl = gRenDev->m_RP.m_pCurLight;
  float brightness = dl->m_fRadius;
  
  if (type == GL_UNSIGNED_BYTE)
  {
    for (int v=0; v<numVerts; v++, dst+=StrDst,lv+=StrLV)
    {
      Vec3d *l = (Vec3d *)lv;
      dst[0] = (byte)CLAMP(l->x * 128.0f + 128.0f, 0.0f, 255.0f);
      dst[1] = (byte)CLAMP(l->y * 128.0f + 128.0f, 0.0f, 255.0f);
      dst[2] = (byte)CLAMP(l->z * 128.0f + 128.0f, 0.0f, 255.0f);
    }
  }
  else
  if (type == GL_FLOAT)
  {
    for (int v=0; v<numVerts; v++, dst+=StrDst,lv+=StrLV)
    {
      Vec3d *l = (Vec3d *)lv;
      float *d = (float *)dst;
      d[0] = CLAMP(l->x * 0.5f + 0.5f, 0.0f, 1.0f);
      d[1] = CLAMP(l->y * 0.5f + 0.5f, 0.0f, 1.0f);
      d[2] = CLAMP(l->z * 0.5f + 0.5f, 0.0f, 1.0f);
    }
  }
}

void SRendItem::mfCalcLightAttenuation(int type, byte *Dst, int StrideDst)
{
  Vec3d lpos;

  CDLight *dl = gRenDev->m_RP.m_pCurLight;
  lpos = dl->m_Origin;
  float delta = 1.0f;
  if (gRenDev->m_bEditor)
    delta = 100.0f;

  int numVerts = gRenDev->m_RP.m_RendNumVerts;
  
  int StrTX, StrTY, StrVrt, StrTZ;
  byte *tangents = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Tangent, &StrTX, GL_FLOAT, eSrcPointer_Tangent, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *binormals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Binormal, &StrTY, GL_FLOAT, eSrcPointer_Binormal, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *tnormals = (byte *)gRenDev->EF_GetPointer(eSrcPointer_TNormal, &StrTZ, GL_FLOAT, eSrcPointer_TNormal, FGP_NOCALC | FGP_SRC | FGP_REAL);
  byte *verts = (byte *)gRenDev->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, FGP_NOCALC | FGP_SRC | FGP_REAL);
  
  if (type == GL_UNSIGNED_BYTE)
  {
    byte *dst = Dst;
    for (int v=0; v<numVerts; v++, dst+=StrideDst,verts+=StrVrt,tangents+=StrTX,binormals+=StrTY,tnormals+=StrTZ)
    {
      Vec3d lvec = lpos - *(Vec3d*)verts;

      float distance = lvec.Length();

      if(distance>dl->m_fRadius+delta)
      {
        *(uint *)dst = 0;
        continue; // no lighting here
      }

      lvec /= distance;
      float attenuation = (dl->m_fRadius-distance)/dl->m_fRadius;
      if (attenuation < 0)
        attenuation = 0;

      // transform
      const Vec3d *lo = &lvec;

      const Vec3d *ty = (Vec3d *)tangents;
      const Vec3d *tx = (Vec3d *)binormals;
      const Vec3d *tz = (Vec3d *)tnormals;

      uchar d = (unsigned char)((0.5f * (tz->x*lo->x + tz->y*lo->y + tz->z*lo->z) + 0.5f)*255);
      dst[3] = (unsigned char)(d>128 ? attenuation*255 : 0); // att
    }
  }
  else
  if (type == GL_FLOAT)
  {
    for (int v=0; v<numVerts; v++, Dst+=StrideDst,verts+=StrVrt,tangents += StrTX,binormals+=StrTY,tnormals+=StrTZ)
    {
      float *dst = (float *)Dst;
      Vec3d lvec = lpos - *(Vec3d*)verts;

      float distance = lvec.Length();

      if(distance>dl->m_fRadius+delta)
      {
        dst[3] = 0;
        continue; // no lighting here
      }

      //lvec /= distance;
      float attenuation = (dl->m_fRadius-distance)/dl->m_fRadius;
      if (attenuation < 0)
        attenuation = 0;

      // transform
      const Vec3d *lo = &lvec;

      const Vec3d *ty = (Vec3d *)tangents;
      const Vec3d *tx = (Vec3d *)binormals;
      const Vec3d *tz = (Vec3d *)tnormals;

      uchar d = (unsigned char)((0.5f * (tz->x*lo->x + tz->y*lo->y + tz->z*lo->z) + 0.5f)*255);
      
      dst[0] = (d>128 ? attenuation : 0); // att
    }
  }
}

void SRendItem::mfComputeTangent(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2, const vec2_t t0, const vec2_t t1, const vec2_t t2, Vec3d &tangent, Vec3d& binormal, float& sign, Vec3d& face_normal)
{
  Vec3d bi;
  Vec3d cp;
  Vec3d e0;
  Vec3d e1;
  sign = 1.0f;

  // x
  e0[0] = v1[0] - v0[0];
  e0[1] = t1[0] - t0[0];
  e0[2] = t1[1] - t0[1];

  e1[0] = v2[0] - v0[0];
  e1[1] = t2[0] - t0[0];
  e1[2] = t2[1] - t0[1];


  cp = e0 ^ e1;

  if ( fabs(cp[0]) > 0.000001f )
  {
    tangent[0] = -cp[1] / cp[0];
    binormal[0] = -cp[2] / cp[0];
  }
  // y
  e0[0] = v1[1] - v0[1];
  e0[1] = t1[0] - t0[0];
  e0[2] = t1[1] - t0[1];

  e1[0] = v2[1] - v0[1];
  e1[1] = t2[0] - t0[0];
  e1[2] = t2[1] - t0[1];

  cp = e0 ^ e1;

  if ( fabs(cp[0]) > 0.000001f )
  {
    tangent[1] = -cp[1] / cp[0];
    binormal[1] = -cp[2] / cp[0];
  }

  // z
  e0[0] = v1[2] - v0[2];
  e0[1] = t1[0] - t0[0];
  e0[2] = t1[1] - t0[1];

  e1[0] = v2[2] - v0[2];
  e1[1] = t2[0] - t0[0];
  e1[2] = t2[1] - t0[1];

  cp = e0 ^ e1;

  if ( fabs(cp[0]) > 0.000001f )
  {
    tangent[2] = -cp[1] / cp[0];
    binormal[2] = -cp[2] / cp[0];
  }

  tangent.Normalize();
  binormal.Normalize();

/*  bi = tangent ^ binormal;

  dot = (float)(bi | face_normal);

  if (dot < 0.0f)
  {
    sign = -1.0f;
    binormal = -binormal;
  }*/
}


void SRendItem::mfCalcTangentSpaceVectors(void)
{
  int i, j;
  int in[3];

  float sign;
  Vec3d tg, bi, nor;
  Vec3d v[3];
  float stc[3][2];

  UPipeVertex ptr = gRenDev->m_RP.m_Ptr;
  int numTris = gRenDev->m_RP.m_RendNumIndices / 3;
  memset(gRenDev->m_RP.m_pBinormals, 0, sizeof(Vec3d)*gRenDev->m_RP.m_RendNumVerts);
  memset(gRenDev->m_RP.m_pTangents, 0, sizeof(Vec3d)*gRenDev->m_RP.m_RendNumVerts);
  memset(gRenDev->m_RP.m_pTNormals, 0, sizeof(Vec3d)*gRenDev->m_RP.m_RendNumVerts);
  for (i=0; i<numTris; i++)
  {
    for(j=0; j<3; j++)
    {
      in[j] = gRenDev->m_RP.m_RendIndices[i*3+j];
      struct_VERTEX_FORMAT_P3F_TEX2F *p = (struct_VERTEX_FORMAT_P3F_TEX2F *)(ptr.PtrB+in[j]*gRenDev->m_RP.m_Stride);
      v[j] = p->xyz;
      stc[j][0] = p->st[0];
      stc[j][1] = p->st[1];
    }
    Vec3 uu, vv;
    uu = v[1] - v[0];
    vv = v[1] - v[2];
    nor = uu ^ vv;
    nor.Normalize();

    // Calculate basis for this face
    mfComputeTangent(v[0], v[1], v[2], stc[0], stc[1], stc[2], tg, bi, sign, nor);

    gRenDev->m_RP.m_pTangents[in[0]]  -= tg;
    gRenDev->m_RP.m_pBinormals[in[0]] -= bi;

    mfComputeTangent(v[2], v[0], v[1], stc[2], stc[0], stc[1], tg, bi, sign, nor);

    gRenDev->m_RP.m_pTangents[in[1]]  -= tg;
    gRenDev->m_RP.m_pBinormals[in[1]] -= bi;

    mfComputeTangent(v[1], v[2], v[0], stc[1], stc[2], stc[0], tg, bi, sign, nor);

    gRenDev->m_RP.m_pTangents[in[2]]  -= tg;
    gRenDev->m_RP.m_pBinormals[in[2]] -= bi;
  }
  for(i=0; i<gRenDev->m_RP.m_RendNumVerts; i++)
  {
    gRenDev->m_RP.m_pTangents[i].Normalize();
    gRenDev->m_RP.m_pBinormals[i].Normalize();
    gRenDev->m_RP.m_pTNormals[i] = gRenDev->m_RP.m_pTangents[i].Cross(gRenDev->m_RP.m_pBinormals[i]);
    //if(gRenDev->m_RP.mTNormals[i].Dot(gRenDev->m_RP.mNormals[i])) < 0)
    //  m_pBasises[v].tnormal = -m_pBasises[v].tnormal;
  }
}


void *SRendItem::mfGetPointerCommon(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags)
{
  static int sFrameCalcLight;
  static int sFrameCalcLightNoAtt;
  static int sFrameCalcHalfAngles;
  static int sFrameCalcHANoAtt;
  static int sFrameCalcTangents;
  static int sFrameCalcAtten;
  static int sFrameCalcRefract;
  static int sMaskLight0;
  static int sMaskLight1;
  static int sMaskLight2;
  static int sMaskLight3;
  static int sMaskLight4;

  int j;
  switch (ePT)
  {
    case eSrcPointer_Binormal:
    case eSrcPointer_Tangent:
    case eSrcPointer_TNormal:
      {
        if (gRenDev->m_RP.m_Frame != sFrameCalcTangents)
        {
          sFrameCalcTangents = gRenDev->m_RP.m_Frame;
          mfCalcTangentSpaceVectors();
        }
      }
      *Stride = sizeof(Vec3d);
      if (ePT == eSrcPointer_Binormal)
        return gRenDev->m_RP.m_pBinormals;
      if (ePT == eSrcPointer_Tangent)
        return gRenDev->m_RP.m_pTangents;
      if (ePT == eSrcPointer_TNormal)
        return gRenDev->m_RP.m_pTNormals;

    case eSrcPointer_LightVector:
      {
        byte *dst;
        switch (Dst)
        {
          case eSrcPointer_Tex:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 0*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
          case eSrcPointer_TexLM:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 1*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
          case eSrcPointer_Color:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
        }

        if (!(Flags & FGP_NOCALC))
        {
          if (gRenDev->m_RP.m_Frame != sFrameCalcTangents)
          {
            sFrameCalcTangents = gRenDev->m_RP.m_Frame;
            mfCalcTangentSpaceVectors();
          }
          if ((gRenDev->m_RP.m_pShader->m_Flags3 & EF3_PREPARELV) && Dst != eSrcPointer_LightVector)
          {
            byte *Dst = dst;
            int str = *Stride;
            Vec3d *lv = gRenDev->m_RP.m_pLightVectors[gRenDev->m_RP.m_nCurLight];
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
            if (gRenDev->m_RP.m_Frame != sFrameCalcLight)
            {
              sFrameCalcLight = gRenDev->m_RP.m_Frame;
              sMaskLight0 = 0;
            }
            if (!(sMaskLight0 & (1<<gRenDev->m_RP.m_nCurLight)))
            {
              sMaskLight0 |= (1<<gRenDev->m_RP.m_nCurLight);
              mfCalcLightVectors(dst, *Stride);
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
          case eSrcPointer_Tex:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 0*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
          case eSrcPointer_TexLM:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 1*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
          case eSrcPointer_Color:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
        }
        
        if (!(Flags & FGP_NOCALC))
        {
          if (gRenDev->m_RP.m_Frame != sFrameCalcTangents)
          {
            sFrameCalcTangents = gRenDev->m_RP.m_Frame;
            mfCalcTangentSpaceVectors();
          }
          if ((gRenDev->m_RP.m_pShader->m_Flags3 & EF3_PREPARELV) && Dst != eSrcPointer_NormLightVector)
          {
            byte *Dst = dst;
            int str = *Stride;
            Vec3d *lv = gRenDev->m_RP.m_pLightVectors[gRenDev->m_RP.m_nCurLight];
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
            if (gRenDev->m_RP.m_Frame != sFrameCalcLight)
            {
              sFrameCalcLight = gRenDev->m_RP.m_Frame;
              sMaskLight0 = 0;
            }
            if (!(sMaskLight0 & (1<<gRenDev->m_RP.m_nCurLight)))
            {
              sMaskLight0 |= (1<<gRenDev->m_RP.m_nCurLight);
              mfCalcNormLightVectors(dst, *Stride, Type);
            }
          }
        }
        return dst;
      }
      break;
      
      
    case eSrcPointer_HalfAngleVector:
      {
        byte *dst;
        switch (Dst)
        {
          case eSrcPointer_Color:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
          case eSrcPointer_TexLM:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 1*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (gRenDev->m_RP.m_Frame != sFrameCalcTangents)
          {
            sFrameCalcTangents = gRenDev->m_RP.m_Frame;
            mfCalcTangentSpaceVectors();
          }
          if ((gRenDev->m_RP.m_pShader->m_Flags3 & EF3_PREPAREHAV) && Dst != eSrcPointer_HalfAngleVector)
          {
            byte *Dst = dst;
            int str = *Stride;
            Vec3d *lv = gRenDev->m_RP.m_pHalfAngleVectors[gRenDev->m_RP.m_nCurLight];
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
                byte *fd = (byte *)Dst;
                fd[0] = (byte)(lv->x * 255.0f);
                fd[1] = (byte)(lv->y * 255.0f);
                fd[2] = (byte)(lv->z * 255.0f);
              }
            }
          }
          if (gRenDev->m_RP.m_Frame != sFrameCalcHalfAngles)
          {
            sFrameCalcHalfAngles = gRenDev->m_RP.m_Frame;
            sMaskLight1 = 0;
          }
          if (!(sMaskLight1 & (1<<gRenDev->m_RP.m_nCurLight)))
          {
            sMaskLight1 |= (1<<gRenDev->m_RP.m_nCurLight);
            mfCalcHalfAngles(Type, dst, *Stride);
          }
        }
        return dst;
      }
      break;
      
    case eSrcPointer_LAttenuationSpec0:
      {
        byte *dst = NULL;
        switch (Dst)
        {
          case eSrcPointer_Color:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
          case eSrcPointer_Tex:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 0*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
          case eSrcPointer_TexLM:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 1*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
        }
        if ((gRenDev->m_RP.m_pShader->m_Flags3 & EF3_PREPARELAS0) && Dst != eSrcPointer_LAttenuationSpec0)
        {
          byte *Dst = dst;
          int str = *Stride;
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
              byte *fd = (byte *)Dst;
              fd[0] = (byte)CLAMP(lv->x * 255.0f, 0.0f, 255.0f);
              fd[1] = (byte)CLAMP(lv->y * 255.0f, 0.0f, 255.0f);
              fd[2] = (byte)CLAMP(lv->z * 255.0f, 0.0f, 255.0f);
            }
          }
        }
        else
        {
          int StrLV;
          byte *lv = (byte *)mfGetPointerCommon(eSrcPointer_LightVector, &StrLV, GL_FLOAT, eSrcPointer_LightVector, FGP_SRC | FGP_REAL);
          if (dst)
            mfCalcLAttenuationSpec0(dst, *Stride, lv, StrLV, Type);
        }
        return dst;
      }
      break;
      
    case eSrcPointer_LAttenuationSpec1:
      {
        byte *dst = NULL;
        switch (Dst)
        {
          case eSrcPointer_Tex:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 0*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
          case eSrcPointer_TexLM:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 1*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
        }
        if ((gRenDev->m_RP.m_pShader->m_Flags3 & EF3_PREPARELAS1) && Dst != eSrcPointer_LAttenuationSpec1)
        {
          byte *Dst = dst;
          int str = *Stride;
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
              byte *fd = (byte *)Dst;
              fd[0] = (byte)CLAMP(lv->x * 255.0f, 0.0f, 255.0f);
              fd[1] = (byte)CLAMP(lv->y * 255.0f, 0.0f, 255.0f);
              fd[2] = (byte)CLAMP(lv->z * 255.0f, 0.0f, 255.0f);
            }
          }
        }
        else
        {
          int StrLV;
          byte *lv = (byte *)mfGetPointerCommon(eSrcPointer_LightVector, &StrLV, GL_FLOAT, eSrcPointer_LightVector, FGP_SRC | FGP_REAL);
          if (dst)
            mfCalcLAttenuationSpec1(dst, *Stride, lv, StrLV, Type);
        }
        return dst;
      }
      break;

    case eSrcPointer_Detail:
      {
        byte *dst = NULL;
        switch (Dst)
        {
          case eSrcPointer_Tex:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 0*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
          case eSrcPointer_TexLM:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 1*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
        }
        int StrTC;
        byte *tc = (byte *)mfGetPointerCommon(eSrcPointer_Tex, &StrTC, GL_FLOAT, eSrcPointer_Tex, FGP_SRC | FGP_REAL);
        if (dst)
        {
          byte *Dst = dst;
          float scale = CRenderer::CV_r_detailscale;
          for (int i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, tc+=StrTC, Dst+=*Stride)
          {
            float *sr = (float *)tc;
            float *ds = (float *)Dst;
            ds[0] = sr[0] * scale;
            ds[1] = sr[1] * scale;
          }
        }
        return dst;
      }
      break;

    case eSrcPointer_ProjectAttenFromCamera:
      {
        byte *dst = NULL;
        switch (Dst)
        {
          case eSrcPointer_Tex:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 0*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
          case eSrcPointer_TexLM:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 1*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
        }
        if (dst)
          mfCalcProjectAttenFromCamera(dst, *Stride);
        return dst;
      }
      break;
      
    case eSrcPointer_Attenuation:
      {
        byte *dst;
        switch (Dst)
        {
          case eSrcPointer_Color:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
          case eSrcPointer_TexLM:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 1*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (gRenDev->m_RP.m_Frame != sFrameCalcTangents)
          {
            sFrameCalcTangents = gRenDev->m_RP.m_Frame;
            mfCalcTangentSpaceVectors();
          }
          if (gRenDev->m_RP.m_Frame != sFrameCalcAtten)
          {
            sFrameCalcAtten = gRenDev->m_RP.m_Frame;
            sMaskLight2 = 0;
          }
          if (!(sMaskLight2 & (1<<gRenDev->m_RP.m_nCurLight)))
          {
            sMaskLight2 |= (1<<gRenDev->m_RP.m_nCurLight);
            mfCalcLightAttenuation(Type, dst, *Stride);
          }
        }
        return dst;
      }
      break;

    case eSrcPointer_Refract:
      {
        byte *dst;
        switch (Dst)
        {
          case eSrcPointer_Tex:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 0*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
          case eSrcPointer_TexLM:
            dst = (byte *)gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + 1*16;
            *Stride = gRenDev->m_RP.m_Stride;
            break;
        }
        if (!(Flags & FGP_NOCALC))
        {
          if (gRenDev->m_RP.m_Frame != sFrameCalcRefract)
          {
            sFrameCalcRefract = gRenDev->m_RP.m_Frame;
            mfCalcRefractVectors(Type, dst, *Stride);
          }
        }
        return dst;
      }
      break;
      
    case eSrcPointer_Vert:
      *Stride = gRenDev->m_RP.m_Stride;
      return gRenDev->m_RP.m_Ptr.PtrB;

    case eSrcPointer_Color:
      *Stride = gRenDev->m_RP.m_Stride;
      return gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD;

    case eSrcPointer_Tex:
    case eSrcPointer_TexLM:
      *Stride = gRenDev->m_RP.m_Stride;
      j = ePT - eSrcPointer_Tex;
      return gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + j*16;
  }
  return NULL;
}
