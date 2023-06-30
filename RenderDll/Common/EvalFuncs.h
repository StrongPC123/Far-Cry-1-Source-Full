/*=============================================================================
  EvalFuncs.h : Funcs for evaluating shader parms.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#ifndef __EVALFUNCS_H__
#define __EVALFUNCS_H__

const int HALF_RAND = (RAND_MAX / 2);
_inline float RandomNum()
{
  int rn;
  rn = rand();
  return ((float)(rn - HALF_RAND) / (float)HALF_RAND);
}
_inline float UnsRandomNum()
{
  int rn;
  rn = rand();
  return ((float)rn / (float)RAND_MAX);
}

#define SHINE_TABLE_SIZE 16384


struct SEvalFuncs
{
  static float EvalWaveForm(SWaveForm *wf);
  static float EvalWaveForm(SWaveForm2 *wf);
  static float EvalWaveForm2(SWaveForm *wf, float frac);

  virtual void ETC_ShadowMap(int ns)=0;
  virtual void ETC_Environment(int ns)=0;
  virtual void ETC_Projection(int ns, float *Mat, float wdt, float hgt)=0;
  virtual void ETC_SphereMap(int ns)=0;
  virtual void ETC_SphereMapEnvironment(int ns)=0;

  virtual void EALPHA_Object()=0;
  virtual void EALPHA_OneMinusObject()=0;
  virtual void EALPHA_Wave(SWaveForm *wf, UCol& color)=0;
  virtual void EALPHA_Noise(SAlphaGenNoise *wf, UCol& color)=0;
  virtual void EALPHA_Beam()=0;

  virtual void ERGB_Object()=0;
  virtual void ERGB_OneMinusObject()=0;
  virtual void ERGB_Wave(SWaveForm *wf, UCol& col)=0;
  virtual void ERGB_Noise(SRGBGenNoise *wf, UCol& col)=0;

  virtual void EMOD_Deform(void)=0;
  virtual void WaveDeform(SDeform *df)=0;
  virtual void VerticalWaveDeform(SDeform *df)=0;
  virtual void BulgeDeform(SDeform *df)=0;
  virtual void SqueezeDeform(SDeform *df)=0;
  virtual void FromCenterDeform(SDeform *df)=0;
  virtual void FlareDeform(SDeform *df)=0;
  virtual void BeamDeform(SDeform *df)=0;
};

// Shader evaluating functions for mergable geometry
// Software shader pipeline
struct SEvalFuncs_C : public SEvalFuncs
{
  virtual void ETC_ShadowMap(int ns);
  virtual void ETC_Environment(int ns);
  virtual void ETC_Projection(int ns, float *Mat, float wdt, float hgt);
  virtual void ETC_SphereMap(int ns);
  virtual void ETC_SphereMapEnvironment(int ns);

  virtual void EALPHA_Object();
  virtual void EALPHA_OneMinusObject();
  virtual void EALPHA_Wave(SWaveForm *wf, UCol& color);
  virtual void EALPHA_Noise(SAlphaGenNoise *wf, UCol& color);
  virtual void EALPHA_Beam() {};

  virtual void ERGB_Object();
  virtual void ERGB_OneMinusObject();
  virtual void ERGB_Wave(SWaveForm *wf, UCol& col);
  virtual void ERGB_Noise(SRGBGenNoise *wf, UCol& col);

  virtual void EMOD_Deform(void);
  virtual void WaveDeform(SDeform *df);
  virtual void VerticalWaveDeform(SDeform *df);
  virtual void BulgeDeform(SDeform *df);
  virtual void SqueezeDeform(SDeform *df);
  virtual void FromCenterDeform(SDeform *df);
  virtual void FlareDeform(SDeform *df);
  virtual void BeamDeform(SDeform *df);
};

// Shader evaluating functions for render elements
struct SEvalFuncs_RE : public SEvalFuncs_C
{
  virtual void ETC_ShadowMap(int ns);
  virtual void ETC_Environment(int ns);
  virtual void ETC_Projection(int ns, float *Mat, float wdt, float hgt);
  virtual void ETC_SphereMap(int ns);
  virtual void ETC_SphereMapEnvironment(int ns);

  virtual void EALPHA_Beam();

  virtual void BeamDeform(SDeform *df);
  virtual void WaveDeform(SDeform *df);
  virtual void FlareDeform(SDeform *df);
  virtual void VerticalWaveDeform(SDeform *df);
  virtual void BulgeDeform(SDeform *df);
  virtual void SqueezeDeform(SDeform *df);
};


//===================================================================================

#endif	// __EVALFUNCS_H__
