////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_det_tex.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: terrain detail textures
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "terrain_sector.h"
#include "3dengine.h"

void CTerrain::MakeSplatVertex(const int & x,
															 const int & y,
															 const uchar & alpha, 
															 struct_VERTEX_FORMAT_P3F_N_COL4UB & vert, 
															 const int & nTexID,
															 const Vec3d & vDetTexOffset)
{
	m_DetailTexInfo[nTexID].lstIndices.Add(m_DetailTexInfo[nTexID].lstVertArray.Count());
	vert.xyz.x = (float)x;
	vert.xyz.y = (float)y;
	vert.xyz.z = /*0.075f+*/GetZSafe(x,y);
	vert.color.bcolor[3] = alpha;
	vert.xyz += vDetTexOffset;
	m_DetailTexInfo[nTexID].lstVertArray.Add(vert);
}

// get normal from sector main vertex buffer
void CTerrain::SetDetailVertNormal(struct_VERTEX_FORMAT_P3F_N_COL4UB & v0)
{ 
	CSectorInfo * pSec0 = GetSecInfo(v0.xyz);
	if(pSec0 && pSec0->m_pLeafBuffer && pSec0->m_cPrevGeomMML==pSec0->m_cGeometryMML)
	{
		int nPosStride=0;
		byte * pPos = pSec0->m_pLeafBuffer->GetPosPtr(nPosStride);
		int nNormStride=0;
		byte * pNorm = pSec0->m_pLeafBuffer->GetNormalPtr(nNormStride);
		int nStep = (1<<pSec0->m_cGeometryMML)*CTerrain::GetHeightMapUnitSize();

		int nVertId = ((int)(v0.xyz.x+0.2f)-pSec0->m_nOriginX)/nStep*(CTerrain::GetSectorSize()/nStep+1)
			+ ((int)(v0.xyz.y+0.2f)-pSec0->m_nOriginY)/nStep;

		// if sector lod is not 0 - result will ve not 100% correct but acceptable

		if(nVertId>=0 && nVertId < pSec0->m_pLeafBuffer->m_SecVertCount && pPos && pNorm)
		{
			Vec3d * pvPos = (Vec3d *)&pPos[nPosStride*nVertId];
			assert(pSec0->m_cGeometryMML!=0 || (fabs(pvPos->x - v0.xyz.x)<0.25f && fabs(pvPos->y - v0.xyz.y)<0.25f));
			Vec3d * pvNorm = (Vec3d *)&pNorm[nNormStride*nVertId];
			v0.normal = *pvNorm;
		}
		else
			assert(0);
	}
}

void CTerrain::RenderDetailTextures(float _fFogNearDistance, float _fFogFarDistance)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	if(GetCVars()->e_detail_texture && GetCVars()->e_terrain)
		if(GetCVars()->e_detail_texture_quality)
			if(!GetRenderer()->EF_GetHeatVision()) // if no heat vision
				if(m_pViewCamera->GetFov() >= GetCVars()->e_detail_texture_min_fov) // if no big zoom
					if (m_pRETerrainDetailTextureLayers && m_pTerrainDetailTextureLayersEff)
					{
						CCObject * pObj = GetIdentityCCObject();	
						if(!m_nRenderStackLevel)
						{
							pObj->m_nScissorX1 = GetViewCamera().m_ScissorInfo.x1;
							pObj->m_nScissorY1 = GetViewCamera().m_ScissorInfo.y1;
							pObj->m_nScissorX2 = GetViewCamera().m_ScissorInfo.x2;
							pObj->m_nScissorY2 = GetViewCamera().m_ScissorInfo.y2;
						}
						//            GetRenderer()->EF_AddEf(0, m_pRETerrainDetailTextureLayers, m_pTerrainDetailTextureLayersEff, NULL, pObj, 0, NULL, EFSLIST_STENCIL | eS_TerrainDetailTextures/*, eS_Opaque*/);

						DrawDetailTextures(_fFogNearDistance, _fFogFarDistance, true);
					}
}

// todo: remove unesecary copy
void CTerrain::DrawDetailTextures(float _fFogNearDistance, float _fFogFarDistance, bool bRealyDraw)
{
	Vec3d vCamPos = m_pViewCamera->GetPos();  
	if(vCamPos.z - GetZSafe(vCamPos.x,vCamPos.y) > DETAIL_TEXTURE_VIEW_DIST)
		return;

	int nTexID=0;

	Matrix44 mat;
	GetRenderer()->GetModelViewMatrix(mat.GetData());
	Vec3d vForward = -mat.GetColumn(2);
	vForward.Normalize();

	const float fGeometryUpdateStep = 4;
	const float fMaxViewDistSq = DETAIL_TEXTURE_VIEW_DIST*DETAIL_TEXTURE_VIEW_DIST;
	const float fFocusDist = cry_sqrtf(DETAIL_TEXTURE_VIEW_DIST*DETAIL_TEXTURE_VIEW_DIST/2);
	const int nFocusDistAligned = int(fFocusDist+fGeometryUpdateStep)/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize();
	const float fZProjMaxDistRatio = 3.0f;

	const float fGeometryUpdateStepSqr = fGeometryUpdateStep*fGeometryUpdateStep;		// tiny optimization

	// find new focus
	int X = (int)(vCamPos.x + vForward.x*fFocusDist);
	int Y = (int)(vCamPos.y + vForward.y*fFocusDist);

	// align to grid
	X = X/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize();
	Y = Y/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize();

	if(m_nRenderStackLevel==0)
	{
		float dx = float(m_nDetailTexFocusX - X);
		float dy = float(m_nDetailTexFocusY - Y);
		float fDiffSqr = dx*dx+dy*dy;
		if(fDiffSqr > fGeometryUpdateStepSqr)
		{
			const int x1 = X - nFocusDistAligned;
			const int y1 = Y - nFocusDistAligned;
			const int x2 = X + nFocusDistAligned;
			const int y2 = Y + nFocusDistAligned;

			struct_VERTEX_FORMAT_P3F_N_COL4UB vert;
			vert.color.bcolor[0]=vert.color.bcolor[1]=vert.color.bcolor[2]=128;
			vert.color.bcolor[3]=255;
			vert.normal.Set(0,0,0);

			for( nTexID=0; nTexID<MAX_SURFACE_TYPES_COUNT; nTexID++)
			{
				list2<ushort> & lstInds = m_DetailTexInfo[nTexID].lstIndices;
				list2<struct_VERTEX_FORMAT_P3F_N_COL4UB> & lstVerts = m_DetailTexInfo[nTexID].lstVertArray;

				lstVerts.Clear();
				lstInds.Clear();

				if(!m_DetailTexInfo[nTexID].nTexID)
					continue;

				CMatMan * pMatMan = Get3DEngine()->GetMatMan();
				char szMatName[256]="";
				sprintf(szMatName,"Level.Terrain.Layer%d", nTexID);
				m_DetailTexInfo[nTexID].pMatInfo = pMatMan->FindMatInfo(szMatName);

				if(m_DetailTexInfo[nTexID].ucProjAxis != 'Z' && !m_DetailTexInfo[nTexID].pMatInfo)
					if(GetCVars()->e_terrain_single_pass)
						continue;

				// calc vertex offset to move detail textures closer to camera and up, 
				// especially when zmin is very small
				// and especially for not Z projections
				float fOffset = max(0.05f,0.25f-GetViewCamera().GetZMin());
				Vec3d vDetTexOffset = GetViewCamera().GetVCMatrixD3D9().GetOrtZ()*fOffset;
				vDetTexOffset.z += 0.015f;
				vDetTexOffset *= 0.75f;
				if(m_DetailTexInfo[nTexID].ucProjAxis != 'Z')
					vDetTexOffset.z *= 4;

				float fDistRatio = (1.f + (fZProjMaxDistRatio*fZProjMaxDistRatio-1.f)*(m_DetailTexInfo[nTexID].ucProjAxis == 'Z'))/1.7f;

				for(int x=x1; x<x2; x+=CTerrain::GetHeightMapUnitSize())
				{
					bool bPreviousFacePresent = 0; // for vertex sharing

					for(int y=y1; y<y2; y+=CTerrain::GetHeightMapUnitSize())
					{
						const float dx = (float)x - vCamPos.x;
						const float dy = (float)y - vCamPos.y;

						float fDistToVertSq = (dx*dx+dy*dy)*fDistRatio;
						if(fDistToVertSq>fMaxViewDistSq)
							continue;

						uchar ucAlpha = 255;
						bool bNoHole = !GetHoleSafe(x,y);
						const uchar id00 = GetSurfaceTypeID(x          ,y          )==nTexID;
						const uchar id01 = bNoHole && GetSurfaceTypeID(x          ,y+CTerrain::GetHeightMapUnitSize())==nTexID;
						const uchar id10 = bNoHole && GetSurfaceTypeID(x+CTerrain::GetHeightMapUnitSize(),y          )==nTexID;
						const uchar id11 = bNoHole && GetSurfaceTypeID(x+CTerrain::GetHeightMapUnitSize(),y+CTerrain::GetHeightMapUnitSize())==nTexID;

						// face 1
						if( id00 || id10 || id01 )
						{
							if(nTexID)
								nTexID=nTexID;

							if(bPreviousFacePresent)
							{ // only add indices
								const int id1 = lstInds[lstInds.Count()-3];
								lstInds.Add(id1);
								const int id2 = lstInds[lstInds.Count()-2];
								lstInds.Add(id2);
							}
							else
							{ // add vertices and indices
								MakeSplatVertex(x,y,ucAlpha*id00, vert, nTexID, vDetTexOffset);
								MakeSplatVertex(x+CTerrain::GetHeightMapUnitSize(),y,ucAlpha*id10, vert, nTexID, vDetTexOffset);
							}
							MakeSplatVertex(x,y+CTerrain::GetHeightMapUnitSize(),ucAlpha*id01, vert, nTexID, vDetTexOffset);

							if(m_DetailTexInfo[nTexID].pMatInfo)
							{
								SetDetailVertNormal(lstVerts[lstInds[lstInds.Count()-1]]);
								SetDetailVertNormal(lstVerts[lstInds[lstInds.Count()-2]]);
								SetDetailVertNormal(lstVerts[lstInds[lstInds.Count()-3]]);
							}

							bPreviousFacePresent = true;
						}
						else
							bPreviousFacePresent = false;

						// face 2
						if( id11 || id10 || id01 )
						{
							if(bPreviousFacePresent)
							{ // only add indices
								const int id1 = lstInds[lstInds.Count()-1];
								lstInds.Add(id1);
								const int id2 = lstInds[lstInds.Count()-3];
								lstInds.Add(id2);
							}
							else
							{ // add vertices and indices
								MakeSplatVertex(x,y+CTerrain::GetHeightMapUnitSize(),ucAlpha*id01, vert, nTexID, vDetTexOffset);
								MakeSplatVertex(x+CTerrain::GetHeightMapUnitSize(),y,ucAlpha*id10, vert, nTexID, vDetTexOffset);
							}
							MakeSplatVertex(x+CTerrain::GetHeightMapUnitSize(),y+CTerrain::GetHeightMapUnitSize(),ucAlpha*id11, vert, nTexID, vDetTexOffset);

							if(m_DetailTexInfo[nTexID].pMatInfo)
							{
								SetDetailVertNormal(lstVerts[lstInds[lstInds.Count()-1]]);
								SetDetailVertNormal(lstVerts[lstInds[lstInds.Count()-2]]);
								SetDetailVertNormal(lstVerts[lstInds[lstInds.Count()-3]]);
							}

							bPreviousFacePresent = true;
						}
						else
							bPreviousFacePresent = false;
					}
				}

				// update buffer
				int nVertCount = lstVerts .Count();
				if(!lstInds.Count() && nVertCount)
				{ nVertCount=0; assert(0); }

        bool bUpdated = false;
				if(!m_DetailTexInfo[nTexID].pVertBuff || 
					nVertCount > m_DetailTexInfo[nTexID].pVertBuff->m_SecVertCount	 ||
					nVertCount < m_DetailTexInfo[nTexID].pVertBuff->m_SecVertCount/2 ||
					m_DetailTexInfo[nTexID].pVertBuff->m_nVertexFormat != VERTEX_FORMAT_P3F_N_COL4UB)
				{
					GetRenderer()->DeleteLeafBuffer(m_DetailTexInfo[nTexID].pVertBuff);					
					if(nVertCount)
					{
            bUpdated = true;
						m_DetailTexInfo[nTexID].pVertBuff = GetRenderer()->CreateLeafBufferInitialized(
							lstVerts.GetElements(), lstVerts .Count(), VERTEX_FORMAT_P3F_N_COL4UB,
							lstInds.GetElements(), lstInds.Count(), R_PRIMV_TRIANGLES, 
							"TerrainDetailTex", eBT_Static, 1, m_DetailTexInfo[nTexID].nTexID);

						{ // setup shader, texid and texgen
							CLeafBuffer * pLeafBuffer = m_DetailTexInfo[nTexID].pVertBuff;
							int nTextureID = m_DetailTexInfo[nTexID].nTexID;
							{
								pLeafBuffer->SetShader(m_pTerrainLayerEf, nTextureID);

								if(!GetCVars()->e_terrain_single_pass || !GetCVars()->e_detail_texture_quality)
								{
									if(pLeafBuffer->m_pMats->Count())
										for(int i=1; i<MAX_CUSTOM_TEX_BINDS_NUM; i++)
											pLeafBuffer->m_pMats->GetAt(0).pRE->m_CustomTexBind[i] = 0;
								}
								else if(!m_nRenderStackLevel)
								{
									assert(MAX_DETAIL_LAYERS_IN_SECTOR <= MAX_CUSTOM_TEX_BINDS_NUM);

									float * pOutParams = m_DetailTexInfo[nTexID].arrTexOffsets;

									// setup proj direction
									if(m_DetailTexInfo[nTexID].ucProjAxis == 'X')
									{
										pOutParams[0] = 0;
										pOutParams[1] = m_DetailTexInfo[nTexID].fScaleX;
										pOutParams[2] = 0;
										pOutParams[3] = sqrt(fMaxViewDistSq/fDistRatio);

										pOutParams[4] = 0;
										pOutParams[5] = 0;
										pOutParams[6] = m_DetailTexInfo[nTexID].fScaleY;
										pOutParams[7] = 0;
									}
									else if(m_DetailTexInfo[nTexID].ucProjAxis=='Y')
									{
										pOutParams[0] = m_DetailTexInfo[nTexID].fScaleX;
										pOutParams[1] = 0;
										pOutParams[2] = 0;
										pOutParams[3] = sqrt(fMaxViewDistSq/fDistRatio);

										pOutParams[4] = 0;
										pOutParams[5] = 0;
										pOutParams[6] = m_DetailTexInfo[nTexID].fScaleY;
										pOutParams[7] = 0;
									}
									else // Z
									{
										pOutParams[0] = 0;
										pOutParams[1] = m_DetailTexInfo[nTexID].fScaleX;
										pOutParams[2] = 0;
										pOutParams[3] = sqrt(fMaxViewDistSq/fDistRatio);

										pOutParams[4] = m_DetailTexInfo[nTexID].fScaleY;
										pOutParams[5] = 0;
										pOutParams[6] = 0;
										pOutParams[7] = 0;
									}
								}
							}
						}

						if(GetCVars()->e_terrain_log)
							GetLog()->Log("CTerrain::DrawDetailTextures: BufferReMake %d", GetFrameID());
					}
					else
						m_DetailTexInfo[nTexID].pVertBuff = 0;
				}

				if(m_DetailTexInfo[nTexID].pVertBuff)
				{
					assert(lstInds.Count() && nVertCount);
          if (!bUpdated)
          {
					  m_DetailTexInfo[nTexID].pVertBuff->UpdateSysVertices(&lstVerts[0],lstVerts.Count());
					  m_DetailTexInfo[nTexID].pVertBuff->UpdateSysIndices(&lstInds[0],lstInds.Count());
					  //m_DetailTexInfo[nTexID].pVertBuff->UpdateVidVertices(&lstVerts[0],lstVerts.Count());
					  //m_DetailTexInfo[nTexID].pVertBuff->InvalidateVideoBuffer();
          }
					if(GetCVars()->e_terrain_log)
						GetLog()->Log("CTerrain::DrawDetailTextures: BufferUpdate %d", GetFrameID());

					m_DetailTexInfo[nTexID].pVertBuff->SetChunk(m_pTerrainLayerEf,0,lstVerts .Count(),0,lstInds.Count());
					if(m_DetailTexInfo[nTexID].pVertBuff->m_pMats->Get(0)->pRE)
						m_DetailTexInfo[nTexID].pVertBuff->m_pMats->Get(0)->pRE->m_CustomData = m_DetailTexInfo[nTexID].arrTexOffsets;
				}

			}
			m_nDetailTexFocusX = X;
			m_nDetailTexFocusY = Y;
		}
	}

	// render layers
	if(bRealyDraw)
		for( nTexID=0; nTexID<MAX_SURFACE_TYPES_COUNT; nTexID++)
			if(m_DetailTexInfo[nTexID].pVertBuff && m_DetailTexInfo[nTexID].lstIndices.Count())
			{
				CMatMan * pMatMan = Get3DEngine()->GetMatMan();
				char szMatName[256]="";
				sprintf(szMatName,"Level.Terrain.Layer%d", nTexID);
				m_DetailTexInfo[nTexID].pMatInfo = pMatMan->FindMatInfo(szMatName);
				m_DetailTexInfo[nTexID].pVertBuff->AddRenderElements(0,0,-1,0,0,m_DetailTexInfo[nTexID].pMatInfo);
			}
}

