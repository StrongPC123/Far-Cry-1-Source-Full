#ifndef __LM_LIGHT_COLLECTION_H__
#define __LM_LIGHT_COLLECTION_H__

#include "LMCompStructures.h"

#include "RasterCube.h"


//! \sa LMCompLight
enum eLightType
{
	eSpotlight,
	ePoint,
	eDirectional, //!< Sun
};

//class CRadPoly;

struct RasterCubeUserT
{
	float fVertices[3][3];
	Vec3d vNormal;	//plane normal of triangle
//	CRadPoly *pTriangle;
	bool operator == (const RasterCubeUserT& A) const
	{ return memcmp(this, &A, sizeof(RasterCubeUserT)) == 0; };
	bool operator > (const RasterCubeUserT& A) const
	{ return memcmp(this, &A, sizeof(RasterCubeUserT)) == 1; };
}; 

typedef CRasterCube<RasterCubeUserT, true, false> CRasterCubeImpl;//fast version using only two raster and break after first valid hit encounter
//typedef CRasterCube<RasterCubeUserT> CRasterCubeImpl;//old version with 3 rastertables and no early out

inline void CalcNCombineHash( const DWORD indwValue, DWORD &inoutHash )
{
//		inoutHash^=inoutHash*0x1c1a8926 + indwValue + inoutHash;
//		inoutHash^=inoutHash%600011 + indwValue;
		inoutHash^=(inoutHash%600011) + (inoutHash/600011) + indwValue;
}

inline void CalcNCombineHash( const float infValue, DWORD &inoutHash )
{
	CalcNCombineHash(*((DWORD *)(&infValue)),inoutHash);
}

inline void CalcNCombineHash( const Vec3d &invValue, DWORD &inoutHash )
{
	CalcNCombineHash(invValue.x,inoutHash);
	CalcNCombineHash(invValue.y,inoutHash);
	CalcNCombineHash(invValue.z,inoutHash);

}

//! \sa eLightType
struct LMCompLight
{
	LMCompLight():m_CompLightID(std::pair<EntityId, EntityId>(0,0))
	{
		vWorldSpaceLightPos = Vec3d(0.0f, 0.0f, 0.0f);
		vDirection = Vec3d(1.0f, 0.0f, 0.0f);
		fRadius = 1.0f;
		fLightFrustumAngleDegree = 45.0f;
		eType = ePoint;
		fColor[0]=0;fColor[1]=0;fColor[2]=0;
		m_pLastIntersection=0;

		// m_pPointLtRC = NULL;

		m_pLastIntersectionRasterCube = NULL;
    
		uiFlags = 0;
		pVisArea = 0;

		m_bFakeRadiosity = false;
		m_bDot3			 = true;
		m_bOcclType		 = false;
		m_bCastShadow	 = true;
	};

	~LMCompLight(){}

	Vec3d							vWorldSpaceLightPos;				//!<
	Vec3d							vDirection;							//!< For directional / spotlights (normalized)
	float							fRadius;							//!< >=0
	float							fLightFrustumAngleDegree;			//!< For spotlights
	float							fColor[3];							//!< [0..,0..,0..]
	eLightType						eType;								//!<
	 
	bool							m_bDot3;							//!< marks the lightsource to be dot3 vector contributing
	bool							m_bFakeRadiosity;					//!< applies a different lighting model
	bool							m_bOcclType;						//!< indicates a occlusion map light source type	
	bool							m_bCastShadow;						//!< indicates whether this lights casts shadow into lightmap or not	

	CString							m_Name;								//!< name of lightsource
	std::pair<EntityId, EntityId>	m_CompLightID;						//!< id which the light is referred to from GLMIOcclInfo (the serialization index) 

	// CRasterCubeImpl		*m_pPointLtRC;								//!< Raster cube convering all geometry in the point light's radius
	RasterCubeUserT	  *m_pLastIntersection;					//!< Filled when ReturnElement() has a new intersection - to optimize the shadow rays
	CRasterCubeImpl   *m_pLastIntersectionRasterCube;		//!< stores where cached result belongs to

  uint uiFlags;
  void * pVisArea;

	//! /return hashing value to detect changes in the data
	DWORD GetLightHashValue( void ) const
	{
		DWORD dwRet=0;

		CalcNCombineHash(vWorldSpaceLightPos,dwRet);
		CalcNCombineHash(vDirection,dwRet);
		CalcNCombineHash(fRadius,dwRet);
		CalcNCombineHash(fLightFrustumAngleDegree,dwRet);
		CalcNCombineHash(fColor[0],dwRet);
		CalcNCombineHash(fColor[1],dwRet);
		CalcNCombineHash(fColor[2],dwRet);
		CalcNCombineHash((DWORD)eType,dwRet);
		DWORD ciFlagSum = (m_bFakeRadiosity? 0x11111111 : 0) + (m_bDot3? 0x22222222 : 0) + (m_bCastShadow? 0x44444444 : 0) + (m_bOcclType? 0x88888888 : 0);
		CalcNCombineHash(ciFlagSum, dwRet);
		return(dwRet);
	}
};

// \brief Collection class for passing lights to the compiler, converts 3D engine lights
//        to LM compiler lights
// \sa LMCompLight
class CLMLightCollection
{
public:
	CLMLightCollection():m_uiOcclLightSize(0){};
	std::vector<LMCompLight>& GetLights() { return m_vLights; }
	std::vector<CDLight *>& GetSrcLights() { return m_vSrcLights; };
	const unsigned int OcclLightSize() {return m_uiOcclLightSize;}
	//!
	void Release() { delete this; };

	//! /param pLight must not be 0
	void AddLight(CDLight *pLight, const CString& rName, const EntityId& rID, const bool cbCastShadow = true) 
	{
		assert(pLight);

		LMCompLight cNewLight;

		cNewLight.fLightFrustumAngleDegree = pLight->m_fLightFrustumAngle;
		cNewLight.fColor[0] = pLight->m_Color.r;
		cNewLight.fColor[1] = pLight->m_Color.g;
		cNewLight.fColor[2] = pLight->m_Color.b;
		cNewLight.fRadius = pLight->m_fRadius;
		cNewLight.vDirection = pLight->m_Orientation.m_vForward;
		cNewLight.vDirection.Normalize();
		cNewLight.vWorldSpaceLightPos = pLight->m_Origin;
		// for Visible Area
		if(pLight->m_pOwner) 
		{
			cNewLight.uiFlags = pLight->m_Flags;
			cNewLight.pVisArea = (void *) pLight->m_pOwner->GetEntityVisArea();
		}
		if (pLight->m_Flags & DLF_PROJECT)
			cNewLight.eType = eSpotlight;
		else
			cNewLight.eType = ePoint;

		cNewLight.m_bFakeRadiosity   = (pLight->m_Flags & DLF_FAKE_RADIOSITY);
		cNewLight.m_bDot3			 = (pLight->m_Flags & DLF_LMDOT3);
		cNewLight.m_bOcclType		 = (pLight->m_Flags & DLF_LMOCCL);
		cNewLight.m_bCastShadow		 = cbCastShadow;
		if(cNewLight.m_bOcclType) m_uiOcclLightSize++;

		cNewLight.m_Name = rName;

		cNewLight.m_CompLightID.first		= rID;
		cNewLight.m_CompLightID.second		= m_vLights.size();

		m_vLights.push_back(cNewLight);
		m_vSrcLights.push_back(pLight);
	};

protected:
	std::vector<LMCompLight> m_vLights;				//!< lightmap lights
	std::vector<CDLight *> m_vSrcLights;			//!< source entity
	unsigned int	m_uiOcclLightSize;				//!< number of occlusion map light source types
};

#endif