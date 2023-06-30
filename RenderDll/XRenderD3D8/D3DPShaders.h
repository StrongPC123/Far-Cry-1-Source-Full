/*=============================================================================
  D3DPShaders.h : Direct3D pixel shaders interface declaration.
  Copyright 1999 K&M. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#ifndef __D3DPSHADERS_H__
#define __D3DPSHADERS_H__


class CPShader_D3D : public CPShader
{
  uint m_Flags;
  bool m_bWasLoaded;
  char *m_pScript;
  int m_dwCodeSize;
  DWORD m_dwShader;

  TArray<SParam> m_Params;

  bool mfBind();
  bool mfUnbind();

  void mfSetParams(TArray<SParam>& Params, TArray<SParam>* AddParams);
  void mfSetParam(SParam *p);
  
  void mfDeleteParams(TArray<SParam> &Vars);
  bool mfActivate();

public:
  static void mfSetFloat4f(int reg, float *p);
  virtual int Size()
  {
    return 0;
  }
  CPShader_D3D() : CPShader()
  {
    m_bCGType = false;
    m_Flags = 0;
    m_dwShader = 0;
    m_bWasLoaded = false;
    m_pScript = NULL;
  }
  virtual ~CPShader_D3D();
  virtual bool mfCompile(char *scr);
  virtual bool mfSet(bool bStat, SShaderPassHW *slw=NULL);
  virtual bool mfIsCombiner() { return false; }
  virtual void mfReset();
  virtual void mfEnable() {}
  virtual void mfDisable() {}
};    



#endif  // __D3DPSHADERS_H__
