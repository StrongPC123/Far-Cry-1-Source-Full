////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjman.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Loading trees, buildings, ragister/unregister entities for rendering
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "objman.h"
#include "cbuffer.h"
#include "3DEngine.h"

void CStatObjInst::Serialize(bool bSave, ICryPak * pPak, FILE * f)
{
  if(bSave)
    pPak->FWrite(this,sizeof(*this),1,f);
  else
    pPak->FRead(this,sizeof(*this),1,f);
}

// Bending
static float CStatObjInst__sSpline(float x)
{
  float fX = fabsf(x);

  if(fX > 2.0f)
    return 0;
  if(fX > 1.0f)
    return (2.0f-fX)*(2.0f-fX)*(2.0f-fX)/6.0f;
  return 2.0f/3.0f-fX*fX+0.5f*fX*fX*fX;
}

float CStatObjInst::Interpolate(float& pprev, float& prev, float& next, float& nnext, float ppweight, float pweight, float nweight, float nnweight)
{
  return pprev*ppweight + prev*pweight + next*nweight + nnext*nnweight;
}

float CStatObjInst::GetBendingRandomFactor()
{ // calculate random
  static float sInterpValues[16] = {1.2f,1.0f,1.1f,0.8f,0.5f,0.7f,1.0f,1.3f,0.66f,0.8f,0.7f,0.9f,1.1f,1.4f,1.1f,1.3f};

  float time = GetTimer()->GetCurrTime() * 0.666f;
  int itime = fastftol_positive(time);
  float pprev = sInterpValues[(itime-1)&15];
  float prev  = sInterpValues[(itime)&15];
  float next  = sInterpValues[(itime+1)&15];
  float nnext = sInterpValues[(itime+2)&15];

  float ppweight = CStatObjInst__sSpline(time-itime+1.0f);
  float pweight  = CStatObjInst__sSpline(time-itime+0.0f);
  float nweight  = CStatObjInst__sSpline(time-itime-1.0f);
  float nnweight = CStatObjInst__sSpline(time-itime-2.0f);
  return Interpolate(pprev, prev, next, nnext, ppweight, pweight, nweight, nnweight);
}

bool CStatObjInst::DrawEntity(const struct SRendParams & _EntDrawParams)
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

//  if(!GetCVars()->e_vegetation)
  //  return false;

  const Vec3d & vCamPos = GetViewCamera().GetPos();

  float fDistance = m_arrfDistance[m_pObjManager->m_nRenderStackLevel];

/*
  // calculate distance
  float fPrevDist0 = m_fDistance0;
  m_fDistance = GetDist2D( vCamPos.x, vCamPos.y, m_vPos.x, m_vPos.y )*m_pObjManager->m_fZoomFactor;
  if(!m_pObjManager->m_nRenderStackLevel) // remember to be able to restore this value after reqursion drawing
    m_fDistance0 = m_fDistance;*/

//	float fMaxViewDist = GetMaxViewDist();

/*  if(fDistance > fMaxViewDist)//*m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].f MaxViewDistRatio*GetCVars()->e_lod_ratio)
  {
    m_ucAngleSlotId = 255;
    return false;
  }*/
  
  CStatObj * pBody = m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].GetStatObj();
  if(!pBody)
    return false;

//  Vec3d vBoxMin = pBody->m_vBoxMin*m_fScale+m_vPos;
  //Vec3d vBoxMax = pBody->m_vBoxMax*m_fScale+m_vPos;
/*  assert(!_EntDrawParams.bAllInFrustum || cam.IsBoxVisible(vBoxMin,vBoxMax)); // test of bAllInFrustum flag

  if( bNotAllIN && !cam.IsBoxVisibleFast( vBoxMin, vBoxMax)) // NOTE: bNotAllIN can work wrong
  {
    m_ucAngleSlotId = 255;
    return;
  }*/

//  int nLastVisibleFrameID = m_OcclTestVars.nLastVisibleFrameID;
  //assert(m_pObjManager->m_nRenderStackLevel || nLastVisibleFrameID != GetFrameID());

  // recursion is handeled inside IsBoxOccluded()
//  if(m_pObjManager->IsBoxOccluded(vBoxMin, vBoxMax, m_fDistance, &m_OcclTestVars))
  //  return;

  // calculate switch to sprite distance 
  float near_far_dist = (18.f * pBody->GetRadiusVert() * m_fScale)
    * max(0.5f,m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].fSpriteDistRatio)
    + 0.2f*(float)fabs((m_vPos.z + pBody->GetCenter().z*m_fScale) - vCamPos.z);
  /*
  static float fAverageSpriteDistRatio = 0.5f;
  fAverageSpriteDistRatio += m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].fSpriteDistRatio;//(fAverageSpriteDistRatio + m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].fSpriteDistRatio)*0.5f;
  if(m_nStatObjNumPerFrame%10000==0)
  GetLog()->Log("m_nStatObjNumPerFrame = %.4f",fAverageSpriteDistRatio/m_nStatObjNumPerFrame);
  */
  near_far_dist *= GetCVars()->e_vegetation_sprites_distance_ratio;

	int nDynMask = _EntDrawParams.nDLightMask;

	// remove sun bit
	list2<CDLight> * pSources = ((C3DEngine*)m_p3DEngine)->GetDynamicLightSources();
	for(int i=0; i<pSources->Count(); i++)
	{
		CDLight * pDynLight = pSources->Get(i);
		assert(pDynLight->m_Id == i || pDynLight->m_Id == -1);
		if(pDynLight->m_Flags & DLF_SUN)
		{
			nDynMask &= ~(1<<pDynLight->m_Id);
			break;
		}
	}

  if(near_far_dist < GetCVars()->e_vegetation_sprites_min_distance)
    near_far_dist = GetCVars()->e_vegetation_sprites_min_distance;

//  m_nStatObjNumPerFrame++;

  // fade out bending amount
  if(!m_pObjManager->m_nRenderStackLevel && m_fCurrentBending > 0)
    m_fCurrentBending -= GetTimer()->GetFrameTime()*0.25f;
  if(m_fCurrentBending < 0)
    m_fCurrentBending = 0;

  // calculate exact dynamic light mask 
/*  uint nThisDynLightMaskNoSun = nDynLightMaskNoSun;
  if(nThisDynLightMaskNoSun)
  { 
    m_pObjManager->m_p3DEngine->CheckDistancesToLightSources(nThisDynLightMaskNoSun, 
      pBody->GetCenter()*m_fScale+m_vPos, pBody->GetRadius()*m_fScale);
    if(nThisDynLightMaskNoSun) // increase sprite distance if object affected by dynamic light
      near_far_dist *= 2.5f;
  }*/


  // detect lod switch soon
  const float fLodSwitchCountDown = GetCVars()->e_vegetation_sprites_slow_switch ?
    min(1.f, fabsf(fDistance - near_far_dist)/(near_far_dist*0.33f)) : 1.f;

  if(!m_pObjManager->m_nRenderStackLevel)
  { // calculate bending amount
    if(m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].fBending)
    {
      float fBending = GetBendingRandomFactor() * m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].fBending;
      m_fFinalBending = min(1.5f, 2.5f*(m_fCurrentBending + m_pObjManager->m_fWindForce)*fBending) / (pBody->GetRadiusVert()*m_fScale) * 1.8f;          
      m_fFinalBending *= fLodSwitchCountDown;
    }
    else
      m_fFinalBending = 0;
  }

/*
  if(pBody->m_bStreamable && CStatObj::m_fStreamingTimePerFrame<CGF_STREAMING_MAX_TIME_PER_FRAME)
    if(!pBody->GetLeafBuffer() || !pBody->m_nLoadedTrisCount)
      if(GetCVars()->e_stream_cgf && GetCVars()->e_stream_for_visuals)
      { // streaming
        CStatObj::m_fStreamingTimePerFrame -= GetTimer()->GetAsyncCurTime();
        bool bRes = pBody->Load(
          pBody->m_szFileName, 
          pBody->m_szGeomName[0] ? pBody->m_szGeomName : 0, 
          pBody->m_eVertsSharing, 
          pBody->m_bLoadAdditinalInfo, 
          pBody->m_bKeepInLocalSpace, false);
        pBody->m_bStreamable=true;*/
            /*
        if(pBody->GetLeafBuffer() && !((CStatObj*)pBody)->IsSpritesCreated())
          ((CStatObj*)pBody)->UpdateCustomLightingSpritesAndShadowMaps(
          m_pObjManager->m_vOutdoorAmbientColor, 
          m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].nSpriteTexRes, 
          m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].fBackSideLevel, 
          m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].bCalcLighting );
              */
  /*      CStatObj::m_fStreamingTimePerFrame += GetTimer()->GetAsyncCurTime();
      }

      
      pBody->m_nLastRendFrameId = GetFrameID();
       */

  // do we need only 3d version
  bool bUse3DOnly(!m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].bUseSprites || nDynMask);

  // detect intersection with fog volume, note: use only center
  // todo: do not calculate every frame and merge all if(!m_pObjManager->m_nRenderStackLevel) stuff
/*  if(!m_pObjManager->m_nRenderStackLevel)
  {
    if(pFogVolume && pFogVolume->InsideBBox((vBoxMin+vBoxMax)*0.5f))
      m_nFogVolumeID = pFogVolume->nRendererVolumeID;
    else
      m_nFogVolumeID = 0;
  }*/

  // render object in 3d
  if( bUse3DOnly || fDistance < near_far_dist )
  { 
    SRendParams rParms;

    // set sort offset
    rParms.fCustomSortOffset = m_pObjManager->GetSortOffset(m_vPos,vCamPos);

    rParms.nFogVolumeID = m_nFogVolumeID;

    if(GetRenderer()->EF_GetHeatVision())
    {
      rParms.nShaderTemplate = EFT_HEATVISION;
      rParms.vAmbientColor = Vec3d(0.1f, 0.1f, 0.1f);
    }
    else
      rParms.vAmbientColor = _EntDrawParams.vAmbientColor;

    rParms.vPos   = m_vPos;
    rParms.fScale = m_fScale;     
    rParms.nDLightMask = nDynMask;
		rParms.nStrongestDLightMask = nDynMask & _EntDrawParams.nStrongestDLightMask;

    // bending
//    if (useBending == 2)
    rParms.fBending = m_fFinalBending;

    // fade heat amount
    rParms.fHeatAmount = min(2.f*(1.f - fDistance/near_far_dist),1.f);

    // set alpha blend
    if( m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].bUseAlphaBlending )
    {
      rParms.nSortValue = eS_TerrainDetailObjects;
      if(m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].bUseSprites)
      { // set fixed alpha
        rParms.fAlpha = 0.99f;
      }
      else
      { // fade alpha on distance
//        float fAlpha = 1.f - fDistance / (fMaxViewDist);//*m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].fMaxViewDistRati o);
  //      rParms.fAlpha = min(0.99f,fAlpha*3);
				rParms.fAlpha = min(0.99f,_EntDrawParams.fAlpha);
      }

			// entire object will use alpha blending so lets use only one light for it
			rParms.nDLightMask = rParms.nStrongestDLightMask;
    }
    else
    { // no alpha blend
      rParms.fAlpha = 1.f;
      rParms.nSortValue = 0;
    }

    if(!GetCVars()->e_vegetation_debug)
    { // ignore editor colors per instance
      rParms.vColor = Vec3d(CHAR_TO_FLOAT,CHAR_TO_FLOAT,CHAR_TO_FLOAT)*max((float)m_ucBright,32.f);
      rParms.vColor *= m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].fBrightness;
      rParms.vColor.CheckMin(Vec3d(1,1,1));
    }

		// use custom position when render into shadowmap
		if(_EntDrawParams.dwFObjFlags & FOB_RENDER_INTO_SHADOWMAP)
			rParms.vPos = _EntDrawParams.vPos;

		// assign custom template
		rParms.nShaderTemplate = _EntDrawParams.nShaderTemplate;

    // Assign ovverride material.
    rParms.pMaterial = m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].pMaterial;

    // before switching into sprite - orient 3d object to much sprite image
    Matrix44 objMatrix;
    if(fLodSwitchCountDown<1.f && !m_pObjManager->m_nRenderStackLevel && !bUse3DOnly)
    {        
      CStatObj * pBodyLow = pBody;
      if(pBody->m_nLoadedLodsNum && pBody->m_arrpLowLODs[pBody->m_nLoadedLodsNum-1])
        pBodyLow = pBody->m_arrpLowLODs[pBody->m_nLoadedLodsNum-1];

      const float rad2deg = 180.0f/3.14159f;    
      const float far_tex_angle = (360.f/FAR_TEX_COUNT/2);
      float DX = m_vPos.x + pBodyLow->GetCenter().x*m_fScale - vCamPos.x;
      float DY = m_vPos.y + pBodyLow->GetCenter().y*m_fScale - vCamPos.y;
      float DZ = m_vPos.z + pBodyLow->GetCenter().z*m_fScale - vCamPos.z;
      float fRealAngle = rad2deg*cry_atan2f( DX, DY );
      while(fRealAngle<0) fRealAngle+=360;
      while(fRealAngle>=360) fRealAngle-=360;
      assert(fRealAngle>=0 && fRealAngle<360.f);
      float fNewAngle = (float)int(fRealAngle/FAR_TEX_ANGLE+0.5f)*FAR_TEX_ANGLE;
/*      if(fPrevDist0 >= m_fDistance0 && 
        m_ucAngleSlotId!=255 &&
        nLastVisibleFrameID == GetFrameID()-1) // if camera move closer - continue to use last sprite angle
        fNewAngle = (float)int(m_ucAngleSlotId)*FAR_TEX_ANGLE;
      else*/
        m_ucAngleSlotId = int(fRealAngle/FAR_TEX_ANGLE+0.5f);

      float fDiffAngle = fNewAngle - fRealAngle;
      while(fDiffAngle  >  180)
        fDiffAngle-=360;
      while(fDiffAngle <= -180)
        fDiffAngle+=360;

      //        assert(fDiffAngle<=0.5f*FAR_TEX_ANGLE); // can fail if we continue to use sprite angle

      float fAngle1 = fDiffAngle*(1.f-fLodSwitchCountDown);          
      float fAngle2 = cry_atanf(DZ/fDistance)*gf_RADTODEG*(1.f-fLodSwitchCountDown);

      objMatrix.SetIdentity();
      objMatrix=GetTranslationMat(rParms.vPos+pBodyLow->GetCenter()*m_fScale)*objMatrix;

      objMatrix=Matrix44::CreateRotationZYX(-gf_DEGTORAD*Vec3d(0,0,-fRealAngle))*objMatrix; //NOTE: angles in radians and negated 
      objMatrix=Matrix44::CreateRotationZYX(-gf_DEGTORAD*Vec3d(fAngle2,0,0))*objMatrix; //NOTE: angles in radians and negated 
      objMatrix=Matrix44::CreateRotationZYX( gf_DEGTORAD*Vec3d(0,0,-fRealAngle))*objMatrix; //NOTE: angles in radians and negated 


      objMatrix	=	Matrix44::CreateRotationZYX(-gf_DEGTORAD*Vec3d(0,0,fAngle1))*objMatrix; //NOTE: angles in radians and negated 
      objMatrix	=	GetTranslationMat(-pBodyLow->GetCenter()*m_fScale)*objMatrix;
      objMatrix = Matrix33::CreateScale( Vec3d(rParms.fScale,rParms.fScale,rParms.fScale) )*objMatrix;
      rParms.pMatrix = &objMatrix;
      rParms.dwFObjFlags |= FOB_TRANS_MASK;
    }
    else
      rParms.dwFObjFlags |= FOB_TRANS_TRANSLATE | FOB_TRANS_SCALE;

    rParms.pShadowMapCasters = _EntDrawParams.pShadowMapCasters;
		rParms.dwFObjFlags |= (_EntDrawParams.dwFObjFlags & ~FOB_TRANS_MASK);

    // calculate lod and render the object
		int nLod = max(0,(int)(fDistance*GetLodRatioNormilized()/(GetCVars()->e_obj_lod_ratio*GetRenderRadius())));
    pBody->Render( rParms, Vec3(zero), nLod );//int(fDistance/near_far_dist*pBody->m_nLoadedLodsNum*pBody->m_nLoadedLodsNum) );

    /*{ // speed test, render tree directly without shader pipeline - no speed difference
    GetRenderer()->PushMatrix();
    GetRenderer()->TranslateMatrix(rParms.vPos);
    GetRenderer()->EnableAlphaTest(true,0.5f);
    GetRenderer()->SetCullMode(R_CULL_NONE);
    CLeafBuffer *lb = pBody->GetLeafBuffer();
    for (int i=0; i<lb->m_pMats->Count(); i++)
    {
    CMatInfo * pMatInfo = &(*lb->m_pMats)[i];
    IShader * e = (*lb->m_pMats)[i].pShader;
    GetRenderer()->SetTexture(e->GetTexId());
    GetRenderer()->DrawBuffer(lb->m_pVertexBuffer, &lb->m_Indices[pMatInfnFirstIndexId], pMatInfnNumIndices, 0);
    }

    GetRenderer()->PopMatrix();
    }*/

    // add object occlusion volume to the coverage buffer
    if( fDistance<COVERAGEBUFFER_OCCLUDERS_MAX_DISTANCE && 
      GetCVars()->e_cbuffer && pBody->GetLoadedTrisCount()==12 )
    {
      Vec3d vTopMax = m_vPos + pBody->m_vBoxMax*m_fScale; 
      Vec3d vTopMin = m_vPos + pBody->m_vBoxMin*m_fScale; 
      m_pObjManager->m_pCoverageBuffer->AddBBox(vTopMin,vTopMax,vCamPos-m_vPos);
    }
  }
  else // process sprite
  if( !bUse3DOnly && fDistance > near_far_dist)
  { // add to the list of far objects
    if(m_pObjManager->m_lstFarObjects[m_pObjManager->m_nRenderStackLevel].Count()<16384)
    {
      if(!m_pObjManager->m_nRenderStackLevel)
      {
        if(fLodSwitchCountDown<1.f)
        {
          float DZ = m_vPos.z + pBody->GetCenter().z*m_fScale - vCamPos.z;
          float fAngle2 = -cry_atanf(DZ/fDistance)*(1.f-fLodSwitchCountDown);
          fAngle2 = max(0,min(1,(fAngle2+0.5f)));
          m_ucLodAngle = uchar(fAngle2*255.f);
        }
        else
          m_ucLodAngle = 127;
      }

      m_pObjManager->m_lstFarObjects[m_pObjManager->m_nRenderStackLevel].Add(this);
    }
  }

  return true;
}

CObjManager * CStatObjInst::m_pObjManager = 0;
Vec3d CStatObjInst::m_vAngles(0,0,0);

void CStatObjInst::GetRenderBBox(Vec3d & vBoxMin,Vec3d & vBoxMax)
{
  CStatObj * pBody = m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].GetStatObj();
  if(pBody)
  {
    vBoxMin = pBody->m_vBoxMin*m_fScale+m_vPos;
    vBoxMax = pBody->m_vBoxMax*m_fScale+m_vPos;
  }
  else
  {
    vBoxMin = m_vPos-Vec3d(1,1,1);
    vBoxMax = m_vPos+Vec3d(1,1,1);
  }
}

float CStatObjInst::GetRenderRadius(void) const
{
  CStatObj * pBody = m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].GetStatObj();
  if(pBody)
    return pBody->GetRadius()*m_fScale;

  assert(0);
  return 0;
}

IStatObj * CStatObjInst::GetEntityStatObj( unsigned int nSlot, Matrix44 * pMatrix, bool bReturnOnlyVisible ) 
{ 
	if(pMatrix)
	{
		Matrix33diag diag	=	Vec3d( GetScale(),GetScale(),GetScale()	);		//use diag-matrix for scaling
		Matrix34 rt34			=	Matrix34::CreateRotationXYZ(gf_DEGTORAD*GetAngles(1), GetPos(true) );	//set scaling and translation in one function call
		Matrix44 mat1			=	rt34*diag;	//optimised concatenation: m34*diag
		*pMatrix = GetTransposed44(mat1); //TODO: remove this after E3 and use Matrix34 instead of Matrix44
	}

  return m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].GetStatObj(); 
}

CStatObj* CStatObjInst::GetStatObj() const
{
	return m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].GetStatObj();
}

float CStatObjInst::GetMaxViewDist()
{
  if (GetStatObj())
		return max(GetCVars()->e_obj_min_view_dist, GetStatObj()->GetRadius()*m_fScale*GetCVars()->e_obj_view_dist_ratio*GetViewDistRatioNormilized());
	
	return 0;
}

float CStatObjInst::GetViewDistRatioNormilized()
{
	assert(GetStatObj());
	return m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].fMaxViewDistRatio;
}

void CStatObjInst::Physicalize( bool bInstant )
{
	assert(m_nObjectTypeID>=0);
	CStatObj * pBody  = m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].GetStatObj();
	if(!pBody || !pBody->IsPhysicsExist())
		return;

	bool bHideability = m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].bHideability;
	bool bPhysNonColl = m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].bPhysNonColl;

	//////////////////////////////////////////////////////////////////////////
	// Not create instance if no physical geometry.
	if(!pBody->m_arrPhysGeomInfo[0])
		if(!(pBody->m_arrPhysGeomInfo[1] && bHideability))
			if(!(pBody->m_arrPhysGeomInfo[2] && bPhysNonColl))
				return;
	//////////////////////////////////////////////////////////////////////////
	
	if (!GetCVars()->e_on_demand_physics)
		bInstant = true;

  /*
  if( m_pObjManager->m_lstStaticTypes[nObjectTypeID].fSpriteDistRatio == 0 )
    return; // do not phys 'sprite only' trees

  CStatObj * pBody		= m_pObjManager->m_lstStaticTypes[nObjectTypeID].GetStatObj();
  bool bHideability = m_pObjManager->m_lstStaticTypes[nObjectTypeID].bHideability;

  if(pBody && pBody->GetRadius()*m_fScale < 1.f && pBody->GetRadius())
    return;
  */
  if (!bInstant)
  {
    pe_status_placeholder spc;
    if (m_pPhysEnt && m_pPhysEnt->GetStatus(&spc) && spc.pFullEntity)
      GetSystem()->GetIPhysicalWorld()->DestroyPhysicalEntity(spc.pFullEntity);

    pe_params_bbox pbb;
    pbb.BBox[0] = m_vWSBoxMin;
    pbb.BBox[1] = m_vWSBoxMax;
    pe_params_foreign_data pfd;
    pfd.pForeignData = this;
    pfd.iForeignData = 1;

    if (!m_pPhysEnt)
      m_pPhysEnt = GetSystem()->GetIPhysicalWorld()->CreatePhysicalPlaceholder(PE_STATIC,&pbb);
    else
      m_pPhysEnt->SetParams(&pbb);
    m_pPhysEnt->SetParams(&pfd);
    return;
  }

  pBody->CheckLoaded();

  // create new
  pe_params_pos par_pos;
  if (!m_pPhysEnt)
  {
    m_pPhysEnt = GetSystem()->GetIPhysicalWorld()->CreatePhysicalEntity(PE_STATIC,NULL,this,1);        
    if(!m_pPhysEnt)
      return;
  }
  else if (bInstant) // this is on-demand creation, so entity pointer is automatically put into the placeholder
    GetSystem()->GetIPhysicalWorld()->CreatePhysicalEntity(PE_STATIC,5.0f);

	pe_action_remove_all_parts remove_all;
	m_pPhysEnt->Action(&remove_all);

  pe_geomparams params;	  
	// collidable
  if(pBody->m_arrPhysGeomInfo[0])
    m_pPhysEnt->AddGeometry(pBody->m_arrPhysGeomInfo[0], &params);

  params.flags = geom_colltype_ray;				          

	// obstruct
  if(pBody->m_arrPhysGeomInfo[1] && bHideability)
    m_pPhysEnt->AddGeometry(pBody->m_arrPhysGeomInfo[1], &params);

	// leaves
	if(pBody->m_arrPhysGeomInfo[2] && bPhysNonColl)
		m_pPhysEnt->AddGeometry(pBody->m_arrPhysGeomInfo[2], &params);

  if(bHideability)
  {
    pe_params_foreign_data  foreignData;
    m_pPhysEnt->GetParams(&foreignData);
    foreignData.iForeignFlags |= PFF_HIDABLE;
    m_pPhysEnt->SetParams(&foreignData);
  }

  par_pos.pos = m_vPos;
  par_pos.q.SetRotationXYZ( Vec3(0.f,0.f,0.f) );
	par_pos.scale = m_fScale;
  m_pPhysEnt->SetParams(&par_pos);
}

CStatObjInst::~CStatObjInst()
{
  Dephysicalize( );
//  Get3DEngine()->UnRegisterEntity(this);
	Get3DEngine()->FreeEntityRenderState(this);
	
	if(GetEntityRS() && GetEntityRS()->pShadowMapInfo)
		GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer = 0; // here was a pointer structure allocated in CStatObj
}

void CStatObjInst::Dematerialize( )
{
}

void CStatObjInst::Dephysicalize( )
{
	// delete old physics
	if(m_pPhysEnt)
    GetSystem()->GetIPhysicalWorld()->DestroyPhysicalEntity(m_pPhysEnt);
  m_pPhysEnt=0;
}

int CStatObjInst::GetMemoryUsage()
{
  return sizeof(*this) + (m_pEntityRenderState ? sizeof(*m_pEntityRenderState) : 0);
}

unsigned int CStatObjInst::GetRndFlags() 
{ 
	assert(m_nObjectTypeID>=0 && m_nObjectTypeID<m_pObjManager->m_lstStaticTypes.Count());
	return m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].m_dwRndFlags;
}

void CStatObjInst::SetRndFlags(unsigned int dwFlags)
{
	assert(m_nObjectTypeID>=0 && m_nObjectTypeID<m_pObjManager->m_lstStaticTypes.Count());
	m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].m_dwRndFlags = dwFlags;
}

void CStatObjInst::SetRndFlags(unsigned int dwFlags, bool bEnable) 
{ 
	assert(m_nObjectTypeID>=0 && m_nObjectTypeID<m_pObjManager->m_lstStaticTypes.Count());

	if(bEnable) 
		m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].m_dwRndFlags |= dwFlags; 
	else 
		m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].m_dwRndFlags &= ~dwFlags;
}

/*
ShadowMapFrustum * CStatObjInst::GetShadowMapFrustum()
{
	if(m_nObjectTypeID<0 && m_nObjectTypeID>=m_pObjManager->m_lstStaticTypes.Count())
		return 0;

	CStatObj * pBody = m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].GetStatObj();

	if(pBody && pBody->m_pSMLSource && pBody->m_pSMLSource->m_LightFrustums.Count())
		return &(pBody->m_pSMLSource->m_LightFrustums[0]);

	return 0;
}
*/
struct ShadowMapLightSource * CStatObjInst::GetShadowMapFrustumContainer()
{
	if(m_nObjectTypeID<0 && m_nObjectTypeID >= m_pObjManager->m_lstStaticTypes.Count())
		return 0;

	CStatObj * pBody = m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].GetStatObj();

	if(pBody)
		return pBody->m_pSMLSource;

	return 0;
}

list2<ShadowMapLightSourceInstance> * CStatObjInst::GetShadowMapCasters()
{
	if(!m_pEntityRenderState || !m_pEntityRenderState->pShadowMapInfo)
		return 0;

	if(!m_pEntityRenderState->pShadowMapInfo->pShadowMapCasters || !m_pEntityRenderState->pShadowMapInfo->pShadowMapCasters->Count())
	{
		if(m_nObjectTypeID<0 && m_nObjectTypeID >= m_pObjManager->m_lstStaticTypes.Count())
			return 0;

		CStatObj * pBody = m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].GetStatObj();

		if(!m_pEntityRenderState->pShadowMapInfo->pShadowMapCasters)
			m_pEntityRenderState->pShadowMapInfo->pShadowMapCasters = new list2<ShadowMapLightSourceInstance>;
		else
			m_pEntityRenderState->pShadowMapInfo->pShadowMapCasters->Clear();

		ShadowMapLightSourceInstance LightSourceInfo;
		LightSourceInfo.m_pLS               = pBody->m_pSMLSource;
		LightSourceInfo.m_vProjTranslation  = m_vPos;
		LightSourceInfo.m_fProjScale        = m_fScale;
		LightSourceInfo.m_fDistance         = 0;
		m_pEntityRenderState->pShadowMapInfo->pShadowMapCasters->Add(LightSourceInfo);
	}

	// update shadow every frame for nice shadow animations
	if(GetCVars()->e_vegetation_update_shadow_every_frame && !m_nRenderStackLevel)
	if(m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].bUpdateShadowEveryFrame)
	if(float fMaxShadowDist = GetRenderRadius()*GetCVars()->e_shadow_maps_view_dist_ratio)
	{
		float fDistFade = m_arrfDistance[m_nRenderStackLevel]/fMaxShadowDist;
		float fBending = m_fFinalBending * (1.f-fDistFade);
		if(fDistFade<0.3333f)
		{
			CStatObj * pBody = m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].GetStatObj();
			if(pBody && pBody->m_pSMLSource && pBody->m_pSMLSource->GetShadowMapFrustum())
			{
				pBody->m_pSMLSource->GetShadowMapFrustum()->bUpdateRequested = true;
				if(fBending > pBody->m_pSMLSource->GetShadowMapFrustum()->m_fBending)
					pBody->m_pSMLSource->GetShadowMapFrustum()->m_fBending = fBending;
			}
		}
	}

	return m_pEntityRenderState->pShadowMapInfo->pShadowMapCasters;
}

void CStatObjInst::PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime)
{
	if(!GetEntityStatObj(0) || !GetEntityStatObj(0)->GetLeafBuffer())
		return;

	float fDist = fPrevPortalDistance + m_vPos.GetDistance(vPrevPortalPos);

	float fMaxViewDist = GetMaxViewDist();
	if(fDist<fMaxViewDist && fDist<GetViewCamera().GetZMax())
		GetEntityStatObj(0)->PreloadResources(fDist,fTime,0);
}

const char *CStatObjInst::GetName() const
{
	return (m_nObjectTypeID>=0 && m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].GetStatObj() )? 
		m_pObjManager->m_lstStaticTypes[m_nObjectTypeID].GetStatObj()->GetFileName() : "StatObjNotSet";
}
