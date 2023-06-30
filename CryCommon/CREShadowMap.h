 
#ifndef __CRESHADOWMAP_H__
#define __CRESHADOWMAP_H__

//=============================================================

//struct CLeafBuffer;
//struct IEntity;
/*
class CREShadowMap : public CRendElement
{
  friend class CRender3D;

public:

  void * m_pShadowFrustum; // will define projection of shadow from this object
  float m_fAlpha;

  CREShadowMap()
  {
    mfSetType(eDATA_ShadowMap);
    mfUpdateFlags(FCEF_TRANSFORM);
    m_pShadowFrustum=0;
    m_fAlpha=1.f;
  }

  virtual ~CREShadowMap();

  virtual void mfPrepare();
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);
};*/
/*
class CREShadowMapCaster : public CRendElement
{
  friend class CRender3D;

public:

  CLeafBuffer * m_pBuffer;

  CREShadowMapCaster()
  {
    mfSetType(eDATA_ShadowMap);
    mfUpdateFlags(FCEF_TRANSFORM);
  }

  virtual ~CREShadowMapCaster()
  {
  }

  virtual void mfPrepare();
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);
};
*/
#endif  // __CRESHADOWMAP_H__
