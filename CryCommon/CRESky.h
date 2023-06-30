 
#ifndef __CRESKY_H__
#define __CRESKY_H__

//=============================================================

#include "VertexFormats.h"

class CRESky : public CRendElement
{
  friend class CRender3D;

public:

  float m_fTerrainWaterLevel;
	float m_fSkyBoxStretching;
  float m_fAlpha;
  int m_nSphereListId;
	list2<struct_VERTEX_FORMAT_P3F_COL4UB> * m_parrFogLayer;
	list2<struct_VERTEX_FORMAT_P3F_COL4UB> * m_parrFogLayer2;
	
#define MAX_SKY_OCCLAREAS_NUM	8
	struct_VERTEX_FORMAT_P3F_COL4UB m_arrvPortalVerts[MAX_SKY_OCCLAREAS_NUM][4];

  CRESky()
  {
    mfSetType(eDATA_Sky);
    mfUpdateFlags(FCEF_TRANSFORM);
    m_fTerrainWaterLevel = 0;
    m_fAlpha = 1;
    m_nSphereListId = 0;
		m_parrFogLayer = m_parrFogLayer2 = NULL;
		m_fSkyBoxStretching=1.f;
		memset(m_arrvPortalVerts,0,sizeof(m_arrvPortalVerts));
  }

  virtual ~CRESky();

  virtual void mfPrepare();
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);
  void DrawSkySphere(float fHeight);
	bool DrawFogLayer();
	bool DrawBlackPortal();
};

#endif  // __CRESKY_H__
