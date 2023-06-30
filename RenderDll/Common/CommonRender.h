/*=============================================================================
	CommonRender.h: Crytek Common render helper functions and structures declarations.
	Copyright (c) 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Khonich Andrey

=============================================================================*/

#if !defined(COMMONRENDER_H__THIS_LINE_INSERTED_BY_VVP__15_10_1999__INCLUDED_)
#define COMMONRENDER_H__THIS_LINE_INSERTED_BY_VVP__15_10_1999__INCLUDED_

#include "Cry_Math.h"

#include "Defs.h"
#include "ColorDefs.h"
#include "Shaders/Shader.h"



//////////////////////////////////////////////////////////////////////
extern CRenderer *gRenDev;
extern bool g_bProfilerEnabled;


class CryModel;

// Cull functions
int gfCullBox(Vec3d& min, Vec3d& max);
bool gfCullPoint(Vec3d& org);
int gfCullSphere(Vec3d& cent, float radius);
int gfCullBoundBox(float *minmax);

//====================================================================

#define CR_LITTLE_ENDIAN


extern TArray <CFColor> gCurLightStyles;

struct SWaveForm;
struct SShader;

extern bool gbRgb;

_inline DWORD COLCONV (DWORD clr)
{
  return ((clr & 0xff00ff00) | ((clr & 0xff0000)>>16) | ((clr & 0xff)<<16));
}
_inline void COLCONV (CFColor& col)
{
  float v = col[0];
  col[0] = col[2];
  col[2] = v;
}

_inline void f2d(double *dst, float *src)
{
  for (int i=0; i<16; i++)
  {
    dst[i] = src[i];
  }
}

_inline void d2f(float *dst, double *src)
{
  for (int i=0; i<16; i++)
  {
    dst[i] = (float)src[i];
  }
}


//==============================================================================

#define SF_TRANS 4

//==============================================================================

struct SGenTC_NormalMap : public SGenTC
{
  virtual SGenTC *mfCopy();
  virtual bool mfSet(bool bEnable);
  virtual void mfCompile(char *params, SShader *ef);
  virtual int Size()
  {
    int nSize = sizeof(SGenTC_NormalMap);
    return nSize;
  }
};

struct SGenTC_ReflectionMap : public SGenTC
{
  virtual SGenTC *mfCopy();
  virtual bool mfSet(bool bEnable);
  virtual void mfCompile(char *params, SShader *ef);
  virtual int Size()
  {
    int nSize = sizeof(SGenTC_ReflectionMap);
    return nSize;
  }
};

struct SGenTC_SphereMap : public SGenTC
{
  virtual SGenTC *mfCopy();
  virtual bool mfSet(bool bEnable);
  virtual void mfCompile(char *params, SShader *ef);
  virtual int Size()
  {
    int nSize = sizeof(SGenTC_SphereMap);
    return nSize;
  }
};

struct SGenTC_EmbossMap : public SGenTC
{
  virtual SGenTC *mfCopy();
  virtual bool mfSet(bool bEnable);
  virtual void mfCompile(char *params, SShader *ef);
  virtual int Size()
  {
    int nSize = sizeof(SGenTC_EmbossMap);
    return nSize;
  }
};

struct SGenTC_ObjectLinear : public SGenTC
{
  TArray<SParam> m_Params;
  
  virtual ~SGenTC_ObjectLinear()
  {
    m_Params.Free();
  }
  virtual SGenTC *mfCopy();
  virtual bool mfSet(bool bEnable);
  virtual void mfCompile(char *params, SShader *ef);
  virtual int Size()
  {
    int nSize = sizeof(SGenTC_ObjectLinear);
    nSize += m_Params.GetSize() * sizeof(SParam);
    return nSize;
  }
};

struct SGenTC_EyeLinear : public SGenTC
{
  TArray<SParam> m_Params;
  
  virtual ~SGenTC_EyeLinear()
  {
    m_Params.Free();
  }
  virtual SGenTC *mfCopy();
  virtual bool mfSet(bool bEnable);
  virtual void mfCompile(char *params, SShader *ef);
  virtual int Size()
  {
    int nSize = sizeof(SGenTC_EyeLinear);
    nSize += m_Params.GetSize() * sizeof(SParam);
    return nSize;
  }
};


//=================================================================

#endif
