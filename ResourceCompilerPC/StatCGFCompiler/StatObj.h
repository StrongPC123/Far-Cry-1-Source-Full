////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobj.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef STAT_OBJ_H
#define STAT_OBJ_H

const int FAR_TEX_COUNT = 24; // number of sprites per object
const int FAR_TEX_ANGLE = (360/FAR_TEX_COUNT);
const int FAR_TEX_SIZE  = 64;

const int SHADOW_TEX_SIZE  = 256;

class CIndexedMesh;
class CCObject;

#include "../Cry3DEngine/Cry3DEngineBase.h"
#include "list2.h"

struct CStatObjSV;
struct ItShadowVolume;

#include "istatobj.h"

#define STATOBJ_EFT_PLANT             (EFT_USER_FIRST+1)
#define STATOBJ_EFT_PLANT_IN_SHADOW   (EFT_USER_FIRST+2)

struct CStatObj : public Cry3DEngineBase, IStatObj
{
  CStatObj(ISystem	* pSystem);
  ~CStatObj();

  CIndexedMesh * m_pTriData;
  CIndexedMesh * GetTriData() { return m_pTriData; }

  int m_nLoadedTrisCount;
  float GetCenterZ() { return m_vBoxMax.z*0.5f; }
  const Vec3d GetCenter() { return (m_vBoxMax+m_vBoxMin)*0.5f; }
  inline float GetRadius() { return m_fObjectRadius; }

  char m_szFolderName[256];
  char m_szFileName  [256];
  char m_szGeomName  [256];

	bool	m_bDefaultObject;

  TArray<int> m_lstShaderTemplates;
  TArray <SShaderParam> m_ShaderParams;

  uint m_arrSpriteTexID[FAR_TEX_COUNT];

  float m_fObjectRadius;

  void InitParams(float sizeZ);

public:

  // Loader
  bool LoadObject(const char * szFileName, const char * szGeomName, int Stripify, bool bLoadAdditinalInfo, bool bKeepInLocalSpace, bool bLoadLater = false);

  //! Returns script material name
  virtual const char * GetScriptMaterialName(int Id=-1);

  virtual void Render(const SRendParams & rParams, int nLodLevel=0);

	//virtual void RenderModel(const RenderParams *pParams);
	virtual void RenderShadowVolumes(const SRendParams *pParams);

  //! Refresh object ( reload shaders or/and object geometry )
  virtual void Refresh(int nFlags);

  virtual bool SetShaderTemplate(int nTemplate, const char *TemplName, const char *ShaderName, bool bOnlyRegister=false);
  virtual void SetShaderFloat(const char *Name, float Val);
  //virtual void SetRefractFactor(float fRefr) { m_fRefractFactor = fRefr; }

	//! set shadow volume
	ItShadowVolume *GetShadowVolume() { return (m_pSvObj);}

	//! get shadow volume
	void	SetShadowVolume(ItShadowVolume *pSvObj) { m_pSvObj=pSvObj;}

	//Marco's NOTE: NEVER OVERRIDE THESE FLAGS!
	//! get flags
	int	 GetFlags() { return (m_dwFlags); }

	//! set flags
	void SetFlags(int dwFlags) { m_dwFlags=dwFlags; }

	//Marco's NOTE: NEVER OVERRIDE THESE FLAGS!
	//! get flags
	int	 GetFlags2() { return (m_dwFlags); }

	//! set flags
	void SetFlags2(int dwFlags) { m_dwFlags2=dwFlags; }


protected:
  void Physicalize();

  void CreateModelFarImages(int nTexRes);

  CLeafBuffer * m_pLeafBuffer;
	ItShadowVolume *m_pSvObj;
	int	m_dwFlags,m_dwFlags2;

public:

  CLeafBuffer * GetLeafBuffer() { return m_pLeafBuffer; };
	void SetLeafBuffer( CLeafBuffer *buf ) { m_pLeafBuffer = buf; };

  Vec3d m_vBoxMin, m_vBoxMax, m_vBoxCenter;//, m_vGeometryAngles;
	phys_geometry * m_arrPhysGeomInfo[2];
  phys_geometry * GetPhysGeom(int n = 0) { return m_arrPhysGeomInfo[n]; }

	const char *GetFolderName() { return (m_szFolderName); }
	const char *GetFileName()		{ return (m_szFileName); }
	const char *GetGeoName()		{ return (m_szGeomName); }
	bool IsSameObject(const char * szFileName, const char * szGeomName);

	//set object's min/max bbox 
	void	SetBBoxMin(const Vec3d &vBBoxMin) { m_vBoxMin=vBBoxMin; }	
	void	SetBBoxMax(const Vec3d &vBBoxMax) { m_vBoxMax=vBBoxMax; }
  Vec3d & GetBoxMin() { return m_vBoxMin; }
  Vec3d & GetBoxMax() { return m_vBoxMax; }

  int m_nUsers; // reference counter

  ShadowMapLightSource * m_pSMLSource;

  void MakeShadowMaps(const Vec3d vSunPos);
  
protected:
  void MakeBuffers(bool make_tree, int Stripify, char * szCompiledFileName);
	void MakeLeafBuffer( CIndexedMesh *mesh,bool bStripify=false );
  
  void PrepareShadowMaps(const Vec3d & obj_pos, ShadowMapLightSource * pLSource);

public:
//  void DrawShadowMapOnTerrain(const Vec3d & pos, const float fScale, float fAlpha, struct IndexedVertexBuffer ** ppShadowGridBuffer, CTerrain * pTerrain, bool bLMapsGeneration);

  int GetAllocatedBytes();
//  IndexedVertexBuffer * MakeShadowGridBuffer(const Vec3d & pos, const float fScale, ShadowMapFrustum*lf, bool translate_projection, CTerrain * pTerrain);
  void AddShadowPoint(int x, int y, list2<struct_VERTEX_FORMAT_P3F> * pList, CTerrain * pTerrain);

  void SetCurDynMask(int nCurDynMask);

  int FindInPosBuffer(const Vec3d & opt, Vec3d * _vbuff, int _vcount, list2<int> * pHash);
  void CompactPosBuffer(Vec3d * _vbuff, int * _vcount, list2<int> * pindices);

  virtual Vec3d GetHelperPos(const char * szHelperName);
  virtual const char  *GetHelperById(int nId, Vec3d & vPos, Matrix * pMat, int * pnType);
	virtual const Matrix * GetHelperMatrixByName(const char * szHelperName);

  virtual void UpdateCustomLightingSpritesAndShadowMaps(float fStatObjAmbientLevel, int nTexRes);

  //float m_fRefractFactor;
  float m_fRadiusHors;
  float m_fRadiusVert;
//  float m_fBending;
//  int   m_nHideability;
  
  float & GetRadiusVert() { return m_fRadiusVert; }
  float & GetRadiusHors() { return m_fRadiusHors; }

  virtual void RegisterUser();
  virtual void UnregisterUser();

	virtual bool IsDefaultObject() { return (m_bDefaultObject); }
  virtual bool MakeObjectPicture(UCHAR * pRGBAData, int nWidth);

#define MAX_STATOBJ_LODS_NUM 3
  CStatObj * m_arrpLowLODs[MAX_STATOBJ_LODS_NUM];
  void LoadLowLODs(int nStripify,bool bLoadAdditinalInfo,bool bKeepInLocalSpace);
  int m_nLoadedLodsNum;

//  virtual int GetHideability() { return m_nHideability; }
//	virtual void SetHideability(int hideability);
  bool IsSpritesCreated() { return m_arrSpriteTexID[0]>0; }
  float GetDistFromPoint(const Vec3d & vPoint);
  int GetLoadedTrisCount() { return m_nLoadedTrisCount; }

  list2<Vec3d> m_lstOcclVolVerts;
  list2<int>   m_lstOcclVolInds;

  virtual bool GetOcclusionVolume(list2<Vec3d> * & plstOcclVolVerts, list2<int> * & plstOcclVolInds) 
  {
    plstOcclVolVerts = &m_lstOcclVolVerts;
    plstOcclVolInds  = &m_lstOcclVolInds;

    return m_lstOcclVolInds.Count() >= 3;
  }
  list2<struct HelperInfo> m_lstHelpers;	
	list2<CDLight> m_lstLSources;
	bool Serialize(int & nPos, uchar * pSerBuf, bool bSave, char * szFolderName);

	virtual void FreeTriData();
//	virtual void SetBending(float fBending);
	virtual const CDLight * GetLightSources(int nId);
	void RenderDebugInfo(const SRendParams & rParams, const class CCObject * pObj);
	void DrawMatrix(const Matrix & pMat);

	//! Release method.
	void Release() { delete this; }
	void GetMemoryUsage(class ICrySizer* pSizer);
	int GetMemoryUsage();
	void SpawnParticles( ParticleParams & SpawnParticleParams, const Matrix & matWorldSpace, bool bOnlyUpLookingFaces );

	// connectivity object that gets pre-computed for each model once and then
	// used to extract edge topology by the stencil shadowing module
	class IStencilShadowConnectivity* m_pStencilShadowConnectivity;
	// returns the cached connectivity object for stencil shadows
	class IStencilShadowConnectivity* getStencilShadowConnectivity( );
	//////////////////////////////////////////////////////////////////////////
	// builds the connectivity object for stencil shadows
	// PARAMETERS:
	//  iEdgeBuilder - the builder to use to create the connectivity info
	//  pbCastShadow - the array of flags, 1 flag per 1 material, if the flag is true, then this material casts shadow, otherwise not
	//  numMaterials - number of items in the pbCastShadow array
	void buildStencilShadowConnectivity (class IEdgeConnectivityBuilder *inpEdgeCon, const bool* pbCastShadow, unsigned numMaterials);
	void RenderShadowVolumes (const SRendParams *rParams, int nLimitLOD);
	void ShutDown();
	void Init();

	// loading state
	int	 m_nStripify;
	bool m_bLoadAdditinalInfo;
	bool m_bKeepInLocalSpace;
	int  m_nLastRendFrameId;
	bool m_bStreamable;
	static float m_fStreamingTimePerFrame;
	int	 m_nSpriteTexRes;
};

#endif // STAT_OBJ_H
