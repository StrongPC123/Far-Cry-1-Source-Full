////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   visareas.h
//  Version:     v1.00
//  Created:     18/12/2002 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: visibility areas header
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef VisArea_H
#define VisArea_H

#include "basicarea.h"

struct CVisArea : public CBasicArea, public IVisArea
{
	// editor interface
	virtual void Update(const Vec3d * pPoints, int nCount, const char * szName, float fHeight, const Vec3d & vAmbientColor, bool bAfectedByOutLights, bool bSkyOnly, const Vec3d & vDynAmbientColor, float fViewDistRatio, bool bDoubleSide, bool bUseDeepness, bool bUseInIndoors);

	CVisArea(bool bLoadedAsAreaBox);
	~CVisArea();
	bool IsPointInsideVisArea(const Vec3d & vPos);
	bool FindVisArea(IVisArea * pAnotherArea, int nMaxReqursion, bool bSkipDisabledPortals);
	int GetVisFrameId();
	Vec3d GetConnectionNormal(CVisArea * pPortal);
	void DrawVolume(CObjManager * pObjManager, int nReqursionLevel, CCamera CurCamera, CVisArea * pParent, CVisArea * pCurPortal, bool * pbOutdoorVisible, list2<CCamera> * plstOutPortCameras, bool * pbSkyVisible);
	void DrawVolume_NotThisAreaOnlyLights(CObjManager * pObjManager, int nReqursionLevel, CCamera CurCamera, CVisArea * pParent, CVisArea * pCurPortal, bool * pbOutdoorVisible, list2<CCamera> * plstOutPortCameras, bool * pbSkyVisible);
	void UpdatePortalCameraPlanes(CCamera & cam, Vec3d * pVerts, bool bMergeFrustums);
  bool UpdatePortalCameraScissor(CCamera & cam, list2<Vec3d> * lstVerts, bool bMergeFrustums);
	int GetVisAreaConnections(IVisArea ** pAreas, int nMaxConnNum, bool bSkipDisabledPortals = false);
  int GetRealConnections(IVisArea ** pAreas, int nMaxConnNum, bool bSkipDisabledPortals = false);
	bool IsPortalValid();
	bool IsPortalIntersectAreaInValidWay(CVisArea * pPortal);
	bool IsPortal();
	bool IsShapeClockwise();
	bool IsAfectedByOutLights() { return m_bAfectedByOutLights; }
	bool IsActive() { return m_bActive; }
	void UpdateGeometryBBox();
	void MarkForStreaming();
  void DrawAreaBoundsIntoCBuffer(CCoverageBuffer * pCBuffer);
  void ClipPortalVerticesByCameraFrustum(list2<Vec3d> * pPolygon, const CCamera & cam);
  void GetMemoryUsage(ICrySizer*pSizer);
	bool IsConnectedToOutdoor();
	const char * GetName() { return m_sName; }
	bool PreloadVisArea(int nMaxReqursion, bool * pbOutdoorFound, CVisArea * pParentToAvoid, Vec3d vPrevPortalPos, float fPrevPortalDistance);

	Vec3d m_vGeomBoxMin, m_vGeomBoxMax;
	char m_sName[32];
	list2<CVisArea*> m_lstConnections;
	Vec3d m_vConnNormals[2];
	int m_nRndFrameId;
	bool m_bActive;
	int m_nFogVolumeId;
	list2<Vec3d> m_lstShapePoints;
	float m_fHeight;
	Vec3d m_vAmbColor, m_vDynAmbColor;
	bool m_bLoadedAsAreaBox;
	bool m_bAfectedByOutLights;
	float m_fDistance;
  bool m_bSkyOnly;
	float m_fViewDistRatio;
	bool m_bDoubleSide;
//	bool m_bUseDeepness;
	CCamera * m_pOcclCamera;
	OcclusionTestClient m_OcclState;
	Vec3d m_arrvActiveVerts[4];
	bool m_bUseInIndoors;
};

struct CVisAreaManager : public Cry3DEngineBase
{
	CVisArea * m_pCurArea, * m_pCurPortal;
	list2<CVisArea * > m_lstActiveEntransePortals;

	list2<CVisArea*> m_lstVisAreas;
  list2<CVisArea*> m_lstPortals;
  list2<CVisArea*> m_lstOcclAreas;
	list2<CVisArea*> m_lstActiveOcclVolumes;
	list2<CVisArea*> m_lstIndoorActiveOcclVolumes;
  bool m_bOutdoorVisible;
  bool m_bSkyVisible;
  list2<CCamera> m_lstOutdoorPortalCameras;

	CVisAreaManager();
	~CVisAreaManager();
	void SetCurAreas(CObjManager * pObjManager);
	void LoadVisAreaBoxFromXML(XDOM::IXMLDOMDocumentPtr pDoc);
	void PortalsDrawDebug();
	bool IsEntityVisible(IEntityRender * pEntityRS);
	bool IsOutdoorAreasVisible();
  bool IsSkyVisible();
	CVisArea * CreateVisArea();
	bool DeleteVisArea(CVisArea * pVisArea);
	bool SetEntityArea(IEntityRender* pEntityRS);
	void Render(class CObjManager * pObjManager);
	bool UnRegisterEntity(IEntityRender* pEntityRS);
	void ActivatePortal(const Vec3d &vPos, bool bActivate, IEntityRender *pEntity);
	void SetupFogVolumes(CTerrain * pTerrain);
	void LoadVisAreaShapeFromXML(XDOM::IXMLDOMDocumentPtr pDoc);
	void UpdateVisArea(CVisArea * pArea, const Vec3d * pPoints, int nCount, const char * szName, float fHeight, const Vec3d & vAmbientColor, bool bAfectedByOutLights, bool bSkyOnly, CTerrain*pTerrain, const Vec3 & vDynAmbientColor, float fViewDistRatio, bool bDoubleSide, bool bUseDeepness, bool bUseInIndoors);
	void UpdateConnections();
	void MoveAllEntitiesIntoList(list2<IEntityRender*> * plstVisAreasEntities, const Vec3d & vBoxMin, const Vec3d & vBoxMax);
	IVisArea * GetVisAreaFromPos(const Vec3d &vPos);
//	void DefineTrees();
	bool IsEntityVisAreaVisible(IEntityRender * pEnt, bool nCheckNeighbors);
	void SetAreaFogVolume(CTerrain * pTerrain, CVisArea * pVisArea);
	void MakeActiveEntransePortalsList(const CCamera & CurCamera, list2<CVisArea *> & lstActiveEntransePortals, CVisArea * pThisPortal, CObjManager * pObjManager);
  void MergeCameras(CCamera & cam, const CCamera & camPlus);
  void DrawOcclusionAreasIntoCBuffer(CCoverageBuffer * pCBuffer);
  bool IsValidVisAreaPointer(CVisArea * pVisArea);
  void SortStaticInstancesBySize();
  void CheckUnload();
  int m_nLoadedSectors;
  void GetStreamingStatus(int & nLoadedSectors, int & nTotalSectors);
  void GetMemoryUsage(ICrySizer*pSizer);
  bool UnRegisterInAllSectors(IEntityRender * pEntityRS);
	bool PreloadResources();
	bool IsOccludedByOcclVolumes(Vec3d vBoxMin, Vec3d vBoxMax, bool bCheckOnlyIndoorVolumes = false);
	void Preceche(CObjManager * pObjManager);
	void GetObjectsAround(Vec3d vExploPos, float fExploRadius, list2<IEntityRender*> * pEntList);
};

#endif // VisArea_H