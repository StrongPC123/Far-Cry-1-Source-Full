#include "RenderPCH.h"
#include "i3dengine.h"
#include "cryheaders.h"

void CLeafBuffer::AddRE(CCObject *obj, IShader *ef, int nNumSort, IShader * pStateEff)
{
  if(!m_NumIndices || !m_pMats->Count())
    return;

	int nGlobalShaderTemplateId = gRenDev->GetGlobalShaderTemplateId();

  IShader *e;
  SRenderShaderResources *sr;

  for(int i=0; i<(*m_pMats).Count(); i++)
//    if(!CRenderer::CV_r_draw_phys_only || ((*m_pMats)[i].m_Flags & MIF_PHYSIC))
  {
    if (!(*m_pMats)[i].pRE)
      continue;

    if (!ef)
      e = (*m_pMats)[i].shaderItem.m_pShader;
    else
      e = ef;
    sr = (*m_pMats)[i].shaderItem.m_pShaderResources;
    if (e)
		{
      assert((*m_pMats)[i].pRE->m_pChunk->nFirstIndexId<60000);

      if (e->GetREs()->Num())
        gRenDev->EF_AddEf_NotVirtual(0, e->GetREs()->Get(0), e, sr, obj, nGlobalShaderTemplateId, pStateEff, nNumSort);
      else
        gRenDev->EF_AddEf_NotVirtual(0, (*m_pMats)[i].pRE, e, sr, obj, nGlobalShaderTemplateId, pStateEff, nNumSort);
		}
  }
}

void CLeafBuffer::UpdateCustomLighting(float fBackSideLevel, Vec3d vStatObjAmbientColor, const Vec3d & vLight, bool bCalcLighting)
{
  bool bRGB = (gRenDev->GetFeatures() & RFT_RGBA) != 0;

  vStatObjAmbientColor*=0.5f; // compensate overbrightness to make it work same as other objects

  Vec3d vSunColor = iSystem->GetI3DEngine()->GetSunColor();

  byte *pData = (byte *)m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;

  int nPosStride = m_VertexSize[m_pSecVertBuffer->m_vertexformat];
  int nNormStride, nColorStride, nInfoStride;
  bool * arrCullInfo = new bool[m_SecVertCount];
  ushort *pInds = GetIndices(NULL);
  bool bWasBark = false;
  for(int i=0; i<(*m_pMats).Count(); i++)
  {
    if (!(*m_pMats)[i].pRE)
      continue;
    CMatInfo *mi = &(*m_pMats)[i];
    IShader *ef = mi->shaderItem.m_pShader->GetTemplate(-1);
    int nFl = ef->GetFlags3();
    bool bTwoSided;
    if (nFl & EF3_HASVCOLORS)
      bTwoSided = (nFl & EF3_HASALPHATEST) != 0;
    else
      bTwoSided =  (mi->shaderItem.m_pShaderResources->m_ResFlags & MTLFLAG_2SIDED)!=0;//ef && (ef->GetCull() == e CULL_None);
    if (!bTwoSided)
      bWasBark = true;
    for (int j=mi->nFirstIndexId; j<mi->nNumIndices+mi->nFirstIndexId; j++)
    {
      int nIndex = pInds[j];
      assert(nIndex>=0 && nIndex<m_SecVertCount);
      arrCullInfo[nIndex] = bTwoSided;
    }
  }

  if (bWasBark && CRenderer::CV_r_Vegetation_PerpixelLight && m_nVertexFormat != VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F)
    ReCreateSystemBuffer(VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F);

  // get offsets
  byte *pPos      = &pData[0];
  byte *pNorm     = GetNormalPtr(nNormStride);
  uchar*pColor    = GetColorPtr(nColorStride);
  nInfoStride = sizeof(uint);
  bool * pInfo = arrCullInfo;

  int nTangStride, nBinormStride, nTNormStride, nSecColStride;
  byte *pTang = GetTangentPtr(nTangStride);
  byte *pBinorm = GetBinormalPtr(nBinormStride);
  byte *pTNorm = GetTNormalPtr(nTNormStride);
  byte *pSecCol = GetSecColorPtr(nSecColStride);

  assert(pNorm && pPos && pColor);
  assert(m_pMats);

  float fAlpha = 1.0f; // m_pMats->Get(0)->fAlpha;

  for(int i=0; i<m_SecVertCount; i++)
  {
    Vec3 & pos    = *((Vec3*)pPos);

    Vec3 & normal = *((Vec3*)pNorm);

    bool bPerPixel = false;
    if (!pInfo[0] && CRenderer::CV_r_Vegetation_PerpixelLight)
      bPerPixel = true;

    Vec3 vBr(1.f,1.f,1.f);
    Vec3 vLightVec;
    float fDot = 0;
    if(bCalcLighting)
    {
      //IVO: normal is 0 in some cases! 
      //this is no solution, we have to fix it in the exporter 
      if (normal != Vec3(0.0f,0.0f,0.0f))
        fDot = GetNormalized(normal).Dot(GetNormalized(vLight));

      if(fDot<0)
      {
        if(pInfo[0])
          fDot = -fDot; // double side lighting
        else
          fDot = 0;
      }

      if (!bPerPixel)
      {
        vBr.x = pInfo[0] ? ( vStatObjAmbientColor.x*2 + fDot*vSunColor.x*0.5f ) : ( vStatObjAmbientColor.x + fDot*vSunColor.x );
        vBr.y = pInfo[0] ? ( vStatObjAmbientColor.y*2 + fDot*vSunColor.y*0.5f ) : ( vStatObjAmbientColor.y + fDot*vSunColor.y );
        vBr.z = pInfo[0] ? ( vStatObjAmbientColor.z*2 + fDot*vSunColor.z*0.5f ) : ( vStatObjAmbientColor.z + fDot*vSunColor.z );
      }
      else
      {
        vBr = vStatObjAmbientColor;
        vLightVec.x = vLight.Dot(*(Vec3 *)pTang);
        vLightVec.y = vLight.Dot(*(Vec3 *)pBinorm);
        vLightVec.z = vLight.Dot(*(Vec3 *)pTNorm);
        vLightVec.Normalize();
        //vLightVec = *(Vec3 *)pTang;
        //vLightVec.Set(1,0,0);

        if(bRGB)
        {
          pSecCol[0] = (uchar)((vLightVec.x+1.0f)*127.5f);
          pSecCol[1] = (uchar)((vLightVec.y+1.0f)*127.5f);
          pSecCol[2] = (uchar)((vLightVec.z+1.0f)*127.5f);
        }
        else
        {
          pSecCol[0] = (uchar)((vLightVec.z+1.0f)*127.5f);
          pSecCol[1] = (uchar)((vLightVec.y+1.0f)*127.5f);
          pSecCol[2] = (uchar)((vLightVec.x+1.0f)*127.5f);
        }
      }

      if(fBackSideLevel<1.f)
      { // make back side darker
        float fDot2 = Vec3(-pos.x,-pos.y,0).GetNormalized().Dot(Vec3d(vLight.x,vLight.y,vLight.z))*2;
        if(fDot2<0)
          fDot2=0;      

        vBr.x -= fDot2*(1.f-fBackSideLevel);
        vBr.y -= fDot2*(1.f-fBackSideLevel);
        vBr.z -= fDot2*(1.f-fBackSideLevel);

        vBr.CheckMax(vStatObjAmbientColor);
      }
    }

    Vec3d vColor; // take into account original vertex color
    if(!CRenderer::CV_r_Vegetation_IgnoreVertColors && m_pLoadedColors)
    {
      vColor.x = m_pLoadedColors[i].x*vBr.x;
      vColor.y = m_pLoadedColors[i].y*vBr.y;
      vColor.z = m_pLoadedColors[i].z*vBr.z;
    }
    else
    {
      vColor = vBr*255.f;
    }

    vColor.CheckMin(Vec3d(255.f,255.f,255.f));

    if(bRGB)
    {
      pColor[0] = uchar(vColor.x);
      pColor[1] = uchar(vColor.y);
      pColor[2] = uchar(vColor.z);
      pColor[3] = uchar(fDot*255.0f);
    }
    else
    {
      pColor[0] = uchar(vColor.z);
      pColor[1] = uchar(vColor.y);
      pColor[2] = uchar(vColor.x);
      pColor[3] = uchar(fDot*255.0f);
    }

    pPos      += nPosStride;
    pNorm     += nNormStride;
    pColor    += nColorStride;

    pSecCol   += nSecColStride;
    pTang     += nTangStride;
    pBinorm   += nBinormStride;
    pTNorm    += nTNormStride;

    pInfo     ++;
  }

  if(m_pVertexBuffer)
    gRenDev->ReleaseBuffer(m_pVertexBuffer);
  delete [] arrCullInfo;
  m_pVertexBuffer=0;
}
/*
void CLeafBuffer::UpdateColorInBufer(const Vec3d & vColor)
{
  byte *pData = (byte *)m_pSecVertBuffer->m_data;

  int nColorStride;
  uchar*pColor = GetColorPtr(nColorStride);
  
  for(int i=0; i<m_SecVertCount; i++)
  {
    pColor[0] = (uchar)(vColor[0]*255.0f);
    pColor[1] = (uchar)(vColor[1]*255.0f);
    pColor[2] = (uchar)(vColor[2]*255.0f);
    pColor[3] = 255;
      
    pColor += nColorStride;
  }

  if(m_pVertexBuffer)
    gRenDev->ReleaseBuffer(m_pVertexBuffer);
  m_pVertexBuffer=0;
}
*/

void CLeafBuffer::AddRenderElements(CCObject * pObj, int DLightMask, int nTemplate, int nFogVolumeID, int nSortId, IMatInfo * pIMatInfo)
{
  if(!m_NumIndices || !m_pMats->Count())
    return;

  assert(m_pMats);

	if(nTemplate<0)
	{
		int nGlobalShaderTemplateId = gRenDev->GetGlobalShaderTemplateId();
		if(nGlobalShaderTemplateId>=0)
			nTemplate = nGlobalShaderTemplateId;
	}

  for (int i=0; i<m_pMats->Count(); i++)
  {
    CMatInfo * pMat = m_pMats->Get(i);
//    if(!pMat->nNumIndices) // stops rendering detail grass
  //    continue;

    CREOcLeaf * pOrigRE = pMat->pRE;

    // Override object material.
    if (pIMatInfo)
    { // Assume that the root material is the first material, sub materials start from index 1.
      if (i == 0)
        pMat = (CMatInfo*)pIMatInfo;
      else if (i-1 < pIMatInfo->GetSubMtlCount())
        pMat = (CMatInfo*)pIMatInfo->GetSubMtl(i-1);
    }

    IShader * e = pMat->shaderItem.m_pShader;
    SRenderShaderResources *sr = pMat->shaderItem.m_pShaderResources;

    if (e && pOrigRE)// && pMat->nNumIndices)
    {
      TArray<CRendElement *> *pREs = e->GetREs();
      if(nTemplate < EFT_USER_FIRST)
        e->AddTemplate(sr, nTemplate);

      assert(pOrigRE->m_pChunk->nFirstIndexId<60000);

      if (pREs && pREs->Num())
        gRenDev->EF_AddEf_NotVirtual(nFogVolumeID, pREs->Get(0), e, sr, pObj, nTemplate, 0, nSortId);
      else
        gRenDev->EF_AddEf_NotVirtual(nFogVolumeID, pOrigRE, e, sr, pObj, nTemplate, 0, nSortId);

			if(m_nClientTextureBindID)
        break;
    }
  } //i
}
/*
void CLeafBuffer::GenerateParticles(CCObject * pObj, ParticleParams * pParticleParams)
{
  I3DEngine * pEng = (I3DEngine *)iSystem->GetIProcess();

  // spawn particles
  PipVertex * pDst = (PipVertex *)m_pSecVertBuffer->m_data;
  for(int sn=0; sn<m_SecVertCount; sn++, pDst++)
  {
    for(int r=0; r<33 || r<99.f*rand()/RAND_MAX; r++)
      sn++, pDst++;

    if(sn<m_SecVertCount && pDst->nor.nz>0.5)
    {
      pParticleParams->vPosition.x = pDst->pos.x + pObj->m_Trans.x;
      pParticleParams->vPosition.y = pDst->pos.y + pObj->m_Trans.y;
      pParticleParams->vPosition.z = pDst->pos.z + pObj->m_Trans.z;
      pEng->SpawnParticles( *pParticleParams );
    }
  }
}
*/

void CLeafBuffer::Render(const SRendParams & rParams, CCObject * pObj, TArray<int> & ShaderTemplates, int e_overlay_geometry, bool bNotCurArea, IMatInfo *pMaterial, bool bSupportDefaultShaderTemplates)
{
	int nSortValue = (rParams.dwFObjFlags & FOB_NEAREST) ? eS_Nearest : rParams.nSortValue;

	CCObject * pObjTransp = NULL;
	for (int i=0; i<m_pMats->Count(); i++)
	{
		CMatInfo * pMat = m_pMats->Get(i);
		CRendElement * pREOcLeaf = pMat->pRE;

		// Override default material
		if (pMaterial)
		{
			int nMatId = pMat->m_nCGFMaterialID;
			if(nMatId<0)
				continue;

			// Assume that the root material is the first material, sub materials start from index 1.
			if (nMatId == 0)
				pMat = (CMatInfo*)pMaterial;
			else if (nMatId-1 < pMaterial->GetSubMtlCount())
			{
				pMat = (CMatInfo*)pMaterial->GetSubMtl(nMatId-1);
			}
		}

		SShader * pShader = (SShader *)pMat->shaderItem.m_pShader;
		SRenderShaderResources* sr = pMat->shaderItem.m_pShaderResources;

		if (pREOcLeaf && pShader)
		{
			int nTempl = rParams.nShaderTemplate;
			if (bSupportDefaultShaderTemplates && nTempl == -2 && i<ShaderTemplates.Num())
				nTempl = ShaderTemplates[i];

			if (rParams.nShaderTemplate>0)
				pShader->AddTemplate((SRenderShaderResources*)sr, (int&)rParams.nShaderTemplate,(const char *)NULL);

			if(rParams.dwFObjFlags & FOB_FOGPASS)
				if(pShader->mfGetTemplate(-1)->m_Flags & EF_OVERLAY)
					continue; // skip overlays during fog pass - it will be fogged by base geometry fog pass

			bool bTransparent = 
				(pObj->m_Color.a<1.f || !(pShader->mfGetTemplate(-1)->m_Flags2 & EF2_OPAQUE) || (sr && sr->m_Opacity != 1.0f));

			IShader * pStateShader = rParams.pStateShader;

			if(bTransparent)
			{	
				if((rParams.dwFObjFlags & FOB_LIGHTPASS) || (rParams.dwFObjFlags & FOB_FOGPASS) || (nTempl == EFT_INVLIGHT))
					continue;

				if(nSortValue==eS_FogShader)
					nSortValue=eS_FogShader_Trans;

				if(!e_overlay_geometry)
				{
					if(pShader->mfGetTemplate(-1)->m_Flags & EF_OVERLAY)
						continue;
				}
				else if(e_overlay_geometry >= 2 && pShader->mfGetTemplate(-1)->m_Flags & EF_OVERLAY)
				{
					if(bNotCurArea)
						continue;

					if(e_overlay_geometry == 2)
					{
						if(int(iTimer->GetCurrTime()*5)&1)
							pStateShader = gRenDev->EF_LoadShader("NoZTestState", eSH_World, EF_SYSTEM	);
					}
					else
						pStateShader = gRenDev->EF_LoadShader("ZTestGreaterState", eSH_World, EF_SYSTEM	);
				}

				if(!pObjTransp && pObj->m_DynLMMask != rParams.nStrongestDLightMask)
				{ // make object for transparent geometry since it will use different light mask and 
					pObjTransp = gRenDev->EF_GetObject(true);
					pObjTransp->CloneObject(pObj);
					pObjTransp->m_DynLMMask = rParams.nStrongestDLightMask;
				}
			}

			if( rParams.dwFObjFlags & FOB_RENDER_INTO_SHADOWMAP && (pMat->m_Flags & MIF_NOCASTSHADOWS) )
				continue;

			if( nSortValue == EFSLIST_STENCIL && bTransparent )
				gRenDev->EF_AddEf_NotVirtual(rParams.nFogVolumeID, pREOcLeaf, pShader, sr, 
				(bTransparent&&pObjTransp) ? pObjTransp : pObj, 
				nTempl, pStateShader, 0);
			else
				gRenDev->EF_AddEf_NotVirtual(rParams.nFogVolumeID, pREOcLeaf, pShader, sr, 
				(bTransparent&&pObjTransp) ? pObjTransp : pObj, 
				nTempl, pStateShader, nSortValue);
		}
	} //i
}

void CLeafBuffer::RenderDebugLightPass(const Matrix44 & mat, int nLightMask, float fAlpha)
{
	int nLightsNum = 0;
	for(int i=0; i<32; i++)
		if(nLightMask & (1<<i))
			nLightsNum++;

	CCObject * pObj = gRenDev->EF_GetObject(true);
	pObj->m_Matrix = mat;

	IShader * pShader = gRenDev->EF_LoadShader("ObjectColor_VP",eSH_World,EF_SYSTEM);
	pObj->m_Color = CFColor(nLightsNum>=3,nLightsNum==2,nLightsNum==1,fAlpha);

	for (int i=0; i<m_pMats->Count(); i++)
	{
		CRendElement * pREOcLeaf = m_pMats->Get(i)->pRE;
		if (pREOcLeaf)
			gRenDev->EF_AddEf_NotVirtual(0, pREOcLeaf, pShader, 0, pObj, 0);
	}
}
/*
void CLeafBuffer::CopyVertices(byte * pVertsNew, int nVertFormatNew, int nNewVertsCount)
{
	SBufInfoTable *pOffsNew = &gBufInfoTable[nVertFormatNew];
	int nVertSizeNew = m_VertexSize[nVertFormatNew];

	int nPosStride=0;
	if(byte * pPosPtr = GetPosPtr(nPosStride))
		for(int i=0; i<nNewVertsCount; i++)
			*(DWORD*)&pPosPtr[i*nPosStride] = *(DWORD*)&pVertsNew[nVertSizeNew*i];

	int nColorStride=0;
	if(pOffsNew->OffsColor)
		if(byte * pColorPtr = GetColorPtr(nColorStride))
			for(int i=0; i<nNewVertsCount; i++)
				*(DWORD*)&pColorPtr[i*nColorStride] = *(DWORD*)&pVertsNew[pOffsNew->OffsColor + nVertSizeNew*i];

	int nSecColorStride=0;
	if(pOffsNew->OffsSecColor)
		if(byte * pSecColorPtr = GetSecColorPtr(nSecColorStride))
			for(int i=0; i<nNewVertsCount; i++)
				*(DWORD*)&pSecColorPtr[i*nSecColorStride] = *(DWORD*)&pVertsNew[pOffsNew->OffsSecColor + nVertSizeNew*i];

	int nNormalStride=0;
	if(pOffsNew->OffsNormal)
		if(byte * pNormalPtr = GetNormalPtr(nNormalStride))
			for(int i=0; i<nNewVertsCount; i++)
				*(DWORD*)&pNormalPtr[i*nNormalStride] = *(DWORD*)&pVertsNew[pOffsNew->OffsNormal + nVertSizeNew*i];

	int nTCStride=0;
	if(pOffsNew->OffsTC)
		if(byte * pTCPtr = GetUVPtr(nTCStride))
			for(int i=0; i<nNewVertsCount; i++)
				*(DWORD*)&pTCPtr[i*nTCStride] = *(DWORD*)&pVertsNew[pOffsNew->OffsTC + nVertSizeNew*i];
}
*/