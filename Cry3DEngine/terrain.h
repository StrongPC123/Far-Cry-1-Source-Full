////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef TERRAIN_H
#define TERRAIN_H

#include "../Cry3DEngine/Cry3DEngineBase.h"

// hightmap is stored in memory as ushort, this variable will convert it into meters
const float TERRAIN_Z_RATIO = 1.f/256.f;
const float INV_TERRAIN_Z_RATIO = 256.f;

// lowest level of the world
#define BOTTOM_LEVEL 0

// hightmap contain some additional info in last 5 bits
#define STYPE_BIT_MASK     (1|2|4)    // surface type id
#define LIGHT_BIT_MASK          8     // is this point is on the light
#define EXPLO_BIT_MASK         16     // is this point modified by explosion
#define INFO_BITS_MASK   (1|2|4|8|16) // skip this bits when we need to get elevation

#define STYPE_HOLE STYPE_BIT_MASK  // this surface type is reserved for hole in terrain
#define MAX_SURFACE_TYPES_COUNT 7  // max number of surfaces excluding hole type

// update texture lod only every x frames
#define TEXTURE_UPDATE_PERIOD_IN_FRAMES 32

// max view distance for objects shadow is size of object multiplied by this number
#define OBJ_MAX_SHADOW_VIEW_DISTANCE_RATIO 4

// max distance of detail texture layers
#define DETAIL_TEXTURE_VIEW_DIST 120

#define HEIGHT_MAP_FILE_NAME "terrain\\land_map.h16"

#define CHAR_TO_FLOAT 0.01f

#define TERRAIN_SECTORS_MAX_OVERLAPPING 16.f

#include "Vegetation.h"

//! Info about static shadow
/*class CStatObjInstShadow
{
public:
  int m_nObjectTypeID;
  Vec3d m_vPos, m_vParentPos;
  float m_fScale;
  float m_fMaxDist;
	float m_fDistance;
	struct CLeafBuffer * m_arrShadowMapLeafBuffers[4];
	list2<struct ShadowMapLightSourceInstance> * m_pShadowMapCasters;

  CStatObjInstShadow( Vec3d vPos, Vec3d vParentPos, const int nObjectTypeID, const float fScale )
  {
    m_vPos = vPos;
    m_vParentPos = vParentPos;
    m_nObjectTypeID = nObjectTypeID;
    m_fScale = fScale;
    m_fDistance = m_fMaxDist = 0;
		memset(m_arrShadowMapLeafBuffers, 0, sizeof(m_arrShadowMapLeafBuffers));
		m_pShadowMapCasters = 0;
  }
	void Release(IRenderer * pRenderer);
};*/

class CCamera;
struct CSectorInfo;
class CObjManager;
class CDetailGrass;
class CTerrain;

#include "Array2d.h"

// Texture streaming cache
class CTexturePool : public Cry3DEngineBase
{
  struct TexInfo
  {
    TexInfo() { memset(this,0,sizeof(TexInfo)); }
    int nTexId;
    CSectorInfo * pSectorInfo;

		void GetSize (ICrySizer*pSizer)const;
  };
  list2<TexInfo> m_TexturePool[2];

public:

  CTexturePool();
  int MakeTexture(uchar * pData, int nSize, CSectorInfo * pSectorInfo, bool bMakeUncompressedForEditing);
  void RemoveTexture(int nId);
  const char * GetStatusText(CTerrain * pTerrain);
	void GetMemoryUsage (class ICrySizer*pSizer);
};

// HM data
class CHighMap : public Cry3DEngineBase
{
  Array2d<unsigned short> m_arrusHightMapData;

public:
  // Access to hmap data
  float GetZSafe(const int x, const int y);
  float GetZSafe(const float x, const float y);
  float GetZ(const int x, const int y) { return  TERRAIN_Z_RATIO*(m_arrusHightMapData[x>>HMAP_BIT_SHIFT][y>>HMAP_BIT_SHIFT] & (~INFO_BITS_MASK)); }
  float GetZF(const float x, const float y) ;
  bool IsOnTheLight(const int x, const int y) ;
  void SetBurnedOut(int x, int y, bool bBurnedOut);
  bool IsBurnedOut(int x, int y);
  uchar GetSurfaceTypeID(const int x, const int y);
  int GetSurfaceType(const int x, const int y);
  float GetZApr(const float x1, const float y1);
	bool GetHole(const int & x, const int & y) {  return (m_arrusHightMapData[(x+HMAP_BIT_SHIFT)>>HMAP_BIT_SHIFT][(y+HMAP_BIT_SHIFT)>>HMAP_BIT_SHIFT] & STYPE_BIT_MASK) == STYPE_HOLE; }
	bool GetHoleSafe(const int & x, const int & y);
  inline unsigned short & GetHMValue(const int & x, const int & y) { return m_arrusHightMapData[x>>HMAP_BIT_SHIFT][y>>HMAP_BIT_SHIFT]; }
  void SetLightValue(const int x, const int y, const bool bValue) ;
  bool IntersectWithSector(Vec3d vStartPoint, Vec3d vStopPoint, float fDist, int nMaxTestsToScip);
  bool IsPointOccludedByTerrain(const Vec3d & _vPoint, float fDist, const Vec3d & vCamPos, int nMaxTestsToScip);
  bool LoadHighMap(const char * file_name, struct ICryPak * pCryPak);
  int HMAP_BIT_SHIFT;
	void GetHighMapMemoryUsage(ICrySizer*pSizer) { m_arrusHightMapData.GetMemoryUsage(pSizer); }
  bool m_bHightMapModified;
};

struct DetailTexInfo
{
  DetailTexInfo() 
  { memset(this, 0,sizeof(DetailTexInfo)); ucThisSurfaceTypeId=255; }
  int nTexID;
  CLeafBuffer	* pVertBuff;
	list2<struct_VERTEX_FORMAT_P3F_N_COL4UB> lstVertArray;
  list2<ushort> lstIndices;

	float fScaleX, fScaleY;
  uchar ucProjAxis;
  uchar ucThisSurfaceTypeId;
	float arrTexOffsets[16];
	IMatInfo * pMatInfo;
};

struct VolumeInfo
{
  VolumeInfo() { memset(this,0,sizeof(VolumeInfo)); nRendererVolumeID=-1; }
  int nRendererVolumeID;
  Vec3d vBoxMin;
  Vec3d vBoxMax;
  Vec3d vColor;
  float fMaxViewDist;
  IShader * pShader;
  uint nLastRenderedFrameId;
  bool bIndoorOnly;
  bool bOcean;
	bool m_bCaustics;

  bool IntersectBBox( const Vec3 &vObjBoxMin,const Vec3 &vObjBoxMax ) 
  {
    if  (vBoxMin.x>=vObjBoxMax.x) return 0;
    if  (vBoxMin.y>=vObjBoxMax.y) return 0; 
    if 	(vBoxMin.z>=vObjBoxMax.z) return 0; 
    if  (vBoxMax.x<=vObjBoxMin.x) return 0;
    if  (vBoxMax.y<=vObjBoxMin.y) return 0;
    if  (vBoxMax.z<=vObjBoxMin.z) return 0; 
    return 1;
  }

  bool InsideBBox( const Vec3 &vObjPos ) 
  {
    if  (vBoxMin.x>=vObjPos.x) return 0;
    if  (vBoxMin.y>=vObjPos.y) return 0; 
    if 	(vBoxMin.z>=vObjPos.z) return 0; 
    if  (vBoxMax.x<=vObjPos.x) return 0;
    if  (vBoxMax.y<=vObjPos.y) return 0;
    if  (vBoxMax.z<=vObjPos.z) return 0; 
    return 1;
  }
};

// The Terrain Class
class CTerrain : public CHighMap
{
  Vec3d m_vCameraPos;
  int m_nOldSectorsX;
  int m_nOldSectorsY;
	CCamera * m_pViewCamera;
 
public:
  CTerrain();
  ~CTerrain();
  bool LoadTerrain(bool bEditorMode);
  bool m_bCameraInsideBuilding;
  CTexturePool * m_pTexturePool;
  void LoadStatObjInstances();

  void SetSurfaceType(int x, int y, int nType);

  void SetTerainHightMapBlock(int x1, int y1, int nSizeX, int nSizeY, ushort * TerrainBlock, ushort nUpdateMask);

	float GetWaterLevel() { return m_fGlobalWaterLevel;/* ? m_fGlobalWaterLevel : WATER_LEVEL_UNKNOWN;*/ }

  unsigned char m_ucTerrainFrame;
  int  m_nUploadsInFrame;
  int RenderEntitiesOutOfTheMap(CObjManager * pObjManager);
  int RenderTerrainWater(bool bRenderShore);
  int  GetSecMML(int x, int y);
  float m_fDistanceToSectorWithWater;
  void RenderTerrain( CObjManager * pObjManager, const int & DrawFlags );
  void DrawVisibleSectors();

  void PreCacheArea(const Vec3d & pos, float fRadius);
  int      m_nSectorTextureReadedSize;
  int      m_nSectorTextureDataSizeBytes;
  uchar *  m_ucpTmpTexBuffer;
  FILE  *  m_fpTerrainTextureFile;

  Array2d<CSectorInfo *> m_arrSecInfoTable;

  // editing
  bool AddStaticObject(int nObjectID, const Vec3d & vPos, const float fScale, CObjManager * pObjManager, uchar ucBright);
	bool RemoveStaticObject(int nObjectID, const Vec3d & vPos, CObjManager * pObjManager);
  void RemoveAllStaticObjects();

  // detail texture
  void DrawDetailTextures(float fFogNearDistance, float fFogFarDistance, bool bRealyDraw);
	void MakeSplatVertex(const int & x,const int & y,const uchar & alpha, struct_VERTEX_FORMAT_P3F_N_COL4UB & vert, const int & nTexID, const Vec3d & vDetTexOffset);
	void SetDetailVertNormal(struct_VERTEX_FORMAT_P3F_N_COL4UB & v0);

	void RenderDetailTextures(float _fFogNearDistance, float _fFogFarDistance);

  float m_fLodLFactor;

/*  int GetSectorStatics( int x, int y,
                        list2<CStatObjInst> ** _stat_objects, 
                        list2<CStatObjInstShadow> ** _stat_shadows, 
                        list2<CStatObjInstShadow> ** _stat_shadows_plus );
  */
  void ExplodeTerrain(Vec3d vExploPos, float fExploRadius, CObjManager * pObjManager, bool bDeformTerrain);
  
  CSectorInfo * GetSecInfo(int x, int y)
  {
    if( x<0 || y<0 || x>=GetTerrainSize() || y>=GetTerrainSize() )
		  return 0;

    return m_arrSecInfoTable[x/CTerrain::GetSectorSize()][y/CTerrain::GetSectorSize()];
  }

  CSectorInfo * GetSecInfo(const Vec3d & pos)
  {
		int x = fastftol_positive(pos.x);
		int y = fastftol_positive(pos.y);
    if(x<0 || y<0 || x>=GetTerrainSize() || y>=GetTerrainSize() )
		  return 0;

    return m_arrSecInfoTable[x/CTerrain::GetSectorSize()][y/CTerrain::GetSectorSize()];
  }

  CLeafBuffer * m_pTerrainDecals;

  class CBeachGenerator * m_pBeachGenerator;

  CSectorInfo * GetSectorFromPoint(int x, int y);

  bool IsStreamingNow() { return m_nUploadsInFrame>0; }

  list2<VolumeInfo> m_lstFogVolumes;
  void InitFogVolumes();

  IShader *m_pSHShore;

  void MergeLowResTerrainSectorIndices(list2<unsigned short> * pIndices);
  void MergeReflectedTerrainSectorIndices(list2<unsigned short> * pIndices);
  void ResetDLightMaskInSectors();

  list2<struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB> m_lstSectorVertArray;

  void ApplyForceToEnvironment(Vec3d vPos, float fRadius, float fAmountOfForce);

  Vec3d GetTerrainSurfaceNormal(Vec3d vPos);

  bool UnRegisterInAllSectors(IEntityRender * pEntityRS);

  IShader * m_pTerrainEf, *m_pTerrainLightPassEf, *m_pTerrainShadowPassEf;
  IShader * m_pTerrainEf_WithDefaultDetailTexture, *m_pTerrainWithFog;
	IShader * m_pTerrainLayerEf;
  float m_fShoreSize;

  void InitBeaches(bool bEditorMode);
//  void SetTerainSectorTexture(int nSectorOroginX, int nSectorOroginY, unsigned char * pTexData, int nSizeOffTexData);
  void SetDetailTextures(int nId, const char * szFileName, float fScaleX, float fScaleY, uchar ucProjAxis, const char * szSurfName);
  void SetCBuffer(class CCoverageBuffer * pBuffer) { m_pCoverageBuffer = pBuffer; };
  void RenderReflectedTerrain();
  ITexPic * m_pLowLodCoverMapTex;

protected:
  const char * GetLevelFilePath(const char * filename);
  float m_fGlobalWaterLevel;

  list2<CSectorInfo *> m_lstVisSectors;

  void InitSectors(bool bEditorMode);
  void RefineSector(int x1, int x2, int y1, int y2, bool bAllIN);
  void LinkVisSetcors();
  void DrawWholeTerrainFast(bool under_water);
  void AddVisSetcor(CSectorInfo * newsec);

  CRETerrainDetailTextureLayers * m_pRETerrainDetailTextureLayers;
  IShader * m_pTerrainDetailTextureLayersEff; 

  int m_nDetailTexFocusX;
  int m_nDetailTexFocusY;

  CLeafBuffer * m_pLowResTerrainLeafBuffer, * m_pReflectedTerrainLeafBuffer;
  int m_nLowResTerrainVertCount;
  list2<unsigned short> m_lstLowResTerrainIdxArray, m_lstReflectedTerrainIdxArray;
  IShader * m_pLowResTerrainShader;
  void RenderLowResTerrain();
  float m_arrLowResTerrainShaderCustomData[8];
  bool IsSectorNonMergable(CSectorInfo * info);
  class CCoverageBuffer * m_pCoverageBuffer;
  class CObjManager * m_pObjManager;

public:
	void SetObjManager(CObjManager*pObjManager) { m_pObjManager = pObjManager; }
	CObjManager*GetObjManager() { return m_pObjManager; }
  Vec3d	m_vPrevCameraPos;	// cached data camera pos

  DetailTexInfo m_DetailTexInfo[MAX_SURFACE_TYPES_COUNT];

  static const int GetTerrainSize()     { return m_nTerrainSize; }
  static const int GetSectorSize()      { return m_nSectorSize; }
  static const int GetHeightMapUnitSize() { return m_nUnitSize; }
  static const int GetSectorsTableSize() { return m_nSectorsTableSize; }
	static const float GetInvUnitSize()		{ return m_fInvUnitSize; }
  
  static int m_nUnitSize;
	static float m_fInvUnitSize;
  static int m_nTerrainSize; 
  static int m_nSectorSize;
  static int m_nSectorsTableSize; 

  void InitTerrainWater(bool bEditorMode, IShader * pTerrainWaterShader, int nWaterBottomTexId, IShader * pSunRoadShader, float fWaterTranspRatio, float fWaterReflectRatio, float fWaterBumpAmountX, float fWaterBumpAmountY, float fWaterBorderTranspRatio);
	class CWaterOcean * m_pWater;
  CDetailGrass * m_pDetailObjects;

  // low res terrain with smoothed under water areas
  void MakeUnderWaterSmoothHMap(int nWaterUnitSize);
  ushort * GetUnderWaterSmoothHMap(int & nDimensions);
  Array2d<unsigned short> m_arrusUnderWaterSmoothHMap;

//  int RenderStaticObjects(CObjManager * pObjManager);
  bool RenderAreaLeafBuffers(Vec3d vPos, float fRadius, int nDynLMask, 
    CLeafBuffer ** arrLightLeafBuffers, int nMaxLeafBuffersNum, CCObject * pObj, IShader * pShder, 
		bool bRecalcLeafBuffers, const char * szComment, IShader * pEffStencilTest=NULL, int nShaderSortOrder=0, ShadowMapFrustum * pFrustum=NULL, Vec3d * pvFrustPos=NULL, float fFrustScale=1.f);
  bool LoadTerrainSettingsFromXML();

//  void RegisterStatObjInstanceShadow(CObjManager * pObjManager, CStatObjInst objInst, bool bUnregister = false);
//  void RegisterAllStatObjInstanceShadows(CObjManager * pObjManager);
	int LockSectorTexture(int nSectorOriginX, int nSectorOriginY, int & nTexDim);
	int GetTerrainTextureDim(){ return m_nSectorTextureReadedSize * GetSectorsTableSize(); }
  void UnloadOldSectors(float fMaxViewDist);
	void RenderDLightOnHeightMap(CDLight * pDLight);
	void GetMemoryUsage(class ICrySizer*pSizer);
	int m_arrLoadedTexturesNum[2];
	void UnregisterFogVolumeFromOutdoor(VolumeInfo * pFogVolume);
  float m_fTextureLodRatio;
  void MoveAllEntitiesIntoList(list2<IEntityRender*> * plstVisAreasEntities, 
                               const Vec3d & vBoxMin, const Vec3d & vBoxMax);
  void ResetTerrainVertBuffers();
  void SetSectorFogVolume(CSectorInfo * pSecInfo);
  void InitDetailTextureLayers();
  void SortStaticInstancesBySize();
  void CheckUnload();
  int m_nLoadedSectors;
  void GetStreamingStatus(int & nLoadedSectors, int & nTotalSectors);
	bool PreloadResources();
	void GetObjects(list2<struct IEntityRender*> * pLstObjects);

	void CloseTerrainTextureFile();

	bool m_bOceanIsVisibe;
	int m_nDefZoomTexId;

	const void * GetShoreGeometry(int & nPosStride, int & nVertCount, int nSectorX, int nSectorY);
	CMatInfo m_matSecondPass;
	void GetObjectsAround(Vec3d vPos, float fRadius, list2<IEntityRender*> * pEntList);
};

// this structure is used only during loading of positions of static objects,
// file objects.lst contain array of this structures
class CStatObjInstForLoading
{
public:
  // decompress values from short and char into float
  float GetX() { return float(m_nX)*CTerrain::GetTerrainSize()/65536.f; }
  float GetY() { return float(m_nY)*CTerrain::GetTerrainSize()/65536.f; }
  float GetZ() { return float(m_nZ)*CTerrain::GetTerrainSize()/65536.f; }
  float GetScale() { return m_fScale; }
  int GetID() { return (int)m_ucObjectTypeID; }
  uchar GetBrightness() { return m_ucBrightness; }

protected:
  ushort m_nX,m_nY,m_nZ; 
  uchar m_ucObjectTypeID;
  uchar m_ucBrightness;
  float m_fScale;
};

// Shore geometry generator
class CBeachGenerator
{
public:
  CBeachGenerator(CTerrain * pTerrain) 
  { 
    memset(this,0,sizeof(*this)); m_pTerrain = pTerrain;
    m_arrBeachMap.Allocate(CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize()+1);
    m_WaterAreaMap.Allocate(CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize()+1);
  }

	int MarkWaterAreas();
	void RenameWaterArea( int nOld, int nNew );

  CTerrain * m_pTerrain;
  struct CBeachInfo { bool beach,in_water; };
  Array2d<CBeachInfo> m_arrBeachMap;
  Array2d<char>  m_WaterAreaMap;
  list2<int> m_lstWaterAreaSizeTable;
  int m_nMaxAreaSize;
};

#endif // TERRAIN_H
