#ifndef _CRE2DQuad_H_
#define _CRE2DQuad_H_

#include "vertexformats.h"

class CRE2DQuad: public CRendElement
{
  friend class CRender3D;

public:

  CRE2DQuad()
  {
    mfSetType(eDATA_2DQuad);
    mfUpdateFlags(FCEF_TRANSFORM);
  }

  virtual ~CRE2DQuad()
  {
  }

  virtual void mfPrepare();
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);

  virtual void *mfGetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags);

  struct_VERTEX_FORMAT_P3F_TEX2F m_arrVerts[4];
};

#endif _CRE2DQuad_H_
