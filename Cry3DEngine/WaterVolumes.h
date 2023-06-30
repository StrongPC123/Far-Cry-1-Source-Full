#ifndef CWaterVolumeManager_H
#define CWaterVolumeManager_H

class CWaterVolume : public IWaterVolume, public Cry3DEngineBase
{
public:
	CWaterVolume(IRenderer * pRenderer) 
  { 
    m_pRenderer = pRenderer; m_pLeafBuffer = 0; m_pShader = 0; 
    m_fFlowSpeed = 0; 
		m_bAffectToVolFog = false;
		m_fTriMaxSize = m_fTriMinSize = 8.f; 
		m_fPrevTriMaxSize = m_fPrevTriMinSize = 0; 
		m_fHeight = 0; m_vCurrentCamPos.Set(0,0,0); m_nLastRndFrame=0; 
		m_szName[0] = 0;
		m_vWaterLevelOffset.Set(0,0,0);
  }
	void UpdatePoints(const Vec3d * pPoints, int nCount, float fHeight);
	void SetFlowSpeed(float fSpeed) { m_fFlowSpeed = fSpeed; }
	void SetAffectToVolFog(bool bAffectToVolFog) { m_bAffectToVolFog = bAffectToVolFog; }
	void SetTriSizeLimits(float fTriMinSize, float fTriMaxSize);
	void SetShader(const char * szShaderName);
	void SetMaterial( IMatInfo *pMatInfo );
	void SetName(const char * szName) 
	{ 
		strncpy(m_szName,szName,64); 
	}
	void SetPositionOffset(const Vec3d & vNewOffset);
  IMatInfo * GetMaterial();
	void UpdateVisArea();
  void CheckForUpdate(bool bMakeLowestLod);
	bool IsWaterVolumeAreasVisible();
	void TesselateStrip(list2<struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F> & lstVerts, list2<ushort> & lstIndices, list2<Vec3d> & lstDirections);
	bool TesselateFace(list2<struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F> & lstVerts, list2<ushort> & lstIndices, int nFacePos, list2<Vec3d> & lstDirections);
	void UpdateVisAreaFogVolumeLevel(CVisArea*pVisArea);

	list2<Vec3d> m_lstPoints;
	Vec3d m_vWaterLevelOffset;
	list2<Vec3d> m_lstDirections;
	CLeafBuffer * m_pLeafBuffer;
	Vec3d m_vBoxMin, m_vBoxMax;
	IRenderer * m_pRenderer;
	IShader * m_pShader;
	char m_szName[64];
	float m_fHeight;
	float m_fTriMinSize, m_fTriMaxSize;
	float m_fPrevTriMaxSize, m_fPrevTriMinSize; 

	_smart_ptr<IMatInfo> m_pMaterial;

	float m_fFlowSpeed;
	bool m_bAffectToVolFog;
	list2<struct IVisArea *> m_lstVisAreas;
  Vec3d m_vCurrentCamPos;
  int m_nLastRndFrame;
};

struct CWaterVolumeManager : public Cry3DEngineBase
{
	CWaterVolumeManager( );
	~CWaterVolumeManager();
	void LoadWaterVolumesFromXML(XDOM::IXMLDOMDocumentPtr pDoc);

	list2<CWaterVolume*> m_lstWaterVolumes;

	void InitWaterVolumes();
	void RenderWaterVolumes(bool bOutdoorVisible);
	float GetWaterVolumeLevelFor2DPoint(const Vec3d & vPos, Vec3d * pvFlowDir);

	IWaterVolume * CreateWaterVolume();
	void DeleteWaterVolume(IWaterVolume * pWaterVolume);
	void UpdateWaterVolumeVisAreas();

	IWaterVolume * FindWaterVolumeByName(const char * szName);

private:
	// Local shader param.
	TArray<SShaderParam> m_shaderParams;
};

#endif // CWaterVolumeManager_H
