////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   3dengine.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef C3DENGINE_H
#define C3DENGINE_H

#if _MSC_VER > 1000
# pragma once
#endif

class CTerrain;
class CObjManager;
class CRESky;
class CPartManager;
class CDecalManager;
class CBFManager;
class CryCharManager;
struct ICryCharInstance;
class CRainManager;
struct CVisAreaManager;

struct IMatInfo;

// this class is used for additional view port rendering (view from camera of another player)
struct SRenderCamera
{
  CCamera m_Camera;
  float m_fX;
  float m_fY;
  float m_fWidth;
  float m_fHeight;
};

struct SNodeInfo;
struct DLightAmount{ CDLight * pDLight; float fAmount; };

class CMatMan : public Cry3DEngineBase
{
public:
	CMatMan();
  ~CMatMan();

	IMatInfo* CreateMatInfo( const char *sMtlName = NULL );
	void DeleteMatInfo(IMatInfo * pMatInfo);
	void RenameMatInfo( IMatInfo *pMtl,const char *sNewName );
	IMatInfo* FindMatInfo( const char *sMtlName ) const;

	void LoadMaterialsLibrary( const char *sMtlFile,XmlNodeRef &levelDataRoot );

private:
	IMatInfo* LoadMaterial( XmlNodeRef mtlNode,const char *sLibraryName,IMatInfo* pParent );
	bool LoadMaterialShader( IMatInfo *pMtl,const char *sShader,int mtlFlags,unsigned int nShaderGenMask,SInputShaderResources &sr,SLightMaterial &lm,XmlNodeRef &publicsNode );
	void ParsePublicParams( TArray<SShaderParam> &params,XmlNodeRef paramsNode );

	typedef std::set<_smart_ptr<IMatInfo> > MtlSet;
	typedef std::map<string,IMatInfo*,stl::less_stricmp<string> > MtlNameMap;
	MtlSet m_mtlSet;
	MtlNameMap m_mtlNameMap;
};

struct CLightEntity : public IEntityRender, public Cry3DEngineBase
{
  virtual const char * GetEntityClassName(void) const { return "LightEntityClass"; }
  virtual float GetScale(void) const { return 1.f; }
  virtual const char *GetName(void) const { return "LightEntityName"; }
  virtual const Vec3d &GetPos(bool) const;
  virtual const Vec3d &GetAngles(int) const;
  virtual void GetRenderBBox(Vec3d &,Vec3d &);
  virtual float GetRenderRadius(void) const;
  virtual bool DrawEntity(const SRendParams &) { return true; };
  virtual bool IsStatic(void) const { return false; };
  virtual bool IsEntityHasSomethingToRender(void) { return false; }
  virtual bool IsEntityAreasVisible(void) { return true; }
  virtual IPhysicalEntity *GetPhysics(void) const { return 0; }
  virtual void SetPhysics(IPhysicalEntity *) { }
  virtual void SetMaterial(IMatInfo *) { }
  virtual IMatInfo *GetMaterial(void) const { return 0; }
	virtual class CDLight	* GetLight() { return m_pLight; }
	virtual float GetMaxViewDist();
	virtual void PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime){}

  CLightEntity();
  ~CLightEntity();

  CDLight * m_pLight;
  Vec3d m_vPos;
};

struct SLevelInfo
{
  float 
    m_fSkyBoxAngle,
		m_fSkyBoxStretching,
		m_fSunHeightScale,
		m_fWaterTranspRatio, 
    m_fWaterReflectRatio, 
    m_fWaterBumpAmountX, 
    m_fWaterBumpAmountY,
    m_fWaterBorderTranspRatio,
    m_fUnderWaterFogDistance;
  float 
    m_fFogNearDist, 
    m_fFogFarDist, 
    m_fMaxViewDist;
  float 
    m_fDefFogNearDist, 
    m_fDefFogFarDist, 
    m_fDefMaxViewDist;
  float m_fWorldColorRatio;

  Vec3d m_vUnderWaterFogColor;
  Vec3d m_vFogColor; 
  Vec3d m_vDefFogColor; 
  Vec3d m_vSunPosition;
  Vec3d m_vWorldColorConst;
  Vec3d m_vWindForce;
  IShader *m_pSHFullScreenQuad;
	int m_nWaterBottomTexId;
	int m_nShadowSpotTexId;
	bool m_bOceanCaustics;
};

struct SLevelShaders
{
  IShader 
    * m_pTerrainWaterShader, 
    * m_pSunRoadShader, 
    * m_pSHTerrainParticles;
};

struct SRenderElements
{
  CRESky              * m_pRESky;
  //CREOutSpace         * m_pREOutSpace;
  CREShadowMapGen     * m_pREShadowMapGenerator;
  CREDummy            * m_pREDummy; 
  CRETerrainParticles * m_pRETerrainParticles;
  CRE2DQuad           * m_pRE2DQuad;  
  CLeafBuffer         * m_pFogTopPlane;
  
  // tiago: added
  CREScreenProcess    * m_pREScreenProcess;   

  IShader             * m_pSHDefault;

  IShader             * m_pSHScreenTexMap;
  IShader             * m_pSHScreenProcess;    
  IShader             * m_pSHOutSpace;
  IShader             * m_pSHFarTreeSprites;
  IShader             * m_pSHClearStencil;
  IShader             * m_pSHShadowMapGen;
  IShader             * m_pSHBinocularDistortMask;
  IShader             * m_pSHScreenDistort;
  IShader             * m_pSHSniperDistortMask;
  IShader             * m_pSHRainMap;

  IShader             * m_pSHStencil;
  IShader             * m_pSHStencilState;
  IShader             * m_pSHStencilStateInv;

  IShader             * m_pSHSky;
  IShader             * m_pSHLensFlares;
};


//////////////////////////////////////////////////////////////////////
class C3DEngine : public I3DEngine, 
  public SLevelInfo, 
  public SLevelShaders, 
  public SRenderElements,
  public Cry3DEngineBase
{
  // IProcess Implementation
  void	SetFlags(int flags) { m_nFlags=flags; }
	int		GetFlags(void) { return m_nFlags; }
	int		m_nFlags;

public:

  // I3DEngine interface implementation
	virtual void Enable(bool bEnable) { m_bEnabled = bEnable; };
	virtual bool Init();
  virtual void Update();
	virtual void Draw();
	virtual void ShutDown(bool bEditorMode=false);
  virtual void Release() { delete this; };
	virtual void SetLevelPath( const char * szFolderName );
  virtual bool LoadLevel(const char * szFolderName, const char * szMissionName, bool bEditorMode);
  virtual void SetCamera(const CCamera &cam, bool bToTheScreen); // The game set this camera
  virtual void DisplayInfo(float & fTextPosX, float & fTextPosY, float & fTextStepY);
  virtual void SetupDistanceFog();
  virtual IStatObj * MakeObject(const char * szFileName, const char * szGeomName = 0, 
    EVertsSharing eVertsSharing = evs_ShareAndSortForCache, 
    bool bLoadAdditinalInfo = true, 
    bool bKeepInLocalSpace = false);
  virtual IStatObj* MakeObject();
  virtual bool ReleaseObject(IStatObj * pObject);
  virtual void RegisterEntity( IEntityRender * pEntityRS );
  virtual bool UnRegisterEntity( IEntityRender * pEntityRS );
  virtual float GetWaterLevel(const Vec3d * pvPos = NULL, Vec3d * pvFlowDir = NULL);
  virtual float GetWaterLevel(IEntityRender * pEntityRender, Vec3d * pvFlowDir = NULL);
  virtual void SpawnParticles(const ParticleParams & SpawnParticleParams);
  virtual void CreateDecal( const CryEngineDecalInfo& Decal );
  virtual void DrawTerrainDetailTextureLayers();
  virtual void DrawFarTrees();
  virtual void DrawTerrainParticles(IShader * pShader);
  virtual void SetRenderCallback(void (*pFunc)(void *pParams), void *pParams) { m_pRenderCallbackFunc = pFunc; m_pRenderCallbackParams = pParams; }
  virtual void DrawLowDetail(const int & DrawFlags);
  virtual float GetTerrainElevation(float x, float y);
  virtual float GetTerrainZ(int x, int y);
  virtual int GetHeightMapUnitSize();
  virtual int GetTerrainSize();
  virtual float GetMaxViewDist();
  virtual void ActivateLight(const char *szName,bool bActivate);	
  virtual bool IsCharacterFile(const char * szCGFileName);
  virtual bool IsPointInWater(Vec3d vPos); // todo: remove it
  virtual Vec3d GetSunPosition(bool bMoveUp = true);
  virtual ICryCharInstance * MakeCharacter(const char * cid_file_name, unsigned int dwFlags = 0);
  virtual void RemoveCharacter(ICryCharInstance * pCryCharInstance);  
  virtual Vec3d GetWorldColor( bool bScaled=true );
  virtual void SetWorldColor(Vec3d vColor);
	virtual void SetOutdoorAmbientColor(Vec3d vColor);
  virtual void SetWorldColorRatio(float fWorldColorRatio);
  virtual float GetWorldColorRatio();
  virtual void SetSkyBox(const char * szSkyBoxShaderName);
  virtual void OnExplosion(Vec3d vPos, Vec3 vHitDir, float fRadius, int nTexID, bool bDeformTerrain);
  virtual void SetScreenShader(const char * szShaderName);
  //! For editor  
  virtual bool AddStaticObject(int nObjectID, const Vec3d & vPos, const float fScale, uchar ucBright);
  virtual bool RemoveStaticObject(int nObjectID, const Vec3d & vPos);
  virtual bool PhysicalizeStaticObject(void *pForeignData,int iForeignData,int iForeignFlags);
  virtual void RemoveAllStaticObjects();
  virtual void SetTerrainSurfaceType(int x, int y, int nType);
  virtual int  GetTerrainSurfaceType(int x, int y);
  virtual void SetTerainHightMapBlock(int x1, int y1, int nSizeX, int nSizeY, ushort * TerrainBlock, ushort nUpdateMask);
  virtual int LockTerrainSectorTexture(int nSectorOriginX, int nSectorOriginY, int & nTexDim);
  virtual void SetPhysMaterialEnumerator(IPhysMaterialEnumerator * pPhysMaterialEnumerator);
  virtual IPhysMaterialEnumerator * GetPhysMaterialEnumerator();
  virtual void LoadEnvironmentSettingsFromXML(const char * szMissionName, bool bEditorMode, const char * szMissionXMLString, bool bUpdateLightingOnVegetations);
  virtual void AddDynamicLightSource(const class CDLight & LSource, IEntityRender * pEnt, int nEntityLightId=-1, const Matrix44 * pMatrix=NULL);
  virtual void ApplyForceToEnvironment(Vec3d vPos, float fRadius, float fAmountOfForce);
  virtual bool UnRegisterInAllSectors(IEntityRender * pEntityRS);
  virtual bool MakeSectorLightMap(int nSectorOriginX, int nSectorOriginY, unsigned char * pImage, int nImageSize);
  virtual void SetMaxViewDistance(float fMaxViewDistance);
  virtual float GetMaxViewDistance( );
  virtual void SetFogColor(const Vec3d& vFogColor);
  virtual void SetFogStart(const float fFogStart);
  virtual void SetFogEnd(const float fFogEnd);
  virtual Vec3d GetFogColor( );
  virtual float GetFogStart( );
  virtual float GetFogEnd( );
  virtual float GetDistanceToSectorWithWater();
  virtual void SetSkyBoxAlpha(float fAlpha);
  virtual void SetBFCount(int nCount);
  virtual int  GetBFCount();
  virtual void SetGrasshopperCount(int nCount);
  virtual int  GetGrasshopperCount();
  virtual Vec3d GetOutdoorAmbientColor();
  virtual Vec3d GetSunColor();
  virtual uint GetLightMaskFromPosition(const Vec3d & vPos, float fRadius);
  virtual Vec3d GetAmbientColorFromPosition(const Vec3d & vPos, float fRadius);
  virtual void ClearRenderResources(bool bEditor);
  virtual void FreeEntityRenderState(IEntityRender * pEntityRS);
  virtual IEntityRenderState * MakeEntityRenderState();
  virtual void SetGrasshopperCGF( int nSlot, IStatObj * pStatObj );
  virtual IParticleEmitter* CreateParticleEmitter();
  virtual void DeleteParticleEmitter(IParticleEmitter* pPartEmitter);
  virtual const char * GetLevelFilePath(const char * szFileName);
  virtual void MakeUnderWaterSmoothHMap(int nWaterUnitSize);
  virtual ushort * GetUnderWaterSmoothHMap(int & nDimensions);
  virtual void SetTerrainBurnedOut(int x, int y, bool bBurnedOut);
  virtual bool IsTerrainBurnedOut(int x, int y);
  virtual void UpdateDetailObjects();
  virtual int GetTerrainSectorSize();
  virtual void AddWaterSplash (Vec3d vPos, eSplashType eST, float fForce, int Id);
  virtual class IEdgeConnectivityBuilder *GetNewConnectivityBuilder( void );
  //! creates a connectivity object that can be used to deserialize the connectivity data
  virtual class IStencilShadowConnectivity *NewConnectivity();
  virtual class IEdgeConnectivityBuilder *GetNewStaticConnectivityBuilder( void );
  virtual class IEdgeDetector *GetEdgeDetector( void );
  virtual void	LoadTerrainSurfacesFromXML(void * pDoc);
  virtual void EnableHeatVision(bool bEnable);
  virtual int GetTerrainTextureDim();
  virtual bool SetStatInstGroup(int nGroupId, const IStatInstGroup & siGroup);
  virtual bool GetStatInstGroup(int nGroupId,       IStatInstGroup & siGroup);
  virtual void ActivatePortal(const Vec3d &vPos, bool bActivate, IEntityRender *pEntity);
  virtual void GetMemoryUsage(class ICrySizer * pSizer);
  virtual IWaterVolume * CreateWaterVolume();
  virtual void DeleteWaterVolume(IWaterVolume * pWaterVolume);
	virtual IWaterVolume * FindWaterVolumeByName(const char * szName);
  virtual IVisArea * CreateVisArea();
  virtual void DeleteVisArea(IVisArea * pVisArea);
  virtual void UpdateVisArea(IVisArea * pArea, const Vec3d * pPoints, int nCount, const char * szName, float fHeight, const Vec3d & vAmbientColor, bool bAfectedByOutLights, bool bSkyOnly, const Vec3 & vDynAmbientColor, float fViewDistRatio, bool bDoubleSide, bool bUseDeepness, bool bUseInIndoors);
  virtual int GetFogVolumeIdFromBBox(const Vec3d & vBoxMin, const Vec3d & vBoxMax); // todo: remove
  virtual void ResetParticlesAndDecals( );
  virtual IEntityRender * CreateEntityRender();
  virtual IEntityRender * CreateVegetation();
  virtual void DeleteEntityRender(IEntityRender * pEntityRender);
  virtual void DrawRain();
  virtual void SetRainAmount( float fAmount );
  virtual void SetWindForce( const Vec3d & vWindForce );
  virtual float GetLightAmountForEntity(IEntityRender * pEntity, bool bOnlyVisibleLights);
  virtual float GetAmbientLightAmountForEntity(IEntityRender * pEntity);
  virtual IVisArea * GetVisAreaFromPos(const Vec3d &vPos);	
  virtual bool IsVisAreasConnected(IVisArea * pArea1, IVisArea * pArea2, int nMaxReqursion, bool bSkipDisabledPortals);
  virtual ILMSerializationManager * CreateLMSerializationManager();
  void EnableOceanRendering(bool bOcean, bool bShore); // todo: remove
	//////////////////////////////////////////////////////////////////////////
  // Materials access.
  virtual IMatInfo* CreateMatInfo();
  virtual void DeleteMatInfo(IMatInfo * pMatInfo);
  virtual void RenameMatInfo( IMatInfo *pMtl,const char *sNewName );
  virtual IMatInfo* FindMaterial( const char *sMaterialName );
	//////////////////////////////////////////////////////////////////////////
	// ParticleEffect
	virtual IParticleEffect* CreateParticleEffect();
	virtual void DeleteParticleEffect( IParticleEffect* pEffect );
	virtual IParticleEffect* FindParticleEffect( const char *sEffectName );
	//////////////////////////////////////////////////////////////////////////

  virtual INT_PTR AddStaticLightSource(const class CDLight & LSource, IEntityRender * pCreator, ICryCharInstance * pCryCharInstance, const char * szBoneName);	//AMD Port
  virtual bool DeleteStaticLightSource(INT_PTR nLightId);	//AMD Port
  virtual const list2<CDLight*> * GetStaticLightSources();
  virtual bool IsTerainHightMapModifiedByGame();
  virtual bool IsPotentiallyVisible(IEntityRender * pEntityRender, float fAdditionRadius=0 );
  virtual void UpdateBeaches();
  virtual void RestoreTerrainFromDisk();  
  virtual void CheckMemoryHeap();
	virtual void RecompileBeaches();
	virtual float GetObjectsLODRatio();
	virtual float GetObjectsViewDistRatio();
	virtual bool SetMaterialFloat( char * szMatName, int nSubMatId, int nTexSlot, char * szParamName, float fValue );
	virtual void CloseTerrainTextureFile();
	virtual int GetLoadedObjectCount();
	virtual void DeleteEntityDecals(IEntityRender * pEntity);
	virtual void DeleteDecalsInRange( Vec3d vBoxMin, Vec3d vBoxMax, bool bDeleteBigTerrainDecals = true);
	virtual const void * GetShoreGeometry(int & nPosStride, int & nVertCount, int nSectorX, int nSectorY);
	virtual void OnLevelLoaded();
	virtual void LockCGFResources();
	virtual void UnlockCGFResources();
	virtual float GetObjectsMinViewDist();

  // tiago: added
  virtual void SetBlurMask(ITexPic *pMask);
  virtual void SetScreenMask(ITexPic *pMask);
  virtual void SetScreenFx(const char *pEffectName, int iActive);
  virtual void SetScreenFxParam(const char *pEffectName, const char *pEffectParam, void *pValue);  
  virtual int  GetScreenFx(const char *pEffectName);
  virtual int  GetScreenFxParam(const char *pEffectName, const char *pEffectParam, void *&pValue);    
  virtual void ResetScreenFx(void);

	bool SaveCGF(std::vector<IStatObj *>& pObjs);

	int GetCurrentSpec()
	{
		if (m_pSysSpec)
			return m_pSysSpec->GetIVal();
		return 3; // very high spec.
	}

  int GetCurrentLightSpec()
  {
    if (m_pLightQuality)
      return m_pLightQuality->GetIVal();
    return 3; // very high spec.
  }

public:
  C3DEngine( ISystem * pSystem );
  ~C3DEngine();

  void	RenderScene(unsigned int dwDrawFlags);
  CDLight * CheckDistancesToLightSources(uint & nDLightMask, const Vec3d vObjPos, const float fObjRadius, IEntityRender * pEntityRender = 0, int nMaxLightBitsNum = 8, CDLight ** pSelectedLights = NULL, int nMaxSelectedLights = 0, Vec3d * pvSummLightAmmount = NULL);
  uint GetFullLightMask();
  bool IsOutdoorVisible();
  void RenderSkyBox(IShader *pSH);
  void RenderOutSpace();

  // access to components
  CTerrain * GetTerrain() { return m_pTerrain; }
  CObjManager * GetObjManager() { return m_pObjManager; }
  CVars * GetCVars() { return m_pCVars; }
  CVisAreaManager * GetVisAreaManager() { return m_pVisAreaManager; }
	CMatMan * GetMatMan() { return m_pMatMan; }

private:

  // misc
  CCObject *m_SunObject[8],
           *m_pBlurObj,
           *m_pScreenObj;

  int m_nStreamingIconTexID, m_nBlackTexID;
  char m_sGetLevelFilePathTmpBuff[MAX_PATH_LENGTH];
  char m_szLevelFolder[_MAX_PATH];
  list2<DLightAmount> m_lstDL_CDTLS;
  static double m_dLoadLevelTime;
  bool m_bOcean; // todo: remove
  bool m_bShore; // todo: remove
  int m_bTerrainLightMapGenError;
	bool m_bEnabled;

  // interfaces
  IPhysMaterialEnumerator * m_pPhysMaterialEnumerator;
  // this is the shared instance that will calculate the shadows
  // created/freed during class initialization/deinitialization
  class IEdgeDetector * m_pShadowEdgeDetector;										//!< to get rid of the new/delete
  class IEdgeConnectivityBuilder * m_pConnectivityBuilder;				//!< to get rid of the new/delete
  class IEdgeConnectivityBuilder * m_pStaticConnectivityBuilder;

  // data containers
  list2<CDLight> m_lstDynLights;
	list2<CDLight> m_lstDynLightsNoLight;
	int m_nRealLightsNum;
  list2<CLightEntity*> m_lstStaticLights;
#define MAX_LIGHTS_NUM 32
  CCamera m_arrCameraProjectors[MAX_LIGHTS_NUM];

  // 3dengine components
  CTerrain       * m_pTerrain;
  CObjManager    * m_pObjManager;
  CPartManager   * m_pPartManager;
  CDecalManager  * m_pDecalManager;
//  CBFManager     * m_pBFManager;
  CRainManager   * m_pRainManager;
  CVisAreaManager* m_pVisAreaManager;
  CVars          * m_pCVars;
	CMatMan        * m_pMatMan;

	ICVar*					m_pSysSpec;
  ICVar*					m_pLightQuality;

// not sorted

  void (*m_pRenderCallbackFunc)(void *pParams);
  void *m_pRenderCallbackParams;
   
  //! Saving of cgf file
  bool WriteMaterials(TArray<CHUNK_HEADER>& Chunks, TArray<IShader *>& Shaders, FILE *out, int &MatChunk);
  bool WriteNodes(TArray<CHUNK_HEADER>& Chunks, TArray<NODE_CHUNK_DESC>& Nodes, FILE *out, TArray<SNodeInfo>& NI, int& MatChunk, int& ExpFrame, std::vector<IStatObj *>& pObjs);
  bool WriteMesh(TArray<CHUNK_HEADER>& Chunks, TArray<NODE_CHUNK_DESC>& Nodes, TArray<IShader *>& Shaders, FILE *out, TArray<SNodeInfo>& NI, int& MatChunk, int& ExpFrame);
  bool WriteNodeMesh(int nNode, MESH_CHUNK_DESC *chunk, FILE *out, TArray<IShader *>& Shaders, TArray<SNodeInfo>& NI, struct CStatObj *pObj);
  bool WriteLights(TArray<CHUNK_HEADER>& Chunks, TArray<NODE_CHUNK_DESC>& Nodes, FILE *out, std::vector<IStatObj *>& pObjs);

  void DrawFullScreenQuad(IShader *pShader, bool bRectangle, EF_Sort eSort = eS_Unknown);

  void LoadMissionSettingsFromXML(struct XDOM::IXMLDOMNode *pInputNode, bool bEditorMode, bool bUpdateLightingOnVegetations);
  char * GetXMLAttribText(XDOM::IXMLDOMNode * pInputNode, const char * szLevel0,const char * szLevel1,const char * szLevel2,const char * szDefaultValue);

  void RegisterLightSourceInSectors(CDLight * pDynLight);
  void LightSourcesDebug();
  void RenderVolumeFogTopPlane();
  Vec3d GetTerrainSurfaceNormal(Vec3d vPos) { return m_pTerrain ? m_pTerrain->GetTerrainSurfaceNormal(vPos) : Vec3d(0,0,0); }

  void RenderTerrainParticles();
  bool IsBoxVisibleOnTheScreen(const Vec3d & vBoxMin, const Vec3d & vBoxMax, OcclusionTestClient * pOcclusionTestClient );
  bool IsSphereVisibleOnTheScreen(const Vec3d & vPos, const float fRadius, OcclusionTestClient * pOcclusionTestClient );

  bool IsCameraAnd3DEngineInvalid(const CCamera cam, const char * szCaller);

	void	LoadFogVolumesFromXML(XDOM::IXMLDOMDocumentPtr pDoc);
	
	void UpdateScene(bool bAddStaticLights = true, bool bAlwaysAddSun = false);

	void MakeHiResScreenShot();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void SetupLightScissors(CDLight *pLight);

	bool IsOutdoorsVisible();

	void UpdateStaticLightSources();
	void DeleteAllStaticLightSources();
  bool LoadStaticLightSources(const char *pszFileName);
	void LoadParticleEffects( XmlNodeRef &levelDataRoot,bool bEditorMode );

public:

	void UpdateLightSources();
	void PrepareLightSourcesForRendering();

  void FreeLightSourceComponents(CDLight *pLight);
	void RemoveEntityLightSources(IEntityRender * pEntity);

  void CheckPhysicalized(const Vec3d & vBoxMin, const Vec3d & vBoxMax);

  list2<CDLight> * GetDynamicLightSources() { return &m_lstDynLights; }
  void ProcessScreenEffects();
	int GetRealLightsNum() { return m_nRealLightsNum; }
	void CaptureFrameBufferToFile();
	void DrawShadowSpotOnTerrain(Vec3d vPos, float fRadius);
	void SetupClearColor();
	void DrawText(float x, float y, const char * format, ...);
};
  
#endif // C3DENGINE_H
