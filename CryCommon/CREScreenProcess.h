/*
=====================================================================
FILE : CREScreenProcess.h
DESC : Screen processing render element
PROJ : Crytek Engine
CODER: Tiago Sousa

Last Update: 13/06/2003
=====================================================================
*/

#ifndef __CRESCREENPROCESS_H__
#define __CRESCREENPROCESS_H__

// screen processing vars class
class CScreenVars;

// screen processing render element
class CREScreenProcess : public CRendElement
{
  friend class CD3D9Renderer;
  friend class CGLRenderer;

public:

  // constructor/destructor
  CREScreenProcess();

  virtual ~CREScreenProcess();

  // prepare screen processing
  virtual void mfPrepare();
  // render screen processing
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);

  // begin screen processing
  virtual void mfActivate(int iProcess);
  // reset 
  virtual void mfReset(void);

  // set/get methods
  virtual int   mfSetParameter(int iProcess, int iParams, void *dwValue);
  virtual void *mfGetParameter(int iProcess, int iParams);
  
  CScreenVars  *GetVars() { return m_pVars; }

private:
  virtual bool mfDrawLowSpec(SShader *ef, SShaderPass *sfm);

  // screen processing vars class
  CScreenVars *m_pVars;
};

#endif
