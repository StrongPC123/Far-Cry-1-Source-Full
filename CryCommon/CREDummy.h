 
#ifndef __CREDUMMY_H__
#define __CREDUMMY_H__

//=============================================================

class CREDummy : public CRendElement
{
  friend class CRender3D;

public:

  CREDummy()
  {
    mfSetType(eDATA_Dummy);
    mfUpdateFlags(FCEF_TRANSFORM);
  }

  virtual ~CREDummy()
  {
  }

  virtual void mfPrepare();
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);
};

#endif  // __CREMotionBlur_H__
