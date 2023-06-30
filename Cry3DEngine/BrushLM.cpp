////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushlm.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "objman.h"
#include "visareas.h"
#include "terrain_sector.h"
#include "cbuffer.h"
#include "3DEngine.h"
#include "meshidx.h"
#include "watervolumes.h"
#include "LMCompStructures.h"
#include "brush.h"
#include "IEntitySystem.h"

void CBrush::SetLightmap(RenderLMData *pLMData, float *pTexCoords, UINT iNumTexCoords, const unsigned char cucOcclIDCount, const std::vector<std::pair<EntityId, EntityId> >& aIDs)
{
	assert(cucOcclIDCount <= 4);

	memset(m_arrOcclusionLightOwners,0,sizeof(m_arrOcclusionLightOwners));

	if(GetCVars()->e_light_maps_occlusion)
	{
		if(m_bEditorMode)
		{
			for(int i=0; i<4 && i<cucOcclIDCount; ++i)
			{
				EntityId nEntId = aIDs[i].first;
				IEntityRender * pEnt;
				if(nEntId == (EntityId)-1)
					pEnt = (IEntityRender*)-1; // sun
				else
					pEnt = GetSystem()->GetIEntitySystem()->GetEntity(nEntId);
				//			assert(pEnt);
				m_arrOcclusionLightOwners[i] = pEnt;
			}
		}
		else
		{
			const list2<CDLight*> * pStaticLights = Get3DEngine()->GetStaticLightSources();
			for(int i=0; (i<4) && (i<cucOcclIDCount) && (pStaticLights->Count()); ++i)
			{
				EntityId nEntId = aIDs[i].second;
				IEntityRender * pEnt;
				if(nEntId == (EntityId)-1)
					pEnt = (IEntityRender*)-1; // sun
				else
					pEnt = pStaticLights->GetAt(nEntId)->m_pOwner;
				assert(pEnt);
				m_arrOcclusionLightOwners[i] = pEnt;
			}
		}
	}

	SetLightmap(pLMData, pTexCoords, iNumTexCoords, 0);
}

void CBrush::SetLightmap(RenderLMData *pLMData, float *pTexCoords, UINT iNumTexCoords, int nLod)
{
	// ---------------------------------------------------------------------------------------------
	// Set a referenece of a DOT3 Lightmap object for this GLM
	// ---------------------------------------------------------------------------------------------

	IRenderer *pIRenderer = GetRenderer();

	assert(iNumTexCoords);
	assert(!IsBadReadPtr(pTexCoords, sizeof(float) * 2 * iNumTexCoords));

	m_arrLMData[nLod].m_pLMData = pLMData;

	if (m_arrLMData[nLod].m_pLMTCBuffer)
	{
		pIRenderer->DeleteLeafBuffer(m_arrLMData[nLod].m_pLMTCBuffer);
		m_arrLMData[nLod].m_pLMTCBuffer = NULL;
	}

	IStatObj *pIStatObj = GetEntityStatObj(0, NULL);									

	if (pIStatObj == NULL)
		return;

  if(!pIStatObj->EnableLightamapSupport())
    return;

	CLeafBuffer *pLeafBuffer = pIStatObj->GetLeafBuffer();			

	if (pLeafBuffer == NULL)
		return;

	// Renderer expect 2 floats
	std::vector<float> vTexCoord2;
	vTexCoord2.reserve(iNumTexCoords * 2);
	UINT i;
	for (i=0; i<iNumTexCoords; i++)
	{
		vTexCoord2.push_back(pTexCoords[i * 2 + 0]); // S
		vTexCoord2.push_back(pTexCoords[i * 2 + 1]); // T
	}

	if (pLeafBuffer->m_SecVertCount != iNumTexCoords)
	{
		char szBuffer[1024];
		sprintf(szBuffer, "Error: CBrush::SetLightmap: Object at position (%f, %f, %f) has" \
			" texture mismatch (%i coordinates supplied, %i required)\r\n",
			GetPos().x, GetPos().y, GetPos().z, 
			iNumTexCoords, pLeafBuffer->m_SecVertCount);
		Warning(0,pIStatObj->GetFileName(),szBuffer);
		// assert(pLeafBuffer->m_SecVertCount == iNumTexCoords);
		return;
	}

	// Make leafbuffer and fill it with texture coordinates
	m_arrLMData[nLod].m_pLMTCBuffer = GetRenderer()->CreateLeafBufferInitialized(
		&vTexCoord2[0], pLeafBuffer->m_SecVertCount, VERTEX_FORMAT_TEX2F, 
		0/*pLeafBuffer->GetIndices(0)*/, 0/*pLeafBuffer->m_Indices.m_nItems*/, 
		R_PRIMV_TRIANGLES, "LMapTexCoords", eBT_Static, 1, 0, NULL, NULL, false, false);

  C3DEngine *pEng = (C3DEngine *)Get3DEngine();
	m_arrLMData[nLod].m_pLMTCBuffer->SetChunk(pEng->m_pSHDefault,
		0,pLeafBuffer->m_SecVertCount, 0,pLeafBuffer->m_Indices.m_nItems);
}