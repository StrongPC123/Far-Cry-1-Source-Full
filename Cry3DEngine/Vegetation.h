//! Info about instance of static object (trees, rocks),
//! Every sector contain list of such structures
class CStatObjInst : public IEntityRender, public Cry3DEngineBase
{
public:
  Vec3d m_vPos;
  float m_fScale;
  float m_fCurrentBending;
  float m_fFinalBending;
  IPhysicalEntity * m_pPhysEnt;
	uchar m_nObjectTypeID;
	uchar m_ucBright;
	uchar m_ucLodAngle;
	uchar m_ucAngleSlotId;

  static CObjManager * m_pObjManager;
  static Vec3d m_vAngles;

  CStatObjInst()
  {
    m_nObjectTypeID=0;
    m_ucBright=0;
    m_ucLodAngle=0;
    m_ucAngleSlotId=0;
    m_vPos.Set(0,0,0);
    m_fScale=0;
    m_fCurrentBending=0;
    m_fFinalBending=0;
    m_pPhysEnt=0;
  }

  virtual ~CStatObjInst();

  void Init( const Vec3d vPos, const int nObjectTypeID, const uchar ucBright, const float fScale )
  {
    assert(nObjectTypeID<256);
    m_vPos = vPos;
    m_nObjectTypeID = nObjectTypeID;
    m_ucBright = ucBright;
    m_fScale = fScale;
    m_fCurrentBending=0;
    m_fFinalBending=0;
    m_pPhysEnt = 0;
    m_ucLodAngle = 127;
    m_ucAngleSlotId = 255;
  }

  void SetStatObjGroupId(int nStatObjInstanceGroupId) { m_nObjectTypeID = nStatObjInstanceGroupId; }

  const char *GetEntityClassName(void) const { return "StatObjInst"; }
  const Vec3d &GetPos(bool) const { return m_vPos; }
  const Vec3d &GetAngles(int) const { return m_vAngles; }
  float GetScale(void) const { return m_fScale; }
  const char *GetName(void) const;
  void GetRenderBBox(Vec3d &,Vec3d &);
  float GetRenderRadius(void) const;
  bool DrawEntity(const SRendParams & rendParams);
  bool IsStatic(void) const { return true; }
  bool IsEntityHasSomethingToRender(void) { return true; }
  bool IsEntityAreasVisible(void) { return true; }
  IPhysicalEntity *GetPhysics(void) const { return m_pPhysEnt; }
  void SetPhysics(IPhysicalEntity * pPhysEnt) { m_pPhysEnt = pPhysEnt; }
  void SetMaterial(IMatInfo *) {}
  IMatInfo *GetMaterial(void) const { return 0; }

  //  bool DrawEntity(const struct SRendParams & _EntDrawParams);//bool bNotAllIN, const CCamera & cam, int nDynLightMaskNoSun, struct VolumeInfo * pFogVolume);
  float GetBendingRandomFactor();
  float Interpolate(float& pprev, float& prev, float& next, float& nnext, float ppweight, float pweight, float nweight, float nnweight);
  void Physicalize( bool bInstant = false );

  virtual float GetMaxViewDist();
  IStatObj * GetEntityStatObj( unsigned int nSlot, Matrix44 * pMatrix = NULL, bool bReturnOnlyVisible = false);
	CStatObj *GetStatObj() const;

  virtual void Serialize(bool bSave, ICryPak * pPak, FILE * f);
  virtual EERType GetEntityRenderType() { return eERType_Vegetation; }
  void Dephysicalize( );
  void Dematerialize( );
  virtual int GetMemoryUsage();

	virtual unsigned int GetRndFlags(); // get flags from StatObjGroup
	virtual void SetRndFlags(unsigned int dwFlags); // there is no flags in the instance
	virtual void SetRndFlags(unsigned int dwFlags, bool bEnable); // there is no flags in the instance
	virtual struct ShadowMapLightSource * GetShadowMapFrustumContainer();
	virtual list2<struct ShadowMapLightSourceInstance> * GetShadowMapCasters();
	virtual void PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime);
	virtual float GetViewDistRatioNormilized();
};
