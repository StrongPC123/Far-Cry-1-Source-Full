
#ifndef __CREHEAT_H__
#define __CREHEAT_H__

//=============================================================


class CREHeat : public CRendElement
{
public:
  int m_HeatWidth;
  int m_HeatHeight;

public:
  CREHeat()
  {
    m_HeatWidth = CRenderer::CV_r_heatsize;
    m_HeatHeight = CRenderer::CV_r_heatsize;
    mfSetType(eDATA_Heat);
    mfUpdateFlags(FCEF_TRANSFORM);
  }

  virtual ~CREHeat()
  {
  }

  virtual void mfPrepare();
  virtual bool mfDraw(SShader *ef, SShadeLayer *sfm);
};

#endif  // __CREHEAT_H__
