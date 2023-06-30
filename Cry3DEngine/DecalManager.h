////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   decalmanager.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef DECAL_MANAGER
#define DECAL_MANAGER

#define DECAL_COUNT (512)
#define ENTITY_DECAL_DIST_FACTOR (200)

class C3DEngine;

class CDecal
{
public:

  // cur state
  Vec3d  m_vPos;
  Vec3d  m_vRight, m_vUp, m_vFront;
  float  m_fSize;
	Vec3d  m_vWSPos;
	float  m_fWSSize;

  // life style
  float					m_fLifeEndTime;
  int						m_nTexId;
  float					m_fAngle;
  IStatObj *		m_pStatObj;
  uint					m_nDynLMask;
  Vec3d					m_vAmbient;
	IEntityRender * m_pDecalOwner;
	int						m_nDecalOwnerComponentId;
	bool					m_bOnTheGround;
	float					m_fGrowTime;
	float					m_fLifeBeginTime;

	// todo: move it into separate structure
	CLeafBuffer * m_pBigDecalLeafBuffer;
	float					m_arrBigDecalCustomData[16];

  CDecal() { ZeroStruct(*this); }
  void Process(bool & active, IRenderer * pIRenderer, const float fFrameTime, C3DEngine * p3DEngine, IShader * pShader, CCamera* pCamera, float fSortOffset);
	void DrawBigDecalOnTerrain(C3DEngine * p3DEngine, IRenderer * pIRenderer,	float fCurrTime);
  void Render3DObject();
	void DrawComplexDecal(IRenderer * pIRenderer);
};

class CDecalManager : public Cry3DEngineBase
{
  CDecal m_arrDecals[DECAL_COUNT];
  bool m_arrbActiveDecals[DECAL_COUNT];
  int m_nCurDecal;
  C3DEngine * m_p3DEngine;
  list2<CDecal> m_LightHoles;
  IShader * m_pShader_ParticleLight, * m_pShader_Decal_VP, * m_pShader_Decal_2D_VP;
public:
  
  CDecalManager(C3DEngine * p3DEngine);
  void Spawn(CryEngineDecalInfo Decal, float fMaxViewDistance, CTerrain * pTerrain);
  void Render();
  void SubmitLightHolesToRenderer();
  void DrawBigDecalsOnTerrain();
	void OnEntityDeleted(IEntityRender * pEntityRS);
	
	// complex decals
	void FillBigDecalIndices(CLeafBuffer * pLeafBuffer, Vec3d vPos, float fRadius, Vec3d vProjDir, list2<ushort> * plstIndices);
	CLeafBuffer * MakeBigDecalLeafBuffer(CLeafBuffer * pSourceLeafBuffer, Vec3d vPos, float fRadius, Vec3d vProjDir, int nTexID);
	void GetMemoryUsage(ICrySizer*pSizer);
	void Reset() { memset(m_arrbActiveDecals,0,sizeof(m_arrbActiveDecals)); m_nCurDecal=0; }
	void DeleteDecalsInRange( Vec3d vBoxMin, Vec3d vBoxMax, bool bDeleteBigTerrainDecals);
	bool AdjustDecalPosition( CryEngineDecalInfo & DecalInfo, bool bMakeFatTest );
	bool RayLeafBufferIntersection(CLeafBuffer * pLeafBuffer, const Vec3d & vInPos, const Vec3d & vInDir, Vec3d & vOutPos, Vec3d & vOutNormal);
};

#endif // DECAL_MANAGER
