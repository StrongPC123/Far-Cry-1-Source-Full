////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_water.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef CWATER_H
#define CWATER_H 

#define RECURSION_LEVELS_NUM 2
#define CYCLE_BUFFERS_NUM    2

class CWaterOcean : public Cry3DEngineBase
{
  float m_fWaterTranspRatio, m_fWaterReflectRatio, m_fWaterBumpAmountX, m_fWaterBumpAmountY, m_fWaterBorderTranspRatio;
	list2<struct_VERTEX_FORMAT_P3F_COL4UB> Verts_DWQ;
  list2<ushort> Indices_DWQ;
	list2<ushort> lstFirstIdxId;

  CLeafBuffer *m_pLeafBufferWaters[RECURSION_LEVELS_NUM][CYCLE_BUFFERS_NUM];
  CLeafBuffer *m_pLeafBufferBottom, *m_pLeafBufferSunRoad;
  IShader * m_pTerrainWaterShader, *m_pSunRoadShader, *m_pTerrainWaterBottomShader;
	class CREOcclusionQuery * m_pREOcclusionQueries[RECURSION_LEVELS_NUM][CYCLE_BUFFERS_NUM];
	IShader * m_pShaderOcclusionQuery;
  int m_nFogVolumeId;
  int m_nLastVisibleFrameId;
	int m_nBottomTexId;
	float m_fLastFov;

public:
  void SetLastFov(float fLastFov) {m_fLastFov = fLastFov;}
	CWaterOcean(IShader * pTerrainWaterShader, int nBottomTexId, IShader * pSunRoadShader, float fWaterTranspRatio, float fWaterReflectRatio, float fWaterBumpAmount, float fWaterBumpAmountY, float fWaterBorderTranspRatio);
  ~CWaterOcean();
  void Render(const int nRecursionLevel);
	int GetMemoryUsage();
  void SetFogVolumrId(int nFogVolumeId) { m_nFogVolumeId = nFogVolumeId; }
  int GetFogVolumrId() { return m_nFogVolumeId; }
	bool IsWaterVisible();
	void RenderBottom(int nRecursionLevel);

	// Cached parameters.
	TArray<SShaderParam> m_shaderParams;
	ICVar *r_WaterRefractions;
	ICVar *r_WaterReflections;
	ICVar *r_Quality_BumpMapping;
};

#endif // CWATER_H