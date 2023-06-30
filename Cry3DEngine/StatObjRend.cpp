////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjrend.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: prepare and add render element into renderer
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "../RenderDll/Common/shadow_renderer.h"
#include "LMCompStructures.h"

#include "MeshIdx.h"
#include "visareas.h"

//////////////////////////////////////////////////////////////////////
bool CStatObj::SetShaderTemplate(int nTemplate, const char *TemplName, const char *ShaderName, bool bOnlyRegister, int * pnNewTemplateId)
{
	for(int i=0; i<MAX_STATOBJ_LODS_NUM; i++)
		if(m_arrpLowLODs[i])
			m_arrpLowLODs[i]->SetShaderTemplate(nTemplate, TemplName, ShaderName, bOnlyRegister);

  CLeafBuffer *lb = m_pLeafBuffer;
  if (!lb || (lb->m_pMats->Count() != m_lstShaderTemplates.Num()))
    return false;
  if (!ShaderName)
  {
    for (int i=0; i<lb->m_pMats->Count(); i++)
    {
      IShader * e = (*lb->m_pMats)[i].shaderItem.m_pShader;
      SRenderShaderResources *sr = (*lb->m_pMats)[i].shaderItem.m_pShaderResources;
      
      if (e && (*lb->m_pMats)[i].pRE && (*lb->m_pMats)[i].nNumIndices)
      {
        if(nTemplate < EFT_USER_FIRST && nTemplate >= 0)
          e->AddTemplate(sr, nTemplate, NULL, false);
        else
        if (TemplName && TemplName[0])
        {
          if (nTemplate <= EFT_USER_FIRST)
            nTemplate = EFT_USER_FIRST + 1;
          e->AddTemplate(sr, nTemplate, TemplName, false);
        }
        if (!bOnlyRegister)
          m_lstShaderTemplates[i] = nTemplate;
      }
    }
		
		if(pnNewTemplateId)
			*pnNewTemplateId = nTemplate;

    return true;
  }
  bool bRes = false;
  for (int i=0; i<m_lstShaderTemplates.Num(); i++)
  {
    if (!lb->m_pMats->Get(i)->shaderItem.m_pShader)
      continue;
    if (!stricmp(ShaderName, lb->m_pMats->Get(i)->shaderItem.m_pShader->GetName()))
    {
      SRenderShaderResources *sr = (*lb->m_pMats)[i].shaderItem.m_pShaderResources;
      bRes = true;
      if(nTemplate < EFT_USER_FIRST && nTemplate >= 0)
        lb->m_pMats->Get(i)->shaderItem.m_pShader->AddTemplate(sr, nTemplate);
      else
      if (TemplName && TemplName[0])
        lb->m_pMats->Get(i)->shaderItem.m_pShader->AddTemplate(sr, nTemplate, TemplName);
      m_lstShaderTemplates[i] = nTemplate;
    }
  }

	if(bRes && pnNewTemplateId)
		*pnNewTemplateId = nTemplate;

  return bRes;
}

void CStatObj::SetShaderFloat(const char *Name, float Val)
{
	int i;
	for(i=0; i<MAX_STATOBJ_LODS_NUM; i++)
		if(m_arrpLowLODs[i])
			m_arrpLowLODs[i]->SetShaderFloat(Name, Val);

  char name[128];

  strcpy(name, Name);
  strlwr(name);
  for (i=0; i<m_ShaderParams.Num(); i++)
  {
    if (!strcmp(name, m_ShaderParams[i].m_Name))
      break;
  }
  if (i == m_ShaderParams.Num())
  {
    SShaderParam pr;
    strncpy(pr.m_Name, name, 32);
    m_ShaderParams.AddElem(pr);
  }
  m_ShaderParams[i].m_Type = eType_FLOAT;
  m_ShaderParams[i].m_Value.m_Float = Val;
}

void CStatObj::SetColor(const char *Name, float fR, float fG, float fB, float fA)
{
}

void CStatObj::SetupBending(CCObject * pObj, float fBending)
{
	pObj->m_fBending = fBending;

	Vec3d vObjPos = pObj->GetTranslation();

	SWaveForm2 *pWF[2];
	pObj->AddWaves(pWF);
	SWaveForm2 *wf = pWF[0];
	wf->m_Level  = 0.000f; // between 0.001 and 0.1
	wf->m_Freq   = 1.0f/m_fRadiusVert/8.0f+0.2f; // between 0.001 and 0.1
	wf->m_Phase  = vObjPos.x/8.0f;
	wf->m_Amp = 0.002f;

	wf = pWF[1];
	wf->m_Level  = 0.000f; // between 0.001 and 0.1
	wf->m_Freq   = 1.0f/m_fRadiusVert/7.0f+0.2f; // between 0.001 and 0.1
	wf->m_Phase  = vObjPos.y/8.0f;
	wf->m_Amp = 0.002f;

	pObj->m_ObjFlags |= FOB_BENDED;
}

//////////////////////////////////////////////////////////////////////
void CStatObj::Render(const SRendParams & rParams, const Vec3& t, int nLodLevel)
{
  IRenderer * pRend = GetRenderer();

	m_nLastRendFrameId = GetFrameID();
  
////////////////////////////////////////////////////////////////////////////////////////////////////
// Process LODs
////////////////////////////////////////////////////////////////////////////////////////////////////

	if (nLodLevel >= m_nLoadedLodsNum)
    nLodLevel = m_nLoadedLodsNum-1;

  if (nLodLevel && m_arrpLowLODs[nLodLevel] && nLodLevel<m_nLoadedLodsNum	&& GetCVars()->e_cgf_load_lods)
  {
    m_arrpLowLODs[nLodLevel]->Render(rParams,Vec3(zero),0);
    return;
  }

////////////////////////////////////////////////////////////////////////////////////////////////////
// Load object if not loaded
////////////////////////////////////////////////////////////////////////////////////////////////////

	if(m_bUseStreaming && GetCVars()->e_stream_cgf)
		StreamCCGF(false);

//  if(m_bUseStreaming && m_fStreamingTimePerFrame<CGF_STREAMING_MAX_TIME_PER_FRAME)
  //  if(GetCVars()->e_stream_cgf && GetCVars()->e_stream_for_visuals)
    //  CheckLoaded();

  if(!m_pLeafBuffer || !m_nLoadedTrisCount)
    return; // object not loaded yet

////////////////////////////////////////////////////////////////////////////////////////////////////
// Specifiy transformation
////////////////////////////////////////////////////////////////////////////////////////////////////

  CCObject * pObj;
  pObj = pRend->EF_GetObject(true, -1);

	pObj->m_fDistanceToCam = rParams.fSQDistance;

  if (!rParams.pMatrix)
    mathCalcMatrix(pObj->m_Matrix, rParams.vPos, rParams.vAngles, Vec3d(rParams.fScale,rParams.fScale,rParams.fScale), Cry3DEngineBase::m_CpuFlags);
  else
    pObj->m_Matrix = *rParams.pMatrix;

  pObj->m_SortId = rParams.fCustomSortOffset;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Set flags
////////////////////////////////////////////////////////////////////////////////////////////////////

  pObj->m_ObjFlags |= rParams.dwFObjFlags;
  pObj->m_fHeatFactor = rParams.fHeatAmount;

  pObj->m_AmbColor = rParams.vAmbientColor;
	assert(pObj->m_AmbColor.x>=0 && pObj->m_AmbColor.x<=1.f);
	assert(pObj->m_AmbColor.y>=0 && pObj->m_AmbColor.y<=1.f);
	assert(pObj->m_AmbColor.z>=0 && pObj->m_AmbColor.z<=1.f);

  if (GetCVars()->e_shadow_maps)
    pObj->m_pShadowCasters = rParams.pShadowMapCasters;
  else
    pObj->m_pShadowCasters=0;

//  if (rParams.dwFlags & RPF_DRAWNEAR)
  //  pObj->m_ObjFlags |= FOB_NEAREST;

  if (pObj->m_pShadowCasters)
    pObj->m_ObjFlags |= FOB_INSHADOW;
  
  pObj->m_Color = CFColor(
    rParams.vColor.x,
    rParams.vColor.y,
    rParams.vColor.z,
    rParams.fAlpha);

	if (rParams.pShaderParams && rParams.pShaderParams->Num())
		pObj->m_ShaderParams = rParams.pShaderParams;
  else
  if (m_ShaderParams.Num())
    pObj->m_ShaderParams = &m_ShaderParams;
  
////////////////////////////////////////////////////////////////////////////////////////////////////
// Process bending
////////////////////////////////////////////////////////////////////////////////////////////////////

	if (rParams.fBending)
		SetupBending(pObj, rParams.fBending);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Add LOCAL lsources
////////////////////////////////////////////////////////////////////////////////////////////////////

	int dwDynMaskLocal = 0;
	for(int i=0; i<m_lstLSources.Count(); i++)
		if(m_lstLSources[i].m_Flags & DLF_LOCAL)
		if(	((m_lstLSources[i].m_Flags & DLF_HEATSOURCE)	&&	pRend->EF_GetHeatVision()) ||
				((m_lstLSources[i].m_Flags & DLF_LIGHTSOURCE)&& !pRend->EF_GetHeatVision()))
		if(rParams.fDistance < 256)
		{
			if (rParams.pMatrix)
			{
				m_lstLSources[i].m_Origin = 
					rParams.pMatrix->TransformPointOLD(m_lstLSources[i].m_vObjectSpacePos);
			}
			else
			{
				
				//Matrix objMat; 
				//objMat.Identity();
				//objMat=GetTranslationMat(rParams.vPos)*objMat;
				//objMat=GetRotationZYX44(-gf_DEGTORAD*rParams.vAngles)*objMat; //NOTE: angles in radians and negated 
				//objMat=GetScale33( Vec3d(rParams.fScale,rParams.fScale,rParams.fScale) )*objMat;

				//OPTIMISED_BY_IVO  
				Matrix33diag diag	=	Vec3d(rParams.fScale,rParams.fScale,rParams.fScale);		//use diag-matrix for scaling
				Matrix34 rt34			=	Matrix34::CreateRotationXYZ(gf_DEGTORAD*rParams.vAngles, rParams.vPos );	//set scaling and translation in one function call
				Matrix44 objMat		=	rt34*diag;			//optimised concatenation: m34*diag
				objMat	=	GetTransposed44(objMat);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44

				m_lstLSources[i].m_Origin =	objMat.TransformPointOLD(m_lstLSources[i].m_vObjectSpacePos);
			}

			pRend->EF_ADDDlight (&m_lstLSources[i]);
			if(m_lstLSources[i].m_Id>=0)
				dwDynMaskLocal |= (1<<m_lstLSources[i].m_Id);
		}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Add helpers
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  int i;
  for (i=0; i<m_lstHelpers.Count(); i++)
  {
    HelperInfo *pHI = &m_lstHelpers[i];
    if (pHI->m_pShader)
    {
      CCObject * pObject = pRend->EF_GetObject(true);
      Matrix matr;
      if (pObj->m_ObjFlags & FOB_USEMATRIX)
        matr = pObj->m_Matrix;
      else
      {
        matr.Identity();
        matr.Translate(pObj->m_Trans);
        matr.Rotate(pObj->m_Angs);
        if (pObj->m_Scale != 1.0f)
          matr.ScaleMatrix(pObj->m_Scale, pObj->m_Scale, pObj->m_Scale);
      }
      pObject->m_Trans = matr.TransformPoint(pHI->vPos);
      pObject->m_Color = Col_White;
      pObject->m_Angs = Vec3d(0,0,0);
      for (int nr=0; nr<pHI->m_pShader->GetREs()->Num(); nr++)
      {
        pRend->EF_AddEf(0, pHI->m_pShader->GetREs()->Get(nr), pHI->m_pShader, pObject, -1);
      }
    }
  }
*/
  pObj->m_DynLMMask = rParams.nDLightMask;
	pObj->m_DynLMMask |= dwDynMaskLocal;

#ifdef _DEBUG
	int nCount = 0;
	for(int i=0; i<32; i++)
	{
		if(pObj->m_DynLMMask & (1<<i))
			nCount++;
	}
	if(nCount>16)
	{
#if !defined(LINUX)
    Warning( 0,0,"Warning: CStatObj::Render: no more than 16 lsources can be requested");
#endif
	}
#endif

	if(rParams.pLightMapInfo)
	{
		// set object lmaps and texture coordinates
		pObj->m_pLMTCBufferO = rParams.pLMTCBuffer;
		pObj->m_nLMId		= rParams.pLightMapInfo->GetColorLerpTex();
		pObj->m_nLMDirId	= rParams.pLightMapInfo->GetDomDirectionTex();
		pObj->m_nHDRLMId	= rParams.pLightMapInfo->GetHDRColorLerpTex();
    pObj->m_nOcclId	= rParams.pLightMapInfo->GetOcclTex();
    *(DWORD*)pObj->m_OcclLights = *(DWORD*)rParams.arrOcclusionLightIds;
	}

/*	if(!m_nRenderStackLevel)
	{
		pObj->m_nScissorX1 = rParams.nScissorX1;
		pObj->m_nScissorY1 = rParams.nScissorY1;
		pObj->m_nScissorX2 = rParams.nScissorX2;
		pObj->m_nScissorY2 = rParams.nScissorY2;
	}*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// Add render elements
////////////////////////////////////////////////////////////////////////////////////////////////////

	IMatInfo * pMaterial = NULL;
	if (GetCVars()->e_materials && !m_bDefaultObject)
		pMaterial = rParams.pMaterial;

	bool bNotCurArea = false;

	if(GetCVars()->e_overlay_geometry >= 2)
	{
		bNotCurArea = (!rParams.pCaller || !GetVisAreaManager() ||
			((CVisArea *)rParams.pCaller->GetEntityVisArea() && !((CVisArea *)rParams.pCaller->GetEntityVisArea())->
			FindVisArea((CVisArea *)GetVisAreaManager()->m_pCurArea,1, true)));

		SetShaderFloat("offset", 0);
	}
	else if(GetCVars()->e_overlay_geometry < 2 )
		m_ShaderParams.SetNum(0);

	if(GetCVars()->e_debug_lights!=1.f)
		m_pLeafBuffer->Render(rParams,pObj,m_lstShaderTemplates,GetCVars()->e_overlay_geometry,bNotCurArea,pMaterial, false);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Print debug info
////////////////////////////////////////////////////////////////////////////////////////////////////

	if (GetCVars()->e_vegetation_debug)
		RenderDebugInfo(rParams, pObj);

	if(GetCVars()->e_debug_lights)
		if(!(rParams.dwFObjFlags & FOB_LIGHTPASS || rParams.dwFObjFlags & FOB_FOGPASS))
			m_pLeafBuffer->RenderDebugLightPass(pObj->m_Matrix,	pObj->m_DynLMMask, GetCVars()->e_debug_lights);
}
/*
void CStatObj::RenderLeafBuffer(const SRendParams & rParams, CCObject * pObj)
{
	//	static int nAll=0,nUsed=0;

	int nSortValue = rParams.bDrawNear ? eS_Nearest : rParams.nSortValue;

	for (int i=0; i<m_pLeafBuffer->m_pMats->Count(); i++)
	{
		CMatInfo * pMat = m_pLeafBuffer->m_pMats->Get(i);
		CRendElement * pREOcLeaf = pMat->pRE;

		//		nAll++;

		// Override default material
		if (rParams.pMaterial && GetCVars()->e_materials && !m_bDefaultObject)
		{
			int nMatId = pMat->m_nCGFMaterialID;
			if(nMatId<0)
			{
				//				assert(0);
				continue;
			}

			// Assume that the root material is the first material, sub materials start from index 1.
			if (nMatId == 0)
				pMat = (CMatInfo*)rParams.pMaterial;
			else if (nMatId-1 < rParams.pMaterial->GetSubMtlCount())
			{
				pMat = (CMatInfo*)rParams.pMaterial->GetSubMtl(nMatId-1);
			}
		}

		IShader					* e  = pMat->shaderItem.m_pShader;
		SRenderShaderResources* sr = pMat->shaderItem.m_pShaderResources;

		//		assert(e && pREOcLeaf);

		if (pREOcLeaf && e)
		{
			//			nUsed++;

			int nTempl = rParams.nShaderTemplate;
			if (nTempl == -2 && i<m_lstShaderTemplates.Num())
				nTempl = m_lstShaderTemplates[i];

			if(rParams.pfCustomData)
			{
				assert(!pREOcLeaf->m_CustomData);
				pREOcLeaf->m_CustomData = rParams.pfCustomData;
			}

			if (rParams.nShaderTemplate>0)
				e->AddTemplate(sr, (int)rParams.nShaderTemplate,NULL);

			if(rParams.dwFObjFlags & FOB_FOGPASS)
				if(e->GetTemplate(-1)->GetFlags() & EF_OVERLAY)
					continue; // skip overlays during fog pass - it will be fogged by base geometry fog pass

			bool bTransparent = pMat->shaderItem.IsTransparent();
			IShader * pShader = rParams.pStateShader;

			if(bTransparent)
			{	
				if((rParams.dwFlags & RPF_LIGHTPASS) || (rParams.dwFObjFlags & FOB_FOGPASS) || (nTempl == EFT_INVLIGHT))
					continue;

				if(nSortValue==eS_FogShader)
					nSortValue=eS_FogShader_Trans;

				if(!GetCVars()->e_overlay_geometry)
					if(e->GetTemplate(-1)->GetFlags() & EF_OVERLAY)
						continue;

				if(GetCVars()->e_overlay_geometry >= 2 && e->GetTemplate(-1)->GetFlags() & EF_OVERLAY)
				{
					if(!rParams.pCaller || !GetVisAreaManager() ||
						((CVisArea *)rParams.pCaller->GetEntityVisArea() && !((CVisArea *)rParams.pCaller->GetEntityVisArea())->
						FindVisArea((CVisArea *)GetVisAreaManager()->m_pCurArea,1, true)))
						continue;

					if(GetCVars()->e_overlay_geometry == 2)
					{
						if(int(GetTimer()->GetCurrTime()*5)&1)
							pShader = GetRenderer()->EF_LoadShader("NoZTestState", eSH_World, EF_SYSTEM	);
					}
					else
						pShader = GetRenderer()->EF_LoadShader("ZTestGreaterState", eSH_World, EF_SYSTEM	);

					SetShaderFloat("offset", 0);
				}
				else if(GetCVars()->e_overlay_geometry < 2 )
					m_ShaderParams.SetNum(0);
			}

			if( rParams.bRenderIntoShadowMap && (pMat->m_Flags & MIF_NOCASTSHADOWS) )
				continue;

			if( nSortValue == EFSLIST_STENCIL && bTransparent )
				GetRenderer()->EF_AddEf(rParams.nFogVolumeID, pREOcLeaf, e, sr, pObj, nTempl, pShader, 0);
			else
				GetRenderer()->EF_AddEf(rParams.nFogVolumeID, pREOcLeaf, e, sr, pObj, nTempl, pShader, nSortValue);
		}
	} //i
}*/

void CStatObj::RenderDebugInfo(const SRendParams & rParams, const CCObject * pObj)
{
  IRenderer * pRend = GetRenderer();

	// bbox
  pRend->SetMaterialColor(0,1,1,0.5f);
  pRend->Draw3dBBox(m_vBoxMin*rParams.fScale+rParams.vPos,
										m_vBoxMax*rParams.fScale+rParams.vPos);

	// scaled bbox (frustum culling check)
  if(GetCVars()->e_vegetation_debug==2)
  {
    pRend->SetMaterialColor(1,0,1,0.5f);
    pRend->Draw3dBBox(m_vBoxMin*rParams.fScale*1.5f+rParams.vPos,
											m_vBoxMax*rParams.fScale*1.5f+rParams.vPos);
  }

	{ // cgf's name and tris num
		char szName[512];
		strncpy(szName,m_szFileName,sizeof(szName));
		while(strstr(szName,"\\"))
			strcpy(szName, szName+1);
		float color[4] = {0,1,1,1};
		pRend->DrawLabelEx(rParams.vPos/*+GetCenter()*rParams.fScale*/,
      0.75f,color,false,true,"%s(%d tris, %d mats)", szName, m_nLoadedTrisCount, 
      (m_pLeafBuffer && m_pLeafBuffer->m_pMats) ? m_pLeafBuffer->m_pMats->Count() : 0);  
	}

	// helpers
	for( int i=0; i<m_lstHelpers.Count(); i++)
	{
		StatHelperInfo *pHI = &m_lstHelpers[i];

		// make object matrix
		Matrix44 tMat;
  	tMat = pObj->m_Matrix;

		// draw axes
		DrawMatrix(tMat);

		tMat = tMat*pHI->tMat;

		Vec3d vTrans = tMat.GetTranslationOLD();

		float color[4] = {1,1,0,1};

		// text
		pRend->DrawLabelEx(vTrans, 0.75f, color, false, true, "%s", pHI->sName);  

		// draw axes
		DrawMatrix(tMat);
	}
}

void CStatObj::DrawMatrix(const Matrix44 & tMat)
{
  IRenderer * pRend = GetRenderer();

	Vec3d vTrans = tMat.GetTranslationOLD();

	float color[] = {1,1,1,1};

	//CHANGED_BY_IVO
	pRend->SetMaterialColor(1,0,0,0);
	//pRend->Draw3dBBox( vTrans, vTrans+0.05f*Vec3d(tMat.m_values[0]), true );
	//pRend->DrawLabelEx(vTrans+0.05f*Vec3d(tMat.m_values[0]), 0.75f,color,false,true,"x");  
	pRend->Draw3dBBox( vTrans, vTrans+0.05f*tMat.GetOrtX(), true );
	        pRend->DrawLabelEx(vTrans+0.05f*tMat.GetOrtX(), 0.75f,color,false,true,"x");  

	pRend->SetMaterialColor(0,1,0,0);
	//pRend->Draw3dBBox( vTrans, vTrans+0.05f*Vec3d(tMat.m_values[1]), true );
	//pRend->DrawLabelEx(vTrans+0.05f*Vec3d(tMat.m_values[1]), 0.75f,color,false,true,"y");  
	pRend->Draw3dBBox( vTrans, vTrans+0.05f*tMat.GetOrtY(), true );
	        pRend->DrawLabelEx(vTrans+0.05f*tMat.GetOrtY(), 0.75f,color,false,true,"y");  

	pRend->SetMaterialColor(0,0,1,0);
	//pRend->Draw3dBBox( vTrans, vTrans+0.05f*Vec3d(tMat.m_values[2]), true );
	//pRend->DrawLabelEx(vTrans+0.05f*Vec3d(tMat.m_values[2]), 0.75f,color,false,true,"z");  
	pRend->Draw3dBBox( vTrans, vTrans+0.05f*tMat.GetOrtZ(), true );
	        pRend->DrawLabelEx(vTrans+0.05f*tMat.GetOrtZ(), 0.75f,color,false,true,"z");  
}

const CDLight * CStatObj::GetLightSources(int nId)
{
  if(nId>=0 && nId<m_lstLSources.Count())
		return &m_lstLSources[nId];

	return 0;
}

void CStatObj::SpawnParticles( ParticleParams & SpawnParticleParams, const Matrix44 & matWorldSpace, bool bOnlyUpLookingFaces )
{	
	// select random vertex
	int n=rand() % m_pLeafBuffer->m_SecVertCount;
//	int n = int(rnd()*m_pLeafBuffer->m_SecVertCount);
//	while(n>=m_pLeafBuffer->m_SecVertCount)
//		n = int(rnd()*m_pLeafBuffer->m_SecVertCount);

	// access pos and normal
	int nPosStride = 0, nNormStride = 0;
	byte * arrPos = m_pLeafBuffer->GetPosPtr(nPosStride);
	byte * arrNor = m_pLeafBuffer->GetNormalPtr(nNormStride);
	Vec3d * pPos = (Vec3d*)&arrPos[n*nPosStride];	
	Vec3d * pNorm = (Vec3d*)&arrNor[n*nNormStride];	

	// transform
	Vec3d vWSPos = matWorldSpace.TransformPointOLD(*pPos);

	Vec3d vWSNorm = matWorldSpace.TransformVectorOLD(*pNorm);
	//Vec3d vWSNorm = GetTransposed44(matWorldSpace)*(*pNorm);

	// spawn
	if(!bOnlyUpLookingFaces || vWSNorm.z>0.25f)
	{
		Get3DEngine()->SpawnParticles(SpawnParticleParams);
		SpawnParticleParams.vPosition = vWSPos;
		SpawnParticleParams.vDirection = (SpawnParticleParams.vDirection+vWSNorm)*0.5f;
	}
}