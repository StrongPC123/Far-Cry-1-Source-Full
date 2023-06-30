#ifndef IENTITYRENDERSTATE_H
#define IENTITYRENDERSTATE_H

template	<class T> class list2;

// !!! don't change the type !!!
typedef unsigned short EntityId;					//! unique identifier for each entity instance

struct IMatInfo;
struct IVisArea;

enum EERType
{
  eERType_Unknown,
  eERType_Brush,
  eERType_Vegetation
};

struct OcclusionTestClient
{
  OcclusionTestClient() { memset(this,0,sizeof(OcclusionTestClient)); bLastResult = true; }
  unsigned char ucOcclusionByTerrainFrames;
  unsigned char ucOcclusionByObjectsFrames;
  bool  bLastResult;
  int   nLastVisibleFrameID;
  //	class CREOcclusionQuery * arrREOcclusionQuery[2];
};

//! rendering properties/state of entity
//! 3dengine/indoor/ should not have access to the game specific actual IEntity 
//! 3dengine only needs access to some functions
//! this is begin of big code cleaning
//! structure and names of classes not finilized

struct IEntityRenderState
{
	IEntityRenderState()
	{	// init vars
    pShadowMapInfo = 0;
//    nScissorX1=nScissorY1=nScissorX2=nScissorY2=0;
	}

	virtual ~IEntityRenderState()
	{
		delete pShadowMapInfo;
		pShadowMapInfo=0;
	}

	// used for shadow maps
  struct ShadowMapInfo{
    ShadowMapInfo() { memset(this,0,sizeof(ShadowMapInfo)); }
		void Release(enum EERType eEntType, struct IRenderer * pRenderer);
    list2<struct ShadowMapLightSourceInstance> * pShadowMapCasters;
    struct ShadowMapLightSource * pShadowMapFrustumContainer;
		struct ShadowMapLightSource * pShadowMapFrustumContainerPassiveCasters;
    list2<struct CLeafBuffer *> * pShadowMapLeafBuffersList;
    Vec3 vPrevTerShadowPos; 
    float fPrevTerShadowRadius;
  } * pShadowMapInfo;

	// tmp flaot (distance to the light source, used for sorting)
	float fTmpDistance;

//  unsigned short nScissorX1, nScissorY1, nScissorX2, nScissorY2;

	unsigned int nStrongestLightId;
};

//! EntityRender flags
#define ERF_SELFSHADOW									0x1
#define ERF_CASTSHADOWVOLUME						0x2
#define ERF_RECVSHADOWMAPS							0x4
#define ERF_CASTSHADOWMAPS							0x8
#define ERF_DONOTCHECKVIS         			0x10
#define ERF_CASTSHADOWINTOLIGHTMAP			0x20
#define ERF_HIDABLE											0x40
#define ERF_HIDDEN											0x80
#define ERF_SELECTED										0x100
#define ERF_USELIGHTMAPS								0x200
#define ERF_OUTDOORONLY									0x400
#define ERF_UPDATE_IF_PV								0x800
#define ERF_EXCLUDE_FROM_TRIANGULATION	0x1000
#define ERF_MERGED											0x2000
#define ERF_RECVSHADOWMAPS_ACTIVE				0x4000
#define ERF_PHYS_NONCOLL								0x8000
#define ERF_MERGED_NEW  								0x10000
#define ERF_FIRST_PERSON_CAMERA_OWNER		0x20000

// Should be the same as FOB_ flags
#define ERF_NOTRANS_ROTATE  					  0x10000000
#define ERF_NOTRANS_SCALE 						  0x20000000
#define ERF_NOTRANS_TRANSLATE					  0x40000000
#define ERF_NOTRANS_MASK (ERF_NOTRANS_ROTATE | ERF_NOTRANS_SCALE | ERF_NOTRANS_TRANSLATE)

struct IEntityRender
{
  IEntityRender()
  {
    m_dwRndFlags = 0;
    m_pEntityRenderState = 0;
    m_narrDrawFrames[0] = m_narrDrawFrames[1] = 0;
    m_narrShadowFrames[0] = m_narrShadowFrames[1] = 0;
    ucViewDistRatio=100;
    ucLodRatio=100;
    m_pSector = 0; 
    m_pVisArea=0;
		m_vWSBoxMin=m_vWSBoxMax=Vec3(0,0,0);
		m_fWSMaxViewDist=m_fWSRadius=0;
		m_arrfDistance[0] = m_arrfDistance[1] = 0;
		m_nFogVolumeID = 0;
		m_bForceBBox = 0;
  }

	virtual const char * GetEntityClassName() const = 0;
	virtual const Vec3 & GetPos(bool bWorldOnly = true) const = 0;
	virtual const Vec3 & GetAngles(int realA=0) const = 0;
	virtual float GetScale() const = 0;
	virtual const char *GetName() const = 0;
	virtual void	GetRenderBBox( Vec3 &mins,Vec3 &maxs ) = 0;
	virtual void	GetBBox( Vec3 &mins,Vec3 &maxs ) { GetRenderBBox( mins, maxs ); }
	virtual float GetRenderRadius() const = 0;
  virtual bool HasChanged() { return false; }
	virtual bool DrawEntity(const struct SRendParams & EntDrawParams) = 0;
	virtual bool IsStatic() const = 0;
  virtual struct IStatObj * GetEntityStatObj( unsigned int nSlot, Matrix44 * pMatrix = NULL, bool bReturnOnlyVisible = false) { return 0; }
  virtual void SetEntityStatObj( unsigned int nSlot, IStatObj * pStatObj, Matrix44 * pMatrix = NULL ) {};
	virtual struct ICryCharInstance* GetEntityCharacter( unsigned int nSlot, Matrix44 * pMatrix = NULL ) { return 0; }
	virtual void Physicalize(bool bInstant=false) {} 
	virtual class CDLight	* GetLight() { return 0; }
	virtual struct IEntityContainer* GetContainer() const { return 0; }
 
	float m_fWSMaxViewDist;

  // rendering flags
  virtual void SetRndFlags(unsigned int dwFlags) { m_dwRndFlags = dwFlags; }
  virtual void SetRndFlags(unsigned int dwFlags, bool bEnable)
  { if(bEnable) m_dwRndFlags |= dwFlags; else m_dwRndFlags &= ~dwFlags; }
  virtual unsigned int GetRndFlags() { return m_dwRndFlags; }
  int m_dwRndFlags; 

  // object draw frames (set if was drawn)
  void SetDrawFrame( int nFrameID, int nRecursionLevel ) { m_narrDrawFrames[nRecursionLevel] = nFrameID; }
  int GetDrawFrame( int nRecursionLevel = 0 ) const{ return m_narrDrawFrames[nRecursionLevel]; }
  int m_narrDrawFrames[2];	

  // shadow draw frames (set if was drawn)
  void SetShadowFrame( unsigned short nFrameID, int nRecursionLevel ) { m_narrShadowFrames[nRecursionLevel] = nFrameID; }
  unsigned short GetShadowFrame( int nRecursionLevel = 0 ) const{ return m_narrShadowFrames[nRecursionLevel]; }

  // current distance to the camera (with reqursioin)
  float m_arrfDistance[2];

  //! contains rendering properties, not 0 only if entity going to be rendered
  struct IEntityRenderState * m_pEntityRenderState;

  unsigned short m_narrShadowFrames[2];	

  Vec3 m_vWSBoxMin, m_vWSBoxMax;
  float m_fWSRadius;
//##  float m_fMaxViewDist;
	unsigned char m_bForceBBox;
  unsigned char ucViewDistRatio;
  unsigned char ucLodRatio;
	unsigned char m_nFogVolumeID;

  // cur areas info
  struct CSectorInfo* m_pSector; 
  struct CVisArea * m_pVisArea;

	// Used for occlusion culling
	OcclusionTestClient OcclState;

  //! Access to the EntityRenderState for 3dengine
  IEntityRenderState * & GetEntityRS() { return m_pEntityRenderState; }

	//## Lightmaps (here dot3lightmaps only)

	// Summary: 
	//   Assigns a texture set reference for dot3 lightmapping. The object will Release() it at the end of its lifetime
  virtual void SetLightmap(struct RenderLMData * pLMData, float *pTexCoords, UINT iNumTexCoords, int nLod=0) {};

	// Summary: 
	//   Assigns a texture set reference for dot3 lightmapping. The object will Release() it at the end of its lifetime, special call from lightmap serializer/compiler to set occlusion map values
	virtual void SetLightmap(RenderLMData *pLMData, float *pTexCoords, UINT iNumTexCoords, const unsigned char cucOcclIDCount, const std::vector<std::pair<EntityId, EntityId> >& aIDs){};

	//  Returns:
	//    true if there are lightmap texture coodinates and a lightmap texture assignment
  virtual bool HasLightmap(int nLod) { return false; };
	//  Returns:
	//    Lightmap texture set for this object, or NULL if there's none assigned. Don't release obtained copy, it's not a reference
	//  See Also: 
	//    SetLightmap
  virtual RenderLMData * GetLightmap(int nLod) { return 0; };
	// Summary:
	//   Returns vertex buffer holding instance specific texture coordinate set for dot3 lightmaps
  virtual struct CLeafBuffer * GetLightmapTexCoord(int nLod) { return 0; };


	virtual bool IsEntityHasSomethingToRender() = 0;

	virtual bool IsEntityAreasVisible() = 0;

  // Returns:
	//   Current VisArea or null if in outdoors or entity was not registered in 3dengine
  IVisArea * GetEntityVisArea() { return (IVisArea*)m_pVisArea; }

  /* Allows to adjust defailt max view distance settings, 
    if fMaxViewDistRatio is 100 - default max view distance is used */
  void SetViewDistRatio(int nViewDistRatio) { ucViewDistRatio = min(254,max(0,nViewDistRatio)); }

	/*! Makes object visible at any distance */
	void SetViewDistUnlimited() { ucViewDistRatio = 255; }

	// Summary:
	//   Retrieves the view distance settings
	int GetViewDistRatio() { return (ucViewDistRatio==255) ? 1000l : ucViewDistRatio; }

  //! return max view distance ratio
  virtual float GetViewDistRatioNormilized() { return 0.01f*GetViewDistRatio(); }

  /*! Allows to adjust defailt lod distance settings, 
  if fLodRatio is 100 - default lod distance is used */
  void SetLodRatio(int nLodRatio) { ucLodRatio = min(255,max(0,nLodRatio)); }

  //! return lod distance ratio
  float GetLodRatioNormilized() { return 0.01f*ucLodRatio; }

	// get/set physical entity
	virtual class IPhysicalEntity* GetPhysics() const = 0;
	virtual void SetPhysics( IPhysicalEntity* pPhys ) = 0;

	virtual ~IEntityRender() {};

	//! Set override material for this instance.
	virtual void SetMaterial( IMatInfo *pMatInfo ) = 0;
	//! Query override material of this instance.
	virtual IMatInfo* GetMaterial() const = 0;
  virtual int GetEditorObjectId() { return 0; }
  virtual void SetEditorObjectId(int nEditorObjectId) {}

  //! Physicalize if it isn't already
  virtual void CheckPhysicalized() {};

  virtual int DestroyPhysicalEntityCallback(IPhysicalEntity *pent) { return 0; }

  virtual void SetStatObjGroupId(int nStatObjInstanceGroupId) { }

  virtual float GetMaxViewDist() { return 0; }

  virtual void Serialize(bool bSave, struct ICryPak * pPak, FILE * f) {}
  virtual EERType GetEntityRenderType() { return eERType_Unknown; }
  virtual void Dephysicalize( ) {}
  virtual void Dematerialize( ) {}
  virtual int GetMemoryUsage() { return 0; }

	virtual list2<struct ShadowMapLightSourceInstance> * GetShadowMapCasters()
	{
		if(m_pEntityRenderState && m_pEntityRenderState->pShadowMapInfo)
			return m_pEntityRenderState->pShadowMapInfo->pShadowMapCasters;
		return 0;
	}

	virtual struct ShadowMapLightSource * GetShadowMapFrustumContainer()
	{
		if(m_pEntityRenderState && m_pEntityRenderState->pShadowMapInfo)
			return m_pEntityRenderState->pShadowMapInfo->pShadowMapFrustumContainer;
		return 0;
	}

	virtual struct ShadowMapLightSource * GetShadowMapFrustumContainerPassiveCasters()
	{
		if(m_pEntityRenderState && m_pEntityRenderState->pShadowMapInfo)
			return m_pEntityRenderState->pShadowMapInfo->pShadowMapFrustumContainerPassiveCasters;
		return 0;
	}

	virtual void PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime) = 0;
	virtual void Precache() {};
};

#endif // IENTITYRENDERSTATE_H
