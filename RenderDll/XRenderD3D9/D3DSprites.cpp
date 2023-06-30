#include "RenderPCH.h"
#include "DriverD3D9.h"

//=========================================================================================

#include "D3DCGVProgram.h"
#include "D3DCGPShader.h"

#include <IStatObj.h>
#include <I3DEngine.h>
#include <IEntityRenderState.h>
#include "../Cry3DEngine/Terrain.h"
#include "../Cry3DEngine/StatObj.h"
#include "../Cry3DEngine/ObjMan.h"

void WriteTGA(byte *dat, int wdt, int hgt, char *name);

uint CD3D9Renderer::Make3DSprite(int nTexSize, float fAngleStep, IStatObj * pStatObj)
{
  return 0;
}

uint CD3D9Renderer::MakeSprite(float _fSpriteDistance, int tex_size, float angle, IStatObj * pStatObj, uchar * _pTmpBuffer, uint def_tid)
{
  HRESULT h;

  char name[256];
  int flags = FT_HASALPHA;
  int flags2 = FT2_DISCARDINCACHE | FT2_FORCEDXT;
	Vec3d vSunColor = iSystem->GetI3DEngine()->GetSunColor();
	Vec3d vSunPos = iSystem->GetI3DEngine()->GetSunPosition();
  sprintf(name, "Spr_$%s$_%d(%d,%d,%.2f,%.2f,%.2f,%.1f,%.1f,%.1f)", 
		pStatObj->GetFileName(), (int)angle, (int)_fSpriteDistance, tex_size,
		vSunColor.x, vSunColor.y, vSunColor.z, 
		vSunPos.x, vSunPos.y, vSunPos.z);
  STexPic *ti = m_TexMan->LoadFromCache(NULL, flags, flags2, name, pStatObj->GetFileName(), eTT_Base);
  if (ti)
    return ti->m_Bind;

	float fRadiusHors = pStatObj->GetRadiusHors();
	float fRadiusVert = pStatObj->GetRadiusVert();

	float fDrawDist = fRadiusVert*_fSpriteDistance;
  m_RP.m_PersFlags |= RBPF_MAKESPRITE;

  EF_PushFog();
  EnableFog(false);

  D3DXMATRIX *m = m_matProj->GetTop();
  D3DXMatrixPerspectiveFovRH(m, 0.565f/_fSpriteDistance*200.f*(gf_PI/180.0f), fRadiusHors/fRadiusVert, fDrawDist-fRadiusHors, fDrawDist+fRadiusHors);
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
  m_bInvertedMatrix = false;

  if (!m_SceneRecurseCount)
    m_pd3dDevice->BeginScene();
  m_SceneRecurseCount++;

  m_Viewport.X = 0;
  m_Viewport.Y = 0;
  m_Viewport.Width = tex_size;
  m_Viewport.Height = tex_size;
  m_Viewport.MinZ = 0.0f;
  m_Viewport.MaxZ = 1.0f;
  m_pd3dDevice->SetViewport(&m_Viewport);

	Vec3d vCenter = (pStatObj->GetBoxMax()+pStatObj->GetBoxMin())*0.5f;

  D3DXVECTOR3 Eye = D3DXVECTOR3(0,0,0);
  D3DXVECTOR3 At = D3DXVECTOR3(-1,0,0);
  D3DXVECTOR3 Up = D3DXVECTOR3(0,0,1);

  m = m_matView->GetTop();  
  D3DXMatrixLookAtRH(m, &Eye, &At, &Up);
  m_matView->TranslateLocal(-fDrawDist,0,0);
  D3DXVECTOR3 Axis = D3DXVECTOR3(0,0,1);
  m_matView->RotateAxisLocal(&Axis, angle*(gf_PI/180.0f));
  m_matView->TranslateLocal(-vCenter.x,-vCenter.y,-vCenter.z);
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
  EF_SetCameraInfo();
  
  LPDIRECT3DSURFACE9 pTargetSurf;
  h = m_pd3dDevice->CreateRenderTarget(tex_size, tex_size, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE, &pTargetSurf, NULL);
  if (FAILED(h))
    return 0;

  h = EF_SetRenderTarget(pTargetSurf, true);

  // render object
  CFColor clearCol = CFColor(0.15f,0.15f,0.15f,0);
  EF_ClearBuffers(true, false, &clearCol[0]);
  EF_SetWorldColor(0.5f,0.5f,0.5f);

  int nPrevStr = CRenderer::CV_r_texturesstreamingsync;
  CRenderer::CV_r_texturesstreamingsync = 1;

  EF_StartEf();  
	SRendParams rParms;
//  rParms.nShaderTemplate = -1;
  pStatObj->Render(rParms,Vec3(zero),0);
  EF_EndEf3D(true);

  CRenderer::CV_r_texturesstreamingsync = nPrevStr;
  h = EF_RestoreRenderTarget();

  STexPic *tp = NULL;
  // Copy data
  D3DLOCKED_RECT d3dlrTarg;
  h = pTargetSurf->LockRect(&d3dlrTarg, NULL, 0);
  if (!FAILED(h))
  {
    // set alpha
    byte *dst = new byte [tex_size*tex_size*4];
    byte *ds = (byte *)d3dlrTarg.pBits;
    for (int i=0; i<tex_size; i++)
    {
      int ni0 = (tex_size-i-1)*tex_size*4;
      int ni1 = i * d3dlrTarg.Pitch;
      memcpy(&dst[ni0], &ds[ni1], tex_size*4);
    }
    h = pTargetSurf->UnlockRect();
    SetTextureAlphaChannelFromRGB(dst, tex_size);
  	//WriteTGA(dst, tex_size, tex_size, "bug.tga", 32);
    tp = m_TexMan->CreateTexture(name, tex_size, tex_size, 1, flags, flags2, dst, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);
    SAFE_DELETE_ARRAY(dst);
  }


  SAFE_RELEASE (pTargetSurf);

  m_matProj->LoadIdentity();
  m = m_matProj->GetTop();
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
  
  m_matView->LoadIdentity();
  m = m_matView->GetTop();
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m);

  SetViewport();  
  EF_PopFog();
  m_RP.m_PersFlags &= ~RBPF_MAKESPRITE;

  m_SceneRecurseCount--;
  if (!m_SceneRecurseCount)
    m_pd3dDevice->EndScene();
  
  return tp->m_Bind;
}



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

#endif // WIN32

void CD3D9Renderer::DrawObjSprites_NoBend (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan)
{
	assert(!"Texcoord correction not implemented");

  HRESULT hr;

  // Sorting far objects front to back since we use alphablending

  //::Sort(&(*pList)[0], pList->Count());
  //pList->SortByDistanceMember_(true);

  float max_view_dist = fMaxViewDist*0.8f;
  const Vec3d & vCamPos = m_RP.m_ViewOrg;
  const float rad2deg = 180.0f/PI;    
  const float far_tex_angle = (FAR_TEX_ANGLE*0.5f);

  CD3D9TexMan::BindNULL(1);
  m_TexMan->SetTexture(TX_FIRSTBIND, eTT_Base);

  int nf = VERTEX_FORMAT_P3F_COL4UB_TEX2F;
  m_RP.m_CurVFormat = nf;
  m_RP.m_FlagsModificators &= ~7;

  hr = EF_SetVertexDeclaration(m_RP.m_FlagsModificators&7, m_RP.m_CurVFormat);
  if (hr != S_OK)
    return;
  int nOffs;

  EF_SetColorOp(eCO_MODULATE2X, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);

  Vec3d vWorldColor = iSystem->GetI3DEngine()->GetWorldColor() * 255.0f;
  int prev_tid=-1;

  IDirect3DVertexBuffer9 *pCurVB = NULL;

  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F vQuad[4];
  vQuad[0].st[0] = 0;
  vQuad[0].st[1] = 0;
  vQuad[1].st[0] = -1.0f;
  vQuad[1].st[1] = 0;
  vQuad[2].st[0] = 0;
  vQuad[2].st[1] = 1.0f;
  vQuad[3].st[0] = -1.0f;
  vQuad[3].st[1] = 1.0f;

  bool bVolFog = false;
  bool bVolFogInit = false;
  D3DXMATRIX *mi = EF_InverseMatrix();
  D3DXMATRIX mat0, mat1;
  D3DXMATRIX mat;
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

    float fMaxDist = o->m_fWSMaxViewDist;// GetMaxViewDist();

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

    float DX = o->m_vPos.x - vCamPos.x;
    float DY = o->m_vPos.y - vCamPos.y;
    float DX1 = vCenter.x + DX;
    float DY1 = vCenter.y + DY;
    float fAngle = rad2deg*cry_atan2f(DX1, DY1);
    while(fAngle<0) fAngle+=360;

    int nSlotId = (int)(fAngle/FAR_TEX_ANGLE+0.5f)%FAR_TEX_COUNT;
    assert(nSlotId>=0 && nSlotId<FAR_TEX_COUNT);
    int tid = pStatObjLOD0->m_arrSpriteTexID[nSlotId];

    if(SRendItem::m_RecurseLevel==1)
      o->m_ucAngleSlotId = nSlotId; // store sprite id for morfing

    if(prev_tid != tid)
    {
      m_TexMan->SetTexture(tid, eTT_Base);
      prev_tid = tid;
    }

    vBright.CheckMin(Vec3d(1,1,1));
    UCol col;
    col.bcolor[0] = (byte)(vBright.x * vWorldColor.z);
    col.bcolor[1] = (byte)(vBright.y * vWorldColor.y);
    col.bcolor[2] = (byte)(vBright.z * vWorldColor.x);
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
        D3DXMatrixIdentity(&mat);
        float intens = fb->m_fMaxDist;
        if (intens <= 0)
          intens = 1.0f;
        intens = -0.25f / intens;
        mat.m[0][0] = intens*m_CameraMatrix(0,2);
        mat.m[1][0] = intens*m_CameraMatrix(1,2);
        mat.m[2][0] = intens*m_CameraMatrix(2,2);
        mat.m[3][0] = intens*m_CameraMatrix(3,2) + 0.5f;

        mat.m[0][1] = mat.m[1][1] = mat.m[2][1] = 0;
        mat.m[3][1] = 0.49f;
        D3DXMatrixMultiply(&mat0, mi, &mat);

        mat.m[0][0] = 0;
        mat.m[1][0] = 0;
        mat.m[2][0] = 0;
        mat.m[3][0] = fb->m_Normal.Dot(m_RP.m_ViewOrg) - fb->m_Dist;
        float fSmooth;
        FogColor = fb->m_Color;
        if (mat.m[3][0] < -0.5f)
          fSmooth = 1.0f;
        else
          fSmooth = 0.1f;
        mat.m[3][0] = mat.m[3][0] * fSmooth + 0.5f;

        mat.m[0][1] = fb->m_Normal.x * fSmooth;
        mat.m[1][1] = fb->m_Normal.y * fSmooth;
        mat.m[2][1] = fb->m_Normal.z * fSmooth;
        mat.m[3][1] = -(fb->m_Dist) * fSmooth + 0.5f;
        D3DXMatrixMultiply(&mat1, mi, &mat);
      }
      if (!bVolFog)
      {
        bVolFog = true;

        m_pd3dDevice->SetTransform(D3DTS_TEXTURE1, &mat0);
        m_pd3dDevice->SetTransform(D3DTS_TEXTURE2, &mat1);
        m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, 2);
        m_pd3dDevice->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, 2);
        m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | 1);
        m_pd3dDevice->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | 2);

        EF_SelectTMU(1);
        gRenDev->m_TexMan->m_Text_Fog->Set();
        EF_SelectTMU(2);
        gRenDev->m_TexMan->m_Text_Fog_Enter->Set();
        EF_SelectTMU(0);

        float param[4];
        if (m_RP.m_RCSprites_FV)
        {
          m_RP.m_RCSprites_FV->mfSet(true, NULL);
          param[0] = FogColor[0]; param[1] = FogColor[1]; param[2] = FogColor[2]; param[3] = FogColor[3];
          CCGPShader_D3D *pRC = (CCGPShader_D3D *)m_RP.m_RCSprites_FV;
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

        D3DXMatrixIdentity(&mat);
        m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        m_pd3dDevice->SetTransform(D3DTS_TEXTURE1, &mat);
        m_pd3dDevice->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        m_pd3dDevice->SetTransform(D3DTS_TEXTURE2, &mat);
        m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1);
        m_pd3dDevice->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 2);

        //EF_PopFog();
        if (m_RP.m_RCSprites_FV)
          m_RP.m_RCSprites_FV->mfSet(false);

        EF_SelectTMU(1);
        EnableTMU(false);
        EF_SelectTMU(2);
        EnableTMU(false);
        EF_SelectTMU(0);
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

    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vDst = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);

    float fX0 = vPos.x+dx;
    float fX1 = vPos.x-dx;
    float fY0 = vPos.y+dy;
    float fY1 = vPos.y-dy;

    // 0,1 verts
    vQuad[0].xyz.x = fX0+vUp.x;
    vQuad[0].xyz.y = fY1+vUp.y;
    vQuad[0].xyz.z = vPos.z+vUp.z;
    vQuad[0].color.dcolor = col.dcolor;

    vQuad[1].xyz.x = fX1+vUp.x;
    vQuad[1].xyz.y = fY0+vUp.y;
    vQuad[1].xyz.z = vPos.z+vUp.z;
    vQuad[1].color.dcolor = col.dcolor;

    // 2,3 verts
    vQuad[2].xyz.x = fX0-vUp.x;
    vQuad[2].xyz.y = fY1-vUp.y;
    vQuad[2].xyz.z = vPos.z-vUp.z;
    vQuad[2].color.dcolor = col.dcolor;

    vQuad[3].xyz.x = fX1-vUp.x;
    vQuad[3].xyz.y = fY0-vUp.y;
    vQuad[3].xyz.z = vPos.z-vUp.z;
    vQuad[3].color.dcolor = col.dcolor;

    memcpy(vDst, vQuad, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F)*4);

    UnlockVB3D();

    if (pCurVB != m_pVB3D[0])
    {
      pCurVB = m_pVB3D[0];
      m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    }

    hr = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, nOffs, 2);

    m_nPolygons += 2;
  }

  if (bVolFog)
  {
    bVolFog = false;

    D3DXMatrixIdentity(&mat);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    m_pd3dDevice->SetTransform(D3DTS_TEXTURE1, &mat);
    m_pd3dDevice->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    m_pd3dDevice->SetTransform(D3DTS_TEXTURE2, &mat);

    //EF_PopFog();
    if (m_RP.m_RCSprites_FV)
      m_RP.m_RCSprites_FV->mfSet(false);

    EF_SelectTMU(1);
    EnableTMU(false);
    EF_SelectTMU(2);
    EnableTMU(false);
    EF_SelectTMU(0);
  }


  m_TexMan->SetTexture(0, eTT_Base);
}

static CObjManager *sObjMan;

static _inline int Compare(CStatObjInst *& p1, CStatObjInst *& p2)
{
  CStatObj * pStatObj1 = sObjMan->m_lstStaticTypes[p1->m_nObjectTypeID].GetStatObj();
  CStatObj * pStatObj2 = sObjMan->m_lstStaticTypes[p2->m_nObjectTypeID].GetStatObj();
  if((UINT_PTR)p1 > (UINT_PTR)p2)
    return 1;
  else
  if((UINT_PTR)p1 < (UINT_PTR)p2)
    return -1;
  
  return 0;
}

void CD3D9Renderer::ObjSpritesFlush (TArray<struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F>& Verts, IDirect3DVertexBuffer9 *&pCurVB)
{
  int nOffs;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vDst = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(Verts.Num(), nOffs);
  cryMemcpy(vDst, &Verts[0], sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F)*Verts.Num());
  UnlockVB3D();

  if (pCurVB != m_pVB3D[0])
  {
    pCurVB = m_pVB3D[0];
    m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  }

  int nPolys = Verts.Num()/3;
  m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, nOffs, nPolys);
  m_nPolygons += nPolys;

  Verts.SetUse(0);
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
	float m_fTexCoordOffset;

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


void CD3D9Renderer::DrawObjSprites_NoBend_Merge (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan)
{
  HRESULT hr;
  int i;

  // Sorting far objects front to back since we use alphablending
  //sObjMan = pObjMan;
  //::Sort(&(*pList)[0], pList->Count());
  //pList->SortByDistanceMember_(true);

  float max_view_dist = fMaxViewDist*0.8f;
  const Vec3d & vCamPos = m_RP.m_ViewOrg;
  const float rad2deg = 180.0f/PI;    
  const float far_tex_angle = (FAR_TEX_ANGLE*0.5f);

  Vec3d vWorldColor = iSystem->GetI3DEngine()->GetWorldColor() * 255.0f;

  static TArray<struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F> sVerts;
  sVerts.SetUse(0);
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
      cryPrefetchT0SSE((byte *)oNext+16);
    }

		assert(SRendItem::m_RecurseLevel>=1 && SRendItem::m_RecurseLevel-1<=2);
		float fDistance = (((IEntityRender*)o))->m_arrfDistance[SRendItem::m_RecurseLevel-1];

    float fMaxDist = o->m_fWSMaxViewDist;// GetMaxViewDist();
    
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

		int nSlotId = FtoI(fAngle/FAR_TEX_ANGLE)%FAR_TEX_COUNT;
		assert(nSlotId>=0 && nSlotId<FAR_TEX_COUNT);
		pSP->m_TexID = pStatObjLOD0->m_arrSpriteTexID[nSlotId];

		if(pStatObjLOD0->m_nSpriteTexRes)
			pSP->m_fTexCoordOffset = 0.5f/pStatObjLOD0->m_nSpriteTexRes;
		else
			pSP->m_fTexCoordOffset = 0.5f/64.f; // 64.f is FAR_TEX_SIZE 

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
    pSP->m_Color.bcolor[0] = (byte)(vBright.x * vWorldColor.z);
    pSP->m_Color.bcolor[1] = (byte)(vBright.y * vWorldColor.y);
    pSP->m_Color.bcolor[2] = (byte)(vBright.z * vWorldColor.x);
    pSP->m_Color.bcolor[3] = 255;

    pSP->m_nFogVolumeID = o->m_nFogVolumeID;
  }

  ::Sort(&sSPInfo[0], sSPInfo.Num());

  int prev_tid = -1;
  CD3D9TexMan::BindNULL(1);
  m_TexMan->SetTexture(TX_FIRSTBIND, eTT_Base);

  int nf = VERTEX_FORMAT_P3F_COL4UB_TEX2F;
  m_RP.m_CurVFormat = nf;
  m_RP.m_FlagsModificators &= ~7;

  hr = EF_SetVertexDeclaration(m_RP.m_FlagsModificators&7, m_RP.m_CurVFormat);
  if (hr != S_OK)
    return;


  CCGPShader_D3D *fpHDRSpr = NULL;
  CCGPShader_D3D *fpHDRSpr_FV = NULL;
  CCGVProgram_D3D *vpHDRSpr = NULL;
  int bFogCorrect = false;
  if (m_RP.m_PersFlags & RBPF_HDR)
  {
    fpHDRSpr = (CCGPShader_D3D *)PShaderForName(m_RP.m_PS_HDR_BaseCol, "CGRC_HDR_BaseCol_PS20");
    if (fpHDRSpr)
      fpHDRSpr->mfSet(true);

    vpHDRSpr = (CCGVProgram_D3D *)VShaderForName(m_RP.m_VP_BaseCol, "CGVProgBaseCol");
    if (vpHDRSpr)
      vpHDRSpr->mfSet(true);
    bFogCorrect = EF_FogCorrection(true, true);
  }
  else
    EF_SetColorOp(eCO_MODULATE2X, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);

  IDirect3DVertexBuffer9 *pCurVB = NULL;

  bool bVolFog = false;
  bool bVolFogInit = false;
  D3DXMATRIX *mi = EF_InverseMatrix();
  D3DXMATRIX mat0, mat1;
  D3DXMATRIX mat;
  CFColor FogColor;

  for (i=0; i<sSPInfo.Num(); i++)
  {
    SSpriteInfo *pSP = &sSPInfo[i];
    if(prev_tid != pSP->m_TexID)
    {
      if (sVerts.Num())
        ObjSpritesFlush(sVerts, pCurVB);
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
        D3DXMatrixIdentity(&mat);
        float intens = fb->m_fMaxDist;
        if (intens <= 0)
          intens = 1.0f;
        intens = -0.25f / intens;
        mat.m[0][0] = intens*m_CameraMatrix(0,2);
        mat.m[1][0] = intens*m_CameraMatrix(1,2);
        mat.m[2][0] = intens*m_CameraMatrix(2,2);
        mat.m[3][0] = intens*m_CameraMatrix(3,2) + 0.5f;

        mat.m[0][1] = mat.m[1][1] = mat.m[2][1] = 0;
        mat.m[3][1] = 0.49f;
        D3DXMatrixMultiply(&mat0, mi, &mat);

        mat.m[0][0] = 0;
        mat.m[1][0] = 0;
        mat.m[2][0] = 0;
        mat.m[3][0] = fb->m_Normal.Dot(m_RP.m_ViewOrg) - fb->m_Dist;
        float fSmooth;
        FogColor = fb->m_Color;
        if (mat.m[3][0] < -0.5f)
          fSmooth = 1.0f;
        else
          fSmooth = 0.1f;
        mat.m[3][0] = mat.m[3][0] * fSmooth + 0.5f;

        mat.m[0][1] = fb->m_Normal.x * fSmooth;
        mat.m[1][1] = fb->m_Normal.y * fSmooth;
        mat.m[2][1] = fb->m_Normal.z * fSmooth;
        mat.m[3][1] = -(fb->m_Dist) * fSmooth + 0.5f;
        D3DXMatrixMultiply(&mat1, mi, &mat);
      }
      if (!bVolFog)
      {
        if (sVerts.Num())
          ObjSpritesFlush(sVerts, pCurVB);

        bVolFog = true;

        m_pd3dDevice->SetTransform(D3DTS_TEXTURE1, &mat0);
        m_pd3dDevice->SetTransform(D3DTS_TEXTURE2, &mat1);
        m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, 2);
        m_pd3dDevice->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, 2);
        m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | 1);
        m_pd3dDevice->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | 2);

        EF_SelectTMU(1);
        gRenDev->m_TexMan->m_Text_Fog->Set();
        EF_SelectTMU(2);
        gRenDev->m_TexMan->m_Text_Fog_Enter->Set();
        EF_SelectTMU(0);

        float param[4];
        if (!(m_RP.m_PersFlags & RBPF_HDR))
        {
          if (m_RP.m_RCSprites_FV)
            fpHDRSpr_FV = (CCGPShader_D3D *)m_RP.m_RCSprites_FV;
        }
        else
          fpHDRSpr_FV = (CCGPShader_D3D *)PShaderForName(m_RP.m_PS_HDR_BaseCol_FV, "CGRC_HDR_BaseCol_FV_PS20");
        param[0] = FogColor[0]; param[1] = FogColor[1]; param[2] = FogColor[2]; param[3] = FogColor[3];
        if (fpHDRSpr_FV)
        {
          fpHDRSpr_FV->mfSet(true);
          fpHDRSpr_FV->mfParameter4f("FogColor", param);
        }
        if (vpHDRSpr)
          vpHDRSpr->mfSet(false);
        //EF_PushFog();
        //EnableFog(false);
      }
    }
    else
    {
      if (bVolFog)
      {
        if (sVerts.Num())
          ObjSpritesFlush(sVerts, pCurVB);

        bVolFog = false;

        D3DXMatrixIdentity(&mat);
        m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        m_pd3dDevice->SetTransform(D3DTS_TEXTURE1, &mat);
        m_pd3dDevice->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        m_pd3dDevice->SetTransform(D3DTS_TEXTURE2, &mat);
        m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1);
        m_pd3dDevice->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 2);

        //EF_PopFog();
        if (fpHDRSpr_FV)
          fpHDRSpr_FV->mfSet(false);
        if (fpHDRSpr)
          fpHDRSpr->mfSet(true);
        if (vpHDRSpr)
          vpHDRSpr->mfSet(true);

        EF_SelectTMU(1);
        EnableTMU(false);
        EF_SelectTMU(2);
        EnableTMU(false);
        EF_SelectTMU(0);
      }
    }

    float fX0 = pSP->m_vPos.x+pSP->m_fDX;
    float fX1 = pSP->m_vPos.x-pSP->m_fDX;
    float fY0 = pSP->m_vPos.y+pSP->m_fDY;
    float fY1 = pSP->m_vPos.y-pSP->m_fDY;

    Vec3d vUp = Vec3d(0,0,-pSP->m_fScaleV);

    if(pSP->m_LodAngle!=127) // rotate sprite to much it with 3d object
      vUp = vUp.rotated(Vec3d(-pSP->m_fDX,pSP->m_fDY,0).normalized(), pSP->m_LodAngle/255.f-0.5f);

    int nOffs = sVerts.Num();
    if (nOffs+6 >= MAX_DYNVB3D_VERTS)
    {
      ObjSpritesFlush(sVerts, pCurVB);
      nOffs = sVerts.Num();
    }
    sVerts.AddIndex(6);
    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pQuad = &sVerts[nOffs];

    // 0,1 verts
    pQuad[0].xyz.x = fX0+vUp.x;
    pQuad[0].xyz.y = fY1+vUp.y;
    pQuad[0].xyz.z = pSP->m_vPos.z+vUp.z;
    pQuad[0].color.dcolor = pSP->m_Color.dcolor;
    pQuad[0].st[0] = 0+pSP->m_fTexCoordOffset;
    pQuad[0].st[1] = 0-pSP->m_fTexCoordOffset;

    pQuad[1].xyz.x = fX1+vUp.x;
    pQuad[1].xyz.y = fY0+vUp.y;
    pQuad[1].xyz.z = pSP->m_vPos.z+vUp.z;
    pQuad[1].color.dcolor = pSP->m_Color.dcolor;
    pQuad[1].st[0] = -1.0f+pSP->m_fTexCoordOffset;
    pQuad[1].st[1] = 0-pSP->m_fTexCoordOffset;

		// 2,3 verts
    pQuad[2].xyz.x = fX0-vUp.x;
    pQuad[2].xyz.y = fY1-vUp.y;
    pQuad[2].xyz.z = pSP->m_vPos.z-vUp.z;
    pQuad[2].color.dcolor = pSP->m_Color.dcolor;
    pQuad[2].st[0] = 0+pSP->m_fTexCoordOffset;
    pQuad[2].st[1] = 1.0f-pSP->m_fTexCoordOffset;

    pQuad[3] = pQuad[1];

    pQuad[4].xyz.x = fX1-vUp.x;
    pQuad[4].xyz.y = fY0-vUp.y;
    pQuad[4].xyz.z = pSP->m_vPos.z-vUp.z;
    pQuad[4].color.dcolor = pSP->m_Color.dcolor;
    pQuad[4].st[0] = -1.0f+pSP->m_fTexCoordOffset;
    pQuad[4].st[1] = 1.0f-pSP->m_fTexCoordOffset;

    pQuad[5] = pQuad[2];
  }

  if (sVerts.Num())
    ObjSpritesFlush(sVerts, pCurVB);

  if (bVolFog)
  {
    bVolFog = false;

    D3DXMatrixIdentity(&mat);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    m_pd3dDevice->SetTransform(D3DTS_TEXTURE1, &mat);
    m_pd3dDevice->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    m_pd3dDevice->SetTransform(D3DTS_TEXTURE2, &mat);

    //EF_PopFog();
    if (fpHDRSpr_FV)
      fpHDRSpr_FV->mfSet(false, 0);

    EF_SelectTMU(1);
    EnableTMU(false);
    EF_SelectTMU(2);
    EnableTMU(false);
    EF_SelectTMU(0);
  }


  if (fpHDRSpr)
    fpHDRSpr->mfSet(false);
  if (vpHDRSpr)
    vpHDRSpr->mfSet(false);
  if (m_RP.m_PersFlags & RBPF_HDR)
    EF_FogRestore(bFogCorrect);

  m_TexMan->SetTexture(0, eTT_Base);
}

void CD3D9Renderer::DrawObjSprites (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan)
{
  HRESULT hr;

  PROFILE_FRAME(Draw_ObjSprites);

  CD3D9TexMan::BindNULL(1);
  //ResetToDefault();
  if(CV_r_VegetationSpritesAlphaBlend)
    EF_SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_ALPHATEST_GREATER0);
  else
    EF_SetState(GS_ALPHATEST_GEQUAL128 | GS_DEPTHWRITE);

  EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  D3DSetCull(eCULL_None);

  hr = m_pd3dDevice->SetStreamSource(1, NULL, 0, 0);
  hr = m_pd3dDevice->SetStreamSource(2, NULL, 0, 0);
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
  m_RP.m_PersFlags &= ~(RBPF_USESTREAM1 | RBPF_USESTREAM2);
  m_RP.m_TexStages[0].TCIndex = 0;
  m_RP.m_TexStages[1].TCIndex = 1;

  m_RP.m_PersFlags &= ~(RBPF_VSNEEDSET | RBPF_PS1NEEDSET);
  m_RP.m_FlagsModificators = 0;
  int nGPU = m_Features & RFT_HW_MASK;
  if (m_FS.m_bEnable && nGPU != RFT_HW_RADEON)
  {
    if (m_FS.m_nCurFogMode!=m_FS.m_nFogMode)
    {
      m_FS.m_nCurFogMode = m_FS.m_nFogMode;
      m_pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
    }
  }

  if (CV_r_VegetationSpritesNoBend)
  {
    if (CV_r_VegetationSpritesNoBend==1)
      DrawObjSprites_NoBend(pList, fMaxViewDist, pObjMan);
    else
      DrawObjSprites_NoBend_Merge(pList, fMaxViewDist, pObjMan);
    return;
  }

	assert(!"Texcoord correction not implemented");

  // Sorting far objects front to back since we use alphablending
//  ::Sort(&(*pList)[0], pList->Count());
  //pList->SortByDistanceMember_(true);

  float max_view_dist = fMaxViewDist*0.8f;
  const Vec3d & vCamPos = m_RP.m_ViewOrg;
  const float rad2deg = 180.0f/PI;    
  const float far_tex_angle = (FAR_TEX_ANGLE*0.5f);

  m_TexMan->SetTexture(TX_FIRSTBIND, eTT_Base);

  float v[4];
  v[3] = 1.0f;
  CCGVProgram_D3D *pVP = NULL;
  CCGVProgram_D3D *pVP_FV = NULL;
  // If device supports vertex shaders use advanced bending for sprites
  if (!m_RP.m_VPPlantBendingSpr && (GetFeatures() & RFT_HW_VS))
  {
    //pVP = (CVProgram_D3D *)CVProgram::mfForName("VProgSimple_Plant_Bended_Sprite", false);
    pVP = (CCGVProgram_D3D *)CVProgram::mfForName("CGVProgSimple_Plant_Bended_Sprite");
    pVP_FV = (CCGVProgram_D3D *)CVProgram::mfForName("CGVProgSimple_Plant_Bended_Sprite_FV");
    m_RP.m_VPPlantBendingSpr = pVP;
    m_RP.m_VPPlantBendingSpr_FV = pVP_FV;
  }
  else
  {
    pVP = (CCGVProgram_D3D *)m_RP.m_VPPlantBendingSpr;
    pVP_FV = (CCGVProgram_D3D *)m_RP.m_VPPlantBendingSpr_FV;
  }

  SCGBind *pBindBend = NULL;
  SCGBind *pBindPos = NULL;
  int nf = VERTEX_FORMAT_P3F_COL4UB_TEX2F;
  m_RP.m_CurVFormat = nf;
  m_RP.m_FlagsModificators &= ~7;

  SMFog *fb = NULL;

  SCGBind *pBindTG00, *pBindTG01, *pBindTG10, *pBindTG11;

  SCGBind *pBindBend_FV = NULL;
  SCGBind *pBindPos_FV = NULL;
  CFColor FogColor;
  Plane plane00, plane01, plane10, plane11;

  SCGBind *pCurBindBend = NULL;
  SCGBind *pCurBindPos = NULL;

  CCGVProgram_D3D *lastvpD3D = NULL;

  SWaveForm2 wfMain;
  if (!m_RP.m_RCSprites)
    m_RP.m_RCSprites = CPShader::mfForName("CGRCTreeSprites");
  if (pVP)
  {
    wfMain.m_eWFType = eWF_Sin;
    wfMain.m_Amp = 0.002f;
    wfMain.m_Level = 0;
    pVP->mfSet(true, NULL, 0);
    pVP->mfSetVariables(false, NULL);

    lastvpD3D = pVP;
    pBindBend = pVP->mfGetParameterBind("Bend");
    pBindPos = pVP->mfGetParameterBind("ObjPos");

    pCurBindBend = pBindBend;
    pCurBindPos = pBindPos;
  }

  hr = EF_SetVertexDeclaration(m_RP.m_FlagsModificators&7, m_RP.m_CurVFormat);
  if (hr != S_OK)
    return;
  int nOffs;

  Vec3d vWorldColor = iSystem->GetI3DEngine()->GetWorldColor();
  int prev_tid=-1;

  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F vQuad[6];
  // 0,1 verts
  vQuad[0].st[0] = 0;
  vQuad[0].st[1] = 0;
  vQuad[1].st[0] = -1;
  vQuad[1].st[1] = 0;
  // 2,3 verts
  vQuad[2].st[0] = 0;
  vQuad[2].st[1] = 0.5f;
  vQuad[3].st[0] = -1;
  vQuad[3].st[1] = 0.5f;
  // 4,5 verts
  vQuad[4].st[0] = 0;
  vQuad[4].st[1] = 1.0f;
  vQuad[5].st[0] = -1.0f;
  vQuad[5].st[1] = 1.0f;

  CCGPShader_D3D *curRC = NULL;
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
      curRC = (CCGPShader_D3D *)m_RP.m_RCSprites_Heat;
      if (curRC)
      curRC->mfParameter4f("Heat", param);
    }
  }
  else
  {
    if (m_RP.m_RCSprites)
      m_RP.m_RCSprites->mfSet(true);
  }

  IDirect3DVertexBuffer9 *pCurVB = NULL;
  float fLastBend = 999999.0f;

  for( int i=pList->Count()-1; i>=0; i-- )
  { 
    CStatObjInst * o = pList->GetAt(i);
		assert(o);
		if(!o)
			continue;
    /*if (i)
    {
      CStatObjInst * oNext = pList->GetAt(i-1);
      _asm
      {
        prefetchnta byte ptr oNext
      }
    }*/

		assert(SRendItem::m_RecurseLevel>=1 && SRendItem::m_RecurseLevel-1<=2);
		float fDistance = (((IEntityRender*)o))->m_arrfDistance[SRendItem::m_RecurseLevel-1];

    float fMaxDist = o->m_fWSMaxViewDist;// GetMaxViewDist();
    
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
      m_TexMan->SetTexture(tid, eTT_Base);
      prev_tid = tid;
    }

		vBright.CheckMin(Vec3d(1,1,1));
		vBright.x *= vWorldColor.x;
		vBright.y *= vWorldColor.y;
		vBright.z *= vWorldColor.z;
    DWORD cCol = D3DRGBA(vBright.x,vBright.y,vBright.z,1.0f);

		float fSpriteScaleV = o->m_fScale*pStatObjLODLowest->GetRadiusVert()*fFadeOut;
		float fSpriteScaleH = o->m_fScale*pStatObjLODLowest->GetRadiusHors()*pObjMan->m_fZoomFactor*fFadeOut;
		Vec3d vPos = o->m_vPos + pStatObjLODLowest->GetCenter()*o->m_fScale*fFadeOut;

		if(CV_r_VolumetricFog && o->m_nFogVolumeID>0)
    {
      // setup Volume Fog Texgen planes
      if (!pBindBend_FV)
      {
        pVP_FV->mfSet(true, NULL);
        pVP_FV->mfSetVariables(false, NULL);
        pBindBend_FV = pVP_FV->mfGetParameterBind("Bend");
        pBindPos_FV = pVP_FV->mfGetParameterBind("ObjPos");

        if (!m_RP.m_RCSprites_FV)
          m_RP.m_RCSprites_FV = CPShader::mfForName("CGRCTreeSprites_FV");
        curRC = (CCGPShader_D3D *)m_RP.m_RCSprites_FV;

        fb = &m_RP.m_FogVolumes[o->m_nFogVolumeID/*o->m_nFogVolumeID*/];
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
        pBindTG00 = pVP_FV->mfGetParameterBind("TexGen00");

        plane01.n.x = plane01.n.y = plane01.n.z = 0;
        plane01.d = 0.49f;
        pBindTG01 = pVP_FV->mfGetParameterBind("TexGen01");

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
        pBindTG10 = pVP_FV->mfGetParameterBind("TexGen10");

        plane10.n.x = fb->m_Normal.x * fSmooth;
        plane10.n.y = fb->m_Normal.y * fSmooth;
        plane10.n.z = fb->m_Normal.z * fSmooth;
        plane10.d     = -(fb->m_Dist) * fSmooth;
        plane10.d += 0.5f;
        pBindTG11 = pVP_FV->mfGetParameterBind("TexGen11");
      }
      if (lastvpD3D != pVP_FV)
      {
        pCurBindBend = pBindBend_FV;
        pCurBindPos = pBindPos_FV;
        lastvpD3D = pVP_FV;
        curRC = (CCGPShader_D3D *)m_RP.m_RCSprites_FV;
        if (pVP_FV)
        {
          pVP_FV->mfSet(true, NULL);
          pVP_FV->mfSetVariables(false, NULL);

          if (pBindTG00)
            pVP_FV->mfParameter4f(pBindTG00, &plane00.n.x);
          if (pBindTG01)
            pVP_FV->mfParameter4f(pBindTG01, &plane01.n.x);
          if (pBindTG10)
            pVP_FV->mfParameter4f(pBindTG10, &plane11.n.x);
          if (pBindTG11)
            pVP_FV->mfParameter4f(pBindTG11, &plane10.n.x);
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
        }
        //EF_PushFog();
        //EnableFog(false);
      }
    }
    else
    {
      if (lastvpD3D != pVP)
      {
        pCurBindBend = pBindBend;
        pCurBindPos = pBindPos;
        curRC = (CCGPShader_D3D *)m_RP.m_RCSprites;

        if (lastvpD3D == pVP_FV)
        {
          //EF_PopFog();
          if (m_RP.m_RCSprites_FV)
            m_RP.m_RCSprites_FV->mfSet(false);
        }
        lastvpD3D = pVP;
        if (pVP)
        {
          pVP->mfSet(true, NULL);
          pVP->mfSetVariables(false, NULL);
        }

        if (curRC)
          curRC->mfSet(true, NULL);

        EF_SelectTMU(1);
        EnableTMU(false);
        EF_SelectTMU(2);
        EnableTMU(false);
        EF_SelectTMU(0);
      }
    }
    
		float dy = DX*fSpriteScaleH/fDistance;
		float dx = DY*fSpriteScaleH/fDistance;
    if (pVP)
    {
      bool bSkip = false;
      if (!o->m_fFinalBending)
      {
        if (fLastBend)
        {
          v[0] = v[1] = v[2] = 0;
        }
        else
          bSkip = true;
      }
      else
      {
			  float fIrv = 1.0f / pStatObjLODLowest->GetRadiusVert();
        float fIScale = 1.0f / o->m_fScale;
        wfMain.m_Freq = fIrv/8.0f+0.2f;
        wfMain.m_Phase = vPos.x/8.0f;
			  v[2] = o->m_fFinalBending;
        v[0] = SEvalFuncs::EvalWaveForm(&wfMain) * fIScale;       // x amount
        wfMain.m_Freq = fIrv/7.0f+0.2f;
        wfMain.m_Phase = vPos.y/8.0f;
        v[1] = SEvalFuncs::EvalWaveForm(&wfMain) * fIScale;       // y amount
      }
      fLastBend = o->m_fFinalBending;

			if (pCurBindBend && !bSkip)
        pVP->mfParameter4f(pCurBindBend, v);

      v[0] = o->m_vPos.x;
      v[1] = o->m_vPos.y;
      v[2] = o->m_vPos.z;      

			if (pCurBindPos)
        pVP->mfParameter4f(pCurBindPos, v);
    }

		Vec3d vUp(0,0,-fSpriteScaleV);

		if(o->m_ucLodAngle!=127) // rotate sprite to much it with 3d object
			vUp = vUp.rotated(Vec3d(-dx,dy,0).normalized(), o->m_ucLodAngle/255.f-0.5f);

    float fX  = vPos.x+dx;
    float fX1 = vPos.x-dx;
    float fY  = vPos.y+dy;
    float fY1 = vPos.y-dy;

    // 0,1 verts
    vQuad[0].xyz.x =	fX+vUp.x;
    vQuad[0].xyz.y =  fY1+vUp.y;
    vQuad[0].xyz.z =	vPos.z+vUp.z;
    vQuad[0].color.dcolor = cCol;

    vQuad[1].xyz.x =  fX1+vUp.x;
    vQuad[1].xyz.y =	fY+vUp.y;
    vQuad[1].xyz.z =  vPos.z+vUp.z;
    vQuad[1].color.dcolor = cCol;

		// 2,3 verts
    vQuad[2].xyz.x =	fX;
    vQuad[2].xyz.y =  fY1;
    vQuad[2].xyz.z =  vPos.z;
    vQuad[2].color.dcolor = cCol;

    vQuad[3].xyz.x =  fX1;
    vQuad[3].xyz.y =	fY;
    vQuad[3].xyz.z =  vPos.z;
    vQuad[3].color.dcolor = cCol;

		// 4,5 verts
    vQuad[4].xyz.x =	fX-vUp.x;
    vQuad[4].xyz.y =  fY1-vUp.y;
    vQuad[4].xyz.z =	vPos.z-vUp.z;
    vQuad[4].color.dcolor = cCol;

    vQuad[5].xyz.x =  fX1-vUp.x;
    vQuad[5].xyz.y =	fY-vUp.y;
    vQuad[5].xyz.z =	vPos.z-vUp.z;
    vQuad[5].color.dcolor = cCol;

    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vDst = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(6, nOffs);
    if (pCurVB != m_pVB3D[0])
    {
      pCurVB = m_pVB3D[0];
      m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    }
    memcpy(vDst, vQuad, 6*sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));

    UnlockVB3D();
    hr = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, nOffs, 4);

    m_nPolygons += 4;
  }

  //if (lastvpD3D == pVP_FV)
  //  EF_PopFog();

  if (lastvpD3D)
    lastvpD3D->mfSet(false, NULL);
  if (curRC)
    curRC->mfSet(false, NULL);

  //pVB->Unlock();

  ResetToDefault();
  m_TexMan->SetTexture(0, eTT_Base);
}

