#ifndef _BASICAREA_H_
#define _BASICAREA_H_

#define DYNAMIC_ENTITIES 0
#define STATIC_ENTITIES 1

enum ESStatus
{
  eSStatus_Unloaded,
  eSStatus_Ready
};

struct IEntityRenderInfo
{
	IEntityRenderInfo(IEntityRender*pEntityRender)
	{
		m_fWSMaxViewDist	= pEntityRender->m_fWSMaxViewDist;
		m_fWSMaxViewDistSQ = pEntityRender->m_fWSMaxViewDist*pEntityRender->m_fWSMaxViewDist;
		m_vWSCenter				=(pEntityRender->m_vWSBoxMin+pEntityRender->m_vWSBoxMax)*0.5f;
		m_fWSRadius				= pEntityRender->m_fWSRadius;
		m_pEntityRender		= pEntityRender;
		m_fEntDistance = 0;
	}

	float m_fWSMaxViewDistSQ;
	float m_fWSMaxViewDist;
	Vec3 m_vWSCenter;
	float m_fEntDistance;
	float m_fWSRadius;
	struct IEntityRender*m_pEntityRender;
};

struct CBasicArea : public Cry3DEngineBase
{
  CBasicArea() { m_nLastUsedFrameId=0; m_eSStatus=eSStatus_Unloaded; m_vBoxMin=m_vBoxMax=m_vAreaBrushFocusPos=Vec3d(0,0,0); m_StaticEntitiesSorted=false; }
	~CBasicArea();

  list2<struct IEntityRender*> m_lstEntities[2];
	list2<IEntityRenderInfo> m_lstStatEntInfoVegetNoCastersNoVolFog, m_lstStatEntInfoOthers;
	list2<struct IEntityRender*> m_lstStaticShadowMapCasters;
  Vec3d m_vBoxMin, m_vBoxMax;
  int m_nLastUsedFrameId;
  ESStatus m_eSStatus;
	bool m_StaticEntitiesSorted;

  void SerializeArea(bool bSave);
  void DrawEntities(int nFogVolumeID, int nDLightMask,
    bool bLMapGeneration, const CCamera & EntViewCamera, Vec3d * pvAmbColor, Vec3d * pvDynAmbColor,
    VolumeInfo * pFogVolume, bool bNotAllInFrustum, float fSectorMinDist,
    CObjManager * pObjManager, bool bAllowBrushMerging, char*fake, uint nStatics);
  bool CheckUnload();
  void CheckPhysicalized();
  void Unload(bool bUnloadOnlyVegetations = false, const Vec3d & vPos = Vec3d(0,0,0));
	void PreloadResources(Vec3d vPrevPortalPos, float fPrevPortalDistance);
	void UnregisterDynamicEntities();
	void SortStaticInstancesBySize(VolumeInfo * pFogVolume = NULL);
	void MakeAreaBrush();
	void UnmakeAreaBrush();
	void FreeAreaBrush(class CBrush * pAreaBrush);
	int GetLastStaticElementIdWithInMaxViewDist(float fMaxViewDist);

	list2<class CBrush *> m_lstAreaBrush;
	Vec3d m_vAreaBrushFocusPos;
};

#endif // _BASICAREA_H_