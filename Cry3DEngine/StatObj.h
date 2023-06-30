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

class CIndexedMesh;
class CCObject;

#include "../Cry3DEngine/Cry3DEngineBase.h"
#include "list2.h"

struct CStatObjSV;
struct ItShadowVolume;

#include "istatobj.h"
#include "istreamengine.h"

#define STATOBJ_EFT_PLANT             (EFT_USER_FIRST+1)
#define STATOBJ_EFT_PLANT_IN_SHADOW   (EFT_USER_FIRST+2)

#define CGF_STREAMING_MAX_TIME_PER_FRAME 0.01f

struct CStatObj : public Cry3DEngineBase, public IStreamCallback, public IStatObj
{
  CStatObj();
  ~CStatObj();

  CIndexedMesh * m_pTriData;
  CIndexedMesh * GetTriData() { return m_pTriData; }

  int m_nLoadedTrisCount;
  const Vec3d GetCenter() { return m_vBoxCenter; }
  inline float GetRadius() { return m_fObjectRadius; }

  char m_szFolderName[256];
  char m_szFileName  [256];
  char m_szGeomName  [256];

	bool	m_bDefaultObject;
	bool	m_bOpenEdgesTested;

  TArray<int> m_lstShaderTemplates;
  TArray <SShaderParam> m_ShaderParams;

  uint m_arrSpriteTexID[FAR_TEX_COUNT];

  float m_fObjectRadius;

  void CalcRadiuses();

public:

  // Loader
  bool Load(const char * szFileName, const char * szGeomName, enum EVertsSharing eVertsSharing, 
    bool bLoadAdditinalInfo, bool bKeepInLocalSpace, bool bUseStreaming = false, 
		bool bMakePhysics = true);

  //! Returns script material name
  virtual const char * GetScriptMaterialName(int Id=-1);

  virtual void Render(const SRendParams & rParams, const Vec3& t, int nLodLevel=0);

	//virtual void RenderModel(const RenderParams *pParams);

	// renders the shadow volumes of this whole object (together with attachments if any)
	// the implementation may or may not implement the limit lod functionality: if it does,
	// it will render the specified or lower LOD
	virtual void RenderShadowVolumes(const SRendParams *pParams,int nLimitLod = -1);

  //! Refresh object ( reload shaders or/and object geometry )
  virtual void Refresh(int nFlags);

  virtual bool SetShaderTemplate(int nTemplate, const char *TemplName, const char *ShaderName, bool bOnlyRegister=false, int * pnNewTemplateId = NULL);
  virtual void SetShaderFloat(const char *Name, float Val);
  //virtual void SetRefractFactor(float fRefr) { m_fRefractFactor = fRefr; }
  //! Sets color parameter
  virtual void SetColor(const char *Name, float fR, float fG, float fB, float fA);

	//! set shadow volume
	ItShadowVolume *GetShadowVolume() { return (m_pSvObj);}

	//! get shadow volume
	void	SetShadowVolume(ItShadowVolume *pSvObj) { m_pSvObj=pSvObj;}

protected:
  void Physicalize();

  void CreateModelFarImages(int nTexRes);

  CLeafBuffer * m_pLeafBuffer;
	ItShadowVolume *m_pSvObj;
	int	m_dwFlags,m_dwFlags2;

public:

  CLeafBuffer * GetLeafBuffer() { return m_pLeafBuffer; };
	void SetLeafBuffer( CLeafBuffer *buf ) { m_pLeafBuffer = buf; };

  Vec3d m_vBoxMin, m_vBoxMax, m_vBoxCenter;
	bool m_bPhysicsExistInCompiledFile;

  Vec3d m_vPhysBoxMin[4], m_vPhysBoxMax[4];
  list2<Vec3d> m_lstProxyVerts[4];
  list2<int>   m_lstProxyInds[4];
  list2<unsigned char> m_lstProxyFaceMaterials[4];

#define MAX_PHYS_GEOMS_IN_CGF 4
	phys_geometry * m_arrPhysGeomInfo[MAX_PHYS_GEOMS_IN_CGF];
  phys_geometry * GetPhysGeom(int n = 0) { return m_arrPhysGeomInfo[n]; }

	const char *GetFolderName() { return (m_szFolderName); }
	const char *GetFileName()		{ return (m_szFileName); }
	const char *GetGeoName()		{ return (m_szGeomName); }
	bool IsSameObject(const char * szFileName, const char * szGeomName);

	//set object's min/max bbox 
	void	SetBBoxMin(const Vec3d &vBBoxMin) { m_vBoxMin=vBBoxMin; }	
	void	SetBBoxMax(const Vec3d &vBBoxMax) { m_vBoxMax=vBBoxMax; }
  Vec3d GetBoxMin() { return m_vBoxMin; }
  Vec3d GetBoxMax() { return m_vBoxMax; }
	void GetBBox(Vec3d& Mins, Vec3d& Maxs)
	{
		Mins = m_vBoxMin;
		Maxs = m_vBoxMax;
	}

  int m_nUsers; // reference counter

  ShadowMapLightSource * m_pSMLSource;

  void MakeShadowMaps(const Vec3d vSunPos);
  
protected:
  void MakeLeafBuffer(bool bSortAndShareVerts);
  
  void PrepareShadowMaps(const Vec3d & obj_pos, ShadowMapLightSource * pLSource);

public:
  int GetAllocatedBytes();

  void SetCurDynMask(int nCurDynMask);

  int FindInPosBuffer(const Vec3d & opt, Vec3d * _vbuff, int _vcount, list2<int> * pHash);
  void CompactPosBuffer(Vec3d * _vbuff, int * _vcount, list2<int> * pindices);

  virtual Vec3d GetHelperPos(const char * szHelperName);
  virtual const char  *GetHelperById(int nId, Vec3d & vPos, Matrix44 * pMat, int * pnType);
	virtual const Matrix44 * GetHelperMatrixByName(const char * szHelperName);

  virtual void UpdateCustomLightingSpritesAndShadowMaps(Vec3d vStatObjAmbientColor, int nTexRes, float fBackSideLevel, bool bCalcLighting);

  float m_fRadiusHors;
  float m_fRadiusVert;
  
  float & GetRadiusVert() { return m_fRadiusVert; }
  float & GetRadiusHors() { return m_fRadiusHors; }

  virtual void RegisterUser();
  virtual void UnregisterUser();

	virtual bool IsDefaultObject() { return (m_bDefaultObject); }
  virtual bool MakeObjectPicture(unsigned char * pRGBAData, int nWidth);

#define MAX_STATOBJ_LODS_NUM 3
  CStatObj * m_arrpLowLODs[MAX_STATOBJ_LODS_NUM];
  void LoadLowLODs(EVertsSharing eVertsSharing, bool bLoadAdditinalInfo, bool bKeepInLocalSpace, bool bLoadLater);
  int m_nLoadedLodsNum;

  bool IsSpritesCreated() { return m_arrSpriteTexID[0]>0; }
  float GetDistFromPoint(const Vec3d & vPoint);
  int GetLoadedTrisCount() { return m_nLoadedTrisCount; }
	int GetRenderTrisCount();

  list2<Vec3d> m_lstOcclVolVerts;
  list2<int>   m_lstOcclVolInds;

  virtual bool GetOcclusionVolume(list2<Vec3d> * & plstOcclVolVerts, list2<int> * & plstOcclVolInds) 
  {
    plstOcclVolVerts = &m_lstOcclVolVerts;
    plstOcclVolInds  = &m_lstOcclVolInds;

    return m_lstOcclVolInds.Count() >= 3;
  }
  list2<struct StatHelperInfo> m_lstHelpers;	
	list2<CDLight> m_lstLSources;
	bool Serialize(int & nPos, uchar * pSerBuf, bool bSave, char * szFolderName);

	virtual void FreeTriData();
	virtual const CDLight * GetLightSources(int nId);
	void RenderDebugInfo(const SRendParams & rParams, const class CCObject * pObj);
	void DrawMatrix(const Matrix44 & pMat);

	//! Release method.
	void Release() { delete this; }
	void GetMemoryUsage(class ICrySizer* pSizer);
	int GetMemoryUsage();
	void SpawnParticles( ParticleParams & SpawnParticleParams, const Matrix44 & matWorldSpace, bool bOnlyUpLookingFaces );

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
	void ShutDown();
	void Init();
  void PhysicalizeCompiled();
  bool CompileInNeeded();

	// loading state
	EVertsSharing m_eVertsSharing;
	bool m_bLoadAdditinalInfo;
	bool m_bKeepInLocalSpace;
	int  m_nLastRendFrameId;
	int  m_nMarkedForStreamingFrameId;
	bool m_bUseStreaming;
  bool m_bMakePhysics;
	static float m_fStreamingTimePerFrame;
	int	 m_nSpriteTexRes;
  float m_fBackSideLevel;
  bool m_bCalcLighting;
  static ItShadowVolume * MakeConnectivityInfo(CIndexedMesh * pMesh, const Vec3d & vOrigin, CStatObj * pStatObj);
  ItShadowVolume * MakeConnectivityInfoFromCompiledData(void * pStream, int & nPos, CStatObj * pStatObj);

  bool CheckValidVegetation();
  bool EnableLightamapSupport();
  void CheckLoaded();
	IStatObj * GetLodObject(int nLodLevel);
	void StreamOnProgress (IReadStream* pStream);
	void StreamOnComplete (IReadStream* pStream, unsigned nError);
	
	void StreamCCGF(bool bFinishNow);
	enum ECCGFStreamingStatus
	{
		ecss_NotLoaded,
		ecss_LoadingInProgress,
		ecss_Ready,
		ecss_LoadingError,
		ecss_GeomNotFound,
	} m_eCCGFStreamingStatus;

	IReadStreamPtr m_pReadStream;

	bool LoadUncompiled(const char * szFileName, 
		const char * szGeomName, 
		EVertsSharing eVertsSharing,
		bool bLoadAdditinalInfo,
		bool bKeepInLocalSpace,
		bool bLoadLater,
		bool bMakePhysics);
	void MakeCompiledFileName(char * szCompiledFileName, int nMaxLen);
	int m_bCompilingNotAllowed;
	void ProcessStreamOnCompleteError();

	bool IsPhysicsExist();
	void PreloadResources(float fDist, float fTime, int dwFlags);
	void InitCompiledLightSource(CDLight * pDLight);
	void SetupBending(CCObject * pObj, float fBending);
	bool IsSphereOverlap(const Sphere& sSphere);
};

#endif // STAT_OBJ_H
