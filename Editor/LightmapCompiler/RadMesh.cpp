// ---------------------------------------------------------------------------------------------
//	Crytek CryENGINE source code
//	History:
//	- Created by Marco Corbetta
//  - Rewritten by Tim Schroeder
//	- Partial rewrite for editor integration
// ---------------------------------------------------------------------------------------------

#include "stdafx.h"
#include <float.h>
#include <I3dEngine.h>
#include <IEntityRenderstate.h>
//#include "IndoorLightPatches.h"
#include <list2.h>
#include <float.h> 
#include "IShader.h"
#include "IndoorLightPatches.h"
#include "../Material/Material.h"
#include "../../CryCommon/CryHeaders.h"

//threshold constants
static const float scfBaryThreshold = 1.001f;

extern CAASettings gAASettings;	

//!!!projects point into triangle
const bool CRadPoly::CheckPointInTriangle(Vec3d &inPosition, const Vec3d &rVert0, const Vec3d &rVert1, const Vec3d &rVert2, float &rOutfAlpha, float &rOutfBeta, float &rfArea3d)
{
	//project into triangle plane
	Vec3d oneEdge = rVert0 - rVert1; 
	Vec3d secondEdge = rVert0 - rVert2; 
	Vec3d vTrianglePlaneNormal =  oneEdge % secondEdge; 
	vTrianglePlaneNormal.Normalize();	
	Vec3d vDir = inPosition - rVert0;
	if(vDir * vTrianglePlaneNormal > 0)
		vDir *= -1;
	const float cfPlaneDist2 = -(vDir*vTrianglePlaneNormal); 
	Vec3d r	= vTrianglePlaneNormal*(cfPlaneDist2); 
	const Vec3d vNewInPos = inPosition = (inPosition + r);
	rfArea3d = CalcTriangleArea(rVert0,rVert1,rVert2);
	// to prevent crash
	if(rfArea3d == 0.0f)
	{
		rOutfAlpha = rOutfBeta = 0.0f;
		return false;  
	}
	//first sub tri
	const float a1=CalcTriangleArea(vNewInPos,rVert1,rVert2);
	//second sub tri  
	const float a2=CalcTriangleArea(rVert0,vNewInPos,rVert2);
	//third sub tri  
	const float a3=CalcTriangleArea(rVert0,rVert1,vNewInPos);
	//check sum, must be close to real area
	const float asum = a1+a2+a3;
	bool cbBaryFailCond = false;
	if(fabs(asum - rfArea3d) > rfArea3d*0.01f)
		cbBaryFailCond = true;	//area difference to large
	if(cbBaryFailCond == false)
	{
		//calc alpha beta and gamma components as area ratio
		rOutfAlpha			= a1 / rfArea3d;
		rOutfBeta			= a2 / rfArea3d;
		cbBaryFailCond = ((rOutfAlpha + rOutfBeta) > scfBaryThreshold)?true:false;	//set to fail if barycentric coords are invalid
	}
	return cbBaryFailCond;
}

const Vec3d SComplexOSDot3Texel::TransformIntoTS(Vec3d& rSource)const
{
	assert(pSourcePatch && pSourcePatch->m_lstOriginals.size() == 3);//make sure it is a valid poly
	const float cfSqLength = rSource.GetLengthSquared();
	if(fabs(cfSqLength - 1.f) > 0.01f && cfSqLength > 0.01f)//leave null vector as it is
		rSource.Normalize();
	//correct the values to not let the sum go anywhere but 1.0f
	float cfAlpha = (fAlpha > 1.0f)?1.0f:fAlpha;	
	float cfBeta = (fBeta > 1.0f)?1.0f:fBeta;	
	float cfGamma = (1.0f-cfAlpha-cfBeta);
	if(cfGamma < 0.0f)
	{
		const float cfError = cfAlpha + cfBeta - 1.0f;
		if(cfAlpha >= cfError)
			cfAlpha -= cfError;
		else 
			cfBeta -= cfError;
		cfGamma = 0.0f;
	}
	Matrix33 matTangentTranf;
	const CRadVertex& rVert0 = pSourcePatch->m_lstOriginals[0];
	const CRadVertex& rVert1 = pSourcePatch->m_lstOriginals[1];
	const CRadVertex& rVert2 = pSourcePatch->m_lstOriginals[2];

	Vec3d vTNormal = rVert0.m_vTNormal * cfAlpha + rVert1.m_vTNormal * cfBeta + rVert2.m_vTNormal * cfGamma;
	vTNormal.Normalize();
	Vec3d vBinormal = rVert0.m_vBinormal * cfAlpha + rVert1.m_vBinormal * cfBeta + rVert2.m_vBinormal * cfGamma;
	vBinormal.Normalize();
	Vec3d vTangent = rVert0.m_vTangent * cfAlpha + rVert1.m_vTangent * cfBeta + rVert2.m_vTangent * cfGamma;
	vTangent.Normalize();

	matTangentTranf(0,0) = vTNormal.x;
	matTangentTranf(0,1) = vTNormal.y;
	matTangentTranf(0,2) = vTNormal.z;

	matTangentTranf(1,0) = vBinormal.x; 
	matTangentTranf(1,1) = vBinormal.y;
	matTangentTranf(1,2) = vBinormal.z;

	matTangentTranf(2,0) = vTangent.x;
	matTangentTranf(2,1) = vTangent.y;
	matTangentTranf(2,2) = vTangent.z;

	Vec3d vResult = (matTangentTranf * rSource);
	if(fabs(vResult.GetLengthSquared() - 1.f) > 0.01f)
		vResult.Normalize();
	return vResult;
}

void CRadPoly::ApplyTangentSpaceToVertex(CRadVertex &outVertex, float cfAlpha, float cfBeta)
{
	//correct the values to not let the sum go anywhere but 1.0f
	cfAlpha = (cfAlpha > 1.0f)?1.0f:cfAlpha;	
	cfBeta = (cfBeta > 1.0f)?1.0f:cfBeta;	
	float cfGamma = (1.0f-cfAlpha-cfBeta);
	if(cfGamma < 0.0f)
	{
		const float cfError = cfAlpha + cfBeta - 1.0f;
		if(cfAlpha >= cfError)
			cfAlpha -= cfError;
		else
			cfBeta -= cfError;
		cfGamma = 0.0f;
	}
	outVertex.m_vTNormal = 
		m_lstOriginals[0].m_vTNormal * cfAlpha
		+ m_lstOriginals[1].m_vTNormal * cfBeta
		+ m_lstOriginals[2].m_vTNormal * cfGamma;
	outVertex.m_vTNormal.Normalize();
	outVertex.m_vBinormal = 
		m_lstOriginals[0].m_vBinormal * cfAlpha
		+ m_lstOriginals[1].m_vBinormal * cfBeta
		+ m_lstOriginals[2].m_vBinormal * cfGamma;
	outVertex.m_vBinormal.Normalize();
	outVertex.m_vTangent = 
		m_lstOriginals[0].m_vTangent * cfAlpha
		+ m_lstOriginals[1].m_vTangent * cfBeta
		+ m_lstOriginals[2].m_vTangent * cfGamma;
	outVertex.m_vTangent.Normalize();
}

void CRadPoly::ApplyBaryCoordsToVertex(CRadVertex& rDest, const CRadVertex& rSource0, const CRadVertex& rSource1, const CRadVertex& rSource2, float cfAlpha, float cfBeta, const bool cbCOmputeTS)
{
	//correct the values to not let the sum go anywhere but 1.0f
	cfAlpha = (cfAlpha > 1.0f)?1.0f:cfAlpha;	
	cfBeta = (cfBeta > 1.0f)?1.0f:cfBeta;	
	float cfGamma = (1.0f-cfAlpha-cfBeta);
	if(cfGamma < 0.0f)
	{
		const float cfError = cfAlpha + cfBeta - 1.0f;
		if(cfAlpha >= cfError)
			cfAlpha -= cfError;
		else
			cfBeta -= cfError;
		cfGamma = 0.0f;
	}
	rDest.m_vPos =  
		rSource0.m_vPos * cfAlpha
		+ rSource1.m_vPos * cfBeta
		+ rSource2.m_vPos * cfGamma;
	rDest.m_vNormal = 
		rSource0.m_vNormal * cfAlpha
		+ rSource1.m_vNormal * cfBeta
		+ rSource2.m_vNormal * cfGamma;
	rDest.m_vNormal.Normalize();
	if(cbCOmputeTS)
	{
		rDest.m_vTNormal = 
			rSource0.m_vTNormal * cfAlpha
			+ rSource1.m_vTNormal * cfBeta
			+ rSource2.m_vTNormal * cfGamma;
		rDest.m_vTNormal.Normalize();
		rDest.m_vBinormal = 
			rSource0.m_vBinormal * cfAlpha
			+ rSource1.m_vBinormal * cfBeta
			+ rSource2.m_vBinormal * cfGamma;
		rDest.m_vBinormal.Normalize();
		rDest.m_vTangent = 
			rSource0.m_vTangent * cfAlpha
			+ rSource1.m_vTangent * cfBeta
			+ rSource2.m_vTangent * cfGamma;
		rDest.m_vTangent.Normalize();
	}
}

CRadPoly::CRadPoly() : 
	m_pSource(NULL),	m_pMergeSource(NULL), m_dwFlags(0), m_nX1(0), m_nY1(0), m_nX2(0), m_nY2(0),
	m_fX1(0.f), m_fY1(0.f), m_fX2(0.f), m_fY2(0.f),	m_nW(0), m_nH(0), m_nAx1(0), m_nAx2(0), m_nAxis(0),
	m_pLightmapData(NULL), m_pHDRLightmapData(NULL), m_pDominantDirData(NULL), m_pWSDominantDirData(NULL), m_pOcclMapData(NULL){}

const float CRadPoly::PointInTriangle(const Vec3d &point, const int ciIndex0, const int ciIndex1)
{
	assert(m_lstOriginals.size() == 3);

	float fOutfNearestDistance=FLT_MAX;

	float xt = point[ciIndex0];
	float yt = point[ciIndex1];
  
	float Ax,Ay,Bx,By;
	float s;
  
	bool bFront=false;
	bool bBack=false;
  
	int kk2;

	//check if all the vertices are on the same
	//side of the edges
	for (int k=0;k<3;k++)
	{
    
		kk2=(k+1)%3;
    
		Ax=m_lstOriginals[k].m_vPos[ciIndex0];Bx=m_lstOriginals[kk2].m_vPos[ciIndex0];
		Ay=m_lstOriginals[k].m_vPos[ciIndex1];By=m_lstOriginals[kk2].m_vPos[ciIndex1];

		s=((Ay-yt)*(Bx-Ax)-(Ax-xt)*(By-Ay));

		float fEdgeLength=sqrtf( sqr(Ax-Bx) + sqr(Ay-By) );

		float fDistance=sqrtf(sqr(Ax-xt)+sqr(Ay-yt)) + sqrtf(sqr(Bx-xt)+sqr(By-yt)) - fEdgeLength;
	
		if(fDistance<fOutfNearestDistance)
			fOutfNearestDistance=fDistance;
		//as soon as we find a different sign that means the 
		//point is outside the polygon
		if (s>=0) 
			bFront=true;
		else 		
			bBack=true; 
	} //k
	if(bFront != bBack)
	{
		return 0.f;
	}
	return fOutfNearestDistance;
}

//check if this polygon has connections with another poly
//////////////////////////////////////////////////////////////////////
bool CRadPoly::Connected(CRadPoly *pOther)
{ 
	static const float epsPos				= 0.00001f;		//be more correct, vertex position should really be the same

	CRadVertex *pVert1,*pVert2;
	for (radpolyit i=m_lstMerged.begin();i!=m_lstMerged.end();i++)
	{
		CRadPoly *pPoly=(*i);

		//pass trough all vertices to check if at least one is connected
		for (radvertexit v1=pPoly->m_lstOriginals.begin();v1<pPoly->m_lstOriginals.end();v1++)
		{	
			pVert1=&(*v1);
			for (radvertexit v2=pOther->m_lstOriginals.begin();v2<pOther->m_lstOriginals.end();v2++)
			{				
				pVert2=&(*v2);
				if (IsEquivalent(pVert1->m_vPos,pVert2->m_vPos,epsPos) )
					return (true);
			} //v2
		} //v1
	} //i
    
	return (false);
}

CRadPoly::CRadPoly(CRadPoly *pSource) : 
	m_Plane(pSource->m_Plane),m_dwFlags(pSource->m_dwFlags),m_pSource(pSource),
	m_fX1(pSource->m_fX1),m_fX2(pSource->m_fX2),m_fY1(pSource->m_fY1),m_fY2(pSource->m_fY2), 
	m_nX1(0), m_nY1(0), m_nX2(0), m_nY2(0), m_nW(0), m_nH(0), m_nAx1(0), m_nAx2(0), m_nAxis(0),
	m_pLightmapData(NULL), m_pHDRLightmapData(NULL), m_pDominantDirData(NULL), m_pWSDominantDirData(NULL),m_pOcclMapData(NULL){}

void CRadPoly::CalculateTangentSpace(SUV vUV[3])
{
	assert(m_lstOriginals.size() == 3);//assure it is the real triangle
	//now compute tangent space according to face normal
	const Vec3d vEdge0 = m_lstOriginals[1].m_vPos - m_lstOriginals[0].m_vPos;
	const Vec3d vEdge1 = m_lstOriginals[2].m_vPos - m_lstOriginals[0].m_vPos;
	Vec3d vCross = vEdge0 ^ vEdge1;		vCross.Normalize();
	const real fDeltaU1=vUV[1].u-vUV[0].u;
	const real fDeltaU2=vUV[2].u-vUV[0].u;
	const real fDeltaV1=vUV[1].v-vUV[0].v;
	const real fDeltaV2=vUV[2].v-vUV[0].v;
	const real div	=(fDeltaU1*fDeltaV2-fDeltaU2*fDeltaV1);
	Vec3d vU,vV;
	if(div!=0.0f)
	{
		//	area(u1*v2-u2*v1)/2
		const real fAreaMul2=fabsf(fDeltaU1*fDeltaV2-fDeltaU2*fDeltaV1);  // weight the tangent vectors by the UV triangles area size (fix problems with base UV assignment)

		const float a = (float)fDeltaV2/div;
		const float b = (float)-fDeltaV1/div;
		const float c = (float)-fDeltaU2/div;
		const float d = (float)fDeltaU1/div;

		vU = (vEdge0 * a + vEdge1 * b) * fAreaMul2;	vU.Normalize();
		vV = (vEdge0 * c + vEdge1 * d) * fAreaMul2;	vV.Normalize();
	}
	else  
	{ 
		vU = Vec3d(0,0,0);	vV = Vec3d(0,0,0); 
	}

	for(int k=0; k<3; k++)
	{
		CRadVertex& rVert	= m_lstOriginals[k];
		rVert.m_vNormal		= -vCross;	//use face normal, might get smoothed later on
		rVert.m_vBinormal	= -vV;
		rVert.m_vTangent	= -vU;		
		rVert.m_vTNormal	= -vCross;
	}
}

bool CRadPoly::InterpolateVertexAt( const float infX, const float infY, CRadVertex &outVertex, SComplexOSDot3Texel& rDot3Texel, const bool cbSubSample, const bool cbSnap)
{
	static const real scfLongRay = (real)1180.0;

	const real cfAAInfX		= cbSubSample? gAASettings.RetrieveRealSamplePos(infX) : infX;
	const real cfAAInfY		= cbSubSample? gAASettings.RetrieveRealSamplePos(infY) : infY;
	const CRadPoly *pData	= m_lstOriginals.empty()? m_pSource : this;
	const real fPointOn		= pData->m_lstOriginals[0].m_vPos[(int)m_nAxis];
	const real fXstart		= m_fX1 + (cfAAInfX)/(real)(m_nW-1)*(m_fX2-m_fX1);//if AA is used, invert texture coordinate to get back to the right resolution
	const real fYstart		= m_fY1 + (cfAAInfY)/(real)(m_nH-1)*(m_fY2-m_fY1);//if AA is used, invert texture coordinate to get back to the right resolution

	Vec3d vPos1,vPos2;  
	vPos1[(int)m_nAx1]		= vPos2[(int)m_nAx1] = fXstart;
	vPos1[(int)m_nAx2]		= vPos2[(int)m_nAx2] = fYstart;

	Vec3d& rvPoint = outVertex.m_vPos;
	//axial plane is fast
	if (CalcPlaneType2(m_Plane.n) <= PLANE_Z)
	{
		rvPoint					= vPos1;
		rvPoint[(int)m_nAxis]	= fPointOn;
	}
	else
	{	
		//get the exact point position
		vPos1[(int)m_nAxis] = fPointOn - scfLongRay;
		vPos2[(int)m_nAxis] = fPointOn + scfLongRay;

		const real fDist1	= m_Plane.DistFromPlane(vPos1);
		const real fDist2	= m_Plane.DistFromPlane(vPos2);

		if(abs(fDist1 - fDist2) < (real)0.00001)
			return(false);

		Vec3d vVert = vPos2 - vPos1;
		const real fDot = fDist1 / (fDist1 - fDist2);
		vVert	= vVert * fDot;
		rvPoint = vPos1 + vVert;
	}

	outVertex.m_fpX		= infX;
	outVertex.m_fpY		= infY;

	// interpolate normals and tangent base vectors within the triangle
	CRadPoly* nearestpoly = GetNearestPolyAt(rvPoint);
	if((nearestpoly == NULL) || !nearestpoly->ApplyBarycentricCoordinates(rvPoint, outVertex, rDot3Texel, false, cbSnap))
		return false;
	return true;
}

CRadPoly *CRadPoly::GetNearestPolyAt( const Vec3d &inPosition )
{
	float fBestDistance=FLT_MAX;
	CRadPoly *pBestPoly=0;

	for (radpolyit i=m_lstMerged.begin();i!=m_lstMerged.end();i++)
	{
		float fDistance=FLT_MAX;
		CRadPoly *pPoly=(*i);

		if(pPoly->m_lstOriginals.empty())
			pPoly=pPoly->m_pSource;
 
		fDistance = pPoly->PointInTriangle(inPosition,m_nAx1,m_nAx2);

		if(fDistance == 0.f)
		{
			//force early out
			fBestDistance = 0.f;
			return pPoly;
		}
		if(fDistance<fBestDistance)
		{
			fBestDistance=fDistance;
			pBestPoly=pPoly;
		}
	}
	return(pBestPoly);
}

CRadMesh::~CRadMesh()
{
	radpolyit i;
	for (i=m_lstOldPolys.begin();i!=m_lstOldPolys.end();i++)
	{
		CRadPoly *pPoly=(*i);
		delete pPoly; *i = pPoly = NULL;
	} //i
}

//////////////////////////////////////////////////////////////////////
void CRadMesh::CreateEmpty( IEntityRender *pIEtyRend, const Matrix44& matEtyMtx )
{
	IStatObj *pObj = pIEtyRend->GetEntityStatObj(0, NULL);

	//calc sphere's radius
	Vec3d vVec=(pObj->GetBoxMax()-pObj->GetBoxMin())/2;
	m_fRadius=vVec.Length();
	m_vCenter = matEtyMtx.TransformPointOLD(pObj->GetBoxMin()) + vVec; 
	m_fRadius2=(m_fRadius*m_fRadius);	
	m_dwHashValue=CalcLocalLightHashValue();
	for(int i=0;i<16;i++)
		CalcNCombineHash(matEtyMtx.data[i],m_dwHashValue);
}

const bool CRadMesh::FillInValues(IEntityRender *pIEtyRend, const CBrushObject *pBrushObject, const Matrix44& matEtyMtx)
{
	IStatObj *pObj = pIEtyRend->GetEntityStatObj(0, NULL);
	m_pESource = pIEtyRend;

	CLeafBuffer *pLB = pObj->GetLeafBuffer();
	m_fMaxVariance = 0.f;
	if (pLB->IsEmpty())
		return false;
	m_sGLMName = pBrushObject->GetName();

	//set individual lightmap accuracy and glm name
	m_fLMAccuracy = 1.f;
	assert(pBrushObject);
	CMaterial *pMat = pBrushObject->GetMaterial();
	if(pMat)
	{
		if((pMat->GetShaderResources().m_ResFlags) & MTLFLAG_2SIDED)
		{
			IMatInfo *pMatInfo = pMat->GetMatInfo();
			if(pMatInfo)
			{	
				// Check if the material is not a no_draw one
				IShader *pEff = pMatInfo->GetShaderItem().m_pShader;
				if (pEff)
				{  
					IShader *pShTempl = pEff->GetTemplate(-1);
					if (pShTempl)
					{  
						if(!(pShTempl->GetFlags3() & EF3_NODRAW))
							m_uiFlags |=  DOUBLE_SIDED_MAT; 
					}
				}
			}
		}
	}
	else
	{	
		if(pLB->m_pMats)
		{
			list2<CMatInfo>& rMatInfoList = *pLB->m_pMats;
			CMatInfo* pElems = rMatInfoList.GetElements();
			for(int i=0; i<rMatInfoList.size();i++)
			{
				// Check if the material is not a no_draw one
				IShader *pEff = pElems[i].shaderItem.m_pShader;
				if (pEff)
				{  
					IShader *pShTempl = pEff->GetTemplate(-1);
					if (pShTempl)
					{  
						if(pShTempl->GetFlags3() & EF3_NODRAW)
						{ 
							continue;
						}
					}
				}
				SRenderShaderResources *pRSR = (pElems[i].GetShaderItem()).m_pShaderResources;
				if(pRSR && (pRSR->m_ResFlags & MTLFLAG_2SIDED))
				{
					m_uiFlags |=  DOUBLE_SIDED_MAT; 
					break;
				}
			}
		}
	}
	CVarBlock *pVarBlock = pBrushObject->GetVarBlock();
	if(pVarBlock)
	{
		pLightmapQualityVar = pVarBlock->FindVariable("LightmapQuality");
		if(pLightmapQualityVar && (pLightmapQualityVar->GetType() == IVariable::FLOAT))
		{
			pLightmapQualityVar->Get(m_fLMAccuracy);
		}
		IVariable *pVar = pVarBlock->FindVariable("ReceiveLightmap");
		if(pVar && (pVar->GetType() == IVariable::BOOL))
		{
			pVar->Get(m_bReceiveLightmap);
			if(m_bReceiveLightmap)
				m_uiFlags |= RECEIVES_LIGHTMAP;//necessary since m_bReceiveLightmap will be used for switching it off for other reason as well
		}
	}
	pVarBlock = pBrushObject->GetVarBlock();
	if(pVarBlock)
	{
		IVariable *pVar = pVarBlock->FindVariable("CastLightmap");
		if(pVar && (pVar->GetType() == IVariable::BOOL))
		{
			bool bCastLightmap = false;
			pVar->Get(bCastLightmap);
			if(bCastLightmap)
				m_uiFlags |= CAST_LIGHTMAP;
		}
	}
	//update hash according to flags
	UpdateHash(((m_uiFlags & CAST_LIGHTMAP)?0x44444444:0) | (m_bReceiveLightmap?0x88888888:0));

	// Get indices and vertices
	int iIdxCnt = 0;
	unsigned short *pIndices = pLB->GetIndices(&iIdxCnt);
	int iVtxStride = 0;
	Vec3d *pVertices = reinterpret_cast<Vec3d *> (pLB->GetPosPtr(iVtxStride));

	// Get NBT
	int iNormalStride = 0;
	Vec3d *pNormals = reinterpret_cast<Vec3d *> (pLB->GetNormalPtr(iNormalStride));
	int iTangentStride = 0;
	Vec3d *pTangents = reinterpret_cast<Vec3d *> (pLB->GetTangentPtr(iTangentStride));
	int iBinormalStride = 0;
	Vec3d *pBinormals = reinterpret_cast<Vec3d *> (pLB->GetBinormalPtr(iBinormalStride));
	int iTNormalStride = 0;
	Vec3d *pTNormals = reinterpret_cast<Vec3d *> (pLB->GetTNormalPtr(iTNormalStride));
	int iUVStride = 0;
	SUV *pUV = reinterpret_cast<SUV *> (pLB->GetUVPtr(iUVStride));

	m_uiTexCoordsRequired = pLB->m_SecVertCount;
	m_lstOldPolys.reserve(iIdxCnt / 3);

	UINT iCurMat;
	list2<CMatInfo> *pMaterials = pLB->m_pMats;
	for (iCurMat=0; iCurMat<pMaterials->Count(); iCurMat++)
	{
		CMatInfo cCurMat = (* pMaterials)[iCurMat];
		bool bIsDecalMat = false;

		// Check if the material is opaque, we don't let non-opaque geometry cast shadows
		IShader *pEff = cCurMat.shaderItem.m_pShader;
		if (pEff)
		{  
			IShader *pShTempl = pEff->GetTemplate(-1);
			if (pShTempl)
			{  
				CString sShaderName = pShTempl->GetName();
				if(sShaderName.Find("templdecal") != -1)  	//do not compute decals (i love exceptions)
					bIsDecalMat = true;
				if(pShTempl->GetFlags3() & EF3_NODRAW)
				{ 
					continue;
				}
			}
		}

		// Process all triangles
		UINT iCurTri = 0;
		for (iCurTri=0; iCurTri<cCurMat.nNumIndices / 3; iCurTri++)
		{
			UINT iBaseIdx = cCurMat.nFirstIndexId + iCurTri * 3;

			CRadPoly *pPoly = new CRadPoly;	

			// Recalculate plane equation
			pPoly->m_Plane.CalcPlane(
				matEtyMtx.TransformPointOLD
					(* reinterpret_cast<Vec3d *> (&(reinterpret_cast<byte *>(pVertices)[pIndices[iBaseIdx + 2] * iVtxStride]))),
				matEtyMtx.TransformPointOLD
					(* reinterpret_cast<Vec3d *> (&(reinterpret_cast<byte *>(pVertices)[pIndices[iBaseIdx + 1] * iVtxStride]))),
				matEtyMtx.TransformPointOLD
					(* reinterpret_cast<Vec3d *> (&(reinterpret_cast<byte *>(pVertices)[pIndices[iBaseIdx + 0] * iVtxStride]))));	
			pPoly->m_lstOriginals.reserve(3);
			//cache some information
			Vec3d vNormals[3];
			// Add all 3 vertices
			for (int v=2; v>=0; v--)
			{
				CRadVertex pVert;			

				if(!bIsDecalMat)//don't care about decals
				{
					pVert.m_vPos = matEtyMtx.TransformPointOLD
						((* reinterpret_cast<Vec3d *> (&(reinterpret_cast<byte *>(pVertices)[pIndices[iBaseIdx + v] * iVtxStride]))));
					vNormals[v] = pVert.m_vNormal   = GetTransposed44(matEtyMtx) * 
						((* reinterpret_cast<Vec3d *> (&(reinterpret_cast<byte *>(pNormals)[pIndices[iBaseIdx + v] * iNormalStride]))));
					pVert.m_vBinormal = GetTransposed44(matEtyMtx) * 
						((* reinterpret_cast<Vec3d *> (&(reinterpret_cast<byte *>(pBinormals)[pIndices[iBaseIdx + v] * iBinormalStride]))));
					pVert.m_vTangent  = GetTransposed44(matEtyMtx) * 
						((* reinterpret_cast<Vec3d *> (&(reinterpret_cast<byte *>(pTangents)[pIndices[iBaseIdx + v] * iTangentStride]))));
					// Modified for smooth shading. We take the vertex normal as it is the
					// most important component, this guarantees smoothness across texture coordinate
					// discontinuities when the normals are smooth. Note that we don't orthonormalize the basis 
					// (looks good enough without)
					pVert.m_vTNormal  = GetTransposed44(matEtyMtx) * 
						((* reinterpret_cast<Vec3d *> (&(reinterpret_cast<byte *>(pTNormals)[pIndices[iBaseIdx + v] * iTNormalStride]))));
					//now check all 4 normals
					bool bNotNormalized = false;
					float cfSquareLen = pVert.m_vNormal.GetLengthSquared();
					if(!IsNormalized(cfSquareLen))
					{
						const float fVar = fabs(cfSquareLen - 1.f);
						m_fMaxVariance = (fVar > m_fMaxVariance)?fVar:m_fMaxVariance;
						bNotNormalized = true;
						pVert.m_vNormal.Normalize();
					}
					cfSquareLen = pVert.m_vBinormal.GetLengthSquared();
					if(!IsNormalized(cfSquareLen))
					{
						const float fVar = fabs(cfSquareLen - 1.f);
						m_fMaxVariance = (fVar > m_fMaxVariance)?fVar:m_fMaxVariance;
						bNotNormalized = true;
						pVert.m_vBinormal.Normalize();
					}
					cfSquareLen = pVert.m_vTangent.GetLengthSquared();
					if(!IsNormalized(cfSquareLen))
					{
						const float fVar = fabs(cfSquareLen - 1.f);
						m_fMaxVariance = (fVar > m_fMaxVariance)?fVar:m_fMaxVariance;
						bNotNormalized = true;
						pVert.m_vTangent.Normalize();
					}
					cfSquareLen = pVert.m_vTNormal.GetLengthSquared();
					if(!IsNormalized(cfSquareLen))
					{
						const float fVar = fabs(cfSquareLen - 1.f);
						m_fMaxVariance = (fVar > m_fMaxVariance)?fVar:m_fMaxVariance;
						bNotNormalized = true;
						pVert.m_vTNormal.Normalize();
					}
					if(bNotNormalized)
						m_uiFlags |= NOT_NORMALIZED_NORMALS_FLAG;//mark for later information
				}
				// Add to the initial vertices of the polygon
				pPoly->m_lstOriginals.push_back(pVert);
			} // v
			if(bIsDecalMat)
			{
				pPoly->m_dwFlags |= (DECAL_FLAG | NOLIGHTMAP_FLAG);
			}
			else
			{
				//now check for wrong normals
				if(fabs(vNormals[0] * vNormals[1] + 1.0f) < 0.01f ||
					fabs(vNormals[2] * vNormals[0] + 1.0f) < 0.01f ||
					fabs(vNormals[1] * vNormals[2] + 1.0f) < 0.01f)
				{
					SUV uv[3];
					//fetch uv's needed by tangent space calc
					uv[0] = *(reinterpret_cast<SUV *> (&(reinterpret_cast<byte *>(pUV)[pIndices[iBaseIdx + 0] * iUVStride])));
					uv[1] = *(reinterpret_cast<SUV *> (&(reinterpret_cast<byte *>(pUV)[pIndices[iBaseIdx + 1] * iUVStride])));
					uv[2] = *(reinterpret_cast<SUV *> (&(reinterpret_cast<byte *>(pUV)[pIndices[iBaseIdx + 2] * iUVStride])));
					m_uiFlags |= WRONG_NORMALS_FLAG;//mark for later information
					pPoly->CalculateTangentSpace(uv);
				}
				if(pPoly->m_Plane.n.GetLengthSquared() < 0.5f)//plane could not been computed due to identity of points
				{
					pPoly->m_Plane.n = (pPoly->m_lstOriginals[0]).m_vNormal;//use one vertex normal instead
				}
			}
			// Add to the original polygons of the mesh
			m_lstOldPolys.push_back(pPoly);
		}
	}
	return true;
}

inline DWORD CRadMesh::CalcLocalLightHashValue( void ) const
{
	DWORD dwRet=0;
	for (std::vector<LMCompLight *>::const_iterator ligIt=m_LocalLights.begin(); ligIt!=m_LocalLights.end(); ligIt++)
	{
		LMCompLight *pLight = (*ligIt);
		CalcNCombineHash(pLight->GetLightHashValue(),dwRet);
	}
	return(dwRet);
}

//
//////////////////////////////////////////////////////////////////////
const float DistToLine(const Vec3d& rX0, const Vec3d& rX1, const Vec3d& rPoint)
{
	const float cfDot = (rX0 - rPoint) | (rX1 - rX0);
	return fabs((rX0 - rPoint).GetLengthSquared()*(rX1 - rX0).GetLengthSquared() - cfDot * cfDot) / 
		(rX1 - rX0).GetLengthSquared();
}

//
//////////////////////////////////////////////////////////////////////
const Vec3d ProjectOntoEdge(const Vec3d& rV0, const Vec3d& rV1, const Vec3d& rPoint, const float cfDistLV)
{
	Vec3d vPointOnEdge;
	Vec3d vLine = (rV0 - rV1);
	const float cfLineLength = vLine.GetLengthSquared();	//we need this later on
	if(cfLineLength > cfNormalizeThreshold)
		vLine *= (1.0f/sqrtf(cfLineLength));
	//vectors and square length to points on line
	Vec3d vToV0 = rPoint - rV0;
	Vec3d vToV1 = rPoint - rV1;
	float fDistToV0 = vToV0.GetLengthSquared();
	float fDistToV1 = vToV1.GetLengthSquared();
	//normalize direction
	if(fDistToV0 >= cfNormalizeThreshold)
	{
		vToV0.Normalize();
	}
	//if close enough to one line endpoint, just return this vertex
	if(fDistToV0 < cfNormalizeThreshold || fDistToV1 < cfNormalizeThreshold)
	{
		// the two vectors are on the same line, snap to nearest vertex
		vPointOnEdge = (fDistToV0 > fDistToV1)?rV1:rV0;
		return vPointOnEdge;
	}
	else
	{
		Vec3d vUp = vLine % vToV0;
		vUp.Normalize();
		vUp = vUp % vLine;	//now we got the up vector of the plane
		vPointOnEdge = rPoint + (vUp * -(sqrtf(cfDistLV)));
	}
	//now check whether we are outside the line or not
	fDistToV0 = (vPointOnEdge - rV0).GetLengthSquared();
	fDistToV1 = (vPointOnEdge - rV1).GetLengthSquared();
	if((fDistToV0 + fDistToV1) > (cfLineLength - 0.01f))
	{
		//we are still outside the edge, snap to the nearest vertex
		vPointOnEdge = (fDistToV0 > fDistToV1)?rV1:rV0;
	}
	return vPointOnEdge; 
}

//
//////////////////////////////////////////////////////////////////////
const bool CRadPoly::SnapVertex(Vec3d &inPosition, float &outfAlpha, float &outfBeta, const float cfTriangleArea)
{
	assert(m_lstOriginals.size() == 3);
	//retrieve references to all triangle vertices
	const Vec3d& rV0 = m_lstOriginals[0].m_vPos;
	const Vec3d& rV1 = m_lstOriginals[1].m_vPos;
	const Vec3d& rV2 = m_lstOriginals[2].m_vPos;
	//retrieve square distance to all edges
	const float cfDistLV01	= max(DistToLine(rV0, rV1, inPosition), cfNormalizeThreshold);
	const float cfDistLV02	= max(DistToLine(rV0, rV2, inPosition), cfNormalizeThreshold); 
	const float cfDistLV12	= max(DistToLine(rV1, rV2, inPosition), cfNormalizeThreshold);
	//now retrieve index of nearest edge	
	const int ciNearestEdgeIndex = 
		(cfDistLV01 < cfDistLV02)?(cfDistLV01 < cfDistLV12)?0:2:(cfDistLV12 < cfDistLV02)?2:1;
	//now switch these edges and project vertex accordingly
	//determine whether it has to be projected onto the line between the two vertices on the edge, or to be snapped to the nearest vertex
	Vec3d vPointOnEdge;
	switch(ciNearestEdgeIndex)
	{
		//project first onto line and then determine where we are to be inside the edge	
	case 0:
		{
			vPointOnEdge = ProjectOntoEdge(rV0, rV1, inPosition, cfDistLV01);
		}
		break;
	case 1:
		{
			vPointOnEdge = ProjectOntoEdge(rV0, rV2, inPosition, cfDistLV02);
		}
		break;
	case 2:
		{
			vPointOnEdge = ProjectOntoEdge(rV1, rV2, inPosition, cfDistLV12);
		}
		break;
	}

	//first sub tri
	const float a1=CalcTriangleArea(vPointOnEdge,rV1,rV2);
	//second sub tri  
	const float a2=CalcTriangleArea(rV0,vPointOnEdge,rV2);
	//third sub tri  
	const float a3=CalcTriangleArea(rV0,rV1,vPointOnEdge);
	//check sum, must be close to real area
	const float asum = a1+a2+a3;
	bool cbBaryFailCond = false;
	if(fabs(asum - cfTriangleArea) > cfTriangleArea*0.01f)
		cbBaryFailCond = true;	//area difference to large
	if(cbBaryFailCond == false)
	{
		inPosition = vPointOnEdge;
		//calc alpha beta and gamma components as area ratio
		outfAlpha			= a1 / cfTriangleArea;
		outfBeta			= a2 / cfTriangleArea;
		cbBaryFailCond = ((outfAlpha + outfBeta) > scfBaryThreshold)?true:false;	//set to fail if barycentric coords are invalid
	}
	return (!cbBaryFailCond);
}

//
//////////////////////////////////////////////////////////////////////
bool CRadPoly::ApplyBarycentricCoordinates( Vec3d &inPosition, CRadVertex &outVertex, SComplexOSDot3Texel& rDot3Texel, const bool cbImmedReturn, const bool cbSnap)
{
	assert(m_lstOriginals.size() == 3);
	float fArea3d, outfAlpha, outfBeta;	//variables filled by the function below
	const bool cbBaryFailCond = CheckPointInTriangle(inPosition, m_lstOriginals[0].m_vPos, m_lstOriginals[1].m_vPos, m_lstOriginals[2].m_vPos, outfAlpha, outfBeta, fArea3d);

	rDot3Texel.pSourcePatch = this;	//tangent space will come from here
	//if barycentric coordinates are invalid and the fix is requested, snap vertex onto one triangle edge and retrieve new barycentric coordinates
	if(cbBaryFailCond)
	{
		rDot3Texel.bNotHit = true;	//indicate that this texel was not hit properly (to signal subsampling requirement)
		if(cbImmedReturn)
			return false;	//was used for quick check to cache polygon
		//snap vertex if no sharing polygons are there
		if(m_SharingPolygons.size() == 0)
		{
			if(cbSnap)
			{
				if(!SnapVertex(inPosition,outfAlpha,outfBeta,fArea3d))
					return false;
			}
			else
				return false;
		}
		else
		{
			//try to map into the sharing polygons
			//compute nearest vertex, rotate into other plane by rotating around cross product between both plane up vectors with angle (sin of cross product)
			if(!SmoothVertex(outVertex,inPosition, fArea3d, rDot3Texel))
			{
				if(cbSnap)
				{
					if(!SnapVertex(inPosition,outfAlpha,outfBeta,fArea3d))
						return false;
				}
				else
					return false;
			}
			else
				return true;				
		}
	}
	//now apply the barycentric coordinates
	ApplyBaryCoordsToVertex(outVertex, m_lstOriginals[0], m_lstOriginals[1], m_lstOriginals[2], outfAlpha, outfBeta);
	rDot3Texel.fAlpha = outfAlpha;	
	rDot3Texel.fBeta  = outfBeta;
	return(true);
}

const bool CRadPoly::SmoothVertex(CRadVertex &outVertex, const Vec3d &inPosition, const float cfArea3d, SComplexOSDot3Texel& rDot3Texel)
{
	assert(m_lstOriginals.size() == 3);
	//retrieve references to all triangle vertices
	const Vec3d& rV0 = m_lstOriginals[0].m_vPos;
	const Vec3d& rV1 = m_lstOriginals[1].m_vPos;
	const Vec3d& rV2 = m_lstOriginals[2].m_vPos;
	//fetch square distances to 
	const float cfDistV01	= fabs((rV0 - inPosition).GetLengthSquared());
	const float cfDistV02	= fabs((rV1 - inPosition).GetLengthSquared());
	const float cfDistV03	= fabs((rV2 - inPosition).GetLengthSquared());
	//now retrieve index of nearest edge	
	const int ciNearestVertexIndex = 
		(cfDistV01 < cfDistV02)?(cfDistV01 < cfDistV03)?0:2:(cfDistV03 < cfDistV02)?2:1;
	const int ciFarestVertexIndex = 
		(cfDistV01 > cfDistV02)?(cfDistV01 > cfDistV03)?0:2:(cfDistV03 > cfDistV02)?2:1;
	//get face normal == triangle plane up vector
	Vec3d oneEdge = rV0 - rV1; 
	Vec3d secondEdge = rV0 - rV2; 
	Vec3d vPlaneNormal =  oneEdge % secondEdge; 
	vPlaneNormal.Normalize();
	//retrieve direction vectors to all vertices to use for shared vertex, array is used since we have stored a index
	Vec3d vDir[3]; vDir[0] = inPosition - rV0; vDir[1] = inPosition - rV1; vDir[2] = inPosition - rV2;
	//now iterate all triangles and rotate direction vector into plane of this triangle
	//add this to closest vertex and check barycentric coordinates
	for(SharedIter sharedIter = m_SharingPolygons.begin(); sharedIter != m_SharingPolygons.end(); ++sharedIter)
	{ 
		CRadPoly* pSharedPoly = (*sharedIter).first;
		//fetch vertex index which are shared and make sure that the nearest vertex index(ciNearestVertexIndex) is contained in that shared edge
		const unsigned int cuiIndex0 = ((*sharedIter).second) & 0x000000FF;
		const unsigned int cuiIndex1 = (((*sharedIter).second) & 0x0000FF00)>>8;
		const bool cbOnlyOneVertexShared = (cuiIndex1 == scuiOneVertexShareFlag);
		if(!cbOnlyOneVertexShared && (ciNearestVertexIndex != cuiIndex0 && ciNearestVertexIndex != cuiIndex1))
			continue;//wrong shared edge
		if(cbOnlyOneVertexShared && (ciFarestVertexIndex == cuiIndex0))
			continue;//wrong shared vertex
		//first retrieve triangle plane normal
		const Vec3d& rV02 = pSharedPoly->m_lstOriginals[0].m_vPos;
		const Vec3d& rV12 = pSharedPoly->m_lstOriginals[1].m_vPos;
		const Vec3d& rV22 = pSharedPoly->m_lstOriginals[2].m_vPos;
		//get face normal == triangle plane normal
		Vec3d oneEdge = rV02 - rV12; 
		Vec3d secondEdge = rV02 - rV22; 
		Vec3d vSharedPlaneNormal =  oneEdge % secondEdge; 
		vSharedPlaneNormal.Normalize(); 
		//try to rotate vertex into plane of this shared triangle, if rotations fails, it will just get projected
		const Vec3d& rSource0 = m_lstOriginals[cuiIndex0].m_vPos; 
		const Vec3d& rSource1 = (cbOnlyOneVertexShared)?m_lstOriginals[0].m_vPos:m_lstOriginals[cuiIndex1].m_vPos; 
		Vec3d vNewDir;
		float cfDIst0 = 0.f,  cfDIst1 = 0.f;
		if(!cbOnlyOneVertexShared)
		{
			cfDIst0 = (rSource0 - inPosition).GetLengthSquared();
			cfDIst1 = (rSource1 - inPosition).GetLengthSquared();
			vNewDir = (cfDIst0 < cfDIst1)?
				RotateIntoPlane(vPlaneNormal, vSharedPlaneNormal, inPosition, rSource0) :
				RotateIntoPlane(vPlaneNormal, vSharedPlaneNormal, inPosition, rSource1);
		}
		else
			vNewDir = RotateIntoPlane(vPlaneNormal, vSharedPlaneNormal, inPosition, rSource0);
		float bary0, bary1, fArea3d;	//variables set up by the function below
		bool cbBaryFailCond = CheckPointInTriangle(vNewDir, rV02, rV12, rV22, bary0, bary1, fArea3d);
		if(cbBaryFailCond == true)
		{
			//iterate all triangles of this patch 
			CRadPoly *pMergeSource = ((pSharedPoly->m_dwFlags & MERGE_SOURCE_FLAG) != 0)?pSharedPoly : pSharedPoly->m_pMergeSource;
			//try to rotate into shared triangles of this triangle
			for(std::vector<CRadPoly *>::iterator sharedSharedIter = pMergeSource->m_lstMerged.begin(); sharedSharedIter != pMergeSource->m_lstMerged.end(); ++sharedSharedIter)
			{ 
				CRadPoly* pSharedSharedPoly = *sharedSharedIter;
				if(pSharedSharedPoly == pSharedPoly)
					continue;//don't try the same poly 2 times
				const Vec3d& rV02 = pSharedSharedPoly->m_lstOriginals[0].m_vPos;
				const Vec3d& rV12 = pSharedSharedPoly->m_lstOriginals[1].m_vPos;
				const Vec3d& rV22 = pSharedSharedPoly->m_lstOriginals[2].m_vPos;
				const bool cbSharedBaryFailCond = CheckPointInTriangle(vNewDir, rV02, rV12, rV22, bary0, bary1, fArea3d);//use the position already retrieved
				if(cbSharedBaryFailCond == false)//found the triangle which this texel lies in
				{
					pSharedPoly = pSharedSharedPoly;//assign to use this one
					cbBaryFailCond = false;//change bary condition to enter next code block
					break;
				}
			}
		}
		if(cbBaryFailCond == true)
		{
			//if still not found, try all other shared triangles of this one
			//try to rotate into shared triangles of this triangle
			for(SharedIter sharedSharedIter = pSharedPoly->m_SharingPolygons.begin(); sharedSharedIter != pSharedPoly->m_SharingPolygons.end(); ++sharedSharedIter)
			{ 
				CRadPoly* pSharedSharedPoly = (*sharedSharedIter).first;
				if(pSharedSharedPoly == this)
					continue;//same poly as above
				//first retrieve triangle plane normal
				const Vec3d& rV02 = pSharedSharedPoly->m_lstOriginals[0].m_vPos;
				const Vec3d& rV12 = pSharedSharedPoly->m_lstOriginals[1].m_vPos;
				const Vec3d& rV22 = pSharedSharedPoly->m_lstOriginals[2].m_vPos;
				//get face normal == triangle plane normal
				Vec3d oneEdge = rV02 - rV12; 
				Vec3d secondEdge = rV02 - rV22; 
				Vec3d vSharedSharedPlaneNormal =  oneEdge % secondEdge; 
				vSharedSharedPlaneNormal.Normalize();
				//try to rotate vertex into plane of this shared triangle, if rotations fails, it will just get projected
				if(!cbOnlyOneVertexShared)
				{
					vNewDir = (cfDIst0 < cfDIst1)?
						RotateIntoPlane(vPlaneNormal, vSharedSharedPlaneNormal, inPosition, rSource0) :
						RotateIntoPlane(vPlaneNormal, vSharedSharedPlaneNormal, inPosition, rSource1);
				}
				else
					vNewDir = RotateIntoPlane(vPlaneNormal, vSharedSharedPlaneNormal, inPosition, rSource0);
				const bool cbSharedBaryFailCond = CheckPointInTriangle(vNewDir, rV02, rV12, rV22, bary0, bary1, fArea3d);
				if(cbSharedBaryFailCond == false)//found the triangle which this texel lies in
				{
					pSharedPoly = pSharedSharedPoly;//assign to use this one
					cbBaryFailCond = false;//change bary condition to enter next code block
					break;
				}
			}
		}
		if(cbBaryFailCond == false)
		{ 
			//we have found a triangle which the vertex lies really in
			//keep tangent space from the nearest triangle of this patch
			//snap vertex to get proper tangent space
			float fAlpha = 0.f, fBeta = 1.f;
			Vec3d vCopiedInPos = inPosition;//because it will get altered
			const bool bSnapped = SnapVertex(vCopiedInPos,fAlpha,fBeta,cfArea3d);
			if(bSnapped)
			{
				//apply different tangent space from this triangle to this vertex
				ApplyTangentSpaceToVertex(outVertex, fAlpha, fBeta);
				rDot3Texel.fAlpha = fAlpha;
				rDot3Texel.fBeta = fBeta;
			}
			ApplyBaryCoordsToVertex(outVertex, pSharedPoly->m_lstOriginals[0], pSharedPoly->m_lstOriginals[1], pSharedPoly->m_lstOriginals[2], bary0, bary1, !bSnapped);
			//apply new position
			outVertex.m_vPos = vNewDir;
			if(!bSnapped)
			{
				//shouldnt come here at all
				rDot3Texel.pSourcePatch = pSharedPoly;
				rDot3Texel.fAlpha = bary0;
				rDot3Texel.fBeta  = bary1;
			}
			rDot3Texel.bNotHit	= true;	//indicate this is coming from another patch
			return true;
		}
	}
	return false;
}
