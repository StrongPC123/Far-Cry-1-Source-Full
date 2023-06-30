// ---------------------------------------------------------------------------------------------
//	Crytek CryENGINE source code
//	History:
//	- Created by Marco Corbetta
//  - Changed by Tim Schroeder
//	- Partial rewrite for editor integration
// ---------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "LMCompCommon.h"
#include <ISystem.h>
#include <IStatObj.h>
#include "IndoorLightPatches.h"
#include "IEntityRenderstate.h"
#include <limits.h>	

CAASettings gAASettings;	//to make it global here is codewise a bad things, but it is tool late to apply some fancy interface designs here 

const bool CLightScene::CAnyHit::ReturnElement(RasterCubeUserT &inObject, float &inoutfRayMaxDist)
{
	float fU, fV, fT;

	if(!InsertAlreadyTested(&inObject))
	{
		if (intersect_triangle((float *) m_vRayOrigin, (float *) m_vRayDir, inObject.fVertices[0], 
			inObject.fVertices[1], inObject.fVertices[2], &fT, &fU, &fV))
		{
			if (fT < m_fClosest)
			{
				// Do not take intersections into account which lay in the cube of the texel. This prevents us
				// from using instable normal/epsilon hacks and fixes nicely a bunch of false shadowing cases like
				// in complicated corners etc.
				// we need get shadow for near lying geometry.
				if (fT > 0.05f && fT < m_fRayLen)
				{
					// Make sure our intersection is not on the plane of the triangle, which won't be fixed by
					// the small texel cube and can lead to artifacts for lightsources which hover closer above
					// a triangle. Just fixes 50% of all cases
					Vec3d vHitPos = m_vRayOrigin + m_vRayDir * fT;
					Vec3d vPt;
					float fDist = vHitPos * m_vPNormal + m_fD;
					// we need get shadow for near lying geometry.
					//const float cfThreshold = m_fGridSize/5.0f;
					const float cfThreshold = 0.01f;
					if (fabs(fDist) > cfThreshold)
					{
						// Make sure the ray is not too parallel to the plane, fixed the other 50% of the error cases
						float fDot = inObject.vNormal * m_vRayDir;
						const float cfBias = 0.01f;
						if (fabs(fDot) > cfBias)
						{
							m_fClosest = fT;
							m_pHitResult = &inObject;
							inoutfRayMaxDist = fT;			// don't go further than this (optimizable)
							return false;
						}
					}
				}
			}
		}
	}
	return true;
}

int	CLightScene::CAnyHit::intersect_triangle(float orig[3], float dir[3], float vert0[3], float vert1[3], float vert2[3], float *t, float *u, float *v)
{
	#define EPSILON 0.000001
	#define CROSS(dest,v1,v2) \
						dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
						dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
						dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
	#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
	#define SUB(dest,v1,v2)			\
						dest[0]=v1[0]-v2[0]; \
						dest[1]=v1[1]-v2[1]; \
						dest[2]=v1[2]-v2[2]; 

	float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	float det,inv_det;

	/* find vectors for two edges sharing vert0 */
	SUB(edge1, vert1, vert0);
	SUB(edge2, vert2, vert0);

	/* begin calculating determinant - also used to calculate U parameter */
	CROSS(pvec, dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DOT(edge1, pvec);

	if (det > -EPSILON && det < EPSILON)
		return 0;
	inv_det = 1.0f / det;

	/* calculate distance from vert0 to ray origin */
	SUB(tvec, orig, vert0);

	/* calculate U parameter and test bounds */
	*u = DOT(tvec, pvec) * inv_det;
	if (*u < 0.0f || *u > 1.0f)
		return 0;

	/* prepare to test V parameter */
	CROSS(qvec, tvec, edge1);

	/* calculate V parameter and test bounds */
	*v = DOT(dir, qvec) * inv_det;
	if (*v < 0.0f || *u + *v > 1.0f)
		return 0;

	/* calculate t, ray intersects triangle */
	*t = DOT(edge2, qvec) * inv_det;
	return 1;

	#undef DOT
	#undef CROSS
	#undef EPSILON
}


//find a free block inside a lightmap image
//////////////////////////////////////////////////////////////////////
const bool CLightPatch::GetSpace(int w,int h, uint16 &x, uint16 &y)
{
	int iX = -1, iY = -1;
#ifdef MAKE_BLOCK_ALIGN
 	w = ((w + 3) & 0xfffffffc);
	h = ((h + 3) & 0xfffffffc);
#endif

	UINT iMaxPatchSize = m_nPatchSpace.size();
	int nLevel = iMaxPatchSize;

	bool bFoundPlace = false;
#ifdef MAKE_BLOCK_ALIGN
	for (int i=0;i<=iMaxPatchSize-w;i+=4) //only test every 4th texel
#else
	for (int i=0;i<=iMaxPatchSize-w;i++) 
#endif
	{
		int nLevel2=0;
#ifdef MAKE_BLOCK_ALIGN
		for (int j=0;j<w;j+=4)		//only test every 4th texel
#else
		for (int j=0;j<w;j++) 
#endif
		{
			if (m_nPatchSpace[i+j]>=nLevel) 
				break;

			if (m_nPatchSpace[i+j]>nLevel2) 
				nLevel2=m_nPatchSpace[i+j];			
		} //j

		//here there is free space
		if (j==w)	
		{	
			bFoundPlace = true;
			iX=i;
			iY=nLevel=nLevel2;
		}
	} //i

	assert(bFoundPlace);
	x = (uint16)iX;	y = (uint16)iY;	//set values

	//check the border
	if (nLevel+h>iMaxPatchSize) 
		return (false);

	//increase the "water" level
	for (i=0;i<w;i++) 
		m_nPatchSpace[iX+i]=nLevel+h;

	return (true);
}

void CLightPatch::Reset()
{
	memset(m_pLightmapImage,0,m_nWidth*m_nHeight*4);
	memset(&m_nPatchSpace[0], 0, m_nPatchSpace.size() * sizeof(int));    
#ifdef USE_DOT3_ALPHA
	memset(m_pDominantDirImage,0,m_nWidth*m_nHeight*4);
#else
	memset(m_pDominantDirImage,0,m_nWidth*m_nHeight*3);
#endif
  if (m_pHDRLightmapImage)
	  memset(m_pHDRLightmapImage,0,m_nWidth*m_nHeight*4);
}

//////////////////////////////////////////////////////////////////////
void CLightPatch::CreateDot3Lightmap(int nSizeX,int nSizeY,const bool cbGenerateHDRMaps, const bool cbGenerateOcclMaps)
{
	m_nWidth=nSizeX;
	m_nHeight=nSizeY;
	m_pLightmapImage=new unsigned char [nSizeX*nSizeY*4];		assert(m_pLightmapImage);
#ifdef USE_DOT3_ALPHA
	m_pDominantDirImage=new unsigned char [nSizeX*nSizeY*4];
#else
	m_pDominantDirImage=new unsigned char [nSizeX*nSizeY*3];
#endif
	assert(m_pDominantDirImage);
	if(cbGenerateOcclMaps)
	{
		m_pOcclMapImage=new SOcclusionMapTexel[nSizeX*nSizeY];	assert(m_pOcclMapImage);
	}
	if(cbGenerateHDRMaps)
	{
		m_pHDRLightmapImage=new unsigned char [nSizeX*nSizeY*4];		assert(m_pHDRLightmapImage);
	}
	Reset();
}

//first pass to decide whether subdivide lightmaps or not
//////////////////////////////////////////////////////////////////////
void CRadMesh::PrepareLightmaps(CLightScene *pScene, bool bMergePolys, const float fGridSize, const Vec3d& vMinBB, const Vec3d& vMaxBB)
{
#ifndef DISPLAY_MORE_INFO
	unsigned int uiHugePatchFoundNumber = 0;
#endif
	pScene->m_lstScenePolys.clear();    
	//reset merge flag
	for (radpolyit i=m_lstOldPolys.begin();i!=m_lstOldPolys.end();i++)
		(*i)->m_dwFlags&=~MERGE_FLAG;	//reset merge flag
	m_uiCoarseTexelCount = 0;
	//merge small & coplanar polygons
	const int iMaxPolySize = pScene->m_sParam.m_iTextureResolution/4;			// /4 is the maximum, but less size if better for packing
	for (radpolyit i1=m_lstOldPolys.begin();i1!=m_lstOldPolys.end();i1++)
	{
		CRadPoly *pPoly1=(*i1);
		if((pPoly1->m_dwFlags & DECAL_FLAG) != 0)
			continue;
        //continue if already merged
		if ((pPoly1->m_dwFlags & MERGE_FLAG)) 
			continue;				
		//build a new poly for polygon merging
		CRadPoly *pNewPoly = new CRadPoly(pPoly1);	
		pNewPoly->m_lstMerged.push_back(pPoly1);
		pPoly1->m_dwFlags |= (MERGE_FLAG | MERGE_SOURCE_FLAG); //mark as merged and source

		if (bMergePolys)
		{
			//search for attached polygons with the same normal
			for (radpolyit i2=m_lstOldPolys.begin();i2!=m_lstOldPolys.end();i2++)
			{
				CRadPoly *pPoly2 = (*i2);
				if((pPoly2->m_dwFlags & DECAL_FLAG) != 0)
					continue;
				//already merged
				if ((pPoly1==pPoly2) || (pPoly2->m_dwFlags & MERGE_FLAG)) 
					continue;
				//this polygon can be merged      
				if(pNewPoly->Connected(pPoly2) && pNewPoly->m_Plane.n * pPoly2->m_Plane.n > 0.99f)	
				{
					pNewPoly->m_lstMerged.push_back(pPoly2);
					pPoly2->m_pMergeSource = pNewPoly;
					pPoly2->m_dwFlags|=MERGE_FLAG;
					i2=m_lstOldPolys.begin(); //reset the iterator
				}
			} //i2
		}
		//alter grid size and adapt individually
		float fAlteredGridSize = fGridSize;
		if(m_fLMAccuracy > 1.0f)
			fAlteredGridSize /= m_fLMAccuracy;//use a more accurate texel<->world grid ratio
		else
			//use a less accurate texel<->world grid ratio
			fAlteredGridSize = fGridSize + (scfMaxGridSize - fGridSize) * (1.f - m_fLMAccuracy);
		//calc the size of this new polygon
		IStatObj *pIGeom = m_pESource->GetEntityStatObj(0, NULL);
		const char* pCGFName = pIGeom?pIGeom->GetFileName():"";
		const CString s(pCGFName);
#ifndef DISPLAY_MORE_INFO
		m_uiCoarseTexelCount += pNewPoly->CalcExtent(pScene, m_sGLMName, s, false, fAlteredGridSize, MIN_LIGHTMAP_SIZE,iMaxPolySize, uiHugePatchFoundNumber);
#else
		m_uiCoarseTexelCount += pNewPoly->CalcExtent(pScene, m_sGLMName, s, false, fAlteredGridSize, MIN_LIGHTMAP_SIZE,iMaxPolySize);
#endif
		int nEdgeX = abs(pNewPoly->m_nX2-pNewPoly->m_nX1);
		int nEdgeY = abs(pNewPoly->m_nY2-pNewPoly->m_nY1);

		if ((nEdgeX<1) || (nEdgeY<1))
			pNewPoly->m_dwFlags|=NOLIGHTMAP_FLAG; //too small for a lightmap
		else                        
		{
			//subdivide this polygon for lightmap calculation
			pNewPoly->m_dwFlags &= ~NOLIGHTMAP_FLAG;
		}	
		//add this polygon to the list of polygons to be processed
		pScene->m_lstScenePolys.push_back(pNewPoly);		    
	} //i  
#ifndef DISPLAY_MORE_INFO
	if(uiHugePatchFoundNumber == 1)
	{
		_TRACE("Too huge patch found, lowering resolution...\r\n");
	}
	else
	if(uiHugePatchFoundNumber > 1)
	{
		_TRACE("Too huge patches found, lowering resolution...\r\n");
	}
#endif
	//now check for fitting into a single lightmap if not just the sunlight lights this mesh(otherwise it will get compress easily)
	if(((m_uiFlags & ONLY_SUNLIGHT) == 0) && (m_uiCoarseTexelCount > pScene->m_sParam.m_iTextureResolution * pScene->m_sParam.m_iTextureResolution))
	{
		//how often do i need to halve the resolution?
		unsigned int uiIterations = (m_uiCoarseTexelCount / (pScene->m_sParam.m_iTextureResolution * pScene->m_sParam.m_iTextureResolution));
		//too may texels, halve resolution
		// If the function returns false, we ran out of lightmap space while processing the GLM.
		// We already couldn't fit it in last time, object won't fit into our LM size at all
		_TRACE(pScene->GetLogInfo(), true, "GLM cannot fit into lightmap size %ix%i, adjusting values...\r\n", 
			pScene->m_sParam.m_iTextureResolution, pScene->m_sParam.m_iTextureResolution);
		//now change nLightmapQuality to not let this happen again
		IStatObj *pIGeom = this->m_pESource->GetEntityStatObj(0, NULL);
		const CString s(pIGeom?pIGeom->GetFileName():"");
		if(pLightmapQualityVar)
		{
			float fCurrentValue = 1.f;					float fOldValue = 1.f;
			pLightmapQualityVar->Get(fOldValue);
			if(fCurrentValue > 0.f)//if not already reached maximum grid size
			{
				float fOldCopiedValue = fOldValue;
				while(uiIterations-- > 0)
				{
if(uiIterations == 0)
int michae = 0;
					fCurrentValue = pScene->ComputeHalvedLightmapQuality(fOldCopiedValue);//halve resolution
					fOldCopiedValue = fCurrentValue;//alter for next iteration
				}
				pLightmapQualityVar->Set(fCurrentValue);
				m_fLMAccuracy = fCurrentValue;
				_TRACE("...setting new value for nLightmapQuality to: %f \r\n",fCurrentValue);
			}
			char text[scuiWarningTextAllocation];
			sprintf(text, "GLM: %s (%s) did not fit into a single lightmap, nLightmapQuality has been changed from %f to %f\r\n",(const char*)m_sGLMName, s, fOldValue, fCurrentValue);
			pScene->GetWarningInfo().insert(std::pair<unsigned int, std::string>(EWARNING_NO_FIT, std::string(text)));
		}
		//alter grid size and adapt individually
		float fAlteredGridSize = fGridSize;
		if(m_fLMAccuracy > 1.0f)
			fAlteredGridSize /= m_fLMAccuracy;//use a more accurate texel<->world grid ratio
		else
			//use a less accurate texel<->world grid ratio
			fAlteredGridSize = fGridSize + (scfMaxGridSize - fGridSize) * (1.f - m_fLMAccuracy);
		//now recalculate patch extensions
#ifndef DISPLAY_MORE_INFO
		uiHugePatchFoundNumber = 0;
#endif
		for (radpolyit iter=pScene->m_lstScenePolys.begin(); iter!=pScene->m_lstScenePolys.end(); iter++)
#ifndef DISPLAY_MORE_INFO
			(*iter)->CalcExtent(pScene, m_sGLMName, s, false, fAlteredGridSize, MIN_LIGHTMAP_SIZE,iMaxPolySize, uiHugePatchFoundNumber);
#else
			(*iter)->CalcExtent(pScene, m_sGLMName, s, false, fAlteredGridSize, MIN_LIGHTMAP_SIZE,iMaxPolySize);
#endif
	#ifndef DISPLAY_MORE_INFO
		if(uiHugePatchFoundNumber == 1)
		{
			_TRACE(pScene->GetLogInfo(), true, "Too huge patch found, lowering resolution...\r\n");
		}
		else
		if(uiHugePatchFoundNumber > 1)
		{
			_TRACE(pScene->GetLogInfo(), true, "Too huge patches found, lowering resolution...\r\n");
		}
	#endif
	}
}

//generate the lightmap pictures
//////////////////////////////////////////////////////////////////////		
bool CRadMesh::SaveLightmaps(CLightScene *pScene,bool bDebug)
{  
	// ---------------------------------------------------------------------------------------------
	// Function returns false when the model doesn't fit into the LM (Allocate new and call again)
	// ---------------------------------------------------------------------------------------------
	//first generate all images to be able to blur the respective values
	//pass trough all lightmap meshes and generate image
	for (radpolyit i1=pScene->m_lstScenePolys.begin();i1!=pScene->m_lstScenePolys.end();i1++)
	{
		if((*i1) == NULL)
			continue;//deleted previously
	    CRadPoly *pPoly=(*i1);
		if (pPoly->m_dwFlags & NOLIGHTMAP_FLAG)
		{
			delete pPoly; *i1 = pPoly = NULL;//does only apply to patches
			continue;
		}
		if((pPoly->m_dwFlags & DO_NOT_COMPRESS_FLAG) == 0)
		{
			pPoly->Compress(MIN_LIGHTMAP_SIZE);
			//do not compress again
			pPoly->m_dwFlags |= DO_NOT_COMPRESS_FLAG;
		}
		//allocate aligned number of pixels
		int nW = pPoly->m_nW;
		int nH = pPoly->m_nH;
		//check if there's no space to add this subblock in the lightmap image
		if (!pScene->m_pCurrentPatch->GetSpace(nW, nH, pPoly->m_nOffsetW, pPoly->m_nOffsetH))
			return false;
	}
	//loop again now that we know that all patches fit into this lightmap
	for (radpolyit i1=pScene->m_lstScenePolys.begin();i1!=pScene->m_lstScenePolys.end();i1++)
	{
		if((*i1) == NULL)
			continue;//deleted previously
	    CRadPoly *pPoly=(*i1);

		pPoly->GenerateImage();
		 //GetLightmapSpace
		if (bDebug) 
			pPoly->AddWarningBorders(); //add warning border around the lightmap for debug		
		//copy this subblock in the bigger lightmap picture
		//now it is time to blur the edges of shared triangles, the patch images are generated by now
		pPoly->CopyData(pScene->m_pCurrentPatch, pScene->m_sParam.m_iTextureResolution);

		const double cdInvTexRes = 1.0 / (double)(pScene->m_sParam.m_iTextureResolution);
		//calculate s&t texture coordinates for all polygons
		for (radpolyit i3=pPoly->m_lstMerged.begin();i3!=pPoly->m_lstMerged.end();i3++)			
		{
			CRadPoly *pPoly3=(*i3);
			//pass through all vertices
			assert(pPoly3->m_lstOriginals.size() == 3);
			int k,nVerts=pPoly3->m_lstOriginals.size();			

			for (k=0;k!=nVerts;k++)
			{
				CRadVertex *pVert=&pPoly3->m_lstOriginals[k];
				pVert->m_fpX += (float)pPoly->m_nOffsetW;
				pVert->m_fpY += (float)pPoly->m_nOffsetH;
				pVert->m_fpX = (float)((double)(pVert->m_fpX) * cdInvTexRes);
				pVert->m_fpY = (float)((double)(pVert->m_fpY) * cdInvTexRes);				

				assert(pVert->m_fpX >= 0.0f && pVert->m_fpX <= 1.0f );
				assert(pVert->m_fpY >= 0.0f && pVert->m_fpY <= 1.0f );

			} //k    						
		} //i3
		//this polygon is no longer needed
		delete pPoly; *i1 = pPoly = NULL;
	} //i	  
	pScene->m_lstScenePolys.clear();
	return true;
}

///////////////////////////////////////////////////
void CRadPoly::FreeLightmaps( void )
{
	if(m_pLightmapData) 
	{	
		delete [] m_pLightmapData;m_pLightmapData=NULL; 
	}	
	if(m_pHDRLightmapData) 
	{	
		delete [] m_pHDRLightmapData; m_pHDRLightmapData=NULL; 
	}	
 
	if(m_pDominantDirData) 
	{	
		delete [] m_pDominantDirData;
		m_pDominantDirData=NULL; 
	}	
	if(m_pWSDominantDirData)
	{
		delete [] m_pWSDominantDirData;
		m_pWSDominantDirData = NULL;
	}
	if(m_pOcclMapData)
	{
		delete [] m_pOcclMapData; m_pOcclMapData = NULL;
	}
	
//	SynchronizeLightmaps();
}

// RGBE8 Encoding
// Store a common exponent for RGB into the alpha channel
// HDR_EXP_BASE=1.04 means dynamic range of ~23,000 (1.04^256)
// A bigger base value means:
// 1. Higher dynamic range
// 2. Lower resolution (Mach banding becomes noticeable)
static void sEncodeRGBE8(const float r, const float g, const float b, unsigned char *dataptr)
{
  CFColor vEncoded;
  // Determine the largest color component
	float fMaxComponent = max(max(r, g), b);
  fMaxComponent = max(0.0001f, fMaxComponent);
	// Round to the nearest integer exponent
	float fExp = floor(HDR_LOG(HDR_EXP_BASE, fMaxComponent));
  float fExpHDR = (fExp + HDR_EXP_OFFSET) / 256.0f;
  fExpHDR = CLAMP(fExpHDR, 0.0, 1.0f);
  float fInvExpHDR = 1.0f / pow(HDR_EXP_BASE, fExpHDR*256.0f - HDR_EXP_OFFSET);
  // Divide the components by the shared exponent
  vEncoded.r = r * fInvExpHDR * 255;
	vEncoded.g = g * fInvExpHDR * 255;
	vEncoded.b = b * fInvExpHDR * 255;

  // Store the shared exponent in the alpha channel
	vEncoded.a = fExpHDR * 255;

	dataptr[0] = min(vEncoded.r, 255); 
	dataptr[1] = min(vEncoded.g, 255); 
	dataptr[2] = min(vEncoded.b, 255); 
	dataptr[3] = min(vEncoded.a, 255); 
}

static Vec3 sDecodeRGBE8(unsigned char *dataptr)
{
  Vec3 vColor;

	float fExp = (float)dataptr[3] / 255.0f * 256.0f - HDR_EXP_OFFSET;
	float fScale = powf((float)HDR_EXP_BASE, fExp);

  vColor.x = (float)dataptr[0] / 255.0f * fScale;
  vColor.y = (float)dataptr[1] / 255.0f * fScale;
  vColor.z = (float)dataptr[2] / 255.0f * fScale;

  return vColor;
}

static CFColor sNormalizeColor(const float fColorRLamb, const float fColorGLamb, const float fColorBLamb, const float cfLM_DP3LerpFactor)
{
  float fLM_DP3LerpFactor = cfLM_DP3LerpFactor;
  float fR = fColorRLamb;
  float fG = fColorGLamb;
  float fB = fColorBLamb;

	// Correct overbright areas (fade away bumps)
#ifdef APPLY_COLOUR_FIX
	float fMaxValue = 0.5f * __max(fR, __max(fG, fB));
#else
	float fMaxValue = __max(fR, __max(fG, fB));
#endif
	if (fMaxValue > 1.0f)
	{
		// Saturation curve
		fMaxValue = 2.0f - 1.0f / fMaxValue;

		// DOT3 lerp factor
		fLM_DP3LerpFactor = __max(0.0f, fLM_DP3LerpFactor - fMaxValue + 1.0f);

		// Normalize color
		float fLen = 1.0f / fMaxValue;
		fR *= fLen;
		fG *= fLen;
		fB *= fLen;
	}
  return CFColor(fR, fG, fB, fLM_DP3LerpFactor);
}

#ifdef APPLY_COLOUR_FIX
void CRadPoly::GatherSubSamples(
	const unsigned int cuiX, const unsigned int cuiY, const unsigned int cuiSubSamples,
	const unsigned int *cpaIndicator, const float *cpfColours, const SComplexOSDot3Texel *pDot3,
	unsigned int& ruiMaxComponent, const SOcclusionMapTexel *cpfOccl, const GLMOcclLightInfo& rOcclInfo)
#else
void CRadPoly::GatherSubSamples(
	const unsigned int cuiX, const unsigned int cuiY, const unsigned int cuiSubSamples,
	const unsigned int *cpaIndicator, const float *cpfColours, const SComplexOSDot3Texel *pDot3, const SOcclusionMapTexel *cpfOccl, const GLMOcclLightInfo& rOcclInfo)
#endif
{
	assert(cpaIndicator);	assert(cpfColours);		assert(pDot3);
	assert(cuiX >= 0);		assert(cuiY >= 0);		assert(cuiX < m_nW);		assert(cuiY < m_nH);
	
	//first get handles to center values
	SComplexOSDot3Texel& rDot3Center	=	m_pWSDominantDirData[cuiY * m_nW + cuiX];
	unsigned char *pCenterColour		=	&(m_pLightmapData[4*(cuiY * m_nW + cuiX)]);
  Vec3 *pHDRCenterColour = NULL;
  if (m_pHDRLightmapData)
  	pHDRCenterColour		=	&m_pHDRLightmapData[cuiY * m_nW + cuiX];

	//first check how many valid samples we did gather
	unsigned int uiValidCount = 1;//center is always assumed to be valid
	for(int v=0;v<cuiSubSamples;v++)
	{
		if(cpaIndicator[v] != 0)
			uiValidCount++;
	}
	if(uiValidCount <= 1)
		return;	//nothing to do
	//now go through all samples and average them
	const real cfInvScale = 1.0/(real)uiValidCount;//scale for each sample
	real r = (((real)(*pCenterColour)) * cfInvScale / 255.0f);	real g = (((real)(*(pCenterColour+1))) * cfInvScale / 255.0f);	real b = (((real)(*(pCenterColour+2))) * cfInvScale / 255.0f);	real a = (((real)(*(pCenterColour+3))) * cfInvScale);
  Vec3 HDRColor = Vec3(0,0,0);
  if (pHDRCenterColour)
    HDRColor = *pHDRCenterColour * cfInvScale;
	Vec3d avWSLightDir(rDot3Center.vDot3Light);	
	for(int v=0;v<cuiSubSamples;v++)
	{
		if(cpaIndicator[v] == 0)
			continue;
		const float *pColour = &cpfColours[4*v];
    CFColor col = sNormalizeColor(pColour[0], pColour[1], pColour[2], pColour[3]);
		r += (real)col.r	* cfInvScale;
		g += (real)col.g	* cfInvScale;
		b += (real)col.b	* cfInvScale;
		a += (real)col.a	* cfInvScale;
    if (pHDRCenterColour)
    {
      HDRColor.x += pColour[0] * cfInvScale;
      HDRColor.y += pColour[1] * cfInvScale;
      HDRColor.z += pColour[2] * cfInvScale;
    }
		avWSLightDir += pDot3[v].vDot3Light * cfInvScale;
	}
	//renormalize light direction
	avWSLightDir.Normalize();
	//do occl map stuff
	if(cpfOccl && m_pOcclMapData)
	{
		unsigned int uiVisibilitySum[4] = {0,0,0,0};//count all 4 channels independently
		for(int v=0;v<cuiSubSamples;v++)
		{
			if(cpaIndicator[v] == 0)
				continue;
			const SOcclusionMapTexel& rTexel = cpfOccl[v];
			for(int i=0;i<rOcclInfo.UsedChannelCount();i++)
			{
				uiVisibilitySum[i] += rTexel[(EOCCLCOLOURASSIGNMENT)i];
			}
		}
		//now set the averaged value
		SOcclusionMapTexel& rOcclMapTexel = m_pOcclMapData[cuiY * m_nW + cuiX];
		for(int i=0;i<rOcclInfo.UsedChannelCount();i++)
		{
			const float cfRatio = (double)uiVisibilitySum[i] / (double)(15 * uiValidCount);
			rOcclMapTexel.SetValue((EOCCLCOLOURASSIGNMENT)i, (uint8)RoundFromFloat((float)(cfRatio * 15.0)));
		}
	}
  r *= 255.0f;
  g *= 255.0f;
  b *= 255.0f;
  a *= 255.0f;

  //set new values
#ifdef APPLY_COLOUR_FIX
	//keep same clamping range, get max for later colour scale
	const unsigned int uiR = __min(512, (unsigned int)r);
	const unsigned int uiG = __min(512, (unsigned int)g);
	const unsigned int uiB = __min(512, (unsigned int)b);
	const unsigned int cuiMax = __max(uiR, max(uiG, uiB));
	ruiMaxComponent = __max(cuiMax, ruiMaxComponent);

	*pCenterColour++	= (unsigned char) (__min(255, uiR));
	*pCenterColour++	= (unsigned char) (__min(255, uiG));
	*pCenterColour++	= (unsigned char) (__min(255, uiB));
  
#else
	*pCenterColour++	= (unsigned char)(__min(255.0f, r);
	*pCenterColour++	= (unsigned char)(__min(255.0f, g);
	*pCenterColour++	= (unsigned char)(__min(255.0f, b);
#endif
	*pCenterColour		= (unsigned char)(__min(255.0f, a));

  if (pHDRCenterColour)
    *pHDRCenterColour = HDRColor;

	rDot3Center.vDot3Light = avWSLightDir;
}

// destructor
CRadPoly::~CRadPoly()
{
	FreeLightmaps();
	assert(!m_pLightmapData);
	assert(!m_pHDRLightmapData);
	assert(!m_pDominantDirData);
	assert(!m_pWSDominantDirData);
	assert(!m_pOcclMapData);
}

//add green control lines on the borders (debug)
///////////////////////////////////////////////////
void CRadPoly::AddWarningBorders()
{  
  unsigned char r,g,b;
  r=0;
  g=255;
  b=0;
	// 4 byte version

	unsigned char *dataptr=m_pLightmapData;
	for (int py=0;py<m_nH;py++)
	{				
		//first pixel on the left
		dataptr[0]=0;dataptr[1]=255;dataptr[2]=0;dataptr[3]=0;

		for (int px=0;px<m_nW;px++)
		{	
			//upper line
			if (py==0)
			{
				dataptr[0]=0;dataptr[1]=0;dataptr[2]=255;dataptr[3]=0;
			}
			else
				//bottom line  
				if (py==(m_nH-1))	
				{
					dataptr[0]=255;dataptr[1]=255;dataptr[2]=0;dataptr[3]=0;
				}

				dataptr+=4;
		} //px

		//last pixel
		dataptr-=4;
		dataptr[0]=255;dataptr[1]=0;dataptr[2]=0;dataptr[3]=0;
		dataptr+=4;
	} //py
}

//copy data into subblocks
///////////////////////////////////////////////////
void CRadPoly::CopyData( CLightPatch *inpDestLightPatch, int dw )
{
	if(m_pLightmapData)
	{
		unsigned char *dptr=&inpDestLightPatch->m_pLightmapImage[4*(m_nOffsetH * dw + m_nOffsetW)];
		unsigned char *sptr=m_pLightmapData;

		assert(sptr);
		assert(dptr);

		for (int k=0;k<m_nH;k++)
		{
			memcpy(dptr,sptr,m_nW*4);
			dptr+=dw*4;
			sptr+=m_nW*4;
		}
	}
	if(m_pHDRLightmapData)
	{
		unsigned char *dptr=&inpDestLightPatch->m_pHDRLightmapImage[4*(m_nOffsetH * dw + m_nOffsetW)];
		Vec3 *sptr = m_pHDRLightmapData;

		assert(sptr);
		assert(dptr);

		for (int k=0;k<m_nH;k++)
		{
  		for (int w=0;w<m_nW;w++)
	  	{
        sEncodeRGBE8(sptr[w].x, sptr[w].y, sptr[w].z, &dptr[w*4]);
      }
			dptr+=dw*4;
			sptr+=m_nW;
		}
	}
	if(m_pDominantDirData)
	{

#ifdef USE_DOT3_ALPHA
		unsigned char *dptr=&inpDestLightPatch->m_pDominantDirImage[4*(m_nOffsetH * dw + m_nOffsetW)];
#else
		unsigned char *dptr=&inpDestLightPatch->m_pDominantDirImage[3*((m_nOffsetH * dw)+m_nOffsetW)];
#endif

		unsigned char *sptr=m_pDominantDirData;

		assert(sptr);
		assert(dptr);

		for(int k=0;k<m_nH;k++)
		{
#ifdef USE_DOT3_ALPHA
			memcpy(dptr,sptr,m_nW*4);
			dptr+=dw*4;
			sptr+=m_nW*4;
#else
			memcpy(dptr,sptr,m_nW*3);
			dptr+=dw*3;
			sptr+=m_nW*3;
#endif
		}
	}
	if(m_pOcclMapData)
	{
		uint16 *dptr=&(inpDestLightPatch->m_pOcclMapImage[m_nOffsetH * dw + m_nOffsetW].colour);
		uint16 *sptr=&(m_pOcclMapData->colour);

		assert(sptr);
		assert(dptr);
		uint16 *pTemp = dptr;
		for (int k=0;k<m_nH;k++)
		{
			dptr = pTemp;
			for (int l=0;l<m_nW;l++)
				*dptr++ = *sptr++;
			pTemp += dw;
		}
	}

	FreeLightmaps();		// 
}

//estimate the size of the polygon
#ifndef DISPLAY_MORE_INFO
const unsigned int CRadPoly::CalcExtent(CLightScene *pScene, const CString& rGLMName, const CString& rCGFName, bool bOriginal, const float fGridSize, const UINT iMinBlockSize, const UINT iMaxBlockSize, unsigned int& rHugePatchFoundNumber)
#else
const unsigned int CRadPoly::CalcExtent(CLightScene *pScene, const CString& rGLMName, const CString& rCGFName, bool bOriginal, const float fGridSize, const UINT iMinBlockSize, const UINT iMaxBlockSize)
#endif
{
	int type = CalcPlaneType2(m_Plane.n);
	//find best aligned plane
	if (type == PLANE_X || type == PLANE_MX) 
		m_nAxis = PLANE_X;
	else
	if (type == PLANE_Y || type == PLANE_MY) 
		m_nAxis = PLANE_Y;
	else
		m_nAxis = PLANE_Z;
    
	m_nX1 = m_nY1 = SHRT_MAX;
	m_nX2 = m_nY2 = -SHRT_MAX;
  
	if (m_nAxis == PLANE_X) 
	{
		m_nAx1 = PLANE_Z;
		m_nAx2 = PLANE_Y;		
	}
	else
	if (m_nAxis == PLANE_Y) 
	{
		m_nAx1 = PLANE_X;
		m_nAx2 = PLANE_Z;
	}
	else
	{
		m_nAx1 = PLANE_X;
		m_nAx2 = PLANE_Y;
	}
      
	m_fX1 = static_cast<float>(SHRT_MAX);
	m_fX2 = -static_cast<float>(SHRT_MAX);
	m_fY1 = static_cast<float>(SHRT_MAX);
	m_fY2 = -static_cast<float>(SHRT_MAX);

	//use original polygon    
	if (bOriginal)
	{
		int nNumOrig = m_lstOriginals.size();
		for (int v=0; v<nNumOrig; v++)
		{
			CRadVertex *rv = &m_lstOriginals[v];
      
			if (rv->m_vPos[(int)m_nAx1] < m_fX1) 
				m_fX1 = rv->m_vPos[(int)m_nAx1];
			if (rv->m_vPos[(int)m_nAx1] > m_fX2) 
				m_fX2 = rv->m_vPos[(int)m_nAx1];
      
			if (rv->m_vPos[(int)m_nAx2] < m_fY1) 
				m_fY1 = rv->m_vPos[(int)m_nAx2];
			if (rv->m_vPos[(int)m_nAx2] > m_fY2) 
				m_fY2 = rv->m_vPos[(int)m_nAx2];
		} //v
	}
	else	
	{
		if (m_lstMerged.empty())
			m_fX1=m_fX2=m_fY1=m_fY2=0;          
	    else
		//take into account merged polygons  
		for (radpolyit i3=m_lstMerged.begin(); i3!=m_lstMerged.end(); i3++)			
		{
			CRadPoly *pPoly=(*i3);
      
			int nNumOriginals = pPoly->m_lstOriginals.size();
			for (int v=0; v<nNumOriginals; v++)
			{
				CRadVertex *rv = &pPoly->m_lstOriginals[v];
       
				if (rv->m_vPos[(int)m_nAx1] < m_fX1) 
					m_fX1 = rv->m_vPos[(int)m_nAx1];
				if (rv->m_vPos[(int)m_nAx1] > m_fX2) 
					m_fX2 = rv->m_vPos[(int)m_nAx1];

				if (rv->m_vPos[(int)m_nAx2] < m_fY1) 
					m_fY1 = rv->m_vPos[(int)m_nAx2];
				if (rv->m_vPos[(int)m_nAx2] > m_fY2) 
					m_fY2 = rv->m_vPos[(int)m_nAx2];
			} //v
		} //i3
	}
 	// x1,y1,x2,y2 can be negative 
	m_nX1 = (short)floor(m_fX1 / fGridSize);
	m_nY1 = (short)floor(m_fY1 / fGridSize);
	m_nX2 = (short)ceil(m_fX2 / fGridSize);
	m_nY2 = (short)ceil(m_fY2 / fGridSize);
  
	m_nW = ((m_nX2 - m_nX1) + 1);
	m_nH = ((m_nY2 - m_nY1) + 1);

	int iOldW = m_nW, iOldH = m_nH;
	if (m_nW>iMaxBlockSize || m_nH>iMaxBlockSize)
	{
#ifndef DISPLAY_MORE_INFO
		rHugePatchFoundNumber++;
#endif
		if(m_nW>iMaxBlockSize && m_nH>iMaxBlockSize)
		{
#ifdef DISPLAY_MORE_INFO
			_TRACE(pScene->GetLogInfo(), true, "Huge patch found: %ix%i, allowed maximum: %ix%i texels (lowering resolution...)\r\n",iOldW,iOldH, iMaxBlockSize, iMaxBlockSize);
#else
			_TRACE(pScene->GetLogInfo(), false, "Huge patch found: %ix%i, allowed maximum: %ix%i texels (lowering resolution...)\r\n",iOldW,iOldH, iMaxBlockSize, iMaxBlockSize);
#endif
			char text[scuiWarningTextAllocation];
			sprintf(text, "GLM: %s (%s) too huge patch found: %ix%i, allowed maximum: %ix%i texels (lowering resolution...)!\r\n",(const char*)rGLMName, (const char*)rCGFName,iOldW,iOldH, iMaxBlockSize, iMaxBlockSize);
			pScene->m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_HUGE_PATCH, std::string(text)));
		}
		else
		{
			if(m_nW>iMaxBlockSize)
			{
#ifdef DISPLAY_MORE_INFO
				_TRACE(pScene->GetLogInfo(), true, "Huge patch found: %ix%i, allowed maximum: %ix%i texels (lowering resolution...)\r\n",iOldW,iOldH, iMaxBlockSize, iOldH);
#else
				_TRACE(pScene->GetLogInfo(), false, "Huge patch found: %ix%i, allowed maximum: %ix%i texels (lowering resolution...)\r\n",iOldW,iOldH, iMaxBlockSize, iOldH);
#endif
				char text[scuiWarningTextAllocation];
				sprintf(text, "GLM: %s (%s) too huge patch found: %ix%i, allowed maximum: %ix%i texels (lowering resolution...)!\r\n",(const char*)rGLMName, (const char*)rCGFName,iOldW,iOldH, iMaxBlockSize, iOldH);
				pScene->m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_HUGE_PATCH, std::string(text)));
			}
			else
			{
#ifdef DISPLAY_MORE_INFO
				_TRACE(pScene->GetLogInfo(), true, "Huge patch found: %ix%i, allowed maximum: %ix%i texels (lowering resolution...)\r\n",iOldW,iOldH, iOldW, iMaxBlockSize);
#else
				_TRACE(pScene->GetLogInfo(), false, "Huge patch found: %ix%i, allowed maximum: %ix%i texels (lowering resolution...)\r\n",iOldW,iOldH, iOldW, iMaxBlockSize);
#endif
				char text[scuiWarningTextAllocation];
				sprintf(text, "GLM: %s (%s) too huge patch found: %ix%i, allowed maximum: %ix%i texels (lowering resolution...)!\r\n",(const char*)rGLMName, (const char*)rCGFName,iOldW,iOldH, iOldW, iMaxBlockSize);
				pScene->m_WarningInfo.insert(std::pair<unsigned int, std::string>(EWARNING_HUGE_PATCH, std::string(text)));
			}
		}
		//now do the fGridSize dependent calls again
		// x1,y1,x2,y2 can be negative 
		const float cfNewGridFactor = max((float)m_nW/(float)iMaxBlockSize, (float)m_nH/(float)iMaxBlockSize);
		const float fInvNewGridSize = 1.f/(fGridSize * cfNewGridFactor);
		m_nX1=(short)floor(m_fX1 * fInvNewGridSize);
		m_nY1=(short)floor(m_fY1 * fInvNewGridSize);
		m_nX2=(short)ceil(m_fX2 * fInvNewGridSize);
		m_nY2=(short)ceil(m_fY2 * fInvNewGridSize);
		  
		m_nW=((m_nX2-m_nX1)+1);
		m_nH=((m_nY2-m_nY1)+1);

	}
	return m_nW * m_nH;//texel count
}

inline void CRadPoly::SynchronizeLightmaps()
{
	for (radpolyit it=m_lstMerged.begin();it!=m_lstMerged.end();it++)
    {
        CRadPoly *pPoly=(*it);
		pPoly->m_pWSDominantDirData	= m_pWSDominantDirData;
		pPoly->m_pDominantDirData	= m_pDominantDirData;
		pPoly->m_pLightmapData		= m_pLightmapData;
		pPoly->m_pHDRLightmapData		= m_pHDRLightmapData;
		pPoly->m_pOcclMapData		= m_pOcclMapData;
		pPoly->m_nW					= m_nW;
		pPoly->m_nH					= m_nH;
	}
}

//
///////////////////////////////////////////////////
void CRadPoly::AllocateDot3Lightmap(const bool cbGenerateHDRMaps, const bool cbGenerateOcclMaps )
{
	unsigned int cuiLMSize = m_nW*m_nH;
	m_pLightmapData=new unsigned char [cuiLMSize*4];						assert(m_pLightmapData);
	memset(m_pLightmapData,0,cuiLMSize*4);									//clear the image
	m_pWSDominantDirData = new SComplexOSDot3Texel[cuiLMSize];				assert(m_pWSDominantDirData);
  if (cbGenerateHDRMaps)
  {
	  m_pHDRLightmapData=new Vec3 [cuiLMSize];						assert(m_pHDRLightmapData);
    memset(m_pHDRLightmapData,0,cuiLMSize*sizeof(Vec3));									//clear the image
  }
  else
    m_pHDRLightmapData = NULL;
	if(cbGenerateOcclMaps)
	{
		m_pOcclMapData = new SOcclusionMapTexel[cuiLMSize];						assert(m_pOcclMapData);
	}
	else
		m_pOcclMapData = NULL;//just to make sure
	//pass pointer to contained polys
//	SynchronizeLightmaps();
}

//generate polygon lightmap image taking into account ambient color
///////////////////////////////////////////////////
void CRadPoly::GenerateImage( void )
{
	assert(m_nW>=1);
	assert(m_nH>=1);
  
  float mr=0,mg=0,mb=0;
  int numv=0;	
   
  float xoff=0;
	float yoff=0;

  //translate the subblock
  xoff=(float)(-m_nX1);
  yoff=(float)(-m_nY1);

  //add the list of polygons that are making up this poly   
  for (radpolyit i3=m_lstMerged.begin();i3!=m_lstMerged.end();i3++)			
  {
    CRadPoly *cb=(*i3);

    //calculate pixel position for each vertex
	int nNumOriginals=cb->m_lstOriginals.size();
    for (int v=0;v<nNumOriginals;v++)
    {
		CRadVertex *rv=&cb->m_lstOriginals[v];

		float x=rv->m_vPos[(int)m_nAx1];
		float y=rv->m_vPos[(int)m_nAx2];

		float w=(float)(m_nW-1);							if(w<0)w=0;
		float h=(float)(m_nH-1);							if(h<0)h=0;

		float divX=0.0f;	if(m_fX2-m_fX1>0.001)divX=1.0f/(m_fX2-m_fX1);
		float divY=0.0f;	if(m_fY2-m_fY1>0.001)divY=1.0f/(m_fY2-m_fY1);

		rv->m_fpX=(x-m_fX1)*divX*w+0.5f;
		rv->m_fpY=(y-m_fY1)*divY*h+0.5f;

		// check just in case some bad triangle is in
		if (rv->m_fpX<0.5f) 
			rv->m_fpX=0.5f;
		else 
		if (rv->m_fpX>(m_nW-0.5f)) 
			rv->m_fpX=(float)(m_nW-0.50001f);			
	    if (rv->m_fpY<0.5f) 
			rv->m_fpY=0.5f; 
		else 
		if (rv->m_fpY>(m_nH-0.5f)) 
			rv->m_fpY=(float)(m_nH-0.50001f);
	}
  } //i3    
}

void CRadPoly::SetSimpleLightmapTexel(const float infX, const float infY,
									  const int r, const int g, const int b, unsigned char iDP3Fac)
{ 
	assert(m_pLightmapData);
	const unsigned int cuiIndex = RoundFromFloat(infY)*m_nW + RoundFromFloat(infX);
	assert(cuiIndex < m_nW*m_nH);
	unsigned char *dataptr=&m_pLightmapData[cuiIndex*4];

	dataptr[0]=r>255?255:r; 
	dataptr[1]=g>255?255:g;
	dataptr[2]=b>255?255:b;
	dataptr[3]=iDP3Fac>255?255:iDP3Fac;	
} 

void CRadPoly::SetHDRLightmapTexel(const float infX, const float infY,
									  const float r, const float g, const float b)
{ 
	assert(m_pHDRLightmapData);
	const unsigned int cuiIndex = RoundFromFloat(infY)*m_nW + RoundFromFloat(infX);
	assert(cuiIndex < m_nW*m_nH);
	Vec3 *dataptr=&m_pHDRLightmapData[cuiIndex];
  dataptr->x = r;
  dataptr->y = g;
  dataptr->z = b;
} 

inline void CRadPoly::SetOcclLightmapTexel(const float infX, const float infY, const SOcclusionMapTexel& rTexel)
{ 
	if(!m_pOcclMapData)return;//not used
	const unsigned int cuiIndex = RoundFromFloat(infY)*m_nW + RoundFromFloat(infX);
	assert(cuiIndex < m_nW*m_nH);
	m_pOcclMapData[cuiIndex] = rTexel;
} 

#ifdef APPLY_COLOUR_FIX
const unsigned int CRadPoly::SetDot3LightmapTexel(const CRadVertex& rVertex, 
		                            const float fColorRLamb, const float fColorGLamb, const float fColorBLamb,
					                Vec3d &inLightDir, const float cfLM_DP3LerpFactor,
									SComplexOSDot3Texel& rDot3Texel, const SOcclusionMapTexel& rTexel, bool bHDR)
#else
void CRadPoly::SetDot3LightmapTexel(const CRadVertex& rVertex, 
		                            const float fColorRLamb, const float fColorGLamb, const float fColorBLamb,
					                Vec3d &inLightDir, const float cfLM_DP3LerpFactor,
									SComplexOSDot3Texel& rDot3Texel, const SOcclusionMapTexel& rTexel, bool bHDR)
#endif
{
	const float fInfX = rVertex.m_fpX;
	const float fInfY = rVertex.m_fpY;

	assert(m_pLightmapData);	assert(m_pWSDominantDirData);		
	assert((int)fInfX>=0);		assert((int)fInfY>=0);
	assert((int)fInfX<m_nW);	assert((int)fInfY<m_nH);

  CFColor col = sNormalizeColor(fColorRLamb, fColorGLamb, fColorBLamb, cfLM_DP3LerpFactor);

	const unsigned char iDP3Fac = (unsigned char) (col.a * 255.0f + 1.0f / 512.0f);

#ifdef APPLY_COLOUR_FIX
	//keep same clamping range, get max for later colour scale
	const unsigned int uiR = __min(512, (unsigned int)(col.r * 255.0f));
	const unsigned int uiG = __min(512, (unsigned int)(col.g * 255.0f));
	const unsigned int uiB = __min(512, (unsigned int)(col.b * 255.0f));
	const unsigned int cuiMax = __max(uiR, max(uiG, uiB));

	const unsigned char r = (unsigned char) (__min(255, uiR));
	const unsigned char g = (unsigned char) (__min(255, uiG));
	const unsigned char b = (unsigned char) (__min(255, uiB));
#else
	const unsigned char r = (unsigned char) (__min(255, col.r * 255.0f));
	const unsigned char g = (unsigned char) (__min(255, col.g * 255.0f));
	const unsigned char b = (unsigned char) (__min(255, col.b * 255.0f));
#endif

	SetSimpleLightmapTexel(fInfX,fInfY,r,g,b,iDP3Fac);
	SetOcclLightmapTexel(fInfX,fInfY,rTexel);
  if (bHDR)
  	SetHDRLightmapTexel(fInfX,fInfY,fColorRLamb,fColorGLamb,fColorBLamb);

	if(rDot3Texel.pSourcePatch == NULL)//not set, no light hit pixel
#ifdef APPLY_COLOUR_FIX
		return cuiMax; 
#else
		return;
#endif
	//only store world space light vector
	//set complex information
	assert(rDot3Texel.fAlpha >= 0.f && rDot3Texel.fBeta >= 0.f && rDot3Texel.fBeta + rDot3Texel.fAlpha <= 1.1f);
	rDot3Texel.vDot3Light = inLightDir;
	const unsigned int cuiLMIndex = RoundFromFloat(fInfY)*m_nW + RoundFromFloat(fInfX);
	m_pWSDominantDirData[cuiLMIndex] = rDot3Texel;
#ifdef APPLY_COLOUR_FIX
	return cuiMax;
#endif
}

#ifdef APPLY_COLOUR_FIX
void CRadPoly::SetDot3TSLightmapTexel(const unsigned int cuiX, const unsigned int cuiY, const unsigned int cuiColourFixAlpha, const float cfColourScale)
#else
void CRadPoly::SetDot3TSLightmapTexel(const unsigned int cuiX, const unsigned int cuiY)
#endif
{
	assert(m_pDominantDirData);		assert(m_pWSDominantDirData);		
	const unsigned int cuiLMIndex = cuiY* m_nW + cuiX;
#ifdef USE_DOT3_ALPHA
	unsigned char *domlightdir=&m_pDominantDirData[cuiLMIndex*4];
#else
	unsigned char *domlightdir=&m_pDominantDirData[cuiLMIndex*3];
#endif
	SComplexOSDot3Texel& rDot3Texel = m_pWSDominantDirData[cuiY * m_nW + cuiX];
#ifdef USE_DOT3_ALPHA
#ifdef APPLY_COLOUR_FIX
	domlightdir[3] = (unsigned char)cuiColourFixAlpha;
	//fix colour
	unsigned char *pucColour=&m_pLightmapData[cuiLMIndex*4];
	for(int i=0; i<3;i++)
  {
		pucColour[i] = (unsigned char)((float)pucColour[i] * cfColourScale);
  }
  if (m_pHDRLightmapData)
    m_pHDRLightmapData[cuiLMIndex] *= cfColourScale;
#else
	domlightdir[3] = 255;
#endif
#endif
	if(rDot3Texel.pSourcePatch == NULL)//no light has hit texel
	{
		domlightdir[0]=(unsigned char)128;
		domlightdir[1]=(unsigned char)128;	
		domlightdir[2]=(unsigned char)128;
		return;
	}

	Vec3d vLightDir = rDot3Texel.TransformIntoTS(rDot3Texel.vDot3Light);

	int lx = (int) (vLightDir.x * 127.5f+127.5f);
	int ly = (int) (vLightDir.y * 127.5f+127.5f);
	int lz = (int) (vLightDir.z * 127.5f+127.5f);

	assert(lx >= 0);	assert(lx <= 255);
	assert(ly >= 0);	assert(ly <= 255);
	assert(lz >= 0);	assert(lz <= 255);

	domlightdir[0]=(unsigned char)lz;
	domlightdir[1]=(unsigned char)ly;	
	domlightdir[2]=(unsigned char)lx;
}

// compress patch to 1x1 if constant value
void CRadPoly::Compress( const unsigned int cuiMinBlockSize = 4 )
{
	assert(m_pDominantDirData);
	assert(m_pLightmapData);

#ifdef USE_DOT3_ALPHA
	unsigned char la;
#endif
	unsigned char r,g,b,l,lx,ly,lz;

	r=m_pLightmapData[0];
	g=m_pLightmapData[1];
	b=m_pLightmapData[2];
	l=m_pLightmapData[3];
	lx=m_pDominantDirData[0];
	ly=m_pDominantDirData[1];
	lz=m_pDominantDirData[2];
#ifdef USE_DOT3_ALPHA
	la=m_pDominantDirData[3];
#endif
	unsigned char * pRGBL=(m_pLightmapData+4);		// stride 4

#ifdef USE_DOT3_ALPHA
	unsigned char * pDir=(m_pDominantDirData+4);		// stride 3
#else
	unsigned char * pDir=(m_pDominantDirData+3);		// stride 3
#endif
	if(m_pOcclMapData)
	{
		const SOcclusionMapTexel occl = m_pOcclMapData[0];
		SOcclusionMapTexel * pOccl=m_pOcclMapData+1;

		for(int y=0;y<m_nH;y++)
		for(int x=1;x<m_nW;x++)
		{
			if(r!=pRGBL[0]
			|| g!=pRGBL[1]
			|| b!=pRGBL[2]
			|| l!=pRGBL[3]
			|| lx!=pDir[0]
			|| ly!=pDir[1]
			|| lz!=pDir[2]
	#ifdef USE_DOT3_ALPHA
			|| la!=pDir[3]
	#endif
			|| occl != *pOccl++
			) 
				return;				// not constant color

			pRGBL+=4;
	#ifdef USE_DOT3_ALPHA
			pDir+=4;
	#else
			pDir+=3;
	#endif
		}
	}
	else
	{
		for(int y=0;y<m_nH;y++)
		for(int x=1;x<m_nW;x++)
		{
			if(r!=pRGBL[0]
			|| g!=pRGBL[1]
			|| b!=pRGBL[2]
			|| l!=pRGBL[3]
			|| lx!=pDir[0]
			|| ly!=pDir[1]
			|| lz!=pDir[2]
	#ifdef USE_DOT3_ALPHA
			|| la!=pDir[3]
	#endif
			) 
				return;				// not constant color

			pRGBL+=4;
	#ifdef USE_DOT3_ALPHA
			pDir+=4;
	#else
			pDir+=3;
	#endif
		}
	}
	// 1 texel is enough
	m_nW=m_nH=1;
	//add the list of polygons that are making up this poly   
	for (radpolyit i3=m_lstMerged.begin();i3!=m_lstMerged.end();i3++)			
	{
		CRadPoly *cb=(*i3);
		//pass through all vertices
		assert(cb->m_lstOriginals.size() == 3);
		int nVerts=cb->m_lstOriginals.size();			

		for(int k=0;k!=nVerts;k++)
		{
			CRadVertex *pVert=&cb->m_lstOriginals[k];
			pVert->m_fpX=0.5f;
			pVert->m_fpY=0.5f;
		} //k
	}
}




