//////////////////////////////////////////////////////////////////////
//
//  Crytek CryENGINE Source code
//  
//  File:GLObjSprites.cpp
//  Description: Implementation of vegetation sprites rendering
//
//  History:
//  05/05/2003 Created by Vladimir Kajalin
//
//////////////////////////////////////////////////////////////////////

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "GLCGVProgram.h"
#include "GLCGPShader.h"
#include <I3DEngine.h>
#include <IEntityRenderState.h>

#include "../common/shadow_renderer.h"
#include "limits.h"

#include "I3DEngine.h"
#include "../../Cry3DEngine/Cry3denginebase.h"
#include "../../Cry3DEngine/Terrain.h"
#include "../../Cry3DEngine/StatObj.h"
#include "../../Cry3DEngine/ObjMan.h"

#ifdef WIN32

// duplicated definition (first one is in 3dengine)
ISystem * Cry3DEngineBase::m_pSys=0;
IRenderer * Cry3DEngineBase::m_pRenderer=0;
ITimer * Cry3DEngineBase::m_pTimer=0;
ILog * Cry3DEngineBase::m_pLog=0;
IPhysicalWorld * Cry3DEngineBase::m_pPhysicalWorld=0;
IConsole * Cry3DEngineBase::m_pConsole=0;
I3DEngine * Cry3DEngineBase::m_p3DEngine=0;
CVars * Cry3DEngineBase::m_pCVars=0;
ICryPak * Cry3DEngineBase::m_pCryPak=0;
int Cry3DEngineBase::m_nRenderStackLevel=0;
int Cry3DEngineBase::m_dwRecursionDrawFlags[2]={0,0};

#endif // WIN32

void CGLRenderer::DrawObjSprites_NoBend (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan)
{
  float max_view_dist = fMaxViewDist*0.8f;
  const Vec3d & vCamPos = m_RP.m_ViewOrg;
  const float rad2deg = 180.0f/PI;    
  const float far_tex_angle = (FAR_TEX_ANGLE*0.5f);

  CGLTexMan::BindNULL(1);
  m_TexMan->SetTexture(TX_FIRSTBIND, eTT_Base);

  Vec3d vWorldColor = iSystem->GetI3DEngine()->GetWorldColor();
  int prev_tid=-1;

  m_RP.m_FlagsModificators &= ~7;
  m_RP.m_PersFlags &= ~RBPF_PS1NEEDSET | RBPF_PS2NEEDSET | RBPF_VSNEEDSET | RBPF_TSNEEDSET;
  EF_CommitPS();
  EF_CommitVS();
  EF_SetColorOp(eCO_MODULATE2X, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  float paramf[2][4];
  float paramfe[2][4];

  int nVerts = 0;

  bool bVolFog = false;
  bool bVolFogInit = false;
  CFColor FogColor;

  CStatObjInst *o, *oNext;
  o = pList->GetAt(pList->Count()-1);
  for( int i=pList->Count()-1; i>=0; i-- )
  { 
    assert(o);
    if(!o)
      continue;
    o = pList->GetAt(i);

    // Prefetch next object in cache
    if (i)
    {
      oNext = pList->GetAt(i-1);
      cryPrefetchT0SSE(oNext);
    }

    assert(SRendItem::m_RecurseLevel>=1 && SRendItem::m_RecurseLevel-1<=2);
    float fDistance = (((IEntityRender*)o))->m_arrfDistance[SRendItem::m_RecurseLevel-1];

    float fMaxDist = o->GetMaxViewDist();

    // note: move into sort by size
    if(fMaxDist > max_view_dist)
      fMaxDist = max_view_dist;

    float fFadeOut;
    if(pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].bFadeSize)
    {
      fFadeOut = (1.f-(fDistance*pObjMan->m_fZoomFactor)/(fMaxDist))*8.f;
      if (fFadeOut <= 0)
        continue;
      if(fFadeOut>1.f)
        fFadeOut=1.f;
    }
    else
      fFadeOut = 1.f;

    float fBright = CHAR_TO_FLOAT*max((float)o->m_ucBright,32.f);

    // apply brightness from vegetation group settings
    fBright *= pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].fBrightness;

    // get full lod to get tex id and lowest lod to take size
    CStatObj * pStatObjLOD0 = pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].GetStatObj();
    CStatObj * pStatObjLODLowest = pStatObjLOD0;
    if(pStatObjLOD0->m_nLoadedLodsNum && pStatObjLOD0->m_arrpLowLODs[pStatObjLOD0->m_nLoadedLodsNum-1])
      pStatObjLODLowest = pStatObjLOD0->m_arrpLowLODs[pStatObjLOD0->m_nLoadedLodsNum-1];

    assert(pStatObjLODLowest);
    if(!pStatObjLODLowest)
      continue;

    Vec3d vCenter = pStatObjLODLowest->GetCenter() * o->m_fScale;

    float DX = o->m_vPos.x - vCamPos.x;
    float DY = o->m_vPos.y - vCamPos.y;
    float DX1 = vCenter.x + DX;
    float DY1 = vCenter.y + DY;
    float fAngle = rad2deg*cry_atan2f(DX1, DY1);
    while(fAngle<0) fAngle+=360;

    int nSlotId = QRound(fAngle/FAR_TEX_ANGLE+0.5f)%FAR_TEX_COUNT;
    assert(nSlotId>=0 && nSlotId<FAR_TEX_COUNT);
    int tid = pStatObjLOD0->m_arrSpriteTexID[nSlotId];

    if(SRendItem::m_RecurseLevel==1)
      o->m_ucAngleSlotId = nSlotId; // store sprite id for morfing

    if(prev_tid != tid)
    {
      m_TexMan->SetTexture(tid, eTT_Base);
      prev_tid = tid;
    }

    fBright = min(fBright, 1.0f);
    UCol col;
    col.bcolor[0] = (byte)QInt(fBright * vWorldColor.x * 255.0f);
    col.bcolor[1] = (byte)QInt(fBright * vWorldColor.y * 255.0f);
    col.bcolor[2] = (byte)QInt(fBright * vWorldColor.z * 255.0f);
    col.bcolor[3] = 255;

    if(CV_r_VolumetricFog && o->m_nFogVolumeID>0)
    {
      // setup Volume Fog Texgen planes
      if (!bVolFogInit)
      {
        bVolFogInit = true;

        if (!m_RP.m_RCSprites_FV)
          m_RP.m_RCSprites_FV = CPShader::mfForName("CGRCTreeSprites_FV");

        SMFog *fb = &m_RP.m_FogVolumes[o->m_nFogVolumeID/*o->m_nFogVolumeID*/];
        if (fb->m_FogInfo.m_WaveFogGen.m_eWFType)
        {
          float f = SEvalFuncs::EvalWaveForm(&fb->m_FogInfo.m_WaveFogGen);

          fb->m_fMaxDist = f;
        }
        float intens = fb->m_fMaxDist;
        if (intens <= 0)
          intens = 1.0f;
        intens = -0.25f / intens;
        paramf[0][0] = intens*m_CameraMatrix(0,2);
        paramf[0][1] = intens*m_CameraMatrix(1,2);
        paramf[0][2] = intens*m_CameraMatrix(2,2);
        paramf[0][3] = intens*m_CameraMatrix(3,2) + 0.5f;

        paramf[1][0] = paramf[1][1] = paramf[1][2] = 0;
        paramf[1][3] = 0.49f;

        paramfe[0][0] = 0;
        paramfe[0][1] = 0;
        paramfe[0][2] = 0;
        paramfe[0][3] = fb->m_Normal.Dot(m_RP.m_ViewOrg) - fb->m_Dist;
        float fSmooth;
        FogColor = fb->m_Color;
        if (paramfe[0][3] < -0.5f)
          fSmooth = 1.0f;
        else
          fSmooth = 0.1f;
        paramfe[0][3] = paramfe[0][3] * fSmooth + 0.5f;

        paramfe[1][0] = fb->m_Normal.x * fSmooth;
        paramfe[1][1] = fb->m_Normal.y * fSmooth;
        paramfe[1][2] = fb->m_Normal.z * fSmooth;
        paramfe[1][3] = -(fb->m_Dist) * fSmooth + 0.5f;
      }
      if (!bVolFog)
      {
        bVolFog = true;

        EF_SelectTMU(1);
        gRenDev->m_TexMan->m_Text_Fog->Set();
        glTexGenfv(GL_S, GL_OBJECT_PLANE, paramf[0]);
        glTexGenfv(GL_T, GL_OBJECT_PLANE, paramf[1]);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);

        EF_SelectTMU(2);
        gRenDev->m_TexMan->m_Text_Fog_Enter->Set();
        glTexGenfv(GL_S, GL_OBJECT_PLANE, paramfe[0]);
        glTexGenfv(GL_T, GL_OBJECT_PLANE, paramfe[1]);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);

        EF_SelectTMU(0);

        float param[4];
        if (m_RP.m_RCSprites_FV)
        {
          m_RP.m_RCSprites_FV->mfSet(true, NULL);
          param[0] = FogColor[0]; param[1] = FogColor[1]; param[2] = FogColor[2]; param[3] = FogColor[3];
          CCGPShader_GL *pRC = (CCGPShader_GL *)m_RP.m_RCSprites_FV;
          pRC->mfParameter4f("FogColor", param);
        }
        //EF_PushFog();
        //EnableFog(false);
      }
    }
    else
    {
      if (bVolFog)
      {
        bVolFog = false;

        EF_SelectTMU(1);
        EnableTMU(false);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);

        EF_SelectTMU(2);
        EnableTMU(false);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);

        EF_SelectTMU(0);

        //EF_PopFog();
        if (m_RP.m_RCSprites_FV)
          m_RP.m_RCSprites_FV->mfSet(false);
      }
    }

    float fSpriteScaleV = o->m_fScale*pStatObjLODLowest->GetRadiusVert()*fFadeOut;
    float fSpriteScaleH = o->m_fScale*pStatObjLODLowest->GetRadiusHors()*pObjMan->m_fZoomFactor*fFadeOut;
    Vec3d vPos = o->m_vPos + vCenter*fFadeOut;

    float dy = DX*fSpriteScaleH/fDistance;
    float dx = DY*fSpriteScaleH/fDistance;

    Vec3d vUp(0,0,-fSpriteScaleV);

    if(o->m_ucLodAngle!=127) // rotate sprite to much it with 3d object
      vUp = vUp.rotated(Vec3d(-dx,dy,0).normalized(), o->m_ucLodAngle/255.f-0.5f);

    float fX0 = vPos.x+dx;
    float fX1 = vPos.x-dx;
    float fY0 = vPos.y+dy;
    float fY1 = vPos.y-dy;
    glColor4ubv(&col.bcolor[0]);

    glBegin(GL_TRIANGLE_STRIP);

    glTexCoord2f(0, 0);
    glVertex3f(fX0+vUp.x, fY1+vUp.y, vPos.z+vUp.z);

    glTexCoord2f(-1.0f, 0);
    glVertex3f(fX1+vUp.x, fY0+vUp.y, vPos.z+vUp.z);

    glTexCoord2f(0, 1.0f);
    glVertex3f(fX0-vUp.x, fY1-vUp.y, vPos.z-vUp.z);

    glTexCoord2f(-1.0f, 1.0f);
    glVertex3f(fX1-vUp.x, fY0-vUp.y, vPos.z-vUp.z);

    glEnd();

    m_nPolygons += 2;
  }

  if (bVolFog)
  {
    bVolFog = false;

    EF_SelectTMU(1);
    EnableTMU(false);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    EF_SelectTMU(2);
    EnableTMU(false);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    EF_SelectTMU(0);

    //EF_PopFog();
    if (m_RP.m_RCSprites_FV)
      m_RP.m_RCSprites_FV->mfSet(false);
  }

  m_TexMan->SetTexture(0, eTT_Base);
}

static CObjManager *sObjMan;

static _inline int Compare(CStatObjInst *& p1, CStatObjInst *& p2)
{
  CStatObj * pStatObj1 = sObjMan->m_lstStaticTypes[p1->m_nObjectTypeID].GetStatObj();
  CStatObj * pStatObj2 = sObjMan->m_lstStaticTypes[p2->m_nObjectTypeID].GetStatObj();
  if((UINT_PTR)p1 > (UINT_PTR)p2)						//AMD Port
    return 1;
  else
  if((UINT_PTR)p1 < (UINT_PTR)p2)						//AMD Port
    return -1;
  
  return 0;
}

struct SSpriteInfo
{
  short m_TexID;
  uchar m_nFogVolumeID;
  uchar m_LodAngle;
  Vec3d m_vPos;
  float m_fDX;
  float m_fDY;
  float m_fScaleV;
  UCol m_Color;

  /*_inline SSpriteInfo& operator=(const SSpriteInfo& src)
  {
    _asm
    {
      mov eax, src;
      mov edx, this;
      movq mm0, [eax+0] // Read in source data 
      movq mm1, [eax+8] 
      movq	[edx+0], mm0
      movq	[edx+8], mm1
      movq mm2, [eax+16] // Read in source data 
      movq mm3, [eax+24] 
      movq	[edx+16], mm2
      movq	[edx+24], mm3
      emms
    }
    return *this;
  }*/
};

static _inline int Compare(SSpriteInfo& p1, SSpriteInfo& p2)
{
  if(p1.m_TexID > p2.m_TexID)
    return 1;
  else
  if(p1.m_TexID < p2.m_TexID)
    return -1;

  return 0;
}


void CGLRenderer::DrawObjSprites_NoBend_Merge (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan)
{
  int i;

  float max_view_dist = fMaxViewDist*0.8f;
  const Vec3d & vCamPos = m_RP.m_ViewOrg;
  const float rad2deg = 180.0f/PI;    
  const float far_tex_angle = (FAR_TEX_ANGLE*0.5f);

  Vec3d vWorldColor = iSystem->GetI3DEngine()->GetWorldColor() * 255.0f;

  static TArray<SSpriteInfo> sSPInfo;
  sSPInfo.SetUse(0);

  CStatObjInst *o, *oNext;
  o = pList->GetAt(pList->Count()-1);
  for (i=pList->Count()-1; i>=0; i--)
  { 
    o = pList->GetAt(i);
    //assert(o);
    //if(!o)
   // 	continue;

    // Prefetch next object in cache
    if (i)
    {
      oNext = pList->GetAt(i-1);
      cryPrefetchT0SSE(oNext);
    }

		assert(SRendItem::m_RecurseLevel>=1 && SRendItem::m_RecurseLevel-1<=2);
		float fDistance = (((IEntityRender*)o))->m_arrfDistance[SRendItem::m_RecurseLevel-1];

    float fMaxDist = o->GetMaxViewDist();
    
    // note: move into sort by size
    if(fMaxDist > max_view_dist)
      fMaxDist = max_view_dist;

		float fFadeOut;
		if(pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].bFadeSize)
		{
			fFadeOut = (1.f-(fDistance*pObjMan->m_fZoomFactor)/(fMaxDist))*8.f;
			if (fFadeOut <= 0)
				continue;
			if(fFadeOut>1.f)
				fFadeOut=1.f;
		}
		else
			fFadeOut = 1.f;

    Vec3d vBright = Vec3d(CHAR_TO_FLOAT,CHAR_TO_FLOAT,CHAR_TO_FLOAT)*max((float)o->m_ucBright,32.f);

    // apply brightness from vegetation group settings
    vBright *= pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].fBrightness;

		// get full lod to get tex id and lowest lod to take size
		CStatObj * pStatObjLOD0 = pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].GetStatObj();
		CStatObj * pStatObjLODLowest = pStatObjLOD0;
		if(pStatObjLOD0->m_nLoadedLodsNum && pStatObjLOD0->m_arrpLowLODs[pStatObjLOD0->m_nLoadedLodsNum-1])
			pStatObjLODLowest = pStatObjLOD0->m_arrpLowLODs[pStatObjLOD0->m_nLoadedLodsNum-1];

    assert(pStatObjLODLowest);
    if(!pStatObjLODLowest)
      continue;

    Vec3d vCenter = pStatObjLODLowest->GetCenter() * o->m_fScale;

    int nSPInf = sSPInfo.Num();
    sSPInfo.AddIndex(1);
    SSpriteInfo *pSP = &sSPInfo[nSPInf];

    float DX = o->m_vPos.x - vCamPos.x;
    float DY = o->m_vPos.y - vCamPos.y;
		float DX1 = vCenter.x + DX;
		float DY1 = vCenter.y + DY;
		float fAngle = rad2deg*cry_atan2f(DX1, DY1);
		while(fAngle<0) fAngle+=360;

		int nSlotId = QRound(fAngle/FAR_TEX_ANGLE+0.5f)%FAR_TEX_COUNT;
		assert(nSlotId>=0 && nSlotId<FAR_TEX_COUNT);
		pSP->m_TexID = pStatObjLOD0->m_arrSpriteTexID[nSlotId];

		if(SRendItem::m_RecurseLevel==1)
			o->m_ucAngleSlotId = nSlotId; // store sprite id for morfing

    float fSpriteScaleV = o->m_fScale*pStatObjLODLowest->GetRadiusVert()*fFadeOut;
    float fSpriteScaleH = o->m_fScale*pStatObjLODLowest->GetRadiusHors()*pObjMan->m_fZoomFactor*fFadeOut;
    pSP->m_vPos = o->m_vPos + vCenter*fFadeOut;

    pSP->m_fDY = DX*fSpriteScaleH/fDistance;
    pSP->m_fDX = DY*fSpriteScaleH/fDistance;

    pSP->m_LodAngle = o->m_ucLodAngle;
    pSP->m_fScaleV = fSpriteScaleV;

    vBright.CheckMin(Vec3d(1,1,1));
    pSP->m_Color.bcolor[0] = (byte)QInt(vBright.x * vWorldColor.x);
    pSP->m_Color.bcolor[1] = (byte)QInt(vBright.y * vWorldColor.y);
    pSP->m_Color.bcolor[2] = (byte)QInt(vBright.z * vWorldColor.z);
    pSP->m_Color.bcolor[3] = 255;

    pSP->m_nFogVolumeID = o->m_nFogVolumeID;
  }

  ::Sort(&sSPInfo[0], sSPInfo.Num());

  int prev_tid = -1;
  CGLTexMan::BindNULL(1);
  EF_SelectTMU(0);
  m_TexMan->SetTexture(TX_FIRSTBIND, eTT_Base);

  m_RP.m_FlagsModificators &= ~7;
  m_RP.m_PersFlags &= ~RBPF_PS1NEEDSET | RBPF_PS2NEEDSET | RBPF_VSNEEDSET | RBPF_TSNEEDSET;
  EF_CommitPS();
  EF_CommitVS();
  EF_SetColorOp(eCO_MODULATE2X, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  float paramf[2][4];
  float paramfe[2][4];

  int nVerts = 0;

  bool bVolFog = false;
  bool bVolFogInit = false;
  CFColor FogColor;

  for (i=0; i<sSPInfo.Num(); i++)
  {
    SSpriteInfo *pSP = &sSPInfo[i];
    if(prev_tid != pSP->m_TexID)
    {
      if (nVerts)
      {
        glEnd();
        nVerts = 0;
      }
      m_TexMan->SetTexture(pSP->m_TexID, eTT_Base);
      prev_tid = pSP->m_TexID;
    }

		if(CV_r_VolumetricFog && pSP->m_nFogVolumeID>0)
    {
      // setup Volume Fog Texgen planes
      if (!bVolFogInit)
      {
        bVolFogInit = true;

        if (!m_RP.m_RCSprites_FV)
          m_RP.m_RCSprites_FV = CPShader::mfForName("CGRCTreeSprites_FV");

        SMFog *fb = &m_RP.m_FogVolumes[pSP->m_nFogVolumeID];
        if (fb->m_FogInfo.m_WaveFogGen.m_eWFType)
        {
          float f = SEvalFuncs::EvalWaveForm(&fb->m_FogInfo.m_WaveFogGen);

          fb->m_fMaxDist = f;
        }
        float intens = fb->m_fMaxDist;
        if (intens <= 0)
          intens = 1.0f;
        intens = -0.25f / intens;
        paramf[0][0] = intens*m_CameraMatrix(0,2);
        paramf[0][1] = intens*m_CameraMatrix(1,2);
        paramf[0][2] = intens*m_CameraMatrix(2,2);
        paramf[0][3] = intens*m_CameraMatrix(3,2) + 0.5f;

        paramf[1][0] = paramf[1][1] = paramf[1][2] = 0;
        paramf[1][3] = 0.49f;

        paramfe[0][0] = 0;
        paramfe[0][1] = 0;
        paramfe[0][2] = 0;
        paramfe[0][3] = fb->m_Normal.Dot(m_RP.m_ViewOrg) - fb->m_Dist;
        float fSmooth;
        FogColor = fb->m_Color;
        if (paramfe[0][3] < -0.5f)
          fSmooth = 1.0f;
        else
          fSmooth = 0.1f;
        paramfe[0][3] = paramfe[0][3] * fSmooth + 0.5f;

        paramfe[1][0] = fb->m_Normal.x * fSmooth;
        paramfe[1][1] = fb->m_Normal.y * fSmooth;
        paramfe[1][2] = fb->m_Normal.z * fSmooth;
        paramfe[1][3] = -(fb->m_Dist) * fSmooth + 0.5f;
      }
      if (!bVolFog)
      {
        if (nVerts)
        {
          glEnd();
          nVerts = 0;
        }
        bVolFog = true;
        EF_SelectTMU(1);
        gRenDev->m_TexMan->m_Text_Fog->Set();
        glTexGenfv(GL_S, GL_OBJECT_PLANE, paramf[0]);
        glTexGenfv(GL_T, GL_OBJECT_PLANE, paramf[1]);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);

        EF_SelectTMU(2);
        gRenDev->m_TexMan->m_Text_Fog_Enter->Set();
        glTexGenfv(GL_S, GL_OBJECT_PLANE, paramfe[0]);
        glTexGenfv(GL_T, GL_OBJECT_PLANE, paramfe[1]);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);

        EF_SelectTMU(0);

        float param[4];
        if (m_RP.m_RCSprites_FV)
        {
          m_RP.m_RCSprites_FV->mfSet(true, NULL);
          param[0] = FogColor[0]; param[1] = FogColor[1]; param[2] = FogColor[2]; param[3] = FogColor[3];
          CCGPShader_GL *pRC = (CCGPShader_GL *)m_RP.m_RCSprites_FV;
          pRC->mfParameter4f("FogColor", param);
        }
        EF_CommitPS();
        //EF_PushFog();
        //EnableFog(false);
      }
    }
    else
    {
      if (bVolFog)
      {
        if (nVerts)
        {
          glEnd();
          nVerts = 0;
        }
        bVolFog = false;

        EF_SelectTMU(1);
        EnableTMU(false);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);

        EF_SelectTMU(2);
        EnableTMU(false);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);

        EF_SelectTMU(0);

        //EF_PopFog();
        if (m_RP.m_RCSprites_FV)
          m_RP.m_RCSprites_FV->mfSet(false);

        EF_CommitPS();
      }
    }

    float fX0 = pSP->m_vPos.x+pSP->m_fDX;
    float fX1 = pSP->m_vPos.x-pSP->m_fDX;
    float fY0 = pSP->m_vPos.y+pSP->m_fDY;
    float fY1 = pSP->m_vPos.y-pSP->m_fDY;

    Vec3d vUp = Vec3d(0,0,-pSP->m_fScaleV);

    if(pSP->m_LodAngle!=127) // rotate sprite to much it with 3d object
      vUp = vUp.rotated(Vec3d(-pSP->m_fDX,pSP->m_fDY,0).normalized(), pSP->m_LodAngle/255.f-0.5f);

    if (!nVerts)
      glBegin(GL_TRIANGLES);

    glColor3ubv(&pSP->m_Color.bcolor[0]);

    // 0
    glTexCoord2f(0, 0);
    glVertex3f(fX0+vUp.x, fY1+vUp.y, pSP->m_vPos.z+vUp.z);

    // 1
    glTexCoord2f(-1.0f, 0);
    glVertex3f(fX1+vUp.x, fY0+vUp.y, pSP->m_vPos.z+vUp.z);

    // 2
    glTexCoord2f(0, 1.0f);
    glVertex3f(fX0-vUp.x, fY1-vUp.y, pSP->m_vPos.z-vUp.z);

    // 3
    glTexCoord2f(-1.0f, 0);
    glVertex3f(fX1+vUp.x, fY0+vUp.y, pSP->m_vPos.z+vUp.z);

    // 4
    glTexCoord2f(-1.0f, 1.0f);
    glVertex3f(fX1-vUp.x, fY0-vUp.y, pSP->m_vPos.z-vUp.z);

    // 5
    glTexCoord2f(0, 1.0f);
    glVertex3f(fX0-vUp.x, fY1-vUp.y, pSP->m_vPos.z-vUp.z);

    nVerts += 6;
    m_nPolygons += 2;
  }
  if (nVerts)
    glEnd();

  if (bVolFog)
  {
    bVolFog = false;

    EF_SelectTMU(1);
    EnableTMU(false);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    EF_SelectTMU(2);
    EnableTMU(false);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    EF_SelectTMU(0);

    //EF_PopFog();
    if (m_RP.m_RCSprites_FV)
      m_RP.m_RCSprites_FV->mfSet(false);
  }

  m_TexMan->SetTexture(0, eTT_Base);
}

void CGLRenderer::DrawObjSprites (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan)
{
  PROFILE_FRAME(Draw_ObjSprites);

  if(CV_r_VegetationSpritesAlphaBlend)
    EF_SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_ALPHATEST_GREATER0);
  else
    EF_SetState(GS_ALPHATEST_GEQUAL128 | GS_DEPTHWRITE);

  SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  SetCullMode(R_CULL_DISABLE);

  // Sorting far objects front to back since we use alphablending
//  if(CV_r_VegetationSpritesAlphaBlend)
  //  ::Sort(&(*pList)[0], pList->Count());
  //pList->SortByDistanceMember_(true);

  if (CV_r_VegetationSpritesNoBend)
  {
    if (CV_r_VegetationSpritesNoBend==1)
      DrawObjSprites_NoBend(pList, fMaxViewDist, pObjMan);
    else
      DrawObjSprites_NoBend_Merge(pList, fMaxViewDist, pObjMan);
    return;
  }

  float max_view_dist = fMaxViewDist*0.8f;
  const Vec3d & vCamPos = m_RP.m_ViewOrg;
  const float rad2deg = 180.0f/PI;    
  const float far_tex_angle = (FAR_TEX_ANGLE*0.5f);

  CGLTexMan::BindNULL(0);
  m_TexMan->SetTexture(TX_FIRSTBIND, eTT_Base);

  float v[4];
  v[3] = 1.0f;
  CVProgram *pVP = NULL;
  CVProgram *pVP_FV = NULL;
  // If device supports vertex shaders use advanced bending for sprites
  if (!m_RP.m_VPPlantBendingSpr && (GetFeatures() & RFT_HW_VS))
  {
    pVP = CVProgram::mfForName("CGVProgSimple_Plant_Bended_Sprite");
    pVP_FV = CVProgram::mfForName("CGVProgSimple_Plant_Bended_Sprite_FV");
    //pVP = CVProgram::mfForName("VProgSimple_Plant_Bended_Sprite", false);
    m_RP.m_VPPlantBendingSpr = pVP;
    m_RP.m_VPPlantBendingSpr_FV = pVP_FV;
  }
  else
  {
    pVP = m_RP.m_VPPlantBendingSpr;
    pVP_FV = m_RP.m_VPPlantBendingSpr_FV;
  }

  SCGBind *pBindBend = NULL;
  SCGBind *pBindPos = NULL;
  CCGVProgram_GL *vpGL = NULL;

  SMFog *fb = NULL;

  SCGBind *pBindTG00, *pBindTG01, *pBindTG10, *pBindTG11;

  SCGBind *pBindBend_FV = NULL;
  SCGBind *pBindPos_FV = NULL;
  CCGVProgram_GL *vpGL_FV = NULL;
  CFColor FogColor;
  Plane plane00, plane01, plane10, plane11;

  SCGBind *pCurBindBend = NULL;
  SCGBind *pCurBindPos = NULL;

  CCGVProgram_GL *lastvpGL = NULL;

  SWaveForm2 wfMain;
  if (!m_RP.m_RCSprites)
    m_RP.m_RCSprites = CPShader::mfForName("CGRCTreeSprites");
  if (pVP)
  {
    wfMain.m_eWFType = eWF_Sin;
    wfMain.m_Amp = 0.002f;
    wfMain.m_Level = 0;
    pVP->mfSet(true, NULL);
    pVP->mfSetVariables(false, NULL);

    vpGL = (CCGVProgram_GL *)pVP;
    lastvpGL = vpGL;
    pBindBend = vpGL->mfGetParameterBind("Bend");
    pBindPos = vpGL->mfGetParameterBind("ObjPos");

    pCurBindBend = pBindBend;
    pCurBindPos = pBindPos;

    if (m_RP.m_RCSprites)
      m_RP.m_RCSprites->mfSet(true);
  }

  Vec3d vWorldColor = iSystem->GetI3DEngine()->GetWorldColor();
  int prev_tid=-1;

  if (m_bHeatVision)
  {
    float param[4];
    if (!m_RP.m_RCSprites_Heat)
      m_RP.m_RCSprites_Heat = CPShader::mfForName("CGRCHeat_TreesSprites");
    if (m_RP.m_RCSprites_Heat)
    {
      m_RP.m_RCSprites_Heat->mfSet(true);
      param[0] = param[1] = 0;
      param[2] = 1.0f;
      param[3] = 1.0f;
      CCGPShader_GL *pSH = (CCGPShader_GL *)m_RP.m_RCSprites_Heat;
      pSH->mfParameter4f("Heat", param);
    }
  }

  m_RP.m_PersFlags |= RBPF_PS1NEEDSET | RBPF_VSNEEDSET;
  m_RP.m_PersFlags &= ~RBPF_TSNEEDSET;
  EF_CommitPS();
  EF_CommitVS();

  //EF_SelectTMU(1);
  //EnableTMU(true);

  float param0[4];
  param0[0] = m_WorldColor[0]; param0[1] = m_WorldColor[1]; param0[2] = m_WorldColor[2]; param0[3] = m_WorldColor[3];
  CCGPShader_GL *curRC = (CCGPShader_GL *)m_RP.m_RCSprites;
  if (!m_bHeatVision)
    curRC->mfParameter4f("Ambient", param0);

  for( int i=pList->Count()-1; i>=0; i-- )
  { 
    CStatObjInst * o = pList->GetAt(i);
    assert(o);
    if(!o)
      continue;

    assert(SRendItem::m_RecurseLevel>=1 && SRendItem::m_RecurseLevel-1<=2);
		float fDistance = (((IEntityRender*)o))->m_arrfDistance[SRendItem::m_RecurseLevel-1];

//    if(SRendItem::m_RecurseLevel==1) // m_fDistance was changed during reqursion
  //    fDistance = fDistance0;

//    float fMaxDist = 1000;//o->m_fMaxDist*pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].fMaxViewDistRatio; //o->scale*pStatObj->GetRadius()*80;

    float fMaxDist = o->GetMaxViewDist();// m_fScale*pStatObjLOD0->GetRadiusHors()*60;

    // note: move into sort by size
    if(fMaxDist > max_view_dist)
      fMaxDist = max_view_dist;

    float fFadeOut = (1.f-(fDistance*pObjMan->m_fZoomFactor)/(fMaxDist))*8.f;
    if (fFadeOut <= 0)
      continue;

    if(fFadeOut>1.f)
      fFadeOut=1.f;

    Vec3d vBright = Vec3d(CHAR_TO_FLOAT,CHAR_TO_FLOAT,CHAR_TO_FLOAT)*max((float)o->m_ucBright,32.f);

    // apply brightness from vegetation group settings
    vBright *= pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].fBrightness;

    // get full lod to get tex id and lowest lod to take size
    CStatObj * pStatObjLOD0 = pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].GetStatObj();
    CStatObj * pStatObjLODLowest = pStatObjLOD0;
    if(pStatObjLOD0->m_nLoadedLodsNum && pStatObjLOD0->m_arrpLowLODs[pStatObjLOD0->m_nLoadedLodsNum-1])
      pStatObjLODLowest = pStatObjLOD0->m_arrpLowLODs[pStatObjLOD0->m_nLoadedLodsNum-1];

    float DX = o->m_fScale*pStatObjLODLowest->GetCenter().x + o->m_vPos.x - vCamPos.x;
    float DY = o->m_fScale*pStatObjLODLowest->GetCenter().y + o->m_vPos.y - vCamPos.y;
    float fAngle = rad2deg*cry_atan2f( DX, DY );
    while(fAngle<0) fAngle+=360;
    DX = o->m_vPos.x - vCamPos.x;
    DY = o->m_vPos.y - vCamPos.y;

    assert(pStatObjLODLowest);
    if(!pStatObjLODLowest)
      continue;

    int nSlotId = int(fAngle/FAR_TEX_ANGLE+0.5f)%FAR_TEX_COUNT;
    assert(nSlotId>=0 && nSlotId<FAR_TEX_COUNT);
    int tid = pStatObjLOD0->m_arrSpriteTexID[nSlotId];

    if(SRendItem::m_RecurseLevel==1)
      o->m_ucAngleSlotId = nSlotId; // store sprite id for morfing

    if(prev_tid != tid)
    {
      //EF_SelectTMU(0);
      m_TexMan->SetTexture(tid, eTT_Base);
      //EF_SelectTMU(1);
      //m_TexMan->SetTexture(tid, eTT_Base);
      //param0[3] = 1.0f;
      //CPShader_GL::mfSetFloat4f(0, param0);
      prev_tid = tid;
    }

    vBright.CheckMin(Vec3d(1,1,1));
    vBright.x *= vWorldColor.x;
    vBright.y *= vWorldColor.y;
    vBright.z *= vWorldColor.z;
    glColor4f(vBright.x, vBright.y, vBright.z, 1.f);//o->m_ucAlpha/255.f);

    float fSpriteScaleV = o->m_fScale*pStatObjLODLowest->GetRadiusVert()*fFadeOut;
    float fSpriteScaleH = o->m_fScale*pStatObjLODLowest->GetRadiusHors()*pObjMan->m_fZoomFactor*fFadeOut;
    Vec3d vPos = o->m_vPos + pStatObjLODLowest->GetCenter()*o->m_fScale*fFadeOut;
    /*
    DrawLabel(o->m_vPos,2,"P");
    Draw3dPrim(o->m_vPos-Vec3d(0.01f,0.01f,0.01f),o->m_vPos+Vec3d(0.01f,0.01f,0.01f), DPRIM_SOLID_SPHERE);
    DrawLabel(vPos,2,"C");
    Draw3dPrim(vPos-Vec3d(0.01f,0.01f,0.01f),vPos+Vec3d(0.01f,0.01f,0.01f), DPRIM_SOLID_SPHERE);
    */

    // slow lod switch
    /*    if(o->m_ucAlpha<255)
    {
    float fK = (1.f - o->m_ucAlpha/255.f)*fSpriteScaleH;
    Vec3d vOffset = (vPos-vCamPos)/fDistance;    
    vPos += vOffset * fK;
    float fOffsetScale = 1.f + fK/fDistance;
    fSpriteScaleH *= fOffsetScale;
    fSpriteScaleV *= fOffsetScale;
    }*/

    if(CV_r_VolumetricFog && o->m_nFogVolumeID>0)
    {
      //continue;
      // setup Volume Fog Texgen planes
      if (!vpGL_FV)
      {
        vpGL_FV = (CCGVProgram_GL *)pVP_FV;
        pBindBend_FV = vpGL->mfGetParameterBind("Bend");
        pBindPos_FV = vpGL->mfGetParameterBind("ObjPos");
        pVP_FV->mfSet(true, NULL);
        pVP_FV->mfSetVariables(false, NULL);

        if (!m_RP.m_RCSprites_FV)
          m_RP.m_RCSprites_FV = CPShader::mfForName("CGRCTreeSprites_FV");

        fb = &m_RP.m_FogVolumes[o->m_nFogVolumeID];
        if (fb->m_FogInfo.m_WaveFogGen.m_eWFType)
        {
          float f = SEvalFuncs::EvalWaveForm(&fb->m_FogInfo.m_WaveFogGen);

          fb->m_fMaxDist = f;
        }
        float intens = fb->m_fMaxDist;
        if (intens <= 0)
          intens = 1.0f;
        intens = -0.25f / intens;
        plane00.n.x = intens*m_CameraMatrix(0,2);
        plane00.n.y = intens*m_CameraMatrix(1,2);
        plane00.n.z = intens*m_CameraMatrix(2,2);
        plane00.d   = intens*m_CameraMatrix(3,2);
        plane00.d += 0.5f;
        pBindTG00 = vpGL_FV->mfGetParameterBind("TexGen00");

        plane01.n.x = plane01.n.y = plane01.n.z = 0;
        plane01.d = 0.49f;
        pBindTG01 = vpGL_FV->mfGetParameterBind("TexGen01");

        plane11.n.x = 0;
        plane11.n.y = 0;
        plane11.n.z = 0;
        plane11.d     = fb->m_Normal.Dot(m_RP.m_ViewOrg) - fb->m_Dist;
        float fSmooth;
        FogColor = fb->m_Color;
        if (plane11.d < -0.5f)
          fSmooth = 1.0f;
        else
          fSmooth = 0.1f;
        plane11.d *= fSmooth;
        plane11.d += 0.5f;
        pBindTG10 = vpGL_FV->mfGetParameterBind("TexGen10");

        plane10.n.x = fb->m_Normal.x * fSmooth;
        plane10.n.y = fb->m_Normal.y * fSmooth;
        plane10.n.z = fb->m_Normal.z * fSmooth;
        plane10.d     = -(fb->m_Dist) * fSmooth;
        plane10.d += 0.5f;
        pBindTG11 = vpGL_FV->mfGetParameterBind("TexGen11");
      }
      if (lastvpGL != vpGL_FV)
      {
        pCurBindBend = pBindBend_FV;
        pCurBindPos = pBindPos_FV;
        lastvpGL = vpGL_FV;
        curRC = (CCGPShader_GL *)m_RP.m_RCSprites_FV;
        if (pVP_FV)
        {
          pVP_FV->mfSet(true, NULL);
          pVP->mfSetVariables(false, NULL);

          if (pBindTG00)
            vpGL_FV->mfParameter4f(pBindTG00, &plane00.n.x);
          if (pBindTG01)
            vpGL_FV->mfParameter4f(pBindTG01, &plane01.n.x);
          if (pBindTG10)
            vpGL_FV->mfParameter4f(pBindTG10, &plane11.n.x);
          if (pBindTG11)
            vpGL_FV->mfParameter4f(pBindTG11, &plane10.n.x);
        }

        EF_SelectTMU(1);
        gRenDev->m_TexMan->m_Text_Fog->Set();
        EF_SelectTMU(2);
        gRenDev->m_TexMan->m_Text_Fog_Enter->Set();
        EF_SelectTMU(0);

        float param[4];
        if (curRC)
        {
          curRC->mfSet(true, NULL);
          param[0] = FogColor[0]; param[1] = FogColor[1]; param[2] = FogColor[2]; param[3] = FogColor[3];
          curRC->mfParameter4f("FogColor", param);
          param0[0] = m_WorldColor[0]; param0[1] = m_WorldColor[1]; param0[2] = m_WorldColor[2]; param0[3] = m_WorldColor[3];
          curRC->mfParameter4f("Ambient", param0);
        }
        EF_PushFog();
        EnableFog(false);
      }
    }
    else
    {
      if (lastvpGL != vpGL)
      {
        pCurBindBend = pBindBend;
        pCurBindPos = pBindPos;
        curRC = (CCGPShader_GL *)m_RP.m_RCSprites;

        if (lastvpGL == vpGL_FV)
          EF_PopFog();
        lastvpGL = vpGL;
        if (pVP)
        {
          pVP->mfSet(true, NULL);
          pVP->mfSetVariables(false, NULL);
        }
        if (curRC)
        {
          curRC->mfSet(true, NULL);
          float param0[4];
          param0[0] = m_WorldColor[0]; param0[1] = m_WorldColor[1]; param0[2] = m_WorldColor[2]; param0[3] = m_WorldColor[3];
          curRC->mfParameter4f("Ambient", param0);
        }

        EF_SelectTMU(1);
        EnableTMU(false);
        EF_SelectTMU(2);
        EnableTMU(false);
        EF_SelectTMU(0);
      }
    }

    float dy = DX*fSpriteScaleH/fDistance;
    float dx = DY*fSpriteScaleH/fDistance;
    if (lastvpGL)
    {
      //			float fGroupBending = pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].fBending;

      float fIrv = 1.0f / pStatObjLODLowest->GetRadiusVert();
      float fIScale = 1.0f / o->m_fScale;
      wfMain.m_Freq = fIrv/8.0f+0.2f;
      wfMain.m_Phase = vPos.x/8.0f;
      v[2] = o->m_fFinalBending;
      //        Min(1.5f,2.5f*(o->m_fCurrentBending + pObjMan->m_fWindForce)*fGroupBending*fIrv) * 1.8f;  // Bending factor
      v[0] = SEvalFuncs::EvalWaveForm(&wfMain) * fIScale;       // x amount
      wfMain.m_Freq = fIrv/7.0f+0.2f;
      wfMain.m_Phase = vPos.y/8.0f;
      v[1] = SEvalFuncs::EvalWaveForm(&wfMain) * fIScale;       // y amount
      //CVProgram_GLBase::mfParameter(GL_VERTEX_PROGRAM_NV, 20, v);
      if (pCurBindBend)
        lastvpGL->mfParameter4f(pCurBindBend, v);

      v[0] = o->m_vPos.x;
      v[1] = o->m_vPos.y;
      v[2] = o->m_vPos.z;      
      //CVProgram_GLBase::mfParameter(GL_VERTEX_PROGRAM_NV, 21, v);  // Object position
      if (pCurBindPos)
        lastvpGL->mfParameter4f(pCurBindPos, v);
    }

    Vec3d vUp(0,0,-fSpriteScaleV);

    if(o->m_ucLodAngle!=127) // rotate sprite to much it with 3d object
      vUp = vUp.rotated(Vec3d(-dx,dy,0).normalized(), o->m_ucLodAngle/255.f-0.5f);

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f( 0, 0); 
    glVertex3f  ( dx+vPos.x+vUp.x,-dy+vPos.y+vUp.y,vPos.z+vUp.z);
    glTexCoord2f(  -1, 0); 
    glVertex3f  (-dx+vPos.x+vUp.x, dy+vPos.y+vUp.y,vPos.z+vUp.z);  

    glTexCoord2f(  0, 0.5f); 
    glVertex3f  ( dx+vPos.x,-dy+vPos.y, vPos.z);  
    glTexCoord2f( -1, 0.5f); 
    glVertex3f  ( -dx+vPos.x,dy+vPos.y, vPos.z);

    glTexCoord2f( 0, 1); 
    glVertex3f  ( dx+vPos.x-vUp.x, -dy+vPos.y-vUp.y, vPos.z-vUp.z);  
    glTexCoord2f(  -1, 1); 
    glVertex3f  ( -dx+vPos.x-vUp.x,dy+vPos.y-vUp.y, vPos.z-vUp.z);
    glEnd();

    m_nPolygons += 4;
  }
  if (lastvpGL)
  {
    if (lastvpGL == vpGL_FV)
    {
      //EF_PopFog();
      if (m_RP.m_RCSprites_FV)
        m_RP.m_RCSprites_FV->mfSet(false);
      EF_SelectTMU(1);
      EnableTMU(false);
      EF_SelectTMU(2);
      EnableTMU(false);
      EF_SelectTMU(0);
    }
    lastvpGL->mfSet(false, NULL);
  }

  if (m_bHeatVision)
  {
    if (m_RP.m_RCSprites_Heat)
      m_RP.m_RCSprites_Heat->mfSet(false);
  }
  else
  if (m_RP.m_RCSprites)
    m_RP.m_RCSprites->mfSet(false);

  m_RP.m_LastVP = NULL;

  m_TexMan->SetTexture(0, eTT_Base);
}

//=======================================================================

uint CGLRenderer::MakeSprite(float _fSpriteDistance, int nTexSize, float angle, IStatObj * pStatObj, uchar * _pTmpBuffer, uint def_tid)
{
  char name[256];
  int flags = FT_HASALPHA;
  int flags2 = FT2_DISCARDINCACHE | FT2_FORCEDXT;
  Vec3d vSunColor = iSystem->GetI3DEngine()->GetSunColor();
  Vec3d vSunPos = iSystem->GetI3DEngine()->GetSunPosition();
  sprintf(name, "Spr_$%s$_%d(%d,%d,%.2f,%.2f,%.2f,%.1f,%.1f,%.1f)", 
    pStatObj->GetFileName(), (int)angle, (int)_fSpriteDistance, nTexSize,
    vSunColor.x, vSunColor.y, vSunColor.z, 
    vSunPos.x, vSunPos.y, vSunPos.z);
  STexPic *ti = m_TexMan->LoadFromCache(NULL, flags, flags2, name, pStatObj->GetFileName(), eTT_Base);
  if (ti)
    return ti->m_Bind;

  int bFog=0; // remember fog value
  glGetIntegerv(GL_FOG,&bFog);
  glDisable(GL_FOG);

  glReadBuffer(GL_BACK);
  glDrawBuffer(GL_BACK);

  // render object
  ///glClearColor(0.3f, 0.3f, 0.3f, 1.0);
  //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  ClearColorBuffer(Vec3d(0.15f, 0.15f, 0.15f));
  ClearDepthBuffer();

  // cals vertical/horisontal radiuses
/*  float dxh = (float)max( fabs(pStatObj->GetBoxMax().x), fabs(pStatObj->GetBoxMin().x));
  float dyh = (float)max( fabs(pStatObj->GetBoxMax().y), fabs(pStatObj->GetBoxMin().y));
  float fRadiusHors = (float)sqrt(dxh*dxh + dyh*dyh);//max(dxh,dyh);
  float fRadiusVert = (pStatObj->GetBoxMax().z-pStatObj->GetBoxMin().z)*0.5f;
  */

  float fRadiusHors = pStatObj->GetRadiusHors();
  float fRadiusVert = pStatObj->GetRadiusVert();

  float fDrawDist = fRadiusVert*_fSpriteDistance;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective( 0.565f/_fSpriteDistance*200.f, fRadiusHors/fRadiusVert, fDrawDist-fRadiusHors, fDrawDist+fRadiusHors);
  glMatrixMode(GL_MODELVIEW);

  glViewport(0, 0, nTexSize, nTexSize);
  glLoadIdentity();

  Vec3d vCenter = (pStatObj->GetBoxMax()+pStatObj->GetBoxMin())*0.5f;

  gluLookAt( 
     0,0,0,
    -1,0,0,
     0,0,1 );

  glColor4f(1,1,1,1);
  glPushMatrix();
  glTranslatef(-fDrawDist,0,0);
  glRotatef(angle, 0,0,1);
  glTranslatef(-vCenter.x,-vCenter.y,-vCenter.z);  
  
  ResetToDefault();
  EF_SetWorldColor(0.5f,0.5f,0.5f);

//  float plane[] = {0,0,1,0};
  //gRenDev->SetClipPlane(0,plane);
  gRenDev->SetClipPlane(0,NULL);

  EF_StartEf();  
  SRendParams rParms;
//  rParms.nShaderTemplate = -1;
  pStatObj->Render(rParms,Vec3(zero),0);
  EF_EndEf3D(true);

  gRenDev->SetClipPlane(0,NULL);

  glPopMatrix();    

  // read into buffer and make texture

  assert(nTexSize<=1024);//just warning
  uchar * pMemBuffer = _pTmpBuffer;
  if(!pMemBuffer)
    pMemBuffer = new uchar [nTexSize*nTexSize*4];

  glReadPixels(0, 0, nTexSize, nTexSize, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pMemBuffer);
		
  SetTextureAlphaChannelFromRGB(pMemBuffer, nTexSize);

  STexPic *tp = m_TexMan->CreateTexture(name, nTexSize, nTexSize, 1, flags, flags2, pMemBuffer, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888, pStatObj->GetFileName());
  /*{
    int width = 64;
    int height = 64;
	  static int imid=0;
	  char buff[32]="";
	  sprintf(buff, "sprite%d.tga", imid);
	  imid++;
    for(int x=0; x<nTexSize*nTexSize*4; x+=4)
    {
      Exchange(pMemBuffer[x+0], pMemBuffer[x+2]);
    }

    ::WriteTGA(pMemBuffer, width, height, buff, 32); 
  }*/

  if(!_pTmpBuffer)  
    delete [] pMemBuffer;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetViewport();

  m_TexMan->SetTexture(0, eTT_Base);

  if(bFog)
    glEnable(GL_FOG);

  return tp->m_Bind;
}

