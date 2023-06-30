// ---------------------------------------------------------------------------------------------
//	Crytek CryENGINE source code
//	History:
//	- Created by Marco Corbetta
//  - Changed by Tim Schroeder
//	- Partial rewrite for editor integration
// ---------------------------------------------------------------------------------------------
#pragma once

#ifndef __INDOOR_LIGHT_PATCHES_H__
#define __INDOOR_LIGHT_PATCHES_H__

//#define REDUCE_DOT3LIGHTMAPS_TO_CLASSIC	// for debugging
//#define DISPLAY_MORE_INFO

//typedef float real;
typedef double real;//typedef controlling the accuracy

#define HDR_EXP_BASE    (1.04)
#define HDR_EXP_OFFSET  (64.0)
#define HDR_LOG(a, b)   ( log(b) / log(a) )

#include <float.h>
#include "LMCompCommon.h"
#include "LMCompStructures.h"
#include "IEntityRenderstate.h"
#include "LMLightCollection.h"
#include "..\Objects\BrushObject.h"
#include "I3dEngine.h"
#include <direct.h>

static const float scfMaxGridSize = 2.f;

#define	MIN_LIGHTMAP_SIZE	4

//flags for CRadPoly
#define	NOLIGHTMAP_FLAG		1
#define	MERGE_FLAG			2	
#define	SHARED_FLAG			4		
#define	MERGE_SOURCE_FLAG	8	
#define	DECAL_FLAG	64	
#define	DO_NOT_COMPRESS_FLAG 128	//flag for patches


#define	WRONG_NORMALS_FLAG	16	
#define	NOT_NORMALIZED_NORMALS_FLAG	32	
#define	ONLY_SUNLIGHT 256			//flag for mesh
#define	DOUBLE_SIDED_MAT 512		//flag for mesh
#define	REBUILD_USED 1024			//flag for mesh
#define	CAST_LIGHTMAP 2048			//flag for mesh
#define	RECEIVES_SUNLIGHT 4096		//flag for mesh
#define	HASH_CHANGED 8192			//flag for mesh
#define	RECEIVES_LIGHTMAP (8192<<1)	//flag for mesh

//forces the patches to align on 4 pixel boundaries
#define MAKE_BLOCK_ALIGN	

struct SUV
{
	float u,v;				//texture coordinates
};

inline const AABB MakeSafeAABB(const Vec3& rMin, const Vec3& rMax)
{
	static const float scfMargin = 0.1f;//margin to add 
	Vec3 vMin(rMin);			Vec3 vMax(rMax); 
	if(vMin.x == vMax.x)		vMax.x += scfMargin;
	if(vMin.y == vMax.y)		vMax.y += scfMargin;
	if(vMin.z == vMax.z)		vMax.z += scfMargin;
	return AABB(vMin, vMax);
}

//used for warning gathering
typedef enum
{
	EWARNING_EXPORT_FAILED = 0,		//!< export of lightmaps has failed
	EWARNING_LIGHT_EXPORT_FAILED,	//!< export of lightsources has failed
	EWARNING_DOUBLE_SIDED,			//!< double sided material
	EWARNING_TOO_MANY_OCCL_LIGHTS,	//!< more than 4 active occlusion map light sources at the same time on glm
	EWARNING_NO_FIT,				//!< glm does not fit into a single lightmap
	EWARNING_HUGE_PATCH,			//!< has patch(es) which are larger than halve a lightmap wide or high
	EWARNING_WRONG_NORMALS,			//!< glm has wrong normals
	EWARNING_DENORM_NORMALS,		//!< glm has denormalized normals
	EWARNING_LIGHT_RADIUS,			//!< light has a too little radius
	EWARNING_LIGHT_INTENSITY,		//!< light has a too little intensity
	EWARNING_LIGHT_FRUSTUM			//!< spotlight has invalid frustum
}EWARNING_TYPE;

static const unsigned int scuiWarningTextAllocation = 300;//number of chars allocated on stack for warning string

static inline const bool IsNormalized(const float cfSqLen)
{
	static const float scfThreshold = 0.1f;
	if(fabs(cfSqLen - 1.f) < scfThreshold)
		return true;
	return false; 
}

//supports flexible subsampling patterns, but for simplicity just use 9x or nothing for now
class CAASettings
{
protected:
	unsigned int	m_uiScale;
	float			m_fInvScale;
public:
	bool m_bEnabled;
	CAASettings():m_bEnabled(false),m_uiScale(1),m_fInvScale(1.0f){}
	void SetScale(const unsigned int cuiScale)
	{
		assert(cuiScale != 0);
		m_uiScale = cuiScale;
		m_fInvScale = 1.0f / (float)cuiScale;
	}
	const float GetInvScale()const{return m_fInvScale;}
	const unsigned int GetScale()const{return m_uiScale;}
	const float RetrieveRealSamplePos(const real cfOrig)
	{
		switch(m_uiScale)
		{
		case 1:
			return cfOrig;
			break;
		case 2:
			{
				const real cfNumber = (real)((int)(cfOrig * 0.5f));
				return (cfNumber - (real)1.0f/(real)3.0 + (real)2.0/(real)3.0 * (cfOrig - cfNumber * (real)2.0));//map onto -1/3, 1/3, 2/3, 4/3,...
			}
			break;
		case 3:
			return ((real)1.0f/(real)3.0 * cfOrig - (real)1.0f/(real)3.0);//map onto -1/3, 0/3, 1/3, 2/3, 3/3, 4/3,...
			break;
		default:
			return m_fInvScale * cfOrig;
		}
	}
	//returns the middle index, or tells whether this is the one or not
	const bool IsMiddle(const unsigned int cuiX, const unsigned int cuiY)
	{
		switch(m_uiScale)
		{
		case 1:
			return true;
		case 2:
			return (cuiX == 1 && cuiY == 1);
		case 3:
			return (cuiX == 1 && cuiY == 1);
		}
		return true;
	}
};

class		CLightPatch;
class		CRadPoly;  
class		CRadVertex;
class		CRadMesh;
struct		IStatObj;
class		CLightScene; 

typedef std::vector<CRadPoly *> radpolylist;
typedef std::vector<CRadPoly *>::iterator radpolyit;
typedef std::vector<CRadVertex> radvertexlist;
typedef std::vector<CRadVertex>::iterator radvertexit;
typedef std::list<CLightPatch *> lightpatchlist;
typedef std::list<CLightPatch *>::iterator lightpatchit;
typedef std::list<CRadMesh *> radmeshlist;
typedef std::list<CRadMesh *>::iterator radmeshit;

typedef std::vector<std::pair<CRadPoly*,unsigned int> >::iterator SharedIter;
typedef std::vector<CRadVertex *> radpvertexlist;
typedef std::vector<CRadVertex *>::iterator radpvertexit;

#define PLANE_MX		3
#define PLANE_MY		4
#define PLANE_MZ		5

const float cfNormalizeThreshold = 0.00001f;

//need this shitty class because it is a bit too late to use a smart pointer and let it reference
//use a map and the address as key
class CRasterCubeManager
{
public:
	CRasterCubeManager(){};
	~CRasterCubeManager();
	void AddReference(const CRadMesh* cpRadMesh);
	CRasterCubeImpl* CreateRasterCube();
	const bool RemoveReference(CRadMesh* cpRadMesh);
	void Clear();

protected:
	std::map<CRasterCubeImpl*, std::set<const CRadMesh*> > m_mRasterCubeMap;	//keeps track of each rastercube allocation
};

// contains all information which a certain dot3 dominant light dir texel was stored with
// also controls subsampling
typedef struct SComplexOSDot3Texel
{
	Vec3d vDot3Light;			//!< world space light vector
	CRadPoly *pSourcePatch;		//!< pointer to triangle where tangent space comes from, is always on this patch
	float fAlpha;				//!< barycentric alpha value
	float fBeta;				//!< barycentric beta value
	unsigned int uiLightMask;	//!< lightmask				
	unsigned int bNotHit;		//!< true if this was a snapped texel
	SComplexOSDot3Texel():vDot3Light(),pSourcePatch(NULL),fAlpha(0.f),fBeta(0.f),uiLightMask(0),bNotHit(false){}	
	const Vec3d TransformIntoTS(Vec3d& rSource)const;
}SComplexOSDot3Texel;

//calc the area of a triangle using texture coords
//////////////////////////////////////////////////////////////////////
inline float CalcTriangleArea( const Vec3d &A, const Vec3d &B, const Vec3d &C )
{
	return( GetLength( (C-A).Cross(B-A) )*0.5f );
}

__inline int CalcPlaneType2(const Vec3d& n)	
{
	if (((1.0f-n.x)<0.001f) && ((1.0f-n.x)>=0))
		return PLANE_X;
	else
		if (((1.0f-n.y)<0.001f) && ((1.0f-n.y)>=0)) 
			return PLANE_Y;
		else
			if (((1.0f-n.z)<0.001f) && ((1.0f-n.x)>=0)) 
				return PLANE_Z;            
			else
			{
				float ax = fabs(n[0]);
				float ay = fabs(n[1]);
				float az = fabs(n[2]);

				if (ax>=ay && ax>=az) 
					return PLANE_MX;
				else
					if (ay>=ax && ay>=az) 
						return PLANE_MY;
					else 
						return PLANE_MZ;	
			}
}

	const float DistToLine(const Vec3d& rX0, const Vec3d& rX1, const Vec3d& rPoint);
	const Vec3d ProjectOntoEdge(const Vec3d& rX0, const Vec3d& rX1, const Vec3d& rPoint, const float cfDistLV);

inline const unsigned int RoundFromFloat(const float cfTex)
{
	unsigned int uiNumber = (unsigned int)cfTex;	
	if(cfTex - (float)uiNumber >= 0.5f)
		uiNumber++;
	return uiNumber;
}

const Vec3d RotateIntoPlane(const Vec3d& vPlaneNormal0, const Vec3d& vPlaneNormal1, const Vec3d& rInPosition, const Vec3d& rSource0);

//! \brief used local during generation of light clustering arrangement
struct GLMCluster
{
	Vec3d vMin;
	Vec3d vMax;
	std::set<IEntityRender *> vGLMsAffectingCluster;
};

class CRadVertex
{
public:
	Vec3d			m_vPos;				//!< in world space
	Vec3d			m_vNormal;			//!< vertex normal for smooth lighting calulation
	float			m_fpX,m_fpY;		//!< used as projection coordinates and final texture coordinates as well

	Vec3d			m_vTNormal;			//!<
	Vec3d			m_vBinormal;		//!<
	Vec3d			m_vTangent;			//!<

	CRadVertex() : m_vPos(0.f,0.f,0.f), m_vNormal(0.f,0.f,0.f),	m_fpX(0.f), m_fpY(0.f), m_vTNormal(0.f,0.f,0.f), m_vBinormal(0.f,0.f,0.f), m_vTangent(0.f,0.f,0.f)
	{}
};

class CRadPoly
{
private:	
	void FreeLightmaps( void );

	//! used by GetNearestPolyAt()
	const float PointInTriangle(const Vec3d &point, const int ciIndex0, const int ciIndex1);

public:
	static const unsigned int scuiOneVertexShareFlag = 0x7F;
	//calculates the tangent space from the face information and applies it
	void CalculateTangentSpace(SUV uv[3]);

	//passes pointer to all contained polys to give them access for averaging
	void SynchronizeLightmaps();

	static void ApplyBaryCoordsToVertex(CRadVertex& rDest, const CRadVertex& rSource0, const CRadVertex& rSource1, const CRadVertex& rSource2, float cfAlpha, float cfBeta, const bool cbCOmputeTS = true);
	const bool CheckPointInTriangle(Vec3d &inPosition, const Vec3d &rVert0, const Vec3d &rVert1, const Vec3d &rVert2, float &rOutfAlpha, float &rOutfBeta, float &rfArea3d);

	//! construtor
	CRadPoly();

	//! construtor
	CRadPoly(CRadPoly *pSource);

	//! destructor
	~CRadPoly();

	//! /return true=the given polys have at least one vertex in common
	bool Connected(CRadPoly *pOther);

	//! copy the data to the give patch, the poly lightmap data is removed from memory, too
	//! /param inpDestLightPatch destination
	//! /param nMapS
	//! /param nMapT
	//! /param inpSrcPoly pointer to the poly, must not be 0
	//! /param sw source pitch = width in bytes
	//! /param sh number of lines
	//! /param dw destination pitch
	void CopyData( CLightPatch *inpDestLightPatch, int dw );

	bool IsTextureUniform(const int nTreshold);

	//! compress patch to 1x1 if constant value
	void Compress( const unsigned int cuiMinBlockSize );

	//! gather all subsamples for on etexel and sets the new value
	//! /param cuiX x-coord for texel to subsample
	//! /param cuiY y-coord for texel to subsample 
	//! /param cuiSubSamples subsample count excluding center texel
	//! /param cpaIndicator pointer to indicator array indicating which subsample has received values
	//! /param cpfColours pointer to colour subsample array
	//! /param pDot3 pointer to dot3 subsample values
	//! /param ruiMaxComponent current max of colour patch components
#ifdef APPLY_COLOUR_FIX
	void GatherSubSamples(
		const unsigned int cuiX, const unsigned int cuiY, const unsigned int cuiSubSamples,
		const unsigned int *cpaIndicator, const float *cpfColours, const SComplexOSDot3Texel *pDot3,
		unsigned int& ruiMaxComponent, const SOcclusionMapTexel *cpfOccl, const GLMOcclLightInfo& rOcclInfo);
#else
	void GatherSubSamples(
		const unsigned int cuiX, const unsigned int cuiY, const unsigned int cuiSubSamples,
		const unsigned int *cpaIndicator, const float *cpfColours, const SComplexOSDot3Texel *pDot3, const SOcclusionMapTexel *cpfOccl, const GLMOcclLightInfo& rOcclInfo);
#endif
	//!
	//! /return true=polygon has 3 vertices - calculation is ok, false=its not a triangle (degenerated - or wrong vertex count) calculation failed
	//! alters inPosition by projecting it into the triangls area
	bool ApplyBarycentricCoordinates( Vec3d &inPosition, CRadVertex &outVertex, SComplexOSDot3Texel& rDot3Texel, const bool cbImmedReturn = false, const bool cbSnap = true);

	//! 
	//! /param inPosition
	//! /return could be 0
	CRadPoly *GetNearestPolyAt( const Vec3d &inPosition );

	//lighpatch
	void AddWarningBorders( void );

#ifndef DISPLAY_MORE_INFO
	const unsigned int CalcExtent(CLightScene *pScene, const CString& rGLMName, const CString& rCGFName, bool bOriginal, const float fGridSize, const UINT iMinBlockSize, const UINT iMaxBlockSize, unsigned int& rHugePatchFoundNumber);
#else
	const unsigned int CalcExtent(CLightScene *pScene, const CString& rGLMName, const CString& rCGFName, bool bOriginal, const float fGridSize, const UINT iMinBlockSize, const UINT iMaxBlockSize);
#endif

	//!
	void GenerateImage( void );

	//!
	//! m_nW and m_nH is used
	void AllocateDot3Lightmap(const bool cbGenerateHDRMaps, const bool cbGenerateOcclMaps = false);

	bool InterpolateVertexAt( const float infX, const float infY, CRadVertex &outVertex, SComplexOSDot3Texel& rDot3Texel, const bool cbSubSample, const bool cbSnap=true) ;

	/**
	* snaps a vertex which lies outside a triangle onto the nearest edge to get some valid barycentric coordinates
	* it does not extrapolate
	* @param inPosition vertex outside the triangle
	* @param outfAlpha alpha value for barycentric coordinate
	* @param outfBeta beta value for barycentric coordinate
	* @param cfTriangleArea area of triangle for computation of the new barycentric cooridnates
	*/
	const bool SnapVertex(Vec3d &inPosition, float &outfAlpha, float &outfBeta, const float cfTriangleArea);

	/**
	* tries to map coordinates into triangles sharing at least one vertex of this triangle, performs the smoothing
	* @param outVertex output vertex interpolated from shared triangles
	* @param inPosition vertex outside the triangle
	* @param cfArea3d traingle area for snapping call
	* @param rDot3Texel information for dot3 texel to set up
	* @return true if one shared triangle has been found, false otherwise
	*/
	const bool SmoothVertex(CRadVertex &outVertex, const Vec3d &inPosition, const float cfArea3d, SComplexOSDot3Texel& rDot3Texel);

	/** 
	* applies tangent space vectors from this patch to a vertex having a pos outside this triangle
	* @param outVertex output vertex interpolated from shared triangles
	* @param outfAlpha alpha value for barycentric coordinate
	* @param outfBeta beta value for barycentric coordinate
	*/
	void ApplyTangentSpaceToVertex(CRadVertex &outVertex, float cfAlpha, float cfBeta);
 
	void SetHDRLightmapTexel( const float infX, const float infY, const float lr, const float lg, const float lb);
	void SetOcclLightmapTexel(const float infX, const float infY, const SOcclusionMapTexel& rTexel);
	//! 
	//! /param infX [0..m_nW[
	//! /param infY [0..m_nH[
	//! /param r 0.. , is automatic clamped to 0..255, usual lightmap color
	//! /param g 0.. , is automatic clamped to 0..255, usual lightmap color
	//! /param b 0.. , is automatic clamped to 0..255, usual lightmap color
	void SetSimpleLightmapTexel( const float infX, const float infY, const int lr, const int lg, const int lb, unsigned char iDP3Fac);
	//! stores the world space dot3 light vector and calls SetSimpleLightmapTexel
#ifdef APPLY_COLOUR_FIX
	const unsigned int SetDot3LightmapTexel(const CRadVertex& rVertex, 
		                      const float fColorRLamb, const float fColorGLamb, const float fColorBLamb,
					          Vec3d &inLightDir, const float cfLM_DP3LerpFactor,
							  SComplexOSDot3Texel& rDot3Texel, const SOcclusionMapTexel& rTexel, bool bHDR);	
#else
	void SetDot3LightmapTexel(const CRadVertex& rVertex, 
		                      const float fColorRLamb, const float fColorGLamb, const float fColorBLamb,
					          Vec3d &inLightDir, const float cfLM_DP3LerpFactor,
							  SComplexOSDot3Texel& rDot3Texel, const SOcclusionMapTexel& rTexel, bool bHDR);	
#endif
	//! stores the tangent space light vector as colour
#ifdef APPLY_COLOUR_FIX
	void SetDot3TSLightmapTexel(const unsigned int cuiX, const unsigned int cuiY, const unsigned int cuiColourFixAlpha, const float cfColourScale);	
#else
	void SetDot3TSLightmapTexel(const unsigned int cuiX, const unsigned int cuiY);	
#endif
	//unsigned int consists of 4 chars as follows:  [0]=1st shared vertex of this [1]=2nd shared vertex of this [2]=1st shared vertex of shared triangle  [3]=2nd shared vertex of shared triangle
	std::vector<std::pair<CRadPoly*,unsigned int> >  m_SharingPolygons;	//!< vertex sharing polygons

	radpolylist				m_lstMerged;								//!< only patches merge		

	radvertexlist			m_lstOriginals;								//!<

	Plane					m_Plane;									//!<

	CRadPoly *				m_pSource;									//!<
	
	CRadPoly *				m_pMergeSource;								//!< the source for this radpoly where it is contained in m_lstMerged

	float					m_fX1,m_fY1,m_fX2,m_fY2;					//!< 

	int16					m_nX1,m_nY1,m_nX2,m_nY2;					//!<

	uint16					m_nW,m_nH;									//!< really occupied width and height in the lightmap (always >=0)

	uint8					m_nAx1,m_nAx2;								//!<

	uint8					m_nAxis;									//!<
	
	uint8					m_dwFlags;									//!<

	Vec3        			*m_pHDRLightmapData;					//! RGBE8 Lightmap
	unsigned char			*m_pLightmapData;							//! Lightmap / or colormap when using dot3
	unsigned char			*m_pDominantDirData;						//!< used for Dot3Lightmaps (normalized tangent space direction to the dominant lightsource, dot3 factor in alpha channel)
	SComplexOSDot3Texel		*m_pWSDominantDirData;						//!< used for Dot3Lightmaps, stores the world space vector and where the tangent space comes from	
	SOcclusionMapTexel		*m_pOcclMapData;							//!< used for occlusionmap data

	uint16					m_nOffsetW, m_nOffsetH;						//!< offset into big lightmap

	friend class CLightScene;
};

//////////////////////////////////////////////////////////////////////
class CRadMesh
{
public:
	//! destructor
	~CRadMesh();

	//! call after putting in the light sources to get the right HashValue
	void	CreateEmpty( IEntityRender *pIEtyRend, const Matrix44& matEtyMtx );
	const bool FillInValues( IEntityRender *pIEtyRend, const CBrushObject *pBrushObject, const Matrix44& matEtyMtx );
	void	PrepareLightmaps(CLightScene *pScene, bool bMergePolys,const float fGridSize, const Vec3d& vMinBB, const Vec3d& vMaxBB);	
	bool	SaveLightmaps(CLightScene *pScene,bool bDebug=false);	

	//! /return hashing value to detect changes in the data
	DWORD GetHashValue( void ) const {return(m_dwHashValue);};

	void UpdateHash(const DWORD newHash){CalcNCombineHash(newHash,m_dwHashValue);}

	// --------------------------------------------------

	IEntityRender *										m_pESource;						//!<
	
	radpolylist											m_lstOldPolys;					//!< this class is the owner of these objects (delete is called)

	Vec3d												m_vCenter;						//!<
	float												m_fRadius,m_fRadius2;			//!<

	std::vector<LMCompLight *>							m_LocalLights;					//!< the lights that may affect this RadMesh (this class is not the owner = delete will not be called)

	std::vector<LMCompLight *>							m_LocalOcclLights;				//!< the occlusion map lights types that may affect this RadMesh (this class is not the owner = delete will not be called)

	CString												m_sGLMName;						//!< name of glm corresponding to this mesh

	CRasterCubeImpl										*m_pClusterRC;					//!< Raster cube of the cluster in which this mesh is in

	float												m_fLMAccuracy;					//!< individual GLM accuracy ranging from 0.. with 1 = default global world<->texel ratio
 
	bool												m_bReceiveLightmap;				//!< indicates whether an object will receive lightmaps or not

	unsigned int										m_uiFlags;						//!< flags, currently used during normal check and triangle initialisation		

	float												m_fMaxVariance;					//!< variance in normal length (purely for information)

	IVariable											*pLightmapQualityVar;			//!< lightmap quality variable to adjust it if necessary

	unsigned int										m_uiTexCoordsRequired;			//!< required texture coordinates by engine, should always match number of indices

	GLMOcclLightInfo									m_OcclInfo;						//!< required information what light corresponds to which colour channel

	//! constructor
	CRadMesh() : 
		m_uiCoarseTexelCount(0), pLightmapQualityVar(NULL), m_fMaxVariance(0.f), m_fLMAccuracy(1.f), m_pESource(NULL), 
			m_dwHashValue(0), m_pClusterRC(NULL), m_bReceiveLightmap(true), m_uiFlags(0), m_uiTexCoordsRequired(0){}

private:
	unsigned int										m_uiCoarseTexelCount;			//!< texel count required (simple summation over all patches after CalcExtent)

	DWORD												m_dwHashValue;					//!< for selective recomputation

	DWORD CalcLocalLightHashValue( void ) const;
	
};

/////////////////////////////////////////////////////////////////////
class CLightPatch
{
public:
	CLightPatch(UINT iPatchResolution) : m_pLightmapImage(0),m_pHDRLightmapImage(0),m_pDominantDirImage(0),m_pOcclMapImage(NULL),m_nTextureNum(0)
	{
		m_nPatchSpace.resize(iPatchResolution);
	}

	~CLightPatch()
	{
		if(m_pLightmapImage)				{	delete [] m_pLightmapImage;m_pLightmapImage=NULL; }
		if(m_pHDRLightmapImage)				{	delete [] m_pHDRLightmapImage;m_pHDRLightmapImage=NULL; }
		if(m_pDominantDirImage)				{	delete [] m_pDominantDirImage;m_pDominantDirImage=NULL; }
		if(m_pOcclMapImage)					{	delete [] m_pOcclMapImage;m_pOcclMapImage=NULL; }
	}

	//! /param nSizeX 1..
	//! /param nSizey 1..
	void CreateDot3Lightmap( int nSizeX, int nSizeY,const bool cbGenerateHDRMaps, const bool cbGenerateOcclMaps = false );
	void Reset();

	//!
	const bool GetSpace(int w,int h, uint16 &x, uint16 &y);

	std::vector<int>	m_nPatchSpace;					//!<
	int					m_nTextureNum;					//!<
	int					m_nWidth,m_nHeight;				//!<
	std::string			m_szFilename;					//!<

	unsigned char			*m_pHDRLightmapImage;		//!< HDR Lightmap texture (in Dot3Lightmaps this is used for the non dominant light sources)
	unsigned char			*m_pLightmapImage;			//!< Lightmap texture (in Dot3Lightmaps this is used for the non dominant light sources)
	unsigned char			*m_pDominantDirImage;		//!< only used for Dot3Lightmaps (normalized worldspace direction to the dominant lightsource)
	SOcclusionMapTexel		*m_pOcclMapImage;			//!< used for occlusionmap data
};

//////////////////////////////////////////////////////////////////////
class CLightScene
{
public:
	CLightScene();
	~CLightScene();
	
	bool CreateFromEntity(
		const IndoorBaseInterface &pInterface, 
		LMGenParam sParam,	
		std::vector<std::pair<IEntityRender*, CBrushObject*> >& vNodes, 
		CLMLightCollection& cLights, 
		struct ICompilerProgress *pIProgress, 
		const ELMMode Mode, 
		volatile SSharedLMEditorData* pSharedData, 
		const std::set<const CBrushObject*>& vSelectionIndices,
		bool &rErrorsOccured);

	radmeshlist				m_lstRadMeshes;				//!< this class is the owner of these objects (delete is called)
	radpolylist				m_lstScenePolys;			//!<
	unsigned int			m_uiCurrentPatchNumber;		//!< to number the current lightmap
	CLightPatch	*			m_pCurrentPatch;			//!<
	LMGenParam				m_sParam;

	void CreateNewLightmap( void );

	// The assign queue system
	struct LMAssignQueueItem
	{
		IEntityRender *								pI_GLM;						//!
		std::vector<struct TexCoord2Comp>			vSortedTexCoords;			//!
		DWORD										m_dwHashValue;				//!< hashing value to detect changes in the data
		std::vector<std::pair<EntityId, EntityId> >	vOcclIDs;					//!< occlusion indices corresponding to the 0..4 colour channels
	};

	std::list<LMAssignQueueItem>					m_lstAssignQueue;			//!

	std::vector<CString>& GetLogInfo()
	{
		return m_LogInfo;
	} 

	std::multimap<unsigned int,std::string>& GetWarningInfo()
	{
		return m_WarningInfo;
	}
	
	const float ComputeHalvedLightmapQuality(const float fOldValue);

protected:
	void DoLightCalculation(
		unsigned int &ruiMaxColourComp, 
		const std::vector<LMCompLight*>& vLights, 
		const std::vector<LMCompLight*>& vOcclLights,
		SComplexOSDot3Texel& dot3Texel, 
		CRadMesh *pMesh, 
		CRadPoly *pSource, 
		const CRadVertex& Vert, 
		const bool cbFirstPass, 
		const unsigned int cuiSubSampleIndex = 0);
	void CheckLight(LMCompLight& rLight, const int iCurLightIdx);
	void GenerateTexCoords(const CRadMesh& rRadMesh, LMAssignQueueItem& rNewGLMQueueItm);
	void SelectObjectLMReceiverAndShadowCasterForChanges(
		std::vector<std::pair<IEntityRender*, CBrushObject*> >& vNodes, 
		const std::vector<AABB>& vAABBs, 
		const Matrix33& rMatSunBasis,
		const std::vector<unsigned int>& rvNodeRadMeshIndices,
		const ELMMode Mode);
	void SelectObjectsFromChangedLMShadowCaster(
		std::vector<std::pair<IEntityRender*, CBrushObject*> >&vNodes, 
		const std::vector<AABB>& vAABBs, 
		const Matrix33& rMatSunBasis,
		const std::vector<unsigned int>& rvNodeRadMeshIndices);
	void ComputeSmoothingInformation(const unsigned int uiSmoothAngle = 45, const bool cbTSGeneratedByLM = false);	//computes the sharing information and stores it into the respective per polygon vectors
	void MergeTangentSpace(CRadVertex& rVertex1, CRadVertex& rVertex2);	//merges two tangent spaces according to normal
	const unsigned int CheckNormals(float& rfMaxVariance, const CRadMesh* const pMesh);//returns true if some normals are not normalized, rfMaxVariance is the max variance encountered
	bool Create(const IndoorBaseInterface &pInterface, const Vec3d& vMinBB, const Vec3d& vMaxBB, volatile SSharedLMEditorData *pSharedData, const ELMMode Mode, const unsigned int cuiMeshesToProcessCount);
	void FlushAssignQueue();
#ifdef APPLY_COLOUR_FIX
	void PerformAdaptiveSubSampling(
		CRadMesh *pMesh, 
		CRadPoly *pSource, 
		const std::vector<LMCompLight*>& vLights, 
		const std::vector<LMCompLight*>& vOcclLights,		
		unsigned int uiMaxComponent);
#else
	void PerformAdaptiveSubSampling(
		CRadMesh *pMesh, 
		CRadPoly *pSource, 
		const std::vector<LMCompLight*>& vLights,
		const std::vector<LMCompLight*>& vOcclLights);
#endif
	void SetSubSampleDot3LightmapTexel(
		const unsigned int cuiSubSampleIndex, 
		const float fColorRLamb, const float fColorGLamb, const float fColorBLamb,
		Vec3d &inLightDir, const float cfLM_DP3LerpFactor,
		SComplexOSDot3Texel& rDot3Texel,
		const SOcclusionMapTexel& rOcclTexel, bool bHDR);
	void GatherSubSampleTexel(const CRadPoly *pSource, const int ciX, const int ciY, std::set<std::pair<unsigned int, unsigned int> >& rvSubTexels);
	const bool FlushAndSave(volatile SSharedLMEditorData *pSharedData, const ELMMode Mode);
	void Reset()
	{
		//i need this definitely to be deallocated, thats why clear and resize call (implementations differ to much which really deallocates)
		m_lstRadMeshes.clear();
		m_lstScenePolys.clear();
		m_lstAssignQueue.clear();
		m_lstRadMeshes.resize(0);
		m_lstScenePolys.resize(0);
		m_lstAssignQueue.resize(0);
		m_lstClusters.clear();
		m_lstClusters.resize(0);
	};

	void WriteLogInfo();

	void Init()
	{
		Reset();
		MEMORYSTATUS sStats;
		GlobalMemoryStatus(&sStats);
		m_uiStartMemoryConsumption = ((sStats.dwTotalVirtual - sStats.dwAvailVirtual) / 1024/1024);
	}
	const unsigned int GetUsedMemory()
	{
		MEMORYSTATUS sStats;
		GlobalMemoryStatus(&sStats);
		const unsigned int cuiCurrentTotal((sStats.dwTotalVirtual - sStats.dwAvailVirtual) / 1024/1024);
		if(cuiCurrentTotal <= m_uiStartMemoryConsumption)
			return 0;
		return (cuiCurrentTotal - m_uiStartMemoryConsumption);
	};
	void DumpSysMemStats()
	{
		_TRACE(m_LogInfo, true, "System memory usage: %iMB\r\n", GetUsedMemory());
	};
	void DisplayMemStats(volatile SSharedLMEditorData *pSharedData)
	{
		if(pSharedData != NULL)
		{
			::SendMessage( pSharedData->hwnd, pSharedData->uiMemUsageMessage, min(GetUsedMemory(),1000), 0 );//update progress bar
			::SendMessage( pSharedData->hwnd, pSharedData->uiMemUsageStatic, min(GetUsedMemory(),1000), 0 );//update progress bar static element
			MSG msg;
			while( FALSE != ::PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
			{ 
				::TranslateMessage( &msg );
				::DispatchMessage( &msg );
			}
		}
		else
			DumpSysMemStats();
	}
	static bool CastsLightmap(const CBrushObject *pBrushObject)
	{
		bool bCastLightmap = true;
		if(pBrushObject)
		{
			CVarBlock *pVarBlock = pBrushObject->GetVarBlock();
			if(pVarBlock)
			{
				IVariable *pVar = pVarBlock->FindVariable("CastLightmap");
				if(pVar && (pVar->GetType() == IVariable::BOOL))
				{
					pVar->Get(bCastLightmap);
				}
			}
		}
		return bCastLightmap;
	}
	void InitSubSampling(const unsigned int cuiSampleCount)
	{
		m_puiIndicator	= new unsigned int[cuiSampleCount];							assert(m_puiIndicator);
		m_pSubSamples	= new SComplexOSDot3Texel[cuiSampleCount];					assert(m_pSubSamples);						
		m_pfColours		= new float [cuiSampleCount * 4/*4 colour components*/];	assert(m_pfColours);
		if(m_sParam.m_bGenOcclMaps)
		{
			m_pOcclSamples = new SOcclusionMapTexel[cuiSampleCount];				assert(m_pOcclSamples);
		}
		m_uiSampleCount = cuiSampleCount;
	}
	
private:
	std::multimap<unsigned int,std::string>	m_WarningInfo;					//!< will contain warning summary for logfile
	std::vector<CString>					m_LogInfo;						//!< will contain some logging info

	struct ILMSerializationManager *m_pISerializationManager;
	std::list<GLMCluster>			m_lstClusters;
	CRasterCubeManager				m_RasterCubeManager;

	unsigned int				*m_puiIndicator;							//!< index of each pixel whether a value has been set or not
	SComplexOSDot3Texel			*m_pSubSamples;								//!< subsamples for one texel
	SOcclusionMapTexel			*m_pOcclSamples;							//!< subsamples for occl map texel	
	float						*m_pfColours;								//!< colours
	unsigned int				m_uiSampleCount;							//!< subsample count for one single texel

	unsigned int m_uiStartMemoryConsumption;

	IndoorBaseInterface	m_IndInterface;
	
	bool ComputeRasterCube(CRasterCubeImpl *pRasterCube, const std::set<IEntityRender *>& vGeom, const Matrix33 *pDirLightTransf = NULL);

	void Shadow(LMCompLight& cLight, UINT& iNumHit, CRadMesh *pMesh, CRadPoly *pSource, const CRadVertex& Vert, const Vec3d& vSamplePos);

	#define ALREADY_TESTED_ARRARY_SIZE	1024

	class CEveryObjectOnlyOnce :public CPossibleIntersectionSink<RasterCubeUserT>
	{
	public:
		//! /param invFrom
		//! /param invDir
		//! /param infRayLength
		//! /param inpIgnore for bias free rayshooting from object surface, may be 0
		void SetupRay( const Vec3d& vRayDir, const Vec3d& vRayOrigin, float fRayLen, CRadPoly* pRayDest)
		{
			m_vRayDir = vRayDir;
			m_vRayOrigin = vRayOrigin;
			m_fRayLen = fRayLen;
			m_pRayDest = pRayDest;
			m_dwAlreadyTested=0;
		}
 
		// --------------------------------------------

	protected:
		Vec3d					m_vRayOrigin;										//!< position in world space 
		Vec3d					m_vRayDir;											//!< direction in world space
		DWORD					m_dwAlreadyTested;									//!< 0..ALREADY_TESTED_ARRARY_SIZE
		float					m_fRayLen;											//!<
		const RasterCubeUserT *	m_arrAlreadyTested[ALREADY_TESTED_ARRARY_SIZE];		//!< [0..ALREADY_TESTED_ARRARY_SIZE-1], this is faster than a set<> at least for typical amounts
		CRadPoly*				m_pRayDest;											//!< ray destination polygon 

		//! /return true=object is already tested, false otherwise
		bool InsertAlreadyTested( RasterCubeUserT *inObject )
		{
			const RasterCubeUserT * *ptr=m_arrAlreadyTested;

			for(DWORD i=0;i<m_dwAlreadyTested;i++)
				if(*ptr++==inObject)return(true);

			if(m_dwAlreadyTested<ALREADY_TESTED_ARRARY_SIZE-1) m_arrAlreadyTested[m_dwAlreadyTested++]=inObject;
				else assert(0);		// ALREADY_TESTED_ARRARY_SIZE is not big enougth

			return(false);
		}
	};//CEveryObjectOnlyOnce

	// ---------------------------------------------------------------------------------------------
	// Intersection sink
	// ---------------------------------------------------------------------------------------------
	class CAnyHit : public CEveryObjectOnlyOnce
	{
	public:

		void SetupRay(const Vec3d& vRayDir, const Vec3d& vRayOrigin, float fRayLen, const float infGridSize, CRadPoly* pRayDest = NULL)
		{
			CEveryObjectOnlyOnce::SetupRay(vRayDir,vRayOrigin,fRayLen, pRayDest);
			m_pHitResult = 0;
			m_fClosest = FLT_MAX;
			m_fGridSize=infGridSize;
		};
		bool IsIntersecting() const { return(m_pHitResult!=0); }

		// TODO
		float							m_fD;
		Vec3d							m_vPNormal;
		float							m_fGridSize;

		const bool ReturnElement(RasterCubeUserT &inObject, float &inoutfRayMaxDist);

		RasterCubeUserT *m_pHitResult;
	protected:
		float m_fClosest;

		// Still intersect with backfacing triangles. While this fixes a few false shadow cases, it
		// adds ugly bright seems between caster and shadow, which looks far worse
		int	intersect_triangle(	float orig[3], float dir[3],
								float vert0[3], float vert1[3], float vert2[3],
								float *t, float *u, float *v);
	};//CAnyHit
	friend class CRadPoly;
};//CLightScene

//computes a fade for the light average weight
static inline const float LightInfluence(const float cfDot)
{
	static const float scfMinInfluence = 0.01f;	//influence = 0
	static const float scfFullInfluence = 0.2f; //influence = 1
	if(cfDot >= scfFullInfluence)
		return 1.f;
	if(cfDot <= scfMinInfluence)
		return 0.f;
	return (cfDot - scfMinInfluence) / (scfFullInfluence - scfMinInfluence);
}

#endif // __INDOOR_LIGHT_PATCHES_H__