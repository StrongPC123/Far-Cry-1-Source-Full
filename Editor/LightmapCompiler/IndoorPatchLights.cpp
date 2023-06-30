// ---------------------------------------------------------------------------------------------
//	Crytek CryENGINE source code
//	History:
//	- Created by Marco Corbetta
//  - Rewritten by Tim Schroeder
//	- Partial rewrite for editor GLM integration (Tim Schroeder)
//  - Debug code added (Tim Schroeder)
// ---------------------------------------------------------------------------------------------
#include "stdafx.h"
#include <ILMSerializationManager.h>
#include <float.h>
#include <direct.h>
#include "I3dEngine.h"
#include <list2.h>
#include "IndoorLightPatches.h"
#include "IEntityRenderstate.h"
#include <AABBSV.h>

extern CAASettings gAASettings;

// Needs to be somewhere
ICompilerProgress *g_pIProgress = NULL;

CRasterCubeManager::~CRasterCubeManager()
{
	//just make sure all rastercubes were destroyed
	Clear();
}

void CRasterCubeManager::AddReference(const CRadMesh* cpRadMesh)
{
	if(cpRadMesh->m_pClusterRC == NULL)
		return;
	std::map<CRasterCubeImpl*, std::set<const CRadMesh*> >::iterator iter = m_mRasterCubeMap.find(cpRadMesh->m_pClusterRC);
	assert(iter != m_mRasterCubeMap.end());
	(iter->second).insert(cpRadMesh);
} 

CRasterCubeImpl* CRasterCubeManager::CreateRasterCube()
{
	CRasterCubeImpl* pImpl = new CRasterCubeImpl();
	assert(pImpl);
	m_mRasterCubeMap.insert(std::pair<CRasterCubeImpl*, std::set<const CRadMesh*> >(pImpl, std::set<const CRadMesh*>()));
	return pImpl;
}

const bool CRasterCubeManager::RemoveReference(CRadMesh* cpRadMesh)
{
	if(cpRadMesh->m_pClusterRC == NULL)
		return false;	//there has never been a reference added
	std::map<CRasterCubeImpl*, std::set<const CRadMesh*> >::iterator iter = m_mRasterCubeMap.find(cpRadMesh->m_pClusterRC);
	cpRadMesh->m_pClusterRC = NULL;
	if(iter == m_mRasterCubeMap.end())
		return false;//there has never been a reference added
	(iter->second).erase(cpRadMesh);	//erase from set
	if((iter->second).size() < 1)		//erase rastercube because it has no more references 
	{
		delete (iter->first);	//not needed anymore
		m_mRasterCubeMap.erase(iter);
		return true;
	}
	return false;
}

void CRasterCubeManager::Clear()
{
	//just make sure all rastercubes were destroyed
	for(std::map<CRasterCubeImpl*, std::set<const CRadMesh*> >::iterator iter = m_mRasterCubeMap.begin(); iter != m_mRasterCubeMap.end(); ++iter)
	{
		delete (iter->first);
	}
	m_mRasterCubeMap.clear();
}

const Vec3d RotateIntoPlane(const Vec3d& vPlaneNormal0, const Vec3d& vPlaneNormal1, const Vec3d& rInPosition, const Vec3d& rSource0)
{
	Vec3d vDir = rInPosition - rSource0;
	//first try to rotate
	Vec3d vPlaneCross = vPlaneNormal0 % vPlaneNormal1;	//this is gonna be the rotation axis
	float cfSinPlaneCross = vPlaneCross.GetLength();	//this is the sinus of the angle between both plane normals
	if(cfSinPlaneCross < 0.001f)
		return rInPosition;
	float cfCosPlaneCross = vPlaneNormal0 * vPlaneNormal1;	
	vPlaneCross *= 1.0f / cfSinPlaneCross;	//normalize rotation axis
	Vec3d origin = vPlaneCross*(vPlaneCross|vDir);

	assert(fabs(cfSinPlaneCross*cfSinPlaneCross+cfCosPlaneCross*cfCosPlaneCross - 1.f) < 0.01f);

	Vec3d vRet = (origin + (vDir-origin)*cfCosPlaneCross + (vPlaneCross % vDir)*cfSinPlaneCross);
	//now check	how accurate this is
	const float cfPlaneDist = -(vRet*vPlaneNormal1); 
	//it was a good rotation, use rotated result
	if(fabs(cfPlaneDist) < 0.01f)
	{
		return (vRet + rSource0);
	}
	//result of rotation is close to the plane, correct this difference
	if(fabs(cfPlaneDist) < 0.1f)
	{
		vRet = vRet + vPlaneNormal1*cfPlaneDist;
		return (vRet + rSource0);
	}
	//use plane projection, rotation went wrong
	if(vDir * vPlaneNormal1 > 0)
		vDir *= -1;//opposite direction
	const float cfPlaneDist2 = -(vDir*vPlaneNormal1); 
	Vec3d r	= vPlaneNormal1*(cfPlaneDist2); 
	return (rInPosition + r);
}

CLightScene::CLightScene() : m_uiCurrentPatchNumber(0), m_pCurrentPatch(NULL), m_pISerializationManager(NULL), m_uiStartMemoryConsumption(0), m_puiIndicator(0), m_pOcclSamples(0), m_pSubSamples(0), m_pfColours(0), m_uiSampleCount(0)
{
	memset(&m_IndInterface, 0, sizeof(IndoorBaseInterface));
}

CLightScene::~CLightScene()
{
	if (m_pISerializationManager)
	{
		m_pISerializationManager->Release();
		m_pISerializationManager = NULL;
	}
	if(m_puiIndicator)
	{
		delete [] m_puiIndicator;	m_puiIndicator = NULL;
	}
	if(m_pSubSamples)
	{
		delete [] m_pSubSamples;	m_pSubSamples = NULL;
	}
	if(m_pOcclSamples)
	{
		delete [] m_pOcclSamples;	m_pOcclSamples = NULL;
	}
	if(m_pfColours)
	{
		delete [] m_pfColours;		m_pfColours = NULL;
	}
}

//check normals to be normalized, trace a warning to tell that this is not correct
const unsigned int CLightScene::CheckNormals(float& rfMaxVariance, const CRadMesh* const pMesh)
{
	const bool bNotNormalized = ((pMesh->m_uiFlags & NOT_NORMALIZED_NORMALS_FLAG) == 0)?false : true;
	if(bNotNormalized)
		rfMaxVariance = sqrtf(pMesh->m_fMaxVariance);
	else
		rfMaxVariance = 0.f;
	const bool bWrongNormals = ((pMesh->m_uiFlags & WRONG_NORMALS_FLAG) == 0)?false : true;
	unsigned int uiRet = 0;
	if(bNotNormalized)
	{
		rfMaxVariance = sqrtf(rfMaxVariance);//to retrieve real variance, not the squared one
		uiRet |= NOT_NORMALIZED_NORMALS_FLAG;
	}
	if(bWrongNormals)
		uiRet |=  WRONG_NORMALS_FLAG;
	return uiRet;
}

inline void CLightScene::MergeTangentSpace(CRadVertex& rVertex1, CRadVertex& rVertex2)
{
	//quick hack to use a merged normal
	Vec3d vNewNormal		= (rVertex1.m_vNormal * 0.5f + rVertex2.m_vNormal * 0.5f);	vNewNormal.Normalize();
	rVertex1.m_vNormal = rVertex2.m_vNormal = vNewNormal;
}

/*idea is as follows: 
	- every polygon contains a vector of vertex sharing polygon pointers
	- so iterate all polygons of all lightmap meshes and compare vertices of all polygons of the other patches 
	- during computation of colours and normals, if barycentric coordinates fail on the closest polygon, 
	  they are mapped into the vertex sharing polygons to produce the right, smooth value
    - this function computes the hash_map and sets the hash indices into the respective polygons
*/
void CLightScene::ComputeSmoothingInformation(const unsigned int uiSmoothAngle, const bool cbTSGeneratedByLM)
{
	assert(uiSmoothAngle <= 90);
	//will only smmoth an edge if it really is one, otherwise normals are treated differently
	static const float epsPos		= 0.000001f;		//be more correct, vertex position should really be the same
	const float epsNormalSmooth		= cosf(0.001f/*some threshold*/ + (float)uiSmoothAngle/180.f * 3.14159f);		//use a coarser eps for normals, everything which is dotted below 45 degree
	//iterate each lightmap patch
	for (radpolyit iterPatchOuter=m_lstScenePolys.begin(); iterPatchOuter!=m_lstScenePolys.end(); iterPatchOuter++)
	{//outer lightmap mesh loop
		CRadPoly *pPatchOuter=(*iterPatchOuter);
		//with each remaining one 
		for (radpolyit iterPatchInner=iterPatchOuter+1; iterPatchInner!=m_lstScenePolys.end();iterPatchInner++)
		{//inner lightmap mesh loop
			CRadPoly *pPatchInner=(*iterPatchInner);
			if(iterPatchOuter != iterPatchInner)	//do not consider polygons of the same mesh which are smoothed already 
			{
				for (radpolyit iterPolyOuter=pPatchOuter->m_lstMerged.begin();iterPolyOuter!=pPatchOuter->m_lstMerged.end();iterPolyOuter++)
				{//outer lightmap mesh polygon iteration
					CRadPoly& rPolyOuter= *(*iterPolyOuter);
					bool bMerged[3] = {false,false,false};	//keep trac of the tangent space merging
					assert(rPolyOuter.m_lstOriginals.size() == 3);
					const int nVerts1 = rPolyOuter.m_lstOriginals.size();
					for (radpolyit iterPolyInner=pPatchInner->m_lstMerged.begin();iterPolyInner!=pPatchInner->m_lstMerged.end();iterPolyInner++)
					{//inner lightmap mesh polygon iteration
						int iFirstFoundIndexOuter = -1, iFirstFoundIndexInner = -1;	//index saving first found shared indices
						bool bShareCond = false;	//to stop the vertex-vertex loop
						bool bShareCondFound0 = false;	//to notice if at least one vertex was shared
						CRadPoly& rPolyInner = *(*iterPolyInner);
						assert(rPolyInner.m_lstOriginals.size() == 3);
						const int nVerts2 = rPolyInner.m_lstOriginals.size();
						int vCount=0;	//shared vertex counter, to only consider triangles with sharing edges
						for(int k1=0; k1<nVerts1; k1++)
						{//outer lightmap mesh polygon per vertex iteration
							CRadVertex& rVert1 = rPolyOuter.m_lstOriginals[k1];
							for(int k2=0; k2<nVerts2; k2++)
							{//inner lightmap mesh polygon per vertex iteration
								CRadVertex& rVert2 = rPolyInner.m_lstOriginals[k2];
								const float fDot = rVert1.m_vNormal * rVert2.m_vNormal;
								if
								(
									fabs(rVert1.m_vPos.x - rVert2.m_vPos.x) < epsPos &&
									fabs(rVert1.m_vPos.y - rVert2.m_vPos.y) < epsPos &&
									fabs(rVert1.m_vPos.z - rVert2.m_vPos.z) < epsPos &&
									fDot > epsNormalSmooth	
								)
								{
									//now check whether this TS was generated here or was correctly loaded
									if(cbTSGeneratedByLM)
									{
										//if shared vertex from pPatchOuter was already merged, use merged result to this shared one
										//otherwise compute and apply merged tangent space and normal
										if(!bMerged[k1] && rVert2.m_vNormal == rVert2.m_vTNormal)//if second vertex was merged, the tangent normal and vertex normal are different
										{
											MergeTangentSpace(rVert1, rVert2);
											bMerged[k1] = true;
										}
										else
										{
											rVert2.m_vNormal = rVert1.m_vNormal;
										}
									}

									vCount++;//found a shared vertex 
									bShareCondFound0 = true;
									if(vCount<2)
									{
										//save shared indices
										iFirstFoundIndexOuter = k1;
										iFirstFoundIndexInner = k2;
										continue;//shared vertex counter, to only consider triangles with sharing edges
									} 
									//now if both vertices are too close to the first shared one, iterate to the next vertex
									if(iFirstFoundIndexOuter == k1)//continue in outer loop
									{
										vCount--; //correct shared vertex count
										k2 = 3;	//force outer loop to kick in again
										break;
									}
									if(iFirstFoundIndexInner == k2)//continue in inner loop
									{
										vCount--;	//correct shared vertex count
										continue;
									}
									assert(iFirstFoundIndexOuter != -1 && iFirstFoundIndexInner != -1);
									pPatchOuter->m_dwFlags |= SHARED_FLAG; //mark as not to compress due to some sharing polygons
									pPatchInner->m_dwFlags |= SHARED_FLAG; //mark as not to compress due to some sharing polygons
									//both triangles share two vertices
									//prepare second pair values
									const unsigned int uiSecondOuter = ((iFirstFoundIndexOuter | (k1 << 8)) | ((iFirstFoundIndexInner | (k2 << 8)) << 16));
									const unsigned int uiSecondInner = ((iFirstFoundIndexInner | (k2 << 8)) | ((iFirstFoundIndexOuter | (k1 << 8)) << 16));
									rPolyOuter.m_SharingPolygons.push_back(std::pair<CRadPoly*,unsigned int>(&rPolyInner, uiSecondOuter));	//add this polygon to the respective list and vice versa
									rPolyInner.m_SharingPolygons.push_back(std::pair<CRadPoly*,unsigned int>(&rPolyOuter, uiSecondInner));
									bShareCond = true;
								}
								if(bShareCond)
									break;
							}//inner lightmap mesh polygon per vertex iteration
							if(bShareCond)
								break;
							if(bShareCondFound0)
							{
								//only one vertex was shared, do special encoding
								assert(iFirstFoundIndexOuter != -1 && iFirstFoundIndexInner != -1);
								pPatchOuter->m_dwFlags |= SHARED_FLAG; //mark as not to compress due to some sharing polygons
								pPatchInner->m_dwFlags |= SHARED_FLAG; //mark as not to compress due to some sharing polygons
								//both triangles share at least one vertex
								//prepare second pair values
								const unsigned int uiSecondOuter = ((iFirstFoundIndexOuter | (CRadPoly::scuiOneVertexShareFlag << 8)) | ((iFirstFoundIndexInner | (CRadPoly::scuiOneVertexShareFlag << 8)) << 16));
								const unsigned int uiSecondInner = ((iFirstFoundIndexInner | (CRadPoly::scuiOneVertexShareFlag << 8)) | ((iFirstFoundIndexOuter | (CRadPoly::scuiOneVertexShareFlag << 8)) << 16));
								rPolyOuter.m_SharingPolygons.push_back(std::pair<CRadPoly*,unsigned int>(&rPolyInner, uiSecondOuter));	//add this polygon to the respective list and vice versa
								rPolyInner.m_SharingPolygons.push_back(std::pair<CRadPoly*,unsigned int>(&rPolyOuter, uiSecondInner));
							}
						}//outer lightmap mesh polygon per vertex iteration		
					}//inner lightmap mesh polygon iteration
				}//outer lightmap mesh polygon iteration
			}//(iterPatchOuter != iterPatchInner)
		}//inner lightmap mesh loop
	}//outer lightmap mesh loop
}

bool CLightScene::ComputeRasterCube(CRasterCubeImpl *pRasterCube, 
									const std::set<IEntityRender *>& vGeom, 
									const Matrix33 *pDirLightTransf)
{
	// ---------------------------------------------------------------------------------------------
	// Build the Raster Cube structure
	// ---------------------------------------------------------------------------------------------

	UINT iNumTri;
	IStatObj *pObj = NULL;
	Vec3d vTri[3];
	const CObjFace *pFace = NULL;
	CIndoorArea *pArea = NULL;
	INT iCurFace = 0;
	bool bFirstPass;
	RasterCubeUserT sUserTri;
	Vec3d vEdge1, vEdge2;
	CMatInfo *pMat = NULL;
	IShader *pEff = NULL;
	IShader *pShTempl = NULL;
	UINT iNumNonOpaqueSkipped = 0;
	UINT iNumDecalsSkipped = 0;
	UINT iCurGeom = 0;
	IStatObj *pIGeom = NULL;
	Matrix44 matEtyMtx;

	if (vGeom.empty())
		return false;

	// ---------------------------------------------------------------------------------------------
	// Compute number of triangles in mesh
	// ---------------------------------------------------------------------------------------------
	std::set<IEntityRender *>::const_iterator itGeom;
	iNumTri = 0;
	for (std::set<IEntityRender *>::const_iterator itGeom=vGeom.begin(); itGeom!=vGeom.end(); itGeom++)
	{
		// Skip GLMs that are not marked as shadow casters
		if (((* itGeom)->GetRndFlags() & ERF_CASTSHADOWINTOLIGHTMAP) == 0)
			continue;

		iCurGeom = 0;
		while (pIGeom = (* itGeom)->GetEntityStatObj(iCurGeom++, NULL))
		{
			// Old IdxMesh: iNumTri += pIGeom->GetTriData()->m_nFaceCount;
			int iIdxCnt = 0;

			CLeafBuffer *pLB = pIGeom->GetLeafBuffer();
			if (pLB->IsEmpty())
				continue;
			pLB->GetIndices(&iIdxCnt);
			iNumTri += iIdxCnt / 3;
		}
	}
#ifdef DISPLAY_MORE_INFO
	_TRACE(m_LogInfo, true, "Raster cube contains %i triangles distributed over %i GLMs\r\n", iNumTri, vGeom.size());
#else
	_TRACE(m_LogInfo, false, "Raster cube contains %i triangles distributed over %i GLMs\r\n", iNumTri, vGeom.size());
#endif
	// ---------------------------------------------------------------------------------------------
	// Inititalize the raster cube with the bounding box and the face count
	// ---------------------------------------------------------------------------------------------
	Vec3d vMin(FLT_MAX, FLT_MAX, FLT_MAX);
	Vec3d vMax(FLT_MIN, FLT_MIN, FLT_MIN);
	for (std::set<IEntityRender *>::const_iterator itGeom=vGeom.begin(); itGeom!=vGeom.end(); itGeom++)
	{
		// Skip GLMs that are not marked as shadow casters
		if (((* itGeom)->GetRndFlags() & ERF_CASTSHADOWINTOLIGHTMAP) == 0)
			continue;

		IEntityRender *pCurEtyRend = (* itGeom);
		Vec3d vNewMin, vNewMax;

		// NOTE: World space AABB
		pCurEtyRend->GetBBox(vNewMin, vNewMax);

		vMin.x = __min(vMin.x, vNewMin.x);
		vMin.y = __min(vMin.y, vNewMin.y);
		vMin.z = __min(vMin.z, vNewMin.z);

		vMax.x = __max(vMax.x, vNewMax.x);
		vMax.y = __max(vMax.y, vNewMax.y);
		vMax.z = __max(vMax.z, vNewMax.z);
	}

	// Transform AABB to light space if required
	if (pDirLightTransf)
	{
		vMin = (* pDirLightTransf) * vMin;
		vMax = (* pDirLightTransf) * vMax;
	}
	if (!pRasterCube->Init(vMin, vMax, iNumTri))
		return false;
	// ---------------------------------------------------------------------------------------------
	// Add triangles
	// ---------------------------------------------------------------------------------------------
	bFirstPass = true;
	while (true)
	{
		for (std::set<IEntityRender *>::const_iterator itGeom=vGeom.begin(); itGeom!=vGeom.end(); itGeom++)
		{
			// Skip GLMs that are not marked as shadow casters
			if (((* itGeom)->GetRndFlags() & ERF_CASTSHADOWINTOLIGHTMAP) == 0)
				continue;

			iCurGeom = 0;
			while (pObj = (* itGeom)->GetEntityStatObj(iCurGeom++, &matEtyMtx))
			{
				if (pDirLightTransf)
					matEtyMtx *= (* pDirLightTransf);

				// IStatObj * pILODLevel = pObj->GetLodObject(0);

				CLeafBuffer *pLB = pObj->GetLeafBuffer();

				if (pLB->IsEmpty())
					continue;

				// Get indices and vertices
				int iIdxCnt = 0;
				unsigned short *pIndices = pLB->GetIndices(&iIdxCnt);
				int iVtxStride = 0;
				Vec3d *pVertices = reinterpret_cast<Vec3d *> (pLB->GetPosPtr(iVtxStride));

				// Loop through all materials
				UINT iCurMat;
				list2<CMatInfo> *pMaterials = pLB->m_pMats;
				for (iCurMat=0; iCurMat<pMaterials->Count(); iCurMat++)
				{
					CMatInfo cCurMat = (* pMaterials)[iCurMat];
					
					assert(cCurMat.nNumIndices % 3 == 0);
					assert(cCurMat.nFirstIndexId + cCurMat.nNumIndices <= iIdxCnt);

					// Check if the material is opaque, we don't let non-opaque geometry cast shadows
					pEff = cCurMat.shaderItem.m_pShader;
					if (pEff)
					{
						pShTempl = pEff->GetTemplate(-1);
						if (pShTempl)
						{
							CString sShaderName = pShTempl->GetName();
							if(sShaderName.Find("templdecal") != -1)  	//do not compute decals (i love exceptions)
							{
								iNumDecalsSkipped++;
								continue;
							}
							// Don't let faces cast shadows which are marked as unshadowing, invisible or not opaque
							if ((pShTempl->GetFlags2() & EF2_NOCASTSHADOWS) || 
								(pShTempl->GetFlags2() & EF2_OPAQUE) == 0 ||
								(pShTempl->GetFlags3() & EF3_NODRAW))
							{
								iNumNonOpaqueSkipped++;
								continue;
							}
						}
					}

					// Process all triangles
					UINT iCurTri = 0;
					for (iCurTri=0; iCurTri<cCurMat.nNumIndices / 3; iCurTri++)
					{
						UINT iBaseIdx = cCurMat.nFirstIndexId + iCurTri * 3;

						// Transform from object to world space
						Vec3d vWorldSpaceTri[3];
						vWorldSpaceTri[0] = matEtyMtx.TransformPointOLD
							(* reinterpret_cast<Vec3d *> (&(reinterpret_cast<byte *>(pVertices)[pIndices[iBaseIdx + 0] * iVtxStride])));
						vWorldSpaceTri[1] = matEtyMtx.TransformPointOLD
							(* reinterpret_cast<Vec3d *> (&(reinterpret_cast<byte *>(pVertices)[pIndices[iBaseIdx + 1] * iVtxStride])));
						vWorldSpaceTri[2] = matEtyMtx.TransformPointOLD
							(* reinterpret_cast<Vec3d *> (&(reinterpret_cast<byte *>(pVertices)[pIndices[iBaseIdx + 2] * iVtxStride])));

						// Vertices
						vTri[0].x = vWorldSpaceTri[0].x; 
						vTri[0].y = vWorldSpaceTri[0].y;
						vTri[0].z = vWorldSpaceTri[0].z;
						vTri[1].x = vWorldSpaceTri[1].x;
						vTri[1].y = vWorldSpaceTri[1].y;
						vTri[1].z = vWorldSpaceTri[1].z;
						vTri[2].x = vWorldSpaceTri[2].x;
						vTri[2].y = vWorldSpaceTri[2].y;
						vTri[2].z = vWorldSpaceTri[2].z;
						memcpy(&sUserTri.fVertices[0], &vTri[0], sizeof(float) * 3);
						memcpy(&sUserTri.fVertices[1], &vTri[1], sizeof(float) * 3);
						memcpy(&sUserTri.fVertices[2], &vTri[2], sizeof(float) * 3);

						// Face normal
						vEdge1.x = vTri[0].x - vTri[1].x;
						vEdge1.y = vTri[0].y - vTri[1].y;
						vEdge1.z = vTri[0].z - vTri[1].z;
						vEdge2.x = vTri[0].x - vTri[2].x;
						vEdge2.y = vTri[0].y - vTri[2].y;
						vEdge2.z = vTri[0].z - vTri[2].z;
						sUserTri.vNormal = vEdge1 ^ vEdge2;
						sUserTri.vNormal.Normalize();

						// Add it to the raster cube
						pRasterCube->PutInTriangle(vTri, sUserTri);
					}

				}
			}
		}
		// We need to Add/PreProcess twice
		if (bFirstPass)
		{
			char szDebugName[256];

#ifndef WIN64
			sprintf(szDebugName,"%x",pRasterCube);
#endif
			
#ifdef _DEBUG 
			if (!pRasterCube->PreProcess(szDebugName))
				return false;
#else 
			if (!pRasterCube->PreProcess(NULL))
				return false;
#endif
		}
		else
			break;
		bFirstPass = false;
	} 
	if (iNumNonOpaqueSkipped != 0)
#ifdef DISPLAY_MORE_INFO
		_TRACE(m_LogInfo, true, "Skipped %i non-opaque materials for shadow casting\r\n", iNumNonOpaqueSkipped / 2);
#else
		_TRACE(m_LogInfo, false, "Skipped %i non-opaque materials for shadow casting\r\n", iNumNonOpaqueSkipped / 2);
#endif
	if (iNumDecalsSkipped != 0)
#ifdef DISPLAY_MORE_INFO
		_TRACE(m_LogInfo, true, "Skipped %i decal materials for shadow casting\r\n", iNumDecalsSkipped / 2);
#else
		_TRACE(m_LogInfo, false, "Skipped %i decal materials for shadow casting\r\n", iNumDecalsSkipped / 2);
#endif
	return true;
}

//! \brief Only used local during generation of light clustering arrangement
struct LightCluster
{
	CRasterCubeImpl *pPointLtRC;
	std::vector<LMCompLight *> vLights;
	Vec3d vMin;
	Vec3d vMax;
	LightCluster() : vMin(FLT_MAX, FLT_MAX, FLT_MAX), vMax(FLT_MIN, FLT_MIN, FLT_MIN), pPointLtRC(NULL){}
};

//returns true if there is a conservative 2D sun space overlap
static const bool CheckOverlapInSunSpace(const Vec3d& rClusterMin, const Vec3d& rClusterMax, const Vec3d& rGLMMin, const Vec3d& rGLMMax, const Matrix33& rSunSpace, const bool cbUpdateMinMax)
{
	//first build vertices of cluster box
	static Vec3d vCluster[8];
	static float fMinXCluster, fMinYCluster, fMaxXCluster, fMaxYCluster;
	if(cbUpdateMinMax)
	{
		fMinXCluster = FLT_MAX; fMinYCluster = FLT_MAX; fMaxXCluster = -FLT_MAX; fMaxYCluster = -FLT_MAX;
		vCluster[0]/*Min,Min,Min*/ = vCluster[1] = vCluster[2] = vCluster[3] = rClusterMin;	
		vCluster[1].z = rClusterMax.z;/*Min,Min,Max*/
		vCluster[2].y = rClusterMax.y;/*Min,Max,Min*/
		vCluster[3].x = rClusterMax.x;/*Max,Min,Min*/
		vCluster[7] = vCluster[4] = vCluster[5] = vCluster[6] = rClusterMax;	/*Max,Max,Max*/
		vCluster[4].z = rClusterMin.z;/*Max,Max,Min*/
		vCluster[5].y = rClusterMin.y;/*Max,Min,Max*/
		vCluster[6].x = rClusterMin.x;/*Min,Max,Max*/
		//transform both into sun space
		for(int i=0;i<8;i++)
		{
			vCluster[i] = vCluster[i]	* rSunSpace;
		}
		for(int j=0;j<8;j++)
		{
			fMinXCluster	= (vCluster[j].x < fMinXCluster)?vCluster[j].x:fMinXCluster;
			fMinYCluster	= (vCluster[j].y < fMinYCluster)?vCluster[j].y:fMinYCluster;
			fMaxXCluster	= (vCluster[j].x > fMaxXCluster)?vCluster[j].x:fMaxXCluster;
			fMaxYCluster	= (vCluster[j].y > fMaxYCluster)?vCluster[j].y:fMaxYCluster;
		}
	}
	Vec3d vGLM[8];
	vGLM[0]/*Min,Min,Min*/ = vGLM[1] = vGLM[2] = vGLM[3] = rGLMMin;	
	vGLM[1].z = rGLMMax.z;/*Min,Min,Max*/
	vGLM[2].y = rGLMMax.y;/*Min,Max,Min*/
	vGLM[3].x = rGLMMax.x;/*Max,Min,Min*/
	vGLM[7] = vGLM[4] = vGLM[5] = vGLM[6] = rGLMMax;	/*Max,Max,Max*/
	vGLM[4].z = rGLMMin.z;/*Max,Max,Min*/
	vGLM[5].y = rGLMMin.y;/*Max,Min,Max*/
	vGLM[6].x = rGLMMin.x;/*Min,Max,Max*/
	//transform both into sun space
	for(int i=0;i<8;i++)
	{
		vGLM[i]		= vGLM[i]		* rSunSpace;
	}
	//get min max x and y
	float fMinXGLM = FLT_MAX, fMinYGLM = FLT_MAX, fMaxXGLM = -FLT_MAX, fMaxYGLM = -FLT_MAX;
	for(int j=0;j<8;j++)
	{
		fMinYGLM		= (vGLM[j].y < fMinYGLM)?vGLM[j].y:fMinYGLM;
		fMinXGLM		= (vGLM[j].x < fMinXGLM)?vGLM[j].x:fMinXGLM;		
		fMaxXGLM		= (vGLM[j].x > fMaxXGLM)?vGLM[j].x:fMaxXGLM;
		fMaxYGLM		= (vGLM[j].y > fMaxYGLM)?vGLM[j].y:fMaxYGLM;
	}
	// Check for overlap
	if (fMaxXGLM < fMinXCluster || fMaxYGLM < fMinYCluster)
		return false;
	if (fMinXGLM > fMaxXCluster || fMinYGLM > fMaxYCluster)
		return false;
	return true;
}

void CLightScene::SelectObjectsFromChangedLMShadowCaster(
	std::vector<std::pair<IEntityRender*, CBrushObject*> >&vNodes, 
	const std::vector<AABB>& vAABBs, 
	const Matrix33& rMatSunBasis,
	const std::vector<unsigned int>& rvNodeRadMeshIndices)
{
	//find out all objects which need to be detected due to receiving shadows from hash changed objects
	int iMeshCounter = -1;
	for (std::list<CRadMesh *>::iterator itMesh=m_lstRadMeshes.begin(); itMesh!=m_lstRadMeshes.end(); itMesh++)
	{
		iMeshCounter++;
		CRadMesh *pMesh = *itMesh;
		if(pMesh == NULL || !(pMesh->m_uiFlags & REBUILD_USED) || !(pMesh->m_uiFlags & HASH_CHANGED))
			continue;//was not valid or does not receive lightmaps
		//get bounding box for this glm
		const AABB& rAABBReceiver = vAABBs[iMeshCounter];
		//create shadow volumes for all lights
		std::vector<Shadowvolume> vSVs;	//shadow volumes
		//prepare shadow volumes
		bool bHasSunLight = false;
		for(std::vector<LMCompLight *>::const_iterator lightIter=pMesh->m_LocalLights.begin(); lightIter != pMesh->m_LocalLights.end(); ++lightIter)
		{
			const LMCompLight& rLight = *(*lightIter);
			if(rLight.eType == eDirectional)
			{
				bHasSunLight = true;
				continue;
			}
			Shadowvolume sv;
			NAABB_SV::AABB_ShadowVolume(rLight.vWorldSpaceLightPos, rAABBReceiver, sv, rLight.fRadius);//build occluder shadow volumes
			vSVs.push_back(sv);	
		}
		//iterate all glms and test against these shadow volumes
		int iCasterCounter = -1;
		for (std::list<CRadMesh *>::iterator itMeshSV=m_lstRadMeshes.begin(); itMeshSV!=m_lstRadMeshes.end(); itMeshSV++)
		{
			iCasterCounter++;
			CRadMesh *pShadowMesh = *itMeshSV;
			if(pShadowMesh == NULL || pShadowMesh == pMesh )
				continue;//just to make sure
			if((pShadowMesh->m_uiFlags & HASH_CHANGED) || !(pShadowMesh->m_uiFlags & RECEIVES_LIGHTMAP))
				continue;//don't test with itself or with objects not receiving lightmaps
			//get bounding box for this glm
			const AABB& rAABBcaster = vAABBs[iCasterCounter];
			//first check sun space
			if(bHasSunLight && (pShadowMesh->m_uiFlags & RECEIVES_SUNLIGHT))
			{
				if(CheckOverlapInSunSpace(rAABBReceiver.min, rAABBReceiver.max, rAABBcaster.min, rAABBcaster.max, rMatSunBasis, true))
				{
					pShadowMesh->m_uiFlags |= REBUILD_USED;
					pShadowMesh->m_bReceiveLightmap = true;
					continue;
				}
			}
			//now check all other lights
			bool bBreak = false;
			for(std::vector<Shadowvolume>::const_iterator iter = vSVs.begin(); iter != vSVs.end();++iter)
			{
				if(bBreak)
					break;
				if(NAABB_SV::Is_AABB_In_ShadowVolume(*iter, rAABBcaster))
				{
					pShadowMesh->m_uiFlags |= REBUILD_USED;
					pShadowMesh->m_bReceiveLightmap = true;
					bBreak = true;
					continue;
				}
			}
		}
	}	
}

void CLightScene::SelectObjectLMReceiverAndShadowCasterForChanges(
	std::vector<std::pair<IEntityRender*, CBrushObject*> >& vNodes, 
	const std::vector<AABB>& vAABBs, 
	const Matrix33& rMatSunBasis,
	const std::vector<unsigned int>& rvNodeRadMeshIndices,
	const ELMMode Mode)
{
	//gets called in case of not rebuildALL
	//goes through all radmeshes and for those who have been/needs to be changed, all objects get detected which are important for correct shadowing
	//for each object goes through all ligthsources and create frustum to check all other objects affecting lighting
	//flag for all objects whether there are needed or not at all
	int iMeshCounter = -1;
	for (std::list<CRadMesh *>::iterator itMesh=m_lstRadMeshes.begin(); itMesh!=m_lstRadMeshes.end(); itMesh++)
	{
		iMeshCounter++;
		CRadMesh *pMesh = *itMesh;
		if(pMesh == NULL || pMesh->m_bReceiveLightmap == false)
			continue;//was not valid or does not receive lightmaps
		//get bounding box for this glm
		const AABB& rAABBReceiver = vAABBs[iMeshCounter];
		//create shadow volumes for all lights
		std::vector<Shadowvolume> vSVs;	//shadow volumes
		//prepare shadow volumes
		bool bHasSunLight = false;
		for(std::vector<LMCompLight *>::const_iterator lightIter=pMesh->m_LocalLights.begin(); lightIter != pMesh->m_LocalLights.end(); ++lightIter)
		{
			const LMCompLight& rLight = *(*lightIter);
			if(rLight.eType == eDirectional)
			{
				bHasSunLight = true;
				continue;
			}
			Shadowvolume sv;
			NAABB_SV::AABB_ReceiverShadowVolume(rLight.vWorldSpaceLightPos, rAABBReceiver, sv);
			vSVs.push_back(sv);	
		}
		//iterate all glms and test against these shadow volumes
		int iCasterCounter = -1;
		for (std::list<CRadMesh *>::iterator itMeshSV=m_lstRadMeshes.begin(); itMeshSV!=m_lstRadMeshes.end(); itMeshSV++)
		{
			iCasterCounter++;
			CRadMesh *pShadowMesh = *itMeshSV;
			if(pShadowMesh == NULL || pShadowMesh == pMesh )
				continue;//just to make sure
			if((pShadowMesh->m_uiFlags & REBUILD_USED) || ((pShadowMesh->m_uiFlags & CAST_LIGHTMAP) == 0))
				continue;//don't test with itself or with objects not casting shadows
			if(pShadowMesh->m_bReceiveLightmap == true)
				continue;//don't waste time with objects receiving lightmaps as well
			//get bounding box for this glm
			const AABB& rAABBcaster = vAABBs[iCasterCounter];
			//first check sun space
			if(bHasSunLight && (pShadowMesh->m_uiFlags & RECEIVES_SUNLIGHT))
			{
				if(CheckOverlapInSunSpace(rAABBReceiver.min, rAABBReceiver.max, rAABBcaster.min, rAABBcaster.max, rMatSunBasis, true))
				{
					pShadowMesh->m_uiFlags |= REBUILD_USED;
					continue;
				}
			}
			//now check all other lights
			bool bBreak = false;
			for(std::vector<Shadowvolume>::const_iterator iter = vSVs.begin(); iter != vSVs.end();++iter)
			{
				if(bBreak)
					break;
				if(NAABB_SV::Is_AABB_In_ShadowVolume(*iter, rAABBcaster))
				{
					pShadowMesh->m_uiFlags |= REBUILD_USED;
					bBreak = true;
					continue;
				}
			}
		}
	}
	//in case of changes, all objects need to be detected which receive shadows from hash changed objects
	if(Mode == ELMMode_CHANGES)
		SelectObjectsFromChangedLMShadowCaster(vNodes, vAABBs, rMatSunBasis, rvNodeRadMeshIndices);
	//delete unneeded objects
	int iCounter = -1;
	for (std::list<CRadMesh *>::iterator itMesh=m_lstRadMeshes.begin(); itMesh!=m_lstRadMeshes.end(); itMesh++)
	{
		iCounter++;
		if(*itMesh && !((*itMesh)->m_uiFlags & REBUILD_USED))
		{
			delete *itMesh;*itMesh = NULL;//will get tested every time it gets accessed
			//remove from node list as well
			vNodes[rvNodeRadMeshIndices[iCounter]].first = NULL;//will get tested everytime it gets iterated
		}
	}
}

void CLightScene::CheckLight(LMCompLight& rLight, const int iCurLightIdx)
{
	// Too dark ?
	if (rLight.fColor[0] + rLight.fColor[1] + rLight.fColor[2] < 0.05f)
	{
		_TRACE(m_LogInfo, true, "WARNING: Light %i has little or no contribution to the scene, check light color\r\n", iCurLightIdx);
		rLight.fColor[0] = rLight.fColor[1] = rLight.fColor[2] = 0.03f;
		
		char text[scuiWarningTextAllocation];
		sprintf(text, "Light: %s has little or no contribution to the scene\r\n",(const char*)rLight.m_Name);
		m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_LIGHT_INTENSITY, std::string(text)));
	}

	// Radius invalid ?
	if (rLight.fRadius < 0.001f)
	{
		_TRACE(m_LogInfo, true, "WARNING: Light %i has negative or unreasonable small radius\r\n", iCurLightIdx);
		rLight.fRadius = 0.001f;
		char text[scuiWarningTextAllocation];
		sprintf(text, "Light: %s has negative or unreasonable small radius\r\n",(const char*)rLight.m_Name);
		m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_LIGHT_RADIUS, std::string(text)));
	}

	// Frustum invalid ?
    if (rLight.eType == eSpotlight)
	{
		if (rLight.fLightFrustumAngleDegree <= 0.0f || rLight.fLightFrustumAngleDegree > 89.9f )
		{
			_TRACE(m_LogInfo, true, "WARNING: Spotlight %i has an invalid frustum (%f°)\r\n", iCurLightIdx, rLight.fLightFrustumAngleDegree);
			rLight.fLightFrustumAngleDegree = 89.9f;
			char text[scuiWarningTextAllocation];
			sprintf(text, "Spotlight: %s has an invalid frustum (%f°)\r\n",(const char*)rLight.m_Name, rLight.fLightFrustumAngleDegree);
			m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_LIGHT_FRUSTUM, std::string(text)));
		}
	}
}

bool CLightScene::CreateFromEntity(const IndoorBaseInterface &pInterface, LMGenParam sParam,
	std::vector<std::pair<IEntityRender*, CBrushObject*> >& vNodes, CLMLightCollection& cLights,
	ICompilerProgress *pIProgress, const ELMMode Mode, 
	volatile SSharedLMEditorData *pSharedData, const std::set<const CBrushObject*>& vSelectionIndices,
	bool &rErrorsOccured)
{
	m_uiStartMemoryConsumption = 0;	//new run
	rErrorsOccured = false;
	// ---------------------------------------------------------------------------------------------
	// Compute LMs for vNodes
	// ---------------------------------------------------------------------------------------------
	Init();
	std::vector<std::pair<IEntityRender*, CBrushObject*> >::iterator itEty;
	CRadMesh *pMesh = NULL;
	IEntityRender *pIEtyRend = NULL;
	Matrix44 matEtyMtx;
	IStatObj *pIGeom = NULL;
	CRadPoly *pCurPoly = NULL;
	UINT iCurGeom;
	DWORD dwStartTime = GetTickCount();
	radmeshit itMesh;

	g_pIProgress = pIProgress;	 // TODO
	m_sParam = sParam;           // TODO

	m_LogInfo.clear();

	memcpy(&m_IndInterface, &pInterface, sizeof(IndoorBaseInterface));

	_TRACE(m_LogInfo, true, "Lightmap compiler started\r\n");

	if (!m_pISerializationManager)
		m_pISerializationManager = m_IndInterface.m_p3dEngine->CreateLMSerializationManager();
	assert(m_pISerializationManager != NULL);

	bool bReturn = true;
	unsigned int uiMeshesToProcess = 0;

	_TRACE(m_LogInfo, true, "Parameters:\r\n", m_sParam.m_fTexelSize);
	_TRACE(m_LogInfo, true, "Code Version = 4.0\r\n");
	_TRACE(m_LogInfo, true, "Texel size = %f\r\n", m_sParam.m_fTexelSize);
	_TRACE(m_LogInfo, true, "Texture resolution = %i\r\n", m_sParam.m_iTextureResolution);
	_TRACE(m_LogInfo, true, "Subsampling = %s\r\n", (m_sParam.m_iSubSampling == 9)?"on":"off");
	_TRACE(m_LogInfo, true, "Generate Occlusion maps = %s\r\n", (m_sParam.m_bGenOcclMaps)?"True":"False");
	_TRACE(m_LogInfo, true, "Shadows = %s\r\n", m_sParam.m_bComputeShadows ? "True" : "False");
	_TRACE(m_LogInfo, true, "Use sunLight = %s\r\n", m_sParam.m_bUseSunLight? "True" : "False");
	_TRACE(m_LogInfo, true, "Smoothing angle = %i degree\r\n", m_sParam.m_uiSmoothingAngle);
	_TRACE(m_LogInfo, true, "Use spotlights as pointlights = %s\r\n", m_sParam.m_bSpotAsPointlight ? "True" : "False");

	if(m_sParam.m_bUseSunLight)
		_TRACE(m_LogInfo, true, "\r\nProcessing %i GLMs / %i lights + sunlight + %i occlusion map lights\r\n", vNodes.size(), cLights.GetLights().size(), cLights.OcclLightSize()); 
	else
		_TRACE(m_LogInfo, true, "\r\nProcessing %i GLMs / %i lights + %i occlusion map lights\r\n", vNodes.size(), cLights.GetLights().size(), cLights.OcclLightSize());

	//load texture data if needed (for changes to compare hashs)
	if (Mode != ELMMode_ALL) m_pISerializationManager->Load(m_IndInterface.m_p3dEngine->GetFilePath(LM_EXPORT_FILE_NAME), false/*load no textures*/);

	// Check lights
	UINT iCurLightIdx = 0;
	for (std::vector<LMCompLight>::iterator itLight=cLights.GetLights().begin(); itLight!=cLights.GetLights().end(); itLight++)
	{
		CheckLight(*itLight, iCurLightIdx);
		iCurLightIdx++;
	}

	bool bNewZip = true;
	if (sParam.m_bOnlyExportLights || Mode != ELMMode_ALL)
		bNewZip = false;
	_TRACE(m_LogInfo, true, "Exporting static light sources to '%s'...\r\n", LM_STAT_LIGHTS_EXPORT_FILE_NAME);
	string strLMExportFile = m_IndInterface.m_p3dEngine->GetFilePath(LM_STAT_LIGHTS_EXPORT_FILE_NAME);
	if (!m_pISerializationManager->ExportDLights(strLMExportFile.c_str(), (const CDLight **) &cLights.GetSrcLights()[0], 
		(UINT) cLights.GetSrcLights().size(),bNewZip))
	{
		assert(false);
		_TRACE(m_LogInfo, true, "ERROR: Exporting of lightsources to '%s' failed !\r\n", strLMExportFile.c_str());
		char text[scuiWarningTextAllocation];
		sprintf(text, "Exporting of lightsources to '%s' failed !\r\n", strLMExportFile.c_str());
		m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_LIGHT_EXPORT_FAILED, std::string(text)));
	}

	if (sParam.m_bOnlyExportLights)
	{
		rErrorsOccured = (m_WarningInfo.size() != 0);
		return true;
	}
	if(pSharedData != NULL)
		DisplayMemStats(pSharedData);

	// Add sun light
	LMCompLight sSunLight;
	if (m_sParam.m_bUseSunLight) 
	{
		sSunLight.fColor[0] = m_IndInterface.m_p3dEngine->GetSunColor().x * 0.5f;
		sSunLight.fColor[1] = m_IndInterface.m_p3dEngine->GetSunColor().y * 0.5f;
		sSunLight.fColor[2] = m_IndInterface.m_p3dEngine->GetSunColor().z * 0.5f;
		
		sSunLight.eType = eDirectional;
		sSunLight.vDirection = Vec3d(0, 0, 0) - m_IndInterface.m_p3dEngine->GetSunPosition();
		sSunLight.vDirection.Normalize(); 
		sSunLight.fRadius = FLT_MAX;

		sSunLight.m_bFakeRadiosity = false;
		sSunLight.m_bDot3			= true;
		sSunLight.m_bOcclType = true;

		sSunLight.m_CompLightID.first = 0xFFFF;
		sSunLight.m_CompLightID.second = 0xFFFF; 

		cLights.GetLights().push_back(sSunLight);

		_TRACE(m_LogInfo, true, "Sunlight with direction %f %f %f added\r\n", 
			sSunLight.vDirection.x, sSunLight.vDirection.y, sSunLight.vDirection.z);
	}

	_TRACE(m_LogInfo, true, "Preparing meshes (tangents, AABBs, plane equations, local lightsources, etc.)... ");
	std::vector<AABB> vAABBs;
	vAABBs.reserve(vNodes.size());
	std::vector<unsigned int> vNodeRadMeshIndices;	vNodeRadMeshIndices.reserve(vNodes.size());//saves per added mesh the node index
	int iNodeIndex= -1;
	for (itEty=vNodes.begin(); itEty!=vNodes.end(); itEty++)
	{
		iNodeIndex++;
		pIEtyRend = itEty->first;
		if(pIEtyRend == NULL)
			continue;

		iCurGeom = 0;
		if(pSharedData != NULL)
			DisplayMemStats(pSharedData);
		if(pSharedData != NULL && pSharedData->bCancelled == true)
		{
			break;
		}

		pIGeom = pIEtyRend->GetEntityStatObj(iCurGeom++, &matEtyMtx);
		if(pIGeom == NULL)
			continue;
		pMesh = new CRadMesh;

		std::vector<LMCompLight>::iterator itLight;
		// Add the local lightsources (based on attenuation)
		for (itLight=cLights.GetLights().begin(); itLight!=cLights.GetLights().end(); itLight++)
		{
			LMCompLight& cCurLight = (* itLight);
			Vec3d vNewMin, vNewMax;

			// Don't add sunlight for GLMs which are in a vis area and are not connected to the outdoors
			if (pIEtyRend->GetEntityVisArea() != NULL && cCurLight.eType == eDirectional)
				if (!pIEtyRend->GetEntityVisArea()->IsConnectedToOutdoor())
					continue;

			if(cCurLight.eType == eDirectional)
			{
				pMesh->m_uiFlags |= RECEIVES_SUNLIGHT;
				pMesh->m_uiFlags |= ONLY_SUNLIGHT;
				pMesh->m_LocalLights.push_back(&cCurLight);
				pMesh->m_LocalOcclLights.push_back(&cCurLight);
				continue;
			}

			pIEtyRend->GetBBox(vNewMin, vNewMax);

			if (Overlap::Sphere_AABB(
				Sphere(cCurLight.vWorldSpaceLightPos, cCurLight.fRadius),
				MakeSafeAABB(vNewMin, vNewMax)))
			{
				if(cCurLight.uiFlags & DLF_THIS_AREA_ONLY)
				{
					if(cCurLight.pVisArea != 0 && pIEtyRend->GetEntityVisArea()!=0)
					{
						if(pIEtyRend->GetEntityVisArea() != cCurLight.pVisArea)
						{
							// try also portal volumes
							const bool bNearFound = 
								pIEtyRend->GetEntityVisArea()->FindVisArea((IVisArea * )(cCurLight.pVisArea), 
									1+/*int((cLight.uiFlags & DLF_THIS_AREA_ONLY)==0)+*/int(((IVisArea * )(cCurLight.pVisArea))->IsPortal()), false);
							if(!bNearFound)
								continue; // areas do not much
						}
					}
		 			else
					if(cCurLight.pVisArea != 0 || pIEtyRend->GetEntityVisArea()!=0)
					{
						continue;
					}
				}
				pMesh->m_uiFlags &= ~ONLY_SUNLIGHT;
				if(cCurLight.m_bCastShadow)
					pMesh->m_LocalLights.push_back(&cCurLight);
				if(cCurLight.m_bOcclType)
					pMesh->m_LocalOcclLights.push_back(&cCurLight);
			} 
		}

		// Call after putting in the light sources to get the right HashValue
		pMesh->CreateEmpty(pIEtyRend, matEtyMtx);
		const CBrushObject *pBrushObject = itEty->second;
		if(!pMesh->FillInValues(pIEtyRend, pBrushObject, matEtyMtx) || pMesh->m_lstOldPolys.size() <= 0)
		{
			delete pMesh;	pMesh = NULL;
			//remove from node list
			itEty->first = NULL;//will get tested everytime it gets iterated
		}
		else
		{//if at least one polygon has been added
			m_lstRadMeshes.push_back(pMesh);
			//now check for update demand
			if (Mode == ELMMode_CHANGES)
			{
				DWORD dwOldHash = m_pISerializationManager->GetHashValue(pIEtyRend->GetEditorObjectId());
				DWORD dwNewHash = pMesh->GetHashValue();

				if (dwOldHash == dwNewHash)
				{
					pMesh->m_bReceiveLightmap = false;
				}
				else
				{
					pMesh->m_uiFlags |= (REBUILD_USED | HASH_CHANGED);
					uiMeshesToProcess++;
				}
			}
			else
			{
				if(Mode == ELMMode_SEL)
				{
					if(vSelectionIndices.find(pBrushObject) == vSelectionIndices.end())
						pMesh->m_bReceiveLightmap = false;//this will mark this glm as not selected
					else
					{
						pMesh->m_uiFlags |= REBUILD_USED;
						uiMeshesToProcess++;
					}
				}
	 
			}
			//create bounding box
			Vec3 min, max;
			pIEtyRend->GetBBox(min, max);
			vAABBs.push_back(MakeSafeAABB(min, max));
			vNodeRadMeshIndices.push_back((unsigned int)iNodeIndex);
		}
	}
	// Create light space transform for sun light
	Matrix33 matSunBasis;
	Vec3 vAxisA, vAxisB;
	GetOtherBaseVec(sSunLight.vDirection, vAxisA, vAxisB);
	matSunBasis.SetColumn(0, vAxisA);
	matSunBasis.SetColumn(1, vAxisB);
	matSunBasis.SetColumn(2, sSunLight.vDirection);
	matSunBasis.Transpose();

	if (Mode != ELMMode_ALL)
		SelectObjectLMReceiverAndShadowCasterForChanges(vNodes, vAABBs, matSunBasis, vNodeRadMeshIndices, Mode);
	
	if(pSharedData != NULL)
		DisplayMemStats(pSharedData);
	
	_TRACE(m_LogInfo, true, "done\r\n");
	if(m_sParam.m_bComputeShadows && (pSharedData == NULL || (pSharedData != NULL && pSharedData->bCancelled == false)))
	{
		if(pSharedData != NULL)
			DisplayMemStats(pSharedData);

		_TRACE(m_LogInfo, true, "\r\nComputing clusters...");
		{
			// Make each GLM a cluster
			for (itEty=vNodes.begin(); itEty!=vNodes.end(); itEty++)
			{
				IEntityRender *pCurEtyRend = itEty->first;
				if(pCurEtyRend == NULL)
					continue;
				Vec3 vNewMin, vNewMax;
				GLMCluster sNewCluster;
				
				pCurEtyRend->GetBBox(vNewMin, vNewMax);

				sNewCluster.vMin = vNewMin;
				sNewCluster.vMax = vNewMax;

				sNewCluster.vGLMsAffectingCluster.insert(pCurEtyRend);

				m_lstClusters.push_back(sNewCluster);
			}

			std::list<GLMCluster>::iterator itCluster, itClusterSearch,itClusterStart;

			// Merge clusters based on spatial proximity
			const float fMaxClusterSize = 25.0f;
#ifdef DISPLAY_MORE_INFO
			_TRACE(m_LogInfo, true, "Merge clusters based on spatial proximity (MaxClusterSize = %f)...\r\n", fMaxClusterSize);
#else
			_TRACE(m_LogInfo, false, "Merge clusters based on spatial proximity (MaxClusterSize = %f)...\r\n", fMaxClusterSize);
#endif
			for (itCluster=m_lstClusters.begin(); itCluster!=m_lstClusters.end(); ++itCluster)
			{
				if(pSharedData != NULL)
					DisplayMemStats(pSharedData);

				itClusterStart=itCluster;++itClusterStart;
				for (itClusterSearch=itClusterStart; itClusterSearch!=m_lstClusters.end();)
				{
					Vec3 vOldDimensions = (* itCluster).vMax - (* itCluster).vMin;

					vOldDimensions[0] = (* itCluster).vMax.x - (* itCluster).vMin.x;
					vOldDimensions[1] = (* itCluster).vMax.y - (* itCluster).vMin.y;
					vOldDimensions[2] = (* itCluster).vMax.z - (* itCluster).vMin.z;

					Vec3 vMin, vMax; // Bounding box
					
					vMin.x = __min((* itCluster).vMin.x, (* itClusterSearch).vMin.x);
					vMin.y = __min((* itCluster).vMin.y, (* itClusterSearch).vMin.y);
					vMin.z = __min((* itCluster).vMin.z, (* itClusterSearch).vMin.z);

					vMax.x = __max((* itCluster).vMax.x, (* itClusterSearch).vMax.x);
					vMax.y = __max((* itCluster).vMax.y, (* itClusterSearch).vMax.y);
					vMax.z = __max((* itCluster).vMax.z, (* itClusterSearch).vMax.z);

					Vec3 vNewDimensions = vMax - vMin;

					// Already too long side would be even longer -> skip merge process
					if (vOldDimensions.x > fMaxClusterSize && vNewDimensions.x>vOldDimensions.x) { ++itClusterSearch; continue; }
					if (vOldDimensions.y > fMaxClusterSize && vNewDimensions.y>vOldDimensions.y) { ++itClusterSearch; continue; }
					if (vOldDimensions.z > fMaxClusterSize && vNewDimensions.z>vOldDimensions.z) { ++itClusterSearch; continue; }

					const float fIntersectEpsilon = 1.0f;
					const Vec3 vIntersectEpsilon(fIntersectEpsilon, fIntersectEpsilon, fIntersectEpsilon);

					if (Overlap::AABB_AABB(
						Vec3(0, 0, 0), AABB((* itCluster).vMin - vIntersectEpsilon, (* itCluster).vMax + vIntersectEpsilon),
						Vec3(0, 0, 0), AABB((* itClusterSearch).vMin - vIntersectEpsilon, (* itClusterSearch).vMax + vIntersectEpsilon)))
					{
						// Merge AABB
						(* itCluster).vMin = vMin;
						(* itCluster).vMax = vMax;

						// Merge GLM lists
						for (std::set<IEntityRender*>::iterator it = (* itClusterSearch).vGLMsAffectingCluster.begin();
							it != (* itClusterSearch).vGLMsAffectingCluster.end(); ++it)
						{
							(* itCluster).vGLMsAffectingCluster.insert( *it );
						}

						if (itClusterSearch == itClusterStart) // delete the first element?
							++itClusterStart;

						// Remove original
						m_lstClusters.erase(itClusterSearch);

						itClusterSearch = itClusterStart; // Restart  
						continue;
					}

					++itClusterSearch;
				}
			}
			_TRACE(m_LogInfo, true, "done\r\n");
			if(m_lstClusters.size() > 1)
				_TRACE(m_LogInfo, true, "%i clusters created\r\n", m_lstClusters.size());
			else
				if(m_lstClusters.size() == 1)
					_TRACE(m_LogInfo, true, "1 cluster created\r\n");

			// Put all GLMs in clusters which can potentially shadow each other (overlap along the light's axis)
			if (m_sParam.m_bUseSunLight)
			{
				UINT iNumPotentialShadowCasters = 0;

#ifdef DISPLAY_MORE_INFO
				_TRACE(m_LogInfo, true, "Put all GLMs in clusters which can potentially shadow each other because of sunlight...\r\n");
#else
				_TRACE(m_LogInfo, false, "Put all GLMs in clusters which can potentially shadow each other because of sunlight...\r\n");
#endif
				for (itCluster=m_lstClusters.begin(); itCluster!=m_lstClusters.end(); ++itCluster)
				{
					bool bUpdateMinMax = true;					
					for (itEty=vNodes.begin(); itEty!=vNodes.end(); itEty++)
					{
						IEntityRender *pCurEtyRend = itEty->first;
						if(pCurEtyRend == NULL)
							continue;
						Vec3 vGLMLightSpaceMin, vGLMLightSpaceMax;
						pCurEtyRend->GetBBox(vGLMLightSpaceMin, vGLMLightSpaceMax);
						if(!CheckOverlapInSunSpace((* itCluster).vMin, (* itCluster).vMax, vGLMLightSpaceMin, vGLMLightSpaceMax, matSunBasis, bUpdateMinMax))
						{
							bUpdateMinMax = false;
							continue;
						}
		
						bUpdateMinMax = true;
						(* itCluster).vGLMsAffectingCluster.insert(pCurEtyRend);
						iNumPotentialShadowCasters++;
					}
				}

				_TRACE(m_LogInfo, true, "%i potential sunlight shadow casters added\r\n", iNumPotentialShadowCasters);
			}

			// Add shadow casters to clusters
#ifdef DISPLAY_MORE_INFO
			_TRACE(m_LogInfo, true, "Put GLMs in clusters which can potentially shadow on them...\r\n");
#else
			_TRACE(m_LogInfo, false, "Put GLMs in clusters which can potentially shadow on them...\r\n");
#endif
			for (itCluster=m_lstClusters.begin(); itCluster!=m_lstClusters.end(); ++itCluster)
			{
				if(pSharedData != NULL)
					DisplayMemStats(pSharedData);

				std::vector<Shadowvolume> vSVs;	//shadow volumes
				AABB aabbCluster((* itCluster).vMin, (* itCluster).vMax);

				// Find all light sources which overlap with this cluster
				for (std::vector<LMCompLight>::const_iterator itLight=cLights.GetLights().begin(); itLight!=cLights.GetLights().end(); itLight++)
				{
					if ((*itLight).eType != ePoint && (*itLight).eType != eSpotlight) 
						continue;

					if (Overlap::Sphere_AABB(
						Sphere((* itLight).vWorldSpaceLightPos, (* itLight).fRadius),
						MakeSafeAABB((* itCluster).vMin, (* itCluster).vMax)))
					{
						// Light lits cluster, create shadow volume
						Shadowvolume sv;
						NAABB_SV::AABB_ReceiverShadowVolume((*itLight).vWorldSpaceLightPos, aabbCluster, sv);
						vSVs.push_back(sv);	
					}
				}

				// Find new shadow casters and merge list with old set
				for (itEty=vNodes.begin(); itEty!=vNodes.end(); itEty++)
				{
					IEntityRender *pCurEtyRend = itEty->first;
					if(pCurEtyRend == NULL)
						continue;
					Vec3 vEtyMin, vEtyMax;
					pCurEtyRend->GetBBox(vEtyMin, vEtyMax);
					const AABB aabbCaster(MakeSafeAABB(vEtyMin, vEtyMax));
					//now check all other lights
					bool bBreak = false;
					for(std::vector<Shadowvolume>::const_iterator iter = vSVs.begin(); iter != vSVs.end();++iter)
					{
						if(bBreak)
							break;
						if(NAABB_SV::Is_AABB_In_ShadowVolume(*iter, aabbCaster))
						{
							bBreak = true;
							(* itCluster).vGLMsAffectingCluster.insert(pCurEtyRend);
							continue;
						}
					}
				}
			}
			// Compute raster cubes for clusters
			UINT iNumClusters = m_lstClusters.size();
			UINT iCurCluster = 0;
			_TRACE(m_LogInfo, true, "Computing raster cubes for light cluster...");
			for (itCluster=m_lstClusters.begin(); itCluster!=m_lstClusters.end(); itCluster++)
			 {
				if(pSharedData != NULL)
					DisplayMemStats(pSharedData);
				if(pSharedData != NULL && pSharedData->bCancelled == true)
				{
					break;
				}
				// AABB in world space
				AABB cWorldSpaceAABB(MakeSafeAABB((* itCluster).vMin, (* itCluster).vMax));

				CRasterCubeImpl* pImpl = m_RasterCubeManager.CreateRasterCube();
				// Find GLMs in RC AABB and assign this RC to them
				std::list<CRadMesh *>::iterator itMesh;
				UINT iNumGLMsContained = 0;
				for (itMesh=m_lstRadMeshes.begin(); itMesh!=m_lstRadMeshes.end(); itMesh++)
				{
					if(*itMesh == NULL)
						continue;
					assert((* itMesh)->m_pESource);
					IEntityRender *pCurEtyRend = (* itMesh)->m_pESource;
					Vec3 vGLMMin, vGLMMax;

					pCurEtyRend->GetBBox(vGLMMin, vGLMMax);

					if (vGLMMin.x < cWorldSpaceAABB.min.x ||
						vGLMMin.y < cWorldSpaceAABB.min.y ||
						vGLMMin.z < cWorldSpaceAABB.min.z)
					{
						continue;
					}

					if (vGLMMax.x > cWorldSpaceAABB.max.x ||
						vGLMMax.y > cWorldSpaceAABB.max.y ||
						vGLMMax.z > cWorldSpaceAABB.max.z)
					{
						continue;
					}

					(* itMesh)->m_pClusterRC = pImpl;
					m_RasterCubeManager.AddReference((* itMesh));

					iNumGLMsContained++;
				}
#ifdef DISPLAY_MORE_INFO
				_TRACE(m_LogInfo, true, "Computing raster cube for light cluster %i of %i with %i GLMs contained in it / %i GLMs affecting it located" \
					" at (%f, %f, %f) - (%f, %f, %f)\r\n", 
					++iCurCluster, iNumClusters, iNumGLMsContained,  (* itCluster).vGLMsAffectingCluster.size(),
					(* itCluster).vMin.x, (* itCluster).vMin.y, (* itCluster).vMin.z,
					(* itCluster).vMax.x, (* itCluster).vMax.y, (* itCluster).vMax.z);
#else
				_TRACE(m_LogInfo, false, "Computing raster cube for light cluster %i of %i with %i GLMs contained in it / %i GLMs affecting it located" \
					" at (%f, %f, %f) - (%f, %f, %f)\r\n", 
					++iCurCluster, iNumClusters, iNumGLMsContained,  (* itCluster).vGLMsAffectingCluster.size(),
					(* itCluster).vMin.x, (* itCluster).vMin.y, (* itCluster).vMin.z,
					(* itCluster).vMax.x, (* itCluster).vMax.y, (* itCluster).vMax.z);
#endif
				ComputeRasterCube(pImpl, (* itCluster).vGLMsAffectingCluster);
			}
		}
	}

	if(pSharedData == NULL || (pSharedData != NULL && pSharedData->bCancelled == false))
	{
		if(pSharedData != NULL)
			DisplayMemStats(pSharedData);

		// Compute BB
		Vec3d vMinBB(FLT_MAX, FLT_MAX, FLT_MAX);
		Vec3d vMaxBB(FLT_MIN, FLT_MIN, FLT_MIN);
		for (itEty=vNodes.begin(); itEty!=vNodes.end(); itEty++)
		{
			IEntityRender *pCurEtyRend = itEty->first;
			if(pCurEtyRend == NULL)
				continue;

			Vec3d vNewMin, vNewMax;

			pCurEtyRend->GetBBox(vNewMin, vNewMax);

			vMinBB.x = __min(vMinBB.x, vNewMin.x);
			vMinBB.y = __min(vMinBB.y, vNewMin.y);
			vMinBB.z = __min(vMinBB.z, vNewMin.z);

			vMaxBB.x = __max(vMaxBB.x, vNewMax.x);
			vMaxBB.y = __max(vMaxBB.y, vNewMax.y);
			vMaxBB.z = __max(vMaxBB.z, vNewMax.z);
		}

		if (!Create(pInterface, vMinBB, vMaxBB, pSharedData, Mode, (Mode == ELMMode_ALL)?m_lstRadMeshes.size() : uiMeshesToProcess))
		{
			rErrorsOccured = (m_WarningInfo.size() != 0);
			bReturn = false;
		}
		if(pSharedData != NULL && pSharedData->bCancelled)
		{
			//free memory
			for (radpolyit i=m_lstScenePolys.begin();i!=m_lstScenePolys.end();++i)
			{
				CRadPoly *pPoly=(*i);
				//no lightmap for this poly
				delete pPoly; *i = pPoly = NULL;
			}
			m_lstScenePolys.clear();
			m_lstAssignQueue.clear();
			delete m_pCurrentPatch;	m_pCurrentPatch = NULL;
		}
	}

	// Free raster cubes for light clusters
	m_RasterCubeManager.Clear();
	//make sure all rad meshes get deallocated
	for (radmeshit itMesh=m_lstRadMeshes.begin(); itMesh!=m_lstRadMeshes.end(); itMesh++)
	{
		delete *itMesh;
		*itMesh = NULL;
	}

	Reset();

	//release for the sake of memory deallocation
	if (m_pISerializationManager)
	{
		m_pISerializationManager->Release();
		m_pISerializationManager = NULL;
	}

	if(pSharedData != NULL)
		DisplayMemStats(pSharedData);

	if(bReturn)
	{
		unsigned int uiSec = (GetTickCount() - dwStartTime + 499) / 1000;
		const unsigned int cuiMinutes = uiSec / 60;
		uiSec -= cuiMinutes * 60;
		_TRACE(m_LogInfo, true, "Total computation time was %i min  %i s\r\n", cuiMinutes, uiSec);
		_TRACE(m_LogInfo, true, "Lightmap compiler successfully finished\r\n");
		_TRACE("Updating / reloading textures...\r\n");
	}

	rErrorsOccured = (m_WarningInfo.size() != 0);
	//now write log info
	WriteLogInfo();
		
	return bReturn;
}

void CLightScene::Shadow(LMCompLight& cLight, UINT& iNumHit, CRadMesh *pMesh, CRadPoly *pSource, const CRadVertex& Vert, const Vec3d& vSamplePos)
{
	if (pMesh->m_pClusterRC == NULL)
		iNumHit++;

	CAnyHit cAnyHit;
	// Ray from vertex to light surface sample point
	// Vec3d vRayDir = pLight->m_vObjectSpacePos - vSamplePos;
	// Vec3d vRayDirNormalized = pLight->m_vObjectSpacePos - vSamplePos;

	Vec3d vRayDir, vRayDirNormalized;
	float fRayLen;
	if (cLight.eType == eDirectional)
	{
		vRayDirNormalized = vRayDir = -cLight.vDirection/* * 1500.0f*/; // TODO
		fRayLen = 2000.0f;
	}
	else
	{
		vRayDirNormalized = vRayDir = cLight.vWorldSpaceLightPos - vSamplePos;
		vRayDirNormalized.Normalize();
		fRayLen = vRayDir.Length();
	}
	Vec3d vRayOrigin = vSamplePos;
	cAnyHit.SetupRay(vRayDirNormalized, 
		vRayOrigin,
		fRayLen,
		m_sParam.m_fTexelSize,
		pSource);

	// Plane equation
	// TODO: Move into SetupRay()
//	 cAnyHit.m_vPNormal = Vert.m_vNormal;
	cAnyHit.m_fD = -(pSource->m_Plane.n * Vert.m_vPos); // Inverted ?
	cAnyHit.m_vPNormal = pSource->m_Plane.n;

	// Check the last intersection first to exploit coherence
	bool bSkip = false;
	if ((cLight.m_pLastIntersectionRasterCube == pMesh->m_pClusterRC) && cLight.m_pLastIntersection)//do not use cached result from deleted rastercube
	{
		float fDist = FLT_MAX;
		cAnyHit.ReturnElement(* cLight.m_pLastIntersection, fDist);

		if (cAnyHit.IsIntersecting())
			bSkip = true;
	}

	if (!bSkip)
	{
		if (cLight.eType == eDirectional)
		{
			// Use raster cube of cluster for current mesh
			if (pMesh->m_pClusterRC != NULL)
			{
				// Check occlusion
				pMesh->m_pClusterRC->GatherRayHitsDirection(
					CVector3D(vRayOrigin.x, vRayOrigin.y, vRayOrigin.z),
					CVector3D(vRayDirNormalized.x, vRayDirNormalized.y, vRayDirNormalized.z),
					cAnyHit);

				if (!cAnyHit.IsIntersecting()) 
					iNumHit++;
				else  
				{
					cLight.m_pLastIntersection = cAnyHit.m_pHitResult;
					cLight.m_pLastIntersectionRasterCube = pMesh->m_pClusterRC;
				}
			}
			else
				iNumHit++;
		}
		else
		{
			if (pMesh->m_pClusterRC != NULL)
			{
				// Check occlusion using raster cube for the current lightsource
				pMesh->m_pClusterRC->GatherRayHitsDirection(
					CVector3D(vRayOrigin.x, vRayOrigin.y, vRayOrigin.z),
					CVector3D(vRayDirNormalized.x, vRayDirNormalized.y, vRayDirNormalized.z),
					cAnyHit);

				if (!cAnyHit.IsIntersecting()) 
					iNumHit++;
				else
				{
					cLight.m_pLastIntersection = cAnyHit.m_pHitResult;
					cLight.m_pLastIntersectionRasterCube = pMesh->m_pClusterRC;
				}
			}
			else
				iNumHit++;
		}
	}
}

static const bool GetLightIntensity(const LMCompLight& rLight, float& rfIntens, const CRadVertex& rVertex, const Vec3d& rvLightDir )
{
	rfIntens = 1.0f;
	// Backface culling
	float fCos = rvLightDir * rVertex.m_vNormal;
	if (fCos <= 0.01f)
		return false;
	if (rLight.eType != eDirectional)
	{
		const float fRadius = rLight.fRadius;
		const float fRadius2 = fRadius * fRadius;
		const float fDist2 = GetSquaredDistance(rLight.vWorldSpaceLightPos, rVertex.m_vPos);
		if (fDist2 > fRadius2)
			return false; // Surface point outside of light radius
		assert(!_isnan(fRadius));
		assert(fRadius > 0);
		if (fRadius <= 0)
			return false;
		// 1/linear falloff
		if(fDist2 > 0.0001f)
			rfIntens = 1.0f - sqrtf(fDist2) / (fRadius);
	}
	return true;
}

static inline const bool SumDot3Values(const LMCompLight& cLight, float& fIntens, float& fLambLumSum, float& fLumSum, float& fColorRLamb, float& fColorGLamb, float& fColorBLamb, const LMCompLight& rLight, Vec3d& rvSumLightDir, const Vec3d& rvLightDir, const CRadVertex& rVertex)
{
	bool bApplyBetterLighting = false;
	// ---------------------------------------------------------------------------------------------
	// Add in light color and direction
	// ---------------------------------------------------------------------------------------------
	{
		// Divide by 2 to compensate for higher scaling during rendering
		const float fCos = rvLightDir * rVertex.m_vNormal;
		float fLambert = 1.0f;
		if(!cLight.m_bFakeRadiosity)//apply better_lighting model
		{
			// Modulate intensity with Lambert factor
			fLambert = max(0,fCos);
			if(cLight.m_bDot3)
				bApplyBetterLighting = true;
		}
		else
		{
			if (fCos <= 0.0001f)
				fLambert = 0.0f;
		} 
#ifdef APPLY_COLOUR_FIX
		fColorRLamb += rLight.fColor[0] * fLambert * fIntens;
		fColorGLamb += rLight.fColor[1] * fLambert * fIntens;
		fColorBLamb += rLight.fColor[2] * fLambert * fIntens;
		fIntens *= 0.5f;
#else
		fIntens *= 0.5f;
		fColorRLamb += rLight.fColor[0] * fLambert * fIntens;
		fColorGLamb += rLight.fColor[1] * fLambert * fIntens;
		fColorBLamb += rLight.fColor[2] * fLambert * fIntens;
#endif		
		// Accumulate light direction, weight based on amount of light arriving at the vertex		

		float fLightLum = (rLight.fColor[0] + rLight.fColor[1] + rLight.fColor[2]) * fIntens * LightInfluence(fCos);
		if(cLight.m_bDot3)//has influence to the dot3 vector
		{
			rvSumLightDir += rvLightDir * fLightLum;
			fLambLumSum+= fLambert*fLightLum;
		}
		else
		{
			rvSumLightDir += rVertex.m_vNormal * fLightLum;
			fLambLumSum += fLightLum;//don't apply lambert factor here
		}
		fLumSum += fLightLum;
	}
	return bApplyBetterLighting;
}

static void GetDot3Sample(float& fLM_DP3LerpFactor, Vec3d& rvLightDir, float fLumSum, float fLambLumSum, const CRadVertex& rVert, float& fColorRLamb, float& fColorGLamb, float& fColorBLamb, const bool cbApplyBetterLighting = false)
{
	// ---------------------------------------------------------------------------------------------
	// DOT3 light direction / lerp factor
	// ---------------------------------------------------------------------------------------------
	{
		// Intensity of DOT3 is based on how accurate our light vector represents the sum of all
		// light vectors. In the ideal case (all light vectors from the same direction) the length
		// of the accumulated light vector (fLen) is equal to the length of all individual light vectors (fLumSum)
		static const float scfLumSumThreshold = 0.01f;
		if (fLumSum < scfLumSumThreshold)
		{
			fLM_DP3LerpFactor = 0.0f;
			fLumSum = 0.f;
			rvLightDir.x = rvLightDir.y = rvLightDir.z = 0.f;
		}
		else
		{
			const float fLen = rvLightDir.Length();
			fLM_DP3LerpFactor = fLen / fLumSum;
			rvLightDir.Normalize();

#ifndef REDUCE_DOT3LIGHTMAPS_TO_CLASSIC
			if(cbApplyBetterLighting)	//if to reduce to classic lightmaps we need to apply the new lighting model to take the normals into account
#endif
			{
				// Calculate lightmap correction factor (lambert calculation)
				// *fLumSum/max(0.01f,fLambLumSum)                     to correct the already applied lambert calculation
				// (Vert.m_vNormal*rvLightDir) * fLM_DP3LerpFactor      to ???
				const float fDot  = max(0.f,(rVert.m_vNormal * rvLightDir));
				const float fInvLambCorr = fDot * fLM_DP3LerpFactor * (fLumSum/max(scfLumSumThreshold*0.1f,fLambLumSum));
				{
					// correction depends on the lerp factor (fInvLambCorr is for full directional case)
					// fLM_DP3LerpFactor=1    apply corr 100%  *=fInvLambCorr
					// fLM_DP3LerpFactor<1    apply corr less  *=((fInvLambCorr-1)*fLM_DP3LerpFactor +1)
					//fInvLambCorr*=((fInvLambCorr-1)*fLM_DP3LerpFactor +1);
					fColorRLamb *= fInvLambCorr;
					fColorGLamb *= fInvLambCorr;
					fColorBLamb *= fInvLambCorr;
				}
			}

	#ifdef REDUCE_DOT3LIGHTMAPS_TO_CLASSIC
			fLM_DP3LerpFactor = 0.0f;
	#endif  
			/*
			// Fade out the bump when there's not enough illumination (spotlights)
			Vec3d vDir = pVert->m_rvLightDir;
			float fCos = __max(0.25f, vDir * pVert->m_vNormal);
			pVert->m_fLM_DP3LerpFactor *= fCos;
			*/
		}
	}
}

static void AddSpotLightTerm(float& fSpotlightTerm, const LMCompLight& rLight, const Vec3d& rvSamplePos, const Vec3d& vDir)
{
	// ---------------------------------------------------------------------------------------------
	// Spotlight term
	// ---------------------------------------------------------------------------------------------
	float fAccum = 1.0f;
	if (rLight.eType == eSpotlight)
	{	
		float fMaxAngle = sinf(rLight.fLightFrustumAngleDegree * gf_DEGTORAD);
		float fInnerAngle = fMaxAngle / 2.33333f;

		assert(fMaxAngle >= 0.0f);
		assert(fInnerAngle <= fMaxAngle);

		float fCos=-vDir * rLight.vDirection;

		float fAngle = 1.0f - __max(0.0f, fCos);

		if (fAngle > fMaxAngle)
			fAccum = 0.0f;
		else if (fAngle < fInnerAngle)
			fAccum = 1.0f;
		else
			fAccum = 1.0f - (fAngle - fInnerAngle) / (fMaxAngle - fInnerAngle);

		assert(fAccum >= 0.0f);
		assert(fAccum <= 1.0f);
	}
	fSpotlightTerm += fAccum;
}
 
void CLightScene::WriteLogInfo()
{
	FILE *pFile = fopen("Lightmap.log","w+");
	if(pFile == NULL)
	{
		_TRACE(m_LogInfo, true, "\r\n Cannot write log-file! \r\n");
		return;
	}
	FILE *pErrorFile = fopen("LightmapError.log","w+");
	if(pErrorFile == NULL)
	{
		_TRACE(m_LogInfo, true, "\r\n Cannot write error-log-file! \r\n");
		return;
	}
	_TRACE("For errors/warnings and more information see written log-file 'Lightmap.log' and 'LightmapError.log'\r\n", false);
	for(std::vector<CString>::const_iterator iter = m_LogInfo.begin(); iter != m_LogInfo.end(); ++iter)
	{
		const CString rString = *iter;
		fwrite( (const char*)rString,1,rString.GetLength(),pFile);		//write encoded data
	}
	//now write warning summary
	std::string sSummary = "-----ERROR/WARNING SUMMARY----------------------------------------------------------\r\n\r\n";
	fwrite( sSummary.c_str(),1,sSummary.size(),pErrorFile);		//write encoded data
	if(m_WarningInfo.size() != 0)
	{
		unsigned int uiLastType = (m_WarningInfo.begin())->first;
		for(std::multimap<unsigned int,std::string>::const_iterator iter = m_WarningInfo.begin(); iter != m_WarningInfo.end(); ++iter)
		{
			const unsigned int cuiCurrentType = iter->first;
			if(cuiCurrentType != uiLastType)
			{
				fwrite("\r\n", 1, 2, pErrorFile);
				uiLastType = cuiCurrentType;
			}
			fwrite( (iter->second).c_str(),1,(iter->second).size(),pErrorFile);		//write encoded data		
		}
	}
	fclose(pFile);	fclose(pErrorFile);
}

inline const float CLightScene::ComputeHalvedLightmapQuality(const float fOldValue)
{
	const float cfGridSize = m_sParam.m_fTexelSize;
	if(cfGridSize >= scfMaxGridSize)
		return fOldValue;
	if(fOldValue <= 1.f)
		return (-(cfGridSize + 2*((scfMaxGridSize - cfGridSize) * (1.f - fOldValue)))/ (scfMaxGridSize - cfGridSize) + 1.0f);//compiler will optimize it on its own :)
	if(fOldValue <= 2.f)
		return 1.f;//coarse reset to 1, will decrease further if still not enough
	return fOldValue * 0.5f;
}

void CLightScene::DoLightCalculation(
	unsigned int &ruiMaxColourComp, 
	const std::vector<LMCompLight*>& vLights, 
	const std::vector<LMCompLight*>& vOcclLights, 
	SComplexOSDot3Texel& dot3Texel, 
	CRadMesh *pMesh, 
	CRadPoly *pSource, 
	const CRadVertex& Vert, 
	const bool cbFirstPass, 
	const unsigned int cuiSubSampleIndex)
{
	Vec3d vLightDir = Vec3d(0, 0, 0);
	float fColorRLamb = 0.0f, fColorGLamb = 0.0f, fColorBLamb = 0.0f;
	float fLambLumSum=0.0f, fLumSum = 0.0f;
	float fSpotlightTerm = 0.0f;
	bool bApplyBetterLighting = false;
	int uiLightCounter = -1;
	for (std::vector<LMCompLight*>::const_iterator ligIt=vLights.begin(); ligIt<vLights.end(); ligIt++)
	{
		uiLightCounter++;
		LMCompLight& cLight = *(*ligIt);
		Vec3d vDir = -cLight.vDirection;	//for directional light, but dont waste default constructor
		if(cLight.eType != eDirectional)
		{	//get vertex light direction if not directional
			vDir = cLight.vWorldSpaceLightPos - Vert.m_vPos;
			vDir.Normalize();
		}
		//retrieve light intensity according to type and position
		float fIntens;
		if(!GetLightIntensity(cLight, fIntens, Vert, vDir))
			continue;
		if (m_sParam.m_bComputeShadows)								
		{
			UINT iNumHit = 0;
			Shadow(cLight, iNumHit, pMesh, pSource, Vert, Vert.m_vPos);
			if (iNumHit == 0) // Stop calculations if no light reaches the surface
				continue;
		}
		//spotlight term
		if(cLight.eType == eSpotlight && false == m_sParam.m_bSpotAsPointlight)
		{
			fSpotlightTerm = 0.0f;
			AddSpotLightTerm(fSpotlightTerm, cLight, Vert.m_vPos, vDir);
			fIntens *= fSpotlightTerm;
		}
		if (fIntens < 0.0f)
			continue;
		//alter dot3 values according to current light direction and colour
		if(bApplyBetterLighting == false)//do only check whether at least one light was processed with the new lighting model
			bApplyBetterLighting = SumDot3Values(cLight, fIntens, fLambLumSum, fLumSum, fColorRLamb, fColorGLamb, fColorBLamb, cLight, vLightDir, vDir, Vert);
		else
			SumDot3Values(cLight, fIntens, fLambLumSum, fLumSum, fColorRLamb, fColorGLamb, fColorBLamb, cLight, vLightDir, vDir, Vert);
		if(cbFirstPass)
			dot3Texel.uiLightMask |= (1 << (min(uiLightCounter,31)));//just make sure it can handle somehow more than 31 lights
	} // ligit
	//get dot3 values and add texel
	float fLM_DP3LerpFactor;
	GetDot3Sample(fLM_DP3LerpFactor,vLightDir,fLumSum,fLambLumSum,Vert,fColorRLamb,fColorGLamb,fColorBLamb, bApplyBetterLighting); 
	//now the occlusion map lights
	SOcclusionMapTexel occlTexel;
	if(m_sParam.m_bGenOcclMaps)
	{
		GLMOcclLightInfo& rOcclInfo = pMesh->m_OcclInfo;//cache occlusion light info
		fSpotlightTerm = 0.f;
		for (std::vector<LMCompLight*>::const_iterator ligIt=vOcclLights.begin(); ligIt<vOcclLights.end(); ligIt++)
		{
			uiLightCounter++;
			LMCompLight& cLight = *(*ligIt);
			Vec3d vDir = -cLight.vDirection;	//for directional light, but dont waste default constructor
			if(cLight.eType != eDirectional)
			{	//get vertex light direction if not directional
				vDir = cLight.vWorldSpaceLightPos - Vert.m_vPos;
				vDir.Normalize();
			}
			//retrieve light intensity according to type and position
			float fIntens;
			if(!GetLightIntensity(cLight, fIntens, Vert, vDir))
				continue;
			UINT iNumHit = 0;
			Shadow(cLight, iNumHit, pMesh, pSource, Vert, Vert.m_vPos);
			if (iNumHit == 0) // Stop calculations if no light reaches the surface
				continue;
			//spotlight term
			if(cLight.eType == eSpotlight && false == m_sParam.m_bSpotAsPointlight)
			{
				fSpotlightTerm = 0.0f;
				AddSpotLightTerm(fSpotlightTerm, cLight, Vert.m_vPos, vDir);
				fIntens *= fSpotlightTerm;
			}
			if (fIntens < 0.0f)
				continue;
			//else set it to visible
			//fetch channel for this lightsource
			EOCCLCOLOURASSIGNMENT eChannel = rOcclInfo.FindLightSource(cLight.m_CompLightID);
			if(eChannel == EOCCLCOLOURASSIGNMENT_NONE)
			{
				//add light source, first time this light source has received a texel
				eChannel = rOcclInfo.AddLightsource(cLight.m_CompLightID);
			}
			if(eChannel == EOCCLCOLOURASSIGNMENT_NONE)
			{
				//too many active occlusion map light sources for this glm
				char text[scuiWarningTextAllocation];
				sprintf(text, "GLM: %s has more than 4 active affecting occlusion lights\r\n",(const char*)pMesh->m_sGLMName);
				m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_TOO_MANY_OCCL_LIGHTS, std::string(text)));
			}
			occlTexel.SetValue(eChannel, 0xF);//mark as visible
			if(cbFirstPass)
				dot3Texel.uiLightMask |= (1 << (min(uiLightCounter,31)));//just make sure it can handle somehow more than 31 lights
		} // ligit
	}
	if(cbFirstPass)
#ifdef APPLY_COLOUR_FIX
		ruiMaxColourComp = __max(ruiMaxColourComp, pSource->SetDot3LightmapTexel(Vert, fColorRLamb, fColorGLamb, fColorBLamb, vLightDir, fLM_DP3LerpFactor, dot3Texel, occlTexel, m_sParam.m_bHDR));
#else
		pSource->SetDot3LightmapTexel(Vert, fColorRLamb, fColorGLamb, fColorBLamb, vLightDir, fLM_DP3LerpFactor, dot3Texel, occlTexel, m_sParam.m_bHDR);
#endif
	else
		SetSubSampleDot3LightmapTexel(cuiSubSampleIndex, fColorRLamb, fColorGLamb, fColorBLamb, vLightDir, fLM_DP3LerpFactor, dot3Texel, occlTexel, m_sParam.m_bHDR);
}

bool CLightScene::Create( const IndoorBaseInterface &pInterface, const Vec3d& vMinBB, const Vec3d& vMaxBB, volatile SSharedLMEditorData *pSharedData, const ELMMode Mode, const unsigned int cuiMeshesToProcessCount)
{
	//for logging stuff
	static char sTraceString[1024];
	// ---------------------------------------------------------------------------------------------
	// Main lighting computation function
	// ---------------------------------------------------------------------------------------------
	UINT iCurMesh = 0;

	_TRACE(m_LogInfo, true, "\r\nCalculating lighting... \r\n");
	// ---------------------------------------------------------------------------------------------
	// Check for lightsources
	// ---------------------------------------------------------------------------------------------
	memcpy(&m_IndInterface, &pInterface, sizeof(IndoorBaseInterface));
 
	//initialize sample pattern
	switch(m_sParam.m_iSubSampling)
	{
	case 1:
		//already set by default
		gAASettings.SetScale(1);
		gAASettings.m_bEnabled = false;
		break;
	case 5:
		gAASettings.m_bEnabled = true;
		gAASettings.SetScale(2);
		InitSubSampling(5);
		break;
	case 9:
		gAASettings.m_bEnabled = true;
		gAASettings.SetScale(3);
		InitSubSampling(9);
		break;
	default:
		//set to no AA by default
		gAASettings.SetScale(1);
		gAASettings.m_bEnabled = false;
		break;
	}

	bool bSerialize = true;

	unsigned char ucLastProgress = 101;	//initialize to some number outside range 0..100
	// Build individual meshes
	bool bStarted = false;
	//pointers to fetch everywhere
	CRadMesh *pMesh;
	IStatObj *pIGeom;
	//display first GLM processing messages

	unsigned int uiStartIndex = 0;
	//seek to the first relevant GLM
	radmeshit itRadMesh=m_lstRadMeshes.begin();
	if(Mode != ELMMode_ALL)
	{
		for (;itRadMesh!=m_lstRadMeshes.end();itRadMesh++)
		{
			pMesh = (* itRadMesh);
			if(pMesh == NULL || (pMesh->m_bReceiveLightmap == false))
			{
				uiStartIndex++;
				continue;	//does not receive any lightmaps, just casts shadow
			}
			break;
		}
	}
	if(cuiMeshesToProcessCount > 0 && m_lstRadMeshes.size() > 0) 
	{ 
		if(m_uiCurrentPatchNumber == 0)//allocate the space for the first lightmap
			CreateNewLightmap();

		pMesh = *itRadMesh;
		if(pMesh && (pMesh->m_bReceiveLightmap == true))
		{
			pIGeom = pMesh->m_pESource->GetEntityStatObj(0, NULL);
			int iNumIndices = 0;
			pIGeom->GetLeafBuffer()->GetIndices(&iNumIndices);

			sprintf(sTraceString, "Processing GLM No.1 with %i Tri and %i Light%c...\r\n", 
				iNumIndices / 3, pMesh->m_LocalLights.size(), 
		 		pMesh->m_LocalLights.size() == 1 ? ' ' : 's');
			_TRACE(sTraceString);
			sprintf(sTraceString, "Processing GLM No.1 ('%s' with brush type '%s') with %i Tri and %i Light%c...\r\n", 
				pMesh->m_sGLMName, pIGeom?pIGeom->GetFileName():"", iNumIndices / 3, pMesh->m_LocalLights.size(), 
				pMesh->m_LocalLights.size() == 1 ? ' ' : 's');
			m_LogInfo.push_back(CString(sTraceString));
		}
	}
	// basically iterate each receiving GLM	
	for (;itRadMesh!=m_lstRadMeshes.end();itRadMesh++)
	{
		pMesh = (* itRadMesh);
		if(pMesh == NULL || (pMesh->m_bReceiveLightmap == false))
		{
			if(Mode == ELMMode_ALL)
				iCurMesh++;//only count in case all GLMs are supposed to get processed
			continue;	//does not receive any lightmaps, just casts shadow
		}
		pIGeom = pMesh->m_pESource->GetEntityStatObj(0, NULL);
		//alter some display things
		if(pSharedData != NULL)
		{
			const unsigned char cucProgress = static_cast<unsigned int>(static_cast<float>(iCurMesh) / static_cast<float>(cuiMeshesToProcessCount + 1) * 100.0f);
			if(cucProgress != ucLastProgress)
				::SendMessage( pSharedData->hwnd, pSharedData->uiProgressMessage, cucProgress, 0 );//update progress bar
			::SendMessage( pSharedData->hwnd, pSharedData->uiMemUsageMessage, min(GetUsedMemory(),1000), 0 );//update progress bar
			::SendMessage( pSharedData->hwnd, pSharedData->uiMemUsageStatic, min(GetUsedMemory(),1000), 0 );//update progress bar static element
			::SendMessage( pSharedData->hwnd, pSharedData->uiGLMNameEdit, (UINT_PTR)pMesh, (UINT_PTR)pIGeom );//update progress bar static element			

			ucLastProgress = cucProgress;
			MSG msg;
			while( FALSE != ::PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
			{ 
				::TranslateMessage( &msg );
				::DispatchMessage( &msg );
			}
			if(pSharedData->bCancelled == true)
			{
				bSerialize = false;
				break;
			}
		}

		bool bFound = false;
		// Output progress (only if there was no overflow, don't show message again)
		if (bStarted)
		{
			int iNumIndices = 0;
			pIGeom->GetLeafBuffer()->GetIndices(&iNumIndices);

			//different output here
			sprintf(sTraceString, "Done with %i of %i GLMs. Processing GLM No.%i with %i Tri and %i Light%c...\r\n", 
				iCurMesh, cuiMeshesToProcessCount, iCurMesh+1, iNumIndices / 3, pMesh->m_LocalLights.size(), 
				pMesh->m_LocalLights.size() == 1 ? ' ' : 's');
			_TRACE(sTraceString);
			sprintf(sTraceString, "Done with %i of %i GLMs. Processing GLM No.%i ('%s' with brush type '%s') with %i Tri and %i Light%c...\r\n", 
				iCurMesh, cuiMeshesToProcessCount, iCurMesh+1, pMesh->m_sGLMName, pIGeom?pIGeom->GetFileName():"", iNumIndices / 3, pMesh->m_LocalLights.size(), 
				pMesh->m_LocalLights.size() == 1 ? ' ' : 's');
			m_LogInfo.push_back(CString(sTraceString));
		}
		iCurMesh++;
 
		// Prepare lightmap space for this mesh    
		pMesh->PrepareLightmaps(this, !m_sParam.m_bDontMergePolys, m_sParam.m_fTexelSize, vMinBB, vMaxBB);

		// No lightmap to calculate
		if (m_lstScenePolys.empty())
			continue;        

		//set affecting lights for this patch
		std::vector<LMCompLight*>& vLights		= pMesh->m_LocalLights;
		std::vector<LMCompLight*>& vOcclLights	= pMesh->m_LocalOcclLights;
		//check normals
		if((pMesh->m_uiFlags & DOUBLE_SIDED_MAT) != 0)
		{
			_TRACE(m_LogInfo, true, "GLM No.%i has double sided materials where lightmaps can't get applied properly...\r\n", iCurMesh);			
			char text[scuiWarningTextAllocation];
			sprintf(text, "GLM: %s (%s) has double sided materials where lightmaps can't get applied properly\r\n",(const char*)pMesh->m_sGLMName, pIGeom?pIGeom->GetFileName():"");
			m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_DOUBLE_SIDED, std::string(text)));
		}
		float fVariance;//to retrieve max variance within this GLM, artist will know how bad the normal export was
		int retValue = 0;
		if((retValue = CheckNormals(fVariance, pMesh)) > 0)
		{
			if(retValue & WRONG_NORMALS_FLAG)
			{
				_TRACE(m_LogInfo, true, "GLM No.%i has invalid normals, please do re-export, replacing normals...\r\n", iCurMesh);

				char text[scuiWarningTextAllocation];
				sprintf(text, "GLM: %s (%s) has invalid normals, please do re-export, normals replaced by default calculations\r\n",(const char*)pMesh->m_sGLMName, pIGeom?pIGeom->GetFileName():"");
				m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_WRONG_NORMALS, std::string(text)));
			} 
			else
			if(retValue & NOT_NORMALIZED_NORMALS_FLAG)
			{
#ifdef DISPLAY_MORE_INFO
				_TRACE(m_LogInfo, true, "GLM No.%i has denormalized normals, max variance to 1.0: %f, renormalizing...\r\n", iCurMesh, fVariance);
#else	
				_TRACE(m_LogInfo, false, "GLM No.%i has denormalized normals, max variance to 1.0: %f, renormalizing...\r\n", iCurMesh, fVariance);
#endif
				char text[scuiWarningTextAllocation];
				sprintf(text, "GLM: %s (%s) has denormalized normals, max variance to 1.0: %f, automatic renormalization\r\n",(const char*)pMesh->m_sGLMName, pIGeom?pIGeom->GetFileName():"", fVariance);
				m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_DENORM_NORMALS, std::string(text)));
			}
		}
		//compute the smoothing group information for this GLM
		const bool bTSGeneratedByLM = ((pMesh->m_uiFlags & WRONG_NORMALS_FLAG) == 0)?false:true;
		ComputeSmoothingInformation(m_sParam.m_uiSmoothingAngle, bTSGeneratedByLM);		
		//iterate all individual lightmap patches in this receiving GLM
		for (radpolyit i=m_lstScenePolys.begin(); i!=m_lstScenePolys.end(); i++)
		{	
			CRadPoly *pSource = (* i);
			if (pSource->m_dwFlags & NOLIGHTMAP_FLAG)
				continue;
			pSource->AllocateDot3Lightmap(m_sParam.m_bHDR, m_sParam.m_bGenOcclMaps);//for subsampling world space light vector is used
			//iterate all texels of the current patch grid
#ifdef APPLY_COLOUR_FIX
			unsigned int uiMaxColourComp = 0;
#endif
			for(int y=0; y<pSource->m_nH; y++)
			for(int x=0; x<pSource->m_nW; x++)
 			{   
				//choose one as texel center to get one texel smoothed at least
				CRadVertex Vert;
				//snap and smooth if no super sampling enabled
				SComplexOSDot3Texel dot3Texel;
				//snap middle point to make sure tangent space exists
				if(!pSource->InterpolateVertexAt((float)x, (float)y, Vert, dot3Texel, false, true))//snap middle point to ensure hit
					continue;
				DoLightCalculation(uiMaxColourComp, vLights, vOcclLights, dot3Texel, pMesh, pSource, Vert, true);
			} // x, y
			//now perform adaptive subsampling
#ifdef APPLY_COLOUR_FIX
			PerformAdaptiveSubSampling(pMesh, pSource, vLights, vOcclLights, uiMaxColourComp);
#else
			PerformAdaptiveSubSampling(pMesh, pSource, vLights, vOcclLights);
#endif
			//free some memory
			pSource->m_SharingPolygons.clear();
			pSource->m_SharingPolygons.resize(0);
		} // i
		// Create the lightmaps of this mesh
		if (!pMesh->SaveLightmaps(this , m_sParam.m_bDebugBorders))
		{
			// Normal lightmap overflow, create new one
			CreateNewLightmap();
			//try it again
			if (!pMesh->SaveLightmaps(this, m_sParam.m_bDebugBorders))
			{
				// If the function returns false, we ran out of lightmap space while processing the GLM.
				// We already couldn't fit it in last time, object won't fit into our LM size at all
				_TRACE(m_LogInfo, true, "GLM No. %i cannot fit into lightmap size %ix%i, recomputing with lower resolution...\r\n", 
					iCurMesh, m_sParam.m_iTextureResolution, m_sParam.m_iTextureResolution);
				//now change nLightmapQuality to not let this happen again
				if(pMesh->pLightmapQualityVar)
				{
					float fCurrentValue = 1.f;					float fOldValue = 1.f;
					pMesh->pLightmapQualityVar->Get(fOldValue);
					if(fCurrentValue > 0.f)//if not already reached maximum grid size
					{
						fCurrentValue = ComputeHalvedLightmapQuality(fOldValue);//halve resolution
						pMesh->pLightmapQualityVar->Set(fCurrentValue);
						pMesh->m_fLMAccuracy = fCurrentValue;
						_TRACE("...setting new value for nLightmapQuality to: %f \r\n",fCurrentValue);
					}
					char text[scuiWarningTextAllocation];
					sprintf(text, "GLM: %s did not fit into a single lightmap, nLightmapQuality has been changed from %f to %f\r\n",(const char*)pMesh->m_sGLMName, fOldValue, fCurrentValue);
					m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_NO_FIT, std::string(text)));
				}

				assert(m_pCurrentPatch != NULL);
				m_pCurrentPatch->Reset();

				itRadMesh--;
				iCurMesh--;
				continue;
			}
		}
		assert(m_pCurrentPatch != NULL);

		// ---------------------------------------------------------------------------------------------
		// Texture coordinates
		// ---------------------------------------------------------------------------------------------
		LMAssignQueueItem sNewGLMQueueItm;
		//set occlusion id's
		for(int i = 0; i<pMesh->m_OcclInfo.uiLightCount;i++)
			sNewGLMQueueItm.vOcclIDs.push_back(pMesh->m_OcclInfo.iLightIDs[i]);
		GenerateTexCoords(*pMesh, sNewGLMQueueItm);
		// Add the object to the assign queue for this light patch, we will create the
		// RenderLMData object and assigne it to the GLMs when the patch is filled and we are about to create a new one
		m_lstAssignQueue.push_back(sNewGLMQueueItm);
		//remove reference to rastercube
		m_RasterCubeManager.RemoveReference(pMesh);
		//free patch
		delete pMesh; *itRadMesh = pMesh = NULL;
		bStarted = true;
	} // itRadMesh
	m_lstRadMeshes.clear();
	//signal completion
	if(pSharedData != NULL)
	{
		const unsigned char cuiProgress = (bSerialize == true)?static_cast<unsigned int>(static_cast<float>(cuiMeshesToProcessCount) / static_cast<float>(cuiMeshesToProcessCount + 1) * 100.0f):0;
		::SendMessage( pSharedData->hwnd, pSharedData->uiProgressMessage, cuiProgress, 0 );
		::SendMessage( pSharedData->hwnd, pSharedData->uiMemUsageMessage, min(GetUsedMemory(),1000), 0 );//update progress bar
		::SendMessage( pSharedData->hwnd, pSharedData->uiMemUsageStatic, min(GetUsedMemory(),1000), 0 );//update progress bar static element
		::SendMessage( pSharedData->hwnd, pSharedData->uiGLMNameEdit, 0, 0 );//update progress bar static element			
		MSG msg;
		while( FALSE != ::PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
	//leave it as it is if user has pressed cancel or close
	if(bSerialize == false)
		return true;
	return FlushAndSave(pSharedData, Mode);
}

void CLightScene::GenerateTexCoords(const CRadMesh& rRadMesh, LMAssignQueueItem& rNewGLMQueueItm)
{
	rNewGLMQueueItm.pI_GLM = rRadMesh.m_pESource;
	rNewGLMQueueItm.m_dwHashValue=rRadMesh.GetHashValue();
	assert(rRadMesh.m_lstOldPolys.size() != 0);
	rNewGLMQueueItm.vSortedTexCoords.resize(rRadMesh.m_uiTexCoordsRequired);
	//get leafbuffer to write at the right pos
	CLeafBuffer *pLB = (rRadMesh.m_pESource->GetEntityStatObj(0, NULL))->GetLeafBuffer();
	assert(!pLB->IsEmpty());
	int iIdxCnt = 0;
	unsigned short *pIndices = pLB->GetIndices(&iIdxCnt);
	UINT iCurMat;
	list2<CMatInfo> *pMaterials = pLB->m_pMats;
	std::vector<CRadPoly *>::const_iterator itPoly = rRadMesh.m_lstOldPolys.begin();
	for (iCurMat=0; iCurMat<pMaterials->Count(); iCurMat++)
	{
		CMatInfo cCurMat = (* pMaterials)[iCurMat];
		// Check if the material is opaque, we don't let non-opaque geometry cast shadows
		IShader *pEff = cCurMat.shaderItem.m_pShader;
		if (pEff)
		{  
			IShader *pShTempl = pEff->GetTemplate(-1);
			if (pShTempl)
			{  
				if(pShTempl->GetFlags3() & EF3_NODRAW)
					continue;
			}
		}
		// Process all triangles
		UINT iCurTri = 0;
		for (iCurTri=0; iCurTri<cCurMat.nNumIndices / 3; iCurTri++)
		{
			const unsigned int iBaseIdx = cCurMat.nFirstIndexId + iCurTri * 3;
			CRadPoly *pPoly = (* itPoly);	
			assert(pPoly->m_lstOriginals.size() == 3);
			rNewGLMQueueItm.vSortedTexCoords[pIndices[iBaseIdx]] = 
				(TexCoord2Comp(pPoly->m_lstOriginals[2].m_fpX, pPoly->m_lstOriginals[2].m_fpY));
			rNewGLMQueueItm.vSortedTexCoords[pIndices[iBaseIdx+1]] = 
				(TexCoord2Comp(pPoly->m_lstOriginals[1].m_fpX, pPoly->m_lstOriginals[1].m_fpY));
			rNewGLMQueueItm.vSortedTexCoords[pIndices[iBaseIdx+2]] = 
				(TexCoord2Comp(pPoly->m_lstOriginals[0].m_fpX, pPoly->m_lstOriginals[0].m_fpY));
			itPoly++;
		}
	}
}


const bool CLightScene::FlushAndSave(volatile SSharedLMEditorData *pSharedData, const ELMMode Mode)
{
	// Be sure we assign all GLMs a lightmap object. Function may return false here in case
	// we already assigned all or there was no more data to process
	FlushAssignQueue();
	UINT iNumBytes = m_uiCurrentPatchNumber * m_sParam.m_iTextureResolution * m_sParam.m_iTextureResolution * (4 + 4);
	if(Mode != ELMMode_ALL)
		_TRACE(m_LogInfo, true, "Created a total of %i lightmap texture pairs (R8G8B8A8 + R8G8B8A8), ~%3.2f MB of additional lightmap texture data\r\n", 
			m_uiCurrentPatchNumber, iNumBytes / 1024.0f / 1024.0f);
	else
		_TRACE(m_LogInfo, true, "Created a total of %i lightmap texture pairs (R8G8B8A8 + R8G8B8A8), ~%3.2f MB of lightmap texture data\r\n", 
			m_uiCurrentPatchNumber, iNumBytes / 1024.0f / 1024.0f);
	// Export all LM data to LM_EXPORT_FILE_NAME in the directory of the level
	string strLMExportFile = m_IndInterface.m_p3dEngine->GetFilePath(LM_EXPORT_FILE_NAME);
	_TRACE(m_LogInfo, true, "Exporting lightmap data to '%s'\r\n", strLMExportFile.c_str());
	unsigned int res = m_pISerializationManager->Save(strLMExportFile.c_str(), m_sParam, Mode != ELMMode_ALL);
	switch(res)
	{
	case NSAVE_RESULT::ESUCCESS:
		break;
	case NSAVE_RESULT::EPAK_FILE_UPDATE_FAIL:
		_TRACE(m_LogInfo, true, "Could not update lightmap pak-file. Exporting failed!\r\n");
			m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_EXPORT_FAILED, std::string("Could not update lightmap pak-file. Exporting failed!\r\n")));
		return false;
	case NSAVE_RESULT::EDXT_COMPRESS_FAIL:
		_TRACE(m_LogInfo, true, "DXT compression for lightmaps failed. Exporting failed!\r\n");
			m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_EXPORT_FAILED, std::string("DXT compression for lightmaps failed. Exporting failed!\r\n")));
		return false;
	case NSAVE_RESULT::EPAK_FILE_OPEN_FAIL:
		_TRACE(m_LogInfo, true, "Could not open lightmap pak-file. Exporting failed!\r\n");
			m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_EXPORT_FAILED, std::string("Could not open lightmap pak-file. Exporting failed!\r\n")));
		return false;
	default:
		_TRACE(m_LogInfo, true, "Exporting failed!\r\n");
			m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_EXPORT_FAILED, std::string("Exporting of lightmaps failed!\r\n")));
		return false;
	}

	//signal completion
	if(pSharedData != NULL)
	{
		::SendMessage( pSharedData->hwnd, pSharedData->uiProgressMessage, 100, 0 );
		::SendMessage( pSharedData->hwnd, pSharedData->uiMemUsageMessage, min(GetUsedMemory(),1000), 0 );//update progress bar
		::SendMessage( pSharedData->hwnd, pSharedData->uiMemUsageStatic, min(GetUsedMemory(),1000), 0 );//update progress bar static element
		MSG msg;
		while( FALSE != ::PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
		{ 
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		}
	}
	return true;
}

void CLightScene::FlushAssignQueue()
{
	// ---------------------------------------------------------------------------------------------
	// Create a lightmap from the current patch (m_pCurrentPatch) and assign the shared lightmaps
	// as well as instance specific texture coordinates to all GLMs in the assign queue
	// (m_lstAssignQueue)
	// ---------------------------------------------------------------------------------------------

	std::list<LMAssignQueueItem>::iterator itItem;

	if (m_pCurrentPatch == NULL)
		return ;

	if (m_lstAssignQueue.empty())
		return ;

	_TRACE(m_LogInfo, true, "%ix%i lightmap filled with %i objects, flushing\r\n",
		m_sParam.m_iTextureResolution, m_sParam.m_iTextureResolution, m_lstAssignQueue.size());

	std::vector<int> vGLM_IDs_Using;
	for (itItem=m_lstAssignQueue.begin(); itItem!=m_lstAssignQueue.end(); itItem++)
	{
		vGLM_IDs_Using.push_back((* itItem).pI_GLM->GetEditorObjectId());
	}
	m_pISerializationManager->AddRawLMData(
		m_sParam.m_iTextureResolution, m_sParam.m_iTextureResolution, vGLM_IDs_Using,
		m_pCurrentPatch->m_pLightmapImage, m_pCurrentPatch->m_pHDRLightmapImage, m_pCurrentPatch->m_pDominantDirImage, m_pCurrentPatch->m_pOcclMapImage?reinterpret_cast<BYTE*>(&(m_pCurrentPatch->m_pOcclMapImage[0].colour)):0);

	// Create a new lightmap
	RenderLMData_AutoPtr pNewLM = m_pISerializationManager->CreateLightmap
		(NULL, 0, m_sParam.m_iTextureResolution, m_sParam.m_iTextureResolution,
		m_pCurrentPatch->m_pLightmapImage, m_pCurrentPatch->m_pHDRLightmapImage, m_pCurrentPatch->m_pDominantDirImage, m_pCurrentPatch->m_pOcclMapImage?reinterpret_cast<BYTE*>(&(m_pCurrentPatch->m_pOcclMapImage[0].colour)):0);

	for (itItem=m_lstAssignQueue.begin(); itItem!=m_lstAssignQueue.end(); itItem++)
	{
		LMAssignQueueItem& rCurItm = *itItem;
		// Pass a reference of the lightmap and texture coordinates
		if(strcmp(rCurItm.pI_GLM->GetEntityClassName(), "Brush") == 0)
		{
			std::vector<std::pair<EntityId, EntityId> > IDs;
			IDs.resize(4);	
			for(int i=0; i<4; i++)
				IDs[i] = std::pair<EntityId, EntityId>(0,0);
			assert(rCurItm.vOcclIDs.size()<=4);
			for(int i=0; i<rCurItm.vOcclIDs.size(); ++i)
				IDs[i] = rCurItm.vOcclIDs[i];
			rCurItm.pI_GLM->SetLightmap(pNewLM, (float*)&rCurItm.vSortedTexCoords[0], rCurItm.vSortedTexCoords.size(), rCurItm.vOcclIDs.size(), IDs);
		}
		else
			rCurItm.pI_GLM->SetLightmap(pNewLM, (float*)&rCurItm.vSortedTexCoords[0], rCurItm.vSortedTexCoords.size());
		// GLMs are mapped to texture coordinate data by position
		m_pISerializationManager->AddTexCoordData(rCurItm.vSortedTexCoords, rCurItm.pI_GLM->GetEditorObjectId(),rCurItm.m_dwHashValue, rCurItm.vOcclIDs);
	}

	// Empty queue
	m_lstAssignQueue.clear();
	pNewLM = NULL;
	delete m_pCurrentPatch;	m_pCurrentPatch = NULL;
}

void CLightScene::GatherSubSampleTexel(const CRadPoly *pSource, const int ciX, const int ciY, std::set<std::pair<unsigned int, unsigned int> >& rvSubTexels)
{
	//if triangle was not hit properly(needed a snap), subsample
	const SComplexOSDot3Texel *pWSDominantDirData = pSource->m_pWSDominantDirData;
	const unsigned int cuiPatchWidth	= pSource->m_nW;
	const unsigned int cuiPatchHeight	= pSource->m_nH;
	const SComplexOSDot3Texel& rDot3Texel = pWSDominantDirData[ciY*cuiPatchWidth+ciX];
	const unsigned char *pTexelColour = &(pSource->m_pLightmapData[4*(ciY * cuiPatchWidth + ciX)]);
  const unsigned char *pHDRTexelColour = NULL;
	static const unsigned int scuiMaxDiff = 30; //default coarse max value 
	static const float scfMaxDot3Diff = 0.86f;	//default coarse max value cosine for dot product check
	static const unsigned int scuiNotHitMaxDiff = 17;	//finer value if a texel has not been hit the patch
	static const float scfNotHitMaxDot3Diff = 0.9f;	//finer value if a texel has not been hit the patch
	bool bVertexInserted = false;
	//check all neighbour texels
	for(int u = max(ciX-1,0); u <= min(ciX+1,cuiPatchWidth-1); u++)
	for(int v = max(ciY-1,0); v <= min(ciY+1,cuiPatchHeight-1); v++)
	{
		if(u == ciX && v == ciY)
			continue;//do not check with itself
		//subsample if not hit this patch or if lightmask is different
		const SComplexOSDot3Texel& rDot3NeighbourTexel = pWSDominantDirData[v*cuiPatchWidth+u];
		if(rDot3Texel.uiLightMask != rDot3NeighbourTexel.uiLightMask)
		{
			if(!bVertexInserted)
			{
				bVertexInserted = true;
				rvSubTexels.insert(std::pair<unsigned int, unsigned int>(ciX, ciY));
				return;
			}
		}
		const bool cbNotHitCond = (rDot3NeighbourTexel.bNotHit | rDot3Texel.bNotHit);//set to true if any texel has not been hit
		//now check for colour diff
		const unsigned char *pNeighbourTexelColour = &(pSource->m_pLightmapData[4*(v * cuiPatchWidth + u)]);
		for(int i=0; i<3; i++)
		{
			if(abs(pNeighbourTexelColour[i] - pTexelColour[i]) > (cbNotHitCond?scuiNotHitMaxDiff:scuiMaxDiff))
			{
				if(!bVertexInserted)
				{
					bVertexInserted = true;
					rvSubTexels.insert(std::pair<unsigned int, unsigned int>(ciX, ciY));
					return;
				}
			}
		}
		//check whether any light has hit the surface for both texels
		if(rDot3NeighbourTexel.pSourcePatch == NULL && rDot3Texel.pSourcePatch == NULL)
			continue;
		if(rDot3NeighbourTexel.vDot3Light.GetLengthSquared() < 0.5f && rDot3Texel.vDot3Light.GetLengthSquared() < 0.5f)
			continue;//no light has hit the surface for both texels
		//now check dot3 vector diff
		if(rDot3NeighbourTexel.vDot3Light * rDot3Texel.vDot3Light < (cbNotHitCond?scfNotHitMaxDot3Diff:scfMaxDot3Diff))
		{
			if(!bVertexInserted)
			{
				bVertexInserted = true;
				rvSubTexels.insert(std::pair<unsigned int, unsigned int>(ciX, ciY));
				return;
			}
		}
	}
}

void CLightScene::SetSubSampleDot3LightmapTexel(
	const unsigned int cuiSubSampleIndex, 
	const float fColorRLamb, const float fColorGLamb, const float fColorBLamb,
	Vec3d &inLightDir, const float cfLM_DP3LerpFactor,
	SComplexOSDot3Texel& rDot3Texel, const SOcclusionMapTexel& rOcclTexel, bool bHDR)
{
	m_pfColours[4*cuiSubSampleIndex]	  = fColorRLamb;
	m_pfColours[4*cuiSubSampleIndex+1]	= fColorGLamb;
	m_pfColours[4*cuiSubSampleIndex+2]	= fColorBLamb;
	m_pfColours[4*cuiSubSampleIndex+3]	= cfLM_DP3LerpFactor;
	rDot3Texel.vDot3Light = inLightDir;
	m_pSubSamples[cuiSubSampleIndex]	= rDot3Texel;
	if(m_pOcclSamples)
		m_pOcclSamples[cuiSubSampleIndex]	= rOcclTexel;
}

#ifdef APPLY_COLOUR_FIX
void CLightScene::PerformAdaptiveSubSampling(CRadMesh *pMesh, CRadPoly *pSource, const std::vector<LMCompLight*>& vLights, const std::vector<LMCompLight*>& vOcclLights, unsigned int uiMaxComponent)
#else
void CLightScene::PerformAdaptiveSubSampling(CRadMesh *pMesh, CRadPoly *pSource, const std::vector<LMCompLight*>& vLights, const std::vector<LMCompLight*>& vOcclLights)
#endif
{
	/* idea: go through the patch and mark the texels who need some more accurate sampling:
		1. if lightmask of any neightbour texel is different
		2. if triangle source ID is different
		after this, downsample respective points, allocate dot3lightmap, synchronize it, free unused WSlightmap and set tangent space texel
		
		center texel which is already computed will get ignored but used to the GatherSubSamples - routine	
	*/
	if(m_uiSampleCount > 1)//perform subsampling only if requested
	{
		assert(m_puiIndicator);	//enough to ensure others are valid as well
		//now check which texel an accurater sampling scheme need
		std::set<std::pair<unsigned int, unsigned int> > vSubTexels;	//set containing all (unique) texel indices requiring subsampling
		//always check the neighbour texels for any differences
		for(int y=0; y<pSource->m_nH; y++)//supersample
		for(int x=0; x<pSource->m_nW; x++)//supersample
			GatherSubSampleTexel(pSource, x, y, vSubTexels);

		if(vSubTexels.size() > 1)
			pSource->m_dwFlags |= DO_NOT_COMPRESS_FLAG;//optimization

		unsigned int uiMaxColourComp = 0; //not needed here but to be able to share a function
		//now subsample for all these texels
		for(std::set<std::pair<unsigned int, unsigned int> >::const_iterator iter = vSubTexels.begin();	iter != vSubTexels.end(); ++iter)
		{
			int uiSubSampleIndex = -1;//index to access subsample storage arrays
			memset(m_puiIndicator, 0, m_uiSampleCount * sizeof(unsigned int));	// clear indicator
			const unsigned int cuiX = iter->first;	const unsigned int cuiY = iter->second;
			for(int y=cuiY*gAASettings.GetScale(); y<(cuiY+1)*gAASettings.GetScale(); y++)//supersample
			for(int x=cuiX*gAASettings.GetScale(); x<(cuiX+1)*gAASettings.GetScale(); x++)//supersample
 			{   
				if(gAASettings.IsMiddle(x%gAASettings.GetScale(),y%gAASettings.GetScale()))
					continue;//center texel has already been computed
				uiSubSampleIndex++;
				//choose one as texel center to get one texel smoothed at least
				CRadVertex Vert;
				SComplexOSDot3Texel dot3Texel;
				//snap middle point to make sure tangent space exists
				if(!pSource->InterpolateVertexAt((float)x, (float)y, Vert, dot3Texel, true, false))//do subsample and do not snap
					continue;
				m_puiIndicator[uiSubSampleIndex] = 1;
				DoLightCalculation(uiMaxColourComp, vLights, vOcclLights, dot3Texel, pMesh, pSource, Vert, false, uiSubSampleIndex);
			} // x, y
			//apply subsamples
#ifdef APPLY_COLOUR_FIX
			pSource->GatherSubSamples(cuiX, cuiY, uiSubSampleIndex+1, m_puiIndicator, m_pfColours, m_pSubSamples, uiMaxComponent, m_pOcclSamples, pMesh->m_OcclInfo);
#else
			pSource->GatherSubSamples(cuiX, cuiY, uiSubSampleIndex+1, m_puiIndicator, m_pfColours, m_pSubSamples, m_pOcclSamples, pMesh->m_OcclInfo);
#endif
		}
	}
	//now allocate dot3lightmap, synchronize it, free unused WSlightmap and set tangent space texel
	assert(pSource->m_pDominantDirData == NULL);//shouldnt yet be allocated

#ifdef USE_DOT3_ALPHA
	pSource->m_pDominantDirData = new unsigned char [pSource->m_nW*pSource->m_nH*4];					assert(pSource->m_pDominantDirData);
	//bear in mind that the alpha channel will only remain 0 for non set texels to signal not to include it for a possible mipmap generation for low spec
	memset(pSource->m_pDominantDirData,0,pSource->m_nW*pSource->m_nH*4);								//clear the image
#else
	pSource->m_pDominantDirData = new unsigned char [pSource->m_nW*pSource->m_nH*3];					assert(pSource->m_pDominantDirData);
	memset(pSource->m_pDominantDirData,0,pSource->m_nW*pSource->m_nH*3);								//clear the image
#endif
#ifdef APPLY_COLOUR_FIX
	const float cfColourScale = (uiMaxComponent > 3)?255.f / (float)uiMaxComponent : 1.f;//prevent any zero calculation
	const unsigned int cuiColourFixAlpha = (const unsigned char)min(255.0f, 128.f / cfColourScale);
#endif
	//now set the tangent space values
	for(int y=0; y<pSource->m_nH; y++)
	for(int x=0; x<pSource->m_nW; x++)
	{
#ifdef APPLY_COLOUR_FIX
		pSource->SetDot3TSLightmapTexel(x, y, cuiColourFixAlpha, cfColourScale);
#else
		pSource->SetDot3TSLightmapTexel(x, y);
#endif
	}
	//free and synchronize resources
	delete [] pSource->m_pWSDominantDirData;		pSource->m_pWSDominantDirData = NULL;
//	pSource->SynchronizeLightmaps();
}

void CLightScene::CreateNewLightmap()
{
	// ---------------------------------------------------------------------------------------------
	// Create space for a new lightmap
	// ---------------------------------------------------------------------------------------------
	// Make sure we assign all previous GLMs that used 
	// the current patch before we start a new one
	FlushAssignQueue();
	_TRACE(m_LogInfo, true, "New lightmap %ix%i created\r\n", m_sParam.m_iTextureResolution, m_sParam.m_iTextureResolution);
	m_uiCurrentPatchNumber++;
	// Create a new patch
	m_pCurrentPatch = new CLightPatch(m_sParam.m_iTextureResolution);
	m_pCurrentPatch->CreateDot3Lightmap(m_sParam.m_iTextureResolution, m_sParam.m_iTextureResolution,m_sParam.m_bHDR, m_sParam.m_bGenOcclMaps);
	m_pCurrentPatch->m_nTextureNum = m_uiCurrentPatchNumber;
}
