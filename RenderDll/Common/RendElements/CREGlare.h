
#ifndef __CREGLARE_H__
#define __CREGLARE_H__

//=============================================================

struct SByteColor
{
  byte r,g,b,a;
};

struct SLongColor
{
  unsigned long r,g,b,a;
};

class CREGlare : public CRendElement
{
public:
  int m_GlareWidth;
  int m_GlareHeight;
  float m_fGlareAmount;

public:
  CREGlare()
  {
    mfInit();
  }
  void mfInit();

  virtual ~CREGlare()
  {
  }

  virtual void mfPrepare();
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);
};

#endif  // __CREGLARE_H__
