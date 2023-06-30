////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjman.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef CObjManager_H
#define CObjManager_H

#include "StatObj.h"
#include "../RenderDll/Common/shadow_renderer.h"
#include "terrain_sector.h"

#define   ENTITY_MAX_DIST_FACTOR 100

struct CStatObj;
struct IIndoorBase;
struct IEntityRender;
struct ISystem;

class CStatObjInst;

class C3DEngine;
struct IMatInfo;

#define SMC_STATICS  1
#define SMC_DYNAMICS 2
#define SMC_ALLOW_PASSIVE_SHADOWMAP_CASTERS 4

//! contains stat obj instance group properies (vegetation object properties)
struct StatInstGroup : public IStatInstGroup
{
	StatInstGroup() { pStatObj = 0; }
	CStatObj * GetStatObj() { return (CStatObj*)pStatObj; }

	void SetRndFlags()
	{
		m_dwRndFlags = 0;
		if(bCastShadow)
			m_dwRndFlags |= ERF_CASTSHADOWMAPS;
		if(bRecvShadow)
			m_dwRndFlags |= ERF_RECVSHADOWMAPS;
		if(bPrecShadow)
			m_dwRndFlags |= ERF_CASTSHADOWINTOLIGHTMAP;
		if(bHideability)
			m_dwRndFlags |= ERF_HIDABLE;
		if(bPhysNonColl)
			m_dwRndFlags |= ERF_PHYS_NONCOLL;
	}
};

struct SExportedBrushMaterial
{
  int size;
  char material[64];
};

class CObjManager : public Cry3DEngineBase
{
public:
  CObjManager(C3DEngine * p3DEngine);
  ~CObjManager();

  void LoadVegetationModels(const char *szMissionName, bool bEditorMode);
  void UnloadObjects();

	void DrawFarObjects(float fMaxViewDist);
  void RenderFarObjects();

  void RegisterEntity( IEntityRender* pEntityRS );
  bool UnRegisterEntity( IEntityRender* pEntityRS );

  CStatObj * CObjManager::MakeObject(const char * __szFileName, const char * _szGeomName=0,
    EVertsSharing eVertsSharing = evs_NoSharing,
    bool bLoadAdditinalInfo = true,
    bool bKeepInLocalSpace = false,
    bool bLoadLater = false);

	bool ReleaseObject(CStatObj * pObject);

  bool GetSectorBBox(list2<CStatObjInst> * stat_objects, Vec3d &sec_bbmin, Vec3d &sec_bbmax);

  list2<StatInstGroup> m_lstStaticTypes;

  void MakeShadowMapInstancesList(IEntityRender * pEntityRS, float obj_distance,
																	list2<ShadowMapLightSourceInstance> * pLSourceInstances, 
																	int dwAllowedTypes, CDLight * pLight);

  float m_fZoomFactor;

  CTerrain * m_pTerrain;

  void UpdateCustomLighting(const Vec3d & vLight);

  list2<CStatObjInst*> m_lstFarObjects[2];

protected:
	struct string_less : public std::binary_function<CStatObj*,CStatObj*,bool> 
	{
		bool operator()( CStatObj *s1,CStatObj *s2 ) const
		{
			int nFileCmpRes = stricmp(s1->m_szFileName,s2->m_szFileName);
			if(!nFileCmpRes) // if file name is the same - compare just geom names
			{
				int nObjCmpRes = stricmp(s1->m_szGeomName,s2->m_szGeomName);

				if(nObjCmpRes==0)
				{
					if(s1->m_eVertsSharing == s2->m_eVertsSharing)
					{
						assert(s1->m_bKeepInLocalSpace	== s2->m_bKeepInLocalSpace);
						//					assert(s1->m_bMakePhysics				== s2->m_bMakePhysics);
						//					assert(s1->m_bCalcLighting			== s2->m_bCalcLighting);
						return s1->m_bLoadAdditinalInfo < s2->m_bLoadAdditinalInfo;
					}
					
					return s1->m_eVertsSharing < s2->m_eVertsSharing;
				}

				return nObjCmpRes < 0;
			}

			return nFileCmpRes < 0;
		}
	};
	typedef std::set<CStatObj*,string_less> ObjectsMap;
	ObjectsMap m_lstLoadedObjects;

  void InitFarState();

  CREFarTreeSprites * m_REFarTreeSprites;

  ShadowMapFrustum * MakeEntityShadowFrustum(ShadowMapFrustum * pFrustum, 
    ShadowMapLightSource * pLs, IEntityRender * pEntityRS, EShadowType nShadowType, int dwAllowedTypes);

  list2<IEntityRender*> m_lstDebugEntityList;

  void MakeShadowCastersList(IEntityRender * pEntityRS, list2<IEntityRender*> * pEntList, 
		int dwAllowedTypes, Vec3d vLightPos, float fLightRadius);
  list2<CSectorInfo*>         m_lstTmpSectors_MELFP;

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

public:
	int GetLoadedObjectCount() { return m_lstLoadedObjects.size(); }

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif


  void RenderObject( IEntityRender * o,
        int nFogVolumeID, uint nDLightMask, 
        bool bLMapGeneration,
				const CCamera & EntViewCamera,
				Vec3d * pvAmbColor, Vec3d * pvDynAmbColor,
        VolumeInfo * pFogVolume,
        bool bNotAllInFrustum,
				float fMaxViewDist, IEntityRenderInfo * pEntInfo = NULL);

  float GetXYRadius(int nType);
  bool GetStaticObjectBBox(int nType, Vec3d & vBoxMin, Vec3d & vBoxMax);
  void DrawAllShadowsOnTheGroundInSector(list2<IEntityRender*> * pEntList);
  
	float	m_fWindForce;							//!< used for bending plants
  Vec3d m_vOutdoorAmbientColor;		//!< 
  Vec3d m_vSunColor;							//!< 

  IStatObj * GetStaticObjectByTypeID(int nTypeID);
  float GetBendingRandomFactor();
  C3DEngine * m_p3DEngine;

  class CCoverageBuffer * m_pCoverageBuffer; 
  bool IsBoxOccluded(const Vec3d & vBoxMin, const Vec3d & vBoxMax, float fDistance, OcclusionTestClient * pOcclTestVars);

  list2<IEntityRender*> m_lstStatEntitiesShadowMaps;
	list2<IEntityRender*> m_lstEntitiesShadowSpots;

  void RenderEntitiesShadowMapsOnTerrain(bool bLMapGeneration, class CREShadowMapGen * pREShadowMapGenerator);
	void DrawEntitiesShadowSpotsOnTerrain();
  
  void AddPolygonToRenderer(const int nTexBindId, 
                            IShader * pShader, 
                            const int nDynLMask,
                            Vec3d right,
                            Vec3d up,
                            const UCol & ucResCol,
                            const ParticleBlendType eBlendType,
                            const Vec3d & vAmbientColor,
                            Vec3d vPos,
														const SColorVert * pTailVerts = NULL,
														const int nTailVertsNum = 0,
														const byte * pTailIndices = NULL,
														const int nTailIndicesNum = 0,
														const float fSortId = 0,
                            const int dwCCObjFlags = 0,
                            IMatInfo * pCustomMaterial = NULL,
														CStatObjInst * pStatObjInst = NULL,
														list2<struct ShadowMapLightSourceInstance> * pShadowMapCasters = NULL);
  
  // tmp containers (replacement for local static vars)
  list2<IEntityRender*> lstEntList_MLSMCIA;

  void RenderEntityShadowOnTerrain(IEntityRender * pEntityRS, bool bLMapGeneration,	CREShadowMapGen * pREShadowMapGenerator);
  void DrawObjSpritesSorted(list2<CStatObjInst*> *pList, float fMaxViewDist, int useBending);
	void ProcessActiveShadowReceiving(IEntityRender * pEnt, float fEntDistance, CDLight * pLight, bool bLMapGeneration);

	void SetupEntityShadowMapping( IEntityRender * pEntityRS, SRendParams * pDrawParams, bool bLMapGeneration, float fEntDistance, CDLight * pDLight );
	float m_fMaxViewDistanceScale;
	IShader * m_pShaderOcclusionQuery;
	
	bool LoadStaticObjectsFromXML();	
	CStatObj * m_pDefaultCGF;

#define MAX_LIGHTS_NUM 32
	list2<IEntityRender*> m_lstLightEntities[MAX_LIGHTS_NUM];

	void DrawEntitiesLightPass();
	class CREClearStencil * m_pREClearStencil;
	int GetMemoryUsage(class ICrySizer * pSizer);

	struct CWaterVolumeManager * m_pCWaterVolumes;
	float CalculateEntityShadowVolumeExtent(IEntityRender * pEntityRS, CDLight * pDLight);

  list2<class CBrush*> m_lstBrushContainer;
  list2<class CStatObjInst*> m_lstVegetContainer;
	void LoadBrushes();
  void MergeBrushes();
	void ReregisterEntitiesInArea(Vec3d vBoxMin, Vec3d vBoxMax);
	void ProcessEntityParticles(IEntityRender * pEntityRS, float fEntDistance);
	void CheckUnload();
	void PreloadNearObjects();
	// time counters
  static double m_dMakeObjectTime;
	static double m_dCIndexedMesh__LoadMaterial;
	static double m_dUpdateCustomLightingSpritesAndShadowMaps;

  void MakeShadowBBox(Vec3d & vBoxMin, Vec3d & vBoxMax, const Vec3d & vLightPos, float fLightRadius, float fShadowVolumeExtent);
  CFColor CalcShadowOnTerrainColor(float fAlpha, bool bLMapGeneration);
	float GetSortOffset( const Vec3d & vPos, const Vec3d & vCamPos, float fUserWaterLevel = WATER_LEVEL_UNKNOWN );
  void GetObjectsStreamingStatus(int & nReady, int & nTotalInStreaming, int & nTotal);
	bool ProcessShadowMapCasting(IEntityRender * pEnt, CDLight * pDLight);
	void UnloadVegetations();
	void CheckObjectLeaks(bool bDeleteAll = false);
	bool IsSphereAffectedByShadow(IEntityRender * pCaster, IEntityRender * pReceiver, CDLight * pLight);
	void MakeShadowCastersListInArea(CBasicArea*pArea, IEntityRender * pReceiver, 
		list2<IEntityRender*> * pEntList, int dwAllowedTypes, Vec3d vLightPos, float fLightRadius);
	void PrefechObjects();
	void RequestEntityShadowMapGeneration(IEntityRender * pEntityRnd);
	bool m_bLockCGFResources;
	void FreeNotUsedCGFs();
	void RenderObjectVegetationNonCastersNoFogVolume( IEntityRender * pEntityRS,uint nDLightMask,
		const CCamera & EntViewCamera, 
		bool bNotAllInFrustum, float fMaxViewDist, IEntityRenderInfo * pEntInfo);
};

#endif // CObjManager_H
