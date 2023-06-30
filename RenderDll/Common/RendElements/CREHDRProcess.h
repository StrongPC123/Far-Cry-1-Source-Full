/*
=====================================================================
FILE : CREHDRProcess.h
DESC : HDR processing render element
PROJ : Crytek Engine
CODER: Andrey Honich

=====================================================================
*/

#ifndef __CREHDRPROCESS_H__
#define __CREHDRPROCESS_H__


// screen processing render element
class CREHDRProcess : public CRendElement
{
  friend class CD3D9Renderer;
  friend class CGLRenderer;

public:

  // constructor/destructor
  CREHDRProcess();

  virtual ~CREHDRProcess();

  // prepare screen processing
  virtual void mfPrepare();
  // render screen processing
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);

  // begin screen processing
  virtual void mfActivate(int iProcess);
  // reset 
  virtual void mfReset(void);
};

#endif
