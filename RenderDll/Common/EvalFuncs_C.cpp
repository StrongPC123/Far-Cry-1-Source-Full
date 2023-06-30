/*=============================================================================
	EvalFuncs_C : implementation of evaluator functions (Render Buffer data changing).
	Copyright (c) 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#define EVALFUNCS_C_CPP

#include "RenderPCH.h"
#include "shadow_renderer.h"
#include <IEntityRenderState.h>

//===========================================================================

//---------------------------------------------------------------------------
// Wave evaluator

float SEvalFuncs::EvalWaveForm(SWaveForm *wf) 
{
  int val;

  float Amp;
  float Freq;
  float Phase;
  float Level;

  if (wf->m_Flags & WFF_LERP)
  {
    val = (int)(gRenDev->m_RP.m_RealTime * 597.0f);
    val &= 0x3ff;
    float fLerp = gRenDev->m_RP.m_tSinTable[val] * 0.5f + 0.5f;
    
    if (wf->m_Amp != wf->m_Amp1)
      Amp = LERP(wf->m_Amp, wf->m_Amp1, fLerp);
    else
      Amp = wf->m_Amp;

    if (wf->m_Freq != wf->m_Freq1)
      Freq = LERP(wf->m_Freq, wf->m_Freq1, fLerp);
    else
      Freq = wf->m_Freq;

    if (wf->m_Phase != wf->m_Phase1)
      Phase = LERP(wf->m_Phase, wf->m_Phase1, fLerp);
    else
      Phase = wf->m_Phase;

    if (wf->m_Level != wf->m_Level1)
      Level = LERP(wf->m_Level, wf->m_Level1, fLerp);
    else
      Level = wf->m_Level;
  }
  else
  {
    Level = wf->m_Level;
    Amp = wf->m_Amp;
    Phase = wf->m_Phase;
    Freq = wf->m_Freq;
  }

  switch(wf->m_eWFType)
  {
    case eWF_None:
			Warning( 0,0,"WARNING: SEvalFuncs::EvalWaveForm called with 'EWF_None' in Shader '%s'\n", gRenDev->m_RP.m_pShader->m_Name.c_str());
      break;

    case eWF_Sin:
      val = (int)((gRenDev->m_RP.m_RealTime*Freq+Phase)*1024.0f);
      return Amp*gRenDev->m_RP.m_tSinTable[val&0x3ff]+Level;

    case eWF_HalfSin:
      val = QRound((gRenDev->m_RP.m_RealTime*Freq+Phase)*1024.0f);
      return Amp*gRenDev->m_RP.m_tHalfSinTable[val&0x3ff]+Level;

    case eWF_InvHalfSin:
      val = QRound((gRenDev->m_RP.m_RealTime*Freq+Phase)*1024.0f);
      return Amp*(1.0f-gRenDev->m_RP.m_tHalfSinTable[val&0x3ff])+Level;

    case eWF_SawTooth:
      val = QRound((gRenDev->m_RP.m_RealTime*Freq+Phase)*1024.0f);
      val &= 0x3ff;
      return Amp*gRenDev->m_RP.m_tSawtoothTable[val]+Level;

    case eWF_InvSawTooth:
      val = QRound((gRenDev->m_RP.m_RealTime*Freq+Phase)*1024.0f);
      val &= 0x3ff;
      return Amp*gRenDev->m_RP.m_tInvSawtoothTable[val]+Level;

    case eWF_Square:
      val = QRound((gRenDev->m_RP.m_RealTime*Freq+Phase)*1024.0f);
      val &= 0x3ff;
      return Amp*gRenDev->m_RP.m_tSquareTable[val]+Level;

    case eWF_Triangle:
      val = QRound((gRenDev->m_RP.m_RealTime*Freq+Phase)*1024.0f);
      val &= 0x3ff;
      return Amp*gRenDev->m_RP.m_tTriTable[val]+Level;

    case eWF_Hill:
      val = QRound((gRenDev->m_RP.m_RealTime*Freq+Phase)*1024.0f);
      val &= 0x3ff;
      return Amp*gRenDev->m_RP.m_tHillTable[val]+Level;

    case eWF_InvHill:
      val = QRound((gRenDev->m_RP.m_RealTime*Freq+Phase)*1024.0f);
      val &= 0x3ff;
      return Amp*(1.0f-gRenDev->m_RP.m_tHillTable[val])+Level;

    default:
      Warning( 0,0,"WARNING: SEvalFuncs::EvalWaveForm: bad WaveType '%d' in Shader '%s'\n", wf->m_eWFType, gRenDev->m_RP.m_pShader->m_Name.c_str());
      break;
  }
  return 1;
}

float SEvalFuncs::EvalWaveForm(SWaveForm2 *wf) 
{
  int val;

  switch(wf->m_eWFType)
  {
    case eWF_None:
      Warning( 0,0,"WARNING: SEvalFuncs::EvalWaveForm called with 'EWF_None' in Shader '%s'\n", gRenDev->m_RP.m_pShader->m_Name.c_str());
      break;

    case eWF_Sin:
      val = FtoI((gRenDev->m_RP.m_RealTime*wf->m_Freq+wf->m_Phase)*1024.0f);
      return wf->m_Amp*gRenDev->m_RP.m_tSinTable[val&0x3ff]+wf->m_Level;

    case eWF_HalfSin:
      val = QRound((gRenDev->m_RP.m_RealTime*wf->m_Freq+wf->m_Phase)*1024.0f);
      return wf->m_Amp*gRenDev->m_RP.m_tHalfSinTable[val&0x3ff]+wf->m_Level;

    case eWF_InvHalfSin:
      val = QRound((gRenDev->m_RP.m_RealTime*wf->m_Freq+wf->m_Phase)*1024.0f);
      return wf->m_Amp*(1.0f-gRenDev->m_RP.m_tHalfSinTable[val&0x3ff])+wf->m_Level;

    case eWF_SawTooth:
      val = QRound((gRenDev->m_RP.m_RealTime*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*gRenDev->m_RP.m_tSawtoothTable[val]+wf->m_Level;

    case eWF_InvSawTooth:
      val = QRound((gRenDev->m_RP.m_RealTime*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*gRenDev->m_RP.m_tInvSawtoothTable[val]+wf->m_Level;

    case eWF_Square:
      val = QRound((gRenDev->m_RP.m_RealTime*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*gRenDev->m_RP.m_tSquareTable[val]+wf->m_Level;

    case eWF_Triangle:
      val = QRound((gRenDev->m_RP.m_RealTime*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*gRenDev->m_RP.m_tTriTable[val]+wf->m_Level;

    case eWF_Hill:
      val = QRound((gRenDev->m_RP.m_RealTime*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*gRenDev->m_RP.m_tHillTable[val]+wf->m_Level;

    case eWF_InvHill:
      val = QRound((gRenDev->m_RP.m_RealTime*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*(1.0f-gRenDev->m_RP.m_tHillTable[val])+wf->m_Level;

    default:
      Warning( 0,0,"WARNING: SEvalFuncs::EvalWaveForm: bad WaveType '%d' in Shader '%s'\n", wf->m_eWFType, gRenDev->m_RP.m_pShader->m_Name.c_str());
      break;
  }
  return 1;
}

float SEvalFuncs::EvalWaveForm2(SWaveForm *wf, float frac) 
{
  int val;

  if (!(wf->m_Flags & WFF_CLAMP))
  switch(wf->m_eWFType)
  {
    case eWF_None:
      Warning( 0,0,"EvalWaveForm2 called with 'EWF_None' in Shader '%s'\n", gRenDev->m_RP.m_pShader->m_Name.c_str());
      break;

    case eWF_Sin:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*gRenDev->m_RP.m_tSinTable[val]+wf->m_Level;

    case eWF_SawTooth:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*gRenDev->m_RP.m_tSawtoothTable[val]+wf->m_Level;

    case eWF_InvSawTooth:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*gRenDev->m_RP.m_tInvSawtoothTable[val]+wf->m_Level;

    case eWF_Square:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*gRenDev->m_RP.m_tSquareTable[val]+wf->m_Level;

    case eWF_Triangle:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*gRenDev->m_RP.m_tTriTable[val]+wf->m_Level;

    case eWF_Hill:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*gRenDev->m_RP.m_tHillTable[val]+wf->m_Level;

    case eWF_InvHill:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val &= 0x3ff;
      return wf->m_Amp*(1-gRenDev->m_RP.m_tHillTable[val])+wf->m_Level;

    default:
			Warning( 0,0,"Warning: EvalWaveForm2: bad EWF '%d' in Shader '%s'\n", wf->m_eWFType, gRenDev->m_RP.m_pShader->m_Name.c_str());
      break;
  }
  else
  switch(wf->m_eWFType)
  {
    case eWF_None:
			Warning( 0,0,"Warning: EvalWaveForm2 called with 'EWF_None' in Shader '%s'\n", gRenDev->m_RP.m_pShader->m_Name.c_str());
      break;

    case eWF_Sin:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val = min(val, 1023);
      return wf->m_Amp*gRenDev->m_RP.m_tSinTable[val]+wf->m_Level;

    case eWF_SawTooth:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val = min(val, 1023);
      return wf->m_Amp*gRenDev->m_RP.m_tSawtoothTable[val]+wf->m_Level;

    case eWF_InvSawTooth:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val = min(val, 1023);
      return wf->m_Amp*gRenDev->m_RP.m_tInvSawtoothTable[val]+wf->m_Level;

    case eWF_Square:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val = min(val, 1023);
      return wf->m_Amp*gRenDev->m_RP.m_tSquareTable[val]+wf->m_Level;

    case eWF_Triangle:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val = min(val, 1023);
      return wf->m_Amp*gRenDev->m_RP.m_tTriTable[val]+wf->m_Level;

    case eWF_Hill:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val = min(val, 1023);
      return wf->m_Amp*gRenDev->m_RP.m_tHillTable[val]+wf->m_Level;

    case eWF_InvHill:
      val = QRound((frac*wf->m_Freq+wf->m_Phase)*1024.0f);
      val = min(val, 1023);
      return wf->m_Amp*(1.0f-gRenDev->m_RP.m_tHillTable[val])+wf->m_Level;

    default:
			Warning( 0,0,"Warning: EvalWaveForm2: bad EWF '%d' in Shader '%s'\n", wf->m_eWFType, gRenDev->m_RP.m_pShader->m_Name.c_str());
      break;
  }
  return 1;
}

//=========================================================================================
//Calc Lights and generate per-vertex colors


void SEvalFuncs_C::ERGB_Object()
{
  int i;

  if (gRenDev->m_RP.m_pCurObject)
  {
    uint col = gRenDev->m_RP.m_pCurObject->m_Color.GetTrue();
    col = COLCONV(col);

    byte *ptr = gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD;
    for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr+=gRenDev->m_RP.m_Stride)
    {
      *(uint *)(ptr) = col;
    }
  }
}

void SEvalFuncs_C::ERGB_OneMinusObject()
{
  int i;

  if (gRenDev->m_RP.m_pCurObject)
  {
    uint col = !gRenDev->m_RP.m_pCurObject->m_Color.GetTrue();
    col = COLCONV(col);

    byte *ptr = gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD;
    for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr+=gRenDev->m_RP.m_Stride)
    {
      *(uint *)(ptr) = col;
    }
  }
}

void SEvalFuncs_C::EALPHA_Object()
{
  int i;
  byte a;

  if (gRenDev->m_RP.m_pCurObject)
  {
    a = (byte)(gRenDev->m_RP.m_pCurObject->m_Color[3] * 255.0f);

    byte *ptr = gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD + 3;
    for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr+=gRenDev->m_RP.m_Stride)
    {
      ptr[0] = a;
    }
  }
}

void SEvalFuncs_C::EALPHA_OneMinusObject()
{
  int i;
  byte a;

  if (gRenDev->m_RP.m_pCurObject)
  {
    a = 255 - (byte)(gRenDev->m_RP.m_pCurObject->m_Color[3] * 255.0f);

    byte *ptr = gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD + 3;
    for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr+=gRenDev->m_RP.m_Stride)
    {
      ptr[0] = a;
    }
  }
}

void SEvalFuncs_C::ERGB_Wave(SWaveForm *wf, UCol& col)
{
  int i;
  float val = EvalWaveForm(wf);
  if (val < 0)
    val = 0;
  if (val > 1)
    val = 1;

  int v = (int)(val * 255.0f);
  v = CLAMP(v, 0, 255);
  col.bcolor[0] = col.bcolor[1] = col.bcolor[2] = v;
  col.bcolor[3] = 255;
  COLCONV(col.dcolor);
  byte *ptr = gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD;
  for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr+=gRenDev->m_RP.m_Stride)
  {
    *(uint *)(ptr) = col.dcolor;
  }
}

void SEvalFuncs_C::EALPHA_Wave(SWaveForm *wf, UCol& col)
{
  int i;
  float val = EvalWaveForm(wf);

  int v = (int)(val * 255.0f);
  v = CLAMP(v, 0, 255);
  col.bcolor[3] = v;
  byte *ptr = gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD + 3;
  for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr+=gRenDev->m_RP.m_Stride)
  {
    *ptr = v;
  }
}

void SEvalFuncs_C::ERGB_Noise(SRGBGenNoise *wf, UCol& col)
{
  int i;

  float v = RandomNum();
  byte r = (byte)(CLAMP(v * wf->m_RangeR + wf->m_ConstR, 0.0f, 1.0f) * 255.0f);
  v = RandomNum();
  byte g = (byte)(CLAMP(v * wf->m_RangeG + wf->m_ConstG, 0.0f, 1.0f) * 255.0f);
  v = RandomNum();
  byte b = (byte)(CLAMP(v * wf->m_RangeB + wf->m_ConstB, 0.0f, 1.0f) * 255.0f);

  col.bcolor[0] = r;
  col.bcolor[1] = g;
  col.bcolor[2] = b;
  COLCONV(col.dcolor);
  byte *ptr = gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD;
  for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr+=gRenDev->m_RP.m_Stride)
  {
    *(uint *)(ptr) = col.dcolor;
  }
}

void SEvalFuncs_C::EALPHA_Noise(SAlphaGenNoise *wf, UCol& col)
{
  int i;
  float v = RandomNum();
  byte a = (byte)(CLAMP(v * wf->m_RangeA + wf->m_ConstA, 0.0f, 1.0f) * 255.0f);
  
  col.bcolor[3] = a;

  byte *ptr = gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsD + 3;
  for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr+=gRenDev->m_RP.m_Stride)
  {
    *ptr = a;
  }
}

//=======================================================================
// RT Deformations

void SEvalFuncs_C::EMOD_Deform(void)
{
}

_inline void Deform(float val, Vec3d& vrt, byte *nrm)
{
  float *vnrm = (float *)nrm;
  vrt[0] += val * vnrm[0];
  vrt[1] += val * vnrm[1];
  vrt[2] += val * vnrm[2];
}

void SEvalFuncs_C::WaveDeform(SDeform *df)
{
  int i, val;
  float f;

  gRenDev->m_RP.m_Flags |= RBF_MODIF_VERT;
  
  UPipeVertex ptr = gRenDev->m_RP.m_Ptr;
  byte *vnrm = ptr.PtrB+gRenDev->m_RP.m_OffsN;
  float *WaveTable;
  switch (df->m_DeformGen.m_eWFType)
  {
    case eWF_Sin:
    default:
      WaveTable = gRenDev->m_RP.m_tSinTable;
      break;
    case eWF_Triangle:
      WaveTable = gRenDev->m_RP.m_tTriTable;
      break;
    case eWF_Square:
      WaveTable = gRenDev->m_RP.m_tSquareTable;
      break;
    case eWF_SawTooth:
      WaveTable = gRenDev->m_RP.m_tSawtoothTable;
      break;
    case eWF_InvSawTooth:
      WaveTable = gRenDev->m_RP.m_tInvSawtoothTable;
      break;
    case eWF_Hill:
      WaveTable = gRenDev->m_RP.m_tHillTable;
      break;
  }
  for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride, vnrm+=gRenDev->m_RP.m_Stride)
  {
    f = ptr.VBPtr_0->x + ptr.VBPtr_0->y + ptr.VBPtr_0->z;
    f = f*df->m_ScaleVerts + gRenDev->m_RP.m_RealTime*df->m_DeformGen.m_Freq + df->m_DeformGen.m_Phase;
    f *= 1024.0;
    val = QRound(f);

    f = df->m_DeformGen.m_Amp * CRenderer::CV_r_wavescale * WaveTable[val&0x3ff] + df->m_DeformGen.m_Level;

    Deform(f, *ptr.VBPtr_0, vnrm);
  }
}

void SEvalFuncs_C::VerticalWaveDeform(SDeform *df)
{
}

void SEvalFuncs_C::FlareDeform(SDeform *df)
{
}
void SEvalFuncs_C::BeamDeform(SDeform *df)
{
}

void SEvalFuncs_C::SqueezeDeform(SDeform *df)
{
  int i;
  float f;
  UPipeVertex ptr = gRenDev->m_RP.m_Ptr;
  byte *nrm = ptr.PtrB+gRenDev->m_RP.m_OffsN;

  gRenDev->m_RP.m_Flags |= RBF_MODIF_VERT;
  
  f = EvalWaveForm(&df->m_DeformGen);

  for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride, nrm+=gRenDev->m_RP.m_Stride)
  {
    float *vnrm = (float *)nrm;
    ptr.VBPtr_0->x += f * vnrm[0];
    ptr.VBPtr_0->y += f * vnrm[1];
    ptr.VBPtr_0->z += f * vnrm[2];
  }
}

void SEvalFuncs_C::BulgeDeform(SDeform *df)
{
  int i;
  float f;
  int val;
  Vec3d v;

  gRenDev->m_RP.m_Flags |= RBF_MODIF_VERT;

  UPipeVertex ptr = gRenDev->m_RP.m_Ptr;
  byte *nrm = ptr.PtrB+gRenDev->m_RP.m_OffsN;
  
  for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride, nrm+=gRenDev->m_RP.m_Stride)
  {
    f = gRenDev->m_RP.m_pBaseTexCoordPointer[i].vert[0] + gRenDev->m_RP.m_pBaseTexCoordPointer[i].vert[1] + ptr.VBPtr_0->x + ptr.VBPtr_0->y + ptr.VBPtr_0->z;
    f = f * df->m_ScaleVerts + df->m_DeformGen.m_Phase + gRenDev->m_RP.m_RealTime*df->m_DeformGen.m_Freq;
    f *= 1024;
    val = QRound(f);
    val &= 0x3ff;

    f = df->m_DeformGen.m_Amp * CRenderer::CV_r_wavescale * gRenDev->m_RP.m_tSinTable[val] + df->m_DeformGen.m_Level;
    float *vnrm = (float *)nrm;
    v[0] = f * vnrm[0];
    v[1] = f * vnrm[1];
    v[2] = f * vnrm[2];

    *ptr.VBPtr_0 += v;
  }
}

void SEvalFuncs_C::FromCenterDeform(SDeform *df)
{
  if (!gRenDev->m_RP.m_pCurObject)
    return;

  gRenDev->m_RP.m_Flags |= RBF_MODIF_VERT;

  int i;
  float f;
  Vec3d v;
  UPipeVertex ptr = gRenDev->m_RP.m_Ptr;
  Vec3d cent;

  cent = gRenDev->m_RP.m_Center;

  f = EvalWaveForm2(&df->m_DeformGen, gRenDev->m_RP.m_RealTime-gRenDev->m_RP.m_pCurObject->m_StartTime);
  for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride)
  {
    v = *ptr.VBPtr_0 - cent;
    v.Normalize();
    v *= f;
    *ptr.VBPtr_0 += v;
  }
}

//=================================================================
// Texture coords generate

void SEvalFuncs_C::ETC_Environment(int ns)
{
  int i;
  float f, d;
  Vec3d v;

  gRenDev->m_RP.m_Flags |= RBF_MODIF_TC;
  
  UPipeVertex ptr = gRenDev->m_RP.m_Ptr;
  byte *nrm = ptr.PtrB+gRenDev->m_RP.m_OffsN;
  byte *tptr = gRenDev->m_RP.m_Ptr.PtrB + gRenDev->m_RP.m_OffsT + ns*16;
  for (i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr.PtrB+=gRenDev->m_RP.m_Stride, tptr+=gRenDev->m_RP.m_Stride, nrm+=gRenDev->m_RP.m_Stride)
  {
    Vec3d *vnrm = (Vec3d *)nrm;
    v = gRenDev->m_RP.m_ViewOrg - *ptr.VBPtr_0;
    v.Normalize();
    d = v | (*vnrm);
    f = d * vnrm->y;
    f += f;
    *(float *)(tptr) = ((f - v[1]) + 1.0f) * 0.5f;

    f = d * vnrm->z;
    f += f;
    *(float *)(tptr+4) = 0.5f - ((f - v[2]) * 0.5f);
  }
}

void SEvalFuncs_C::ETC_Projection(int ns, float *Mat, float wdt, float hgt)
{
}

void SEvalFuncs_C::ETC_SphereMap(int ns)
{
  int i;
  CCObject *obj = gRenDev->m_RP.m_pCurObject;

  gRenDev->m_RP.m_Flags |= RBF_MODIF_TC;
  
  float r00 = gRenDev->m_ViewMatrix(0,0), r01 = gRenDev->m_ViewMatrix(0,1), r02 = gRenDev->m_ViewMatrix(0,2);
  float r10 = gRenDev->m_ViewMatrix(1,0), r11 = gRenDev->m_ViewMatrix(1,1), r12 = gRenDev->m_ViewMatrix(1,2);
  float r20 = gRenDev->m_ViewMatrix(2,0), r21 = gRenDev->m_ViewMatrix(2,1), r22 = gRenDev->m_ViewMatrix(2,2);

  // Loop through the vertices, transforming each one and calculating
  // the correct texture coordinates.
  byte *ptr = gRenDev->m_RP.m_Ptr.PtrB+gRenDev->m_RP.m_OffsT+ns*16;
  byte *nrm = gRenDev->m_RP.m_Ptr.PtrB+gRenDev->m_RP.m_OffsN;
  for( i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr+=gRenDev->m_RP.m_Stride, nrm+=gRenDev->m_RP.m_Stride )
  {
    Vec3d *vnrm = (Vec3d *)nrm;
    float nx = vnrm->x;
    float ny = vnrm->y;
    float nz = vnrm->z;

    // Check the z-component, to skip any vertices that face backwards
    //if( nx*m13 + ny*m23 + nz*m33 > 0.0f )
    //  continue;

    // Assign the spheremap's texture coordinates
    *(float *)(ptr) = 0.5f * ( 1.0f + ( nx*r00 + ny*r10 + nz*r20 ) );
    *(float *)(ptr+4) = 0.5f * ( 1.0f - ( nx*r01 + ny*r11 + nz*r21 ) );
  }
}

void SEvalFuncs_C::ETC_SphereMapEnvironment(int ns)
{
  int i;
  CCObject *obj = gRenDev->m_RP.m_pCurObject;

  gRenDev->m_RP.m_Flags |= RBF_MODIF_TC;
  
  if (!obj)
    return;
  
  float r00 = obj->m_Matrix(0,0), r01 = obj->m_Matrix(0,1), r02 = obj->m_Matrix(0,2);
  float r10 = obj->m_Matrix(1,0), r11 = obj->m_Matrix(1,1), r12 = obj->m_Matrix(1,2);
  float r20 = obj->m_Matrix(2,0), r21 = obj->m_Matrix(2,1), r22 = obj->m_Matrix(2,2);

  // Loop through the vertices, transforming each one and calculating
  // the correct texture coordinates.
  byte *ptr = gRenDev->m_RP.m_Ptr.PtrB+gRenDev->m_RP.m_OffsT+ns*16;
  byte *nrm = gRenDev->m_RP.m_Ptr.PtrB+gRenDev->m_RP.m_OffsN;
  for( i=0; i<gRenDev->m_RP.m_RendNumVerts; i++, ptr+=gRenDev->m_RP.m_Stride, nrm+=gRenDev->m_RP.m_Stride)
  {
    Vec3d *vnrm = (Vec3d *)nrm;
    float nx = vnrm->x;
    float ny = vnrm->y;
    float nz = vnrm->z;

    // Check the z-component, to skip any vertices that face backwards
    //if( nx*m13 + ny*m23 + nz*m33 > 0.0f )
    //  continue;

    // Assign the spheremap's texture coordinates
    *(float *)(ptr) = 0.5f * ( 1.0f + ( nx*r00 + ny*r10 + nz*r20 ) );
    *(float *)(ptr+4) = 0.5f * ( 1.0f + ( nx*r01 + ny*r11 + nz*r21 ) );
  }
}


//========================================================================================

void SEvalFuncs_C::ETC_ShadowMap(int ns)
{
  CRenderer *rd = gRenDev;
  rd->m_RP.m_RECustomTexBind[ns] = -1;
  
  assert(rd->m_RP.m_FlagsPerFlush & RBSI_SHADOWPASS);
  int nsFrust;
  if (rd->m_RP.m_pShader->m_eSort == eS_TerrainShadowPass)
    nsFrust = 0;
  else
    nsFrust = ns + rd->m_RP.m_nCurStartCaster;

  list2<ShadowMapLightSourceInstance> * lsources = (list2<ShadowMapLightSourceInstance>*)rd->m_RP.m_pCurObject->m_pShadowCasters;

//	bool bActiveShadowReceiving = false;

	// skip this stage if this entity was used to create this shadow map
  if(!lsources || nsFrust>=lsources->Count())
	{
		if(!lsources)
			Warning( 0,0,"Warning: SEvalFuncs_RE::ETC_ShadowMap: !lsources");
		else
    if(nsFrust<lsources->Count())
			Warning( 0,0,"Warning: SEvalFuncs_RE::ETC_ShadowMap: nsFrust<lsources->Count()");

    if ((rd->GetFeatures() & RFT_SHADOWMAP_SELFSHADOW) && !(rd->GetFeatures() & RFT_DEPTHMAPS))
      rd->m_RP.m_RECustomTexBind[ns] = rd->m_TexMan->m_Text_Depth->m_Bind;
    else
      rd->m_RP.m_RECustomTexBind[ns] = rd->m_TexMan->m_Text_WhiteShadow->m_Bind;

    if ((rd->GetFeatures() & RFT_SHADOWMAP_SELFSHADOW) && !(rd->GetFeatures() & RFT_DEPTHMAPS))
    {
      Matrix44 *mt = &rd->m_cEF.m_TempMatrices[ns][7];
      mt->SetIdentity();
    }

		rd->SelectTMU(0);
		return; // cancel this stage
	}
    
  if(!(*lsources)[nsFrust].m_pLS)
    return;

	// get projection frustum
  ShadowMapFrustum * pShadowMapFrustum = (*lsources)[nsFrust].m_pLS->GetShadowMapFrustum(0);
//	if(bActiveShadowReceiving)
	//	pShadowMapFrustum = (*lsources)[nsFrust].m_pLS->GetShadowMapFrustumPassiveCasters(0);
	if(!pShadowMapFrustum)
		return;

  // detect usage of same lsource second time -> use penumbra frustum
  if( nsFrust>0 && (*lsources)[nsFrust].m_pLS == (*lsources)[nsFrust-1].m_pLS && pShadowMapFrustum->pPenumbra)
    pShadowMapFrustum = pShadowMapFrustum->pPenumbra;

  Matrix44 *m = NULL;
  Vec3d vObjTrans;
  if (rd->m_RP.m_ObjFlags & FOB_TRANS_MASK)
  {
    m = &gRenDev->m_RP.m_pCurObject->m_Matrix;
    vObjTrans = rd->m_RP.m_pCurObject->GetTranslation();
  }
  else
    vObjTrans = Vec3(0,0,0);

  // setup projection
  gRenDev->SetupShadowOnlyPass(ns,    
    pShadowMapFrustum, 
    &((*lsources)[nsFrust].m_vProjTranslation), 
    (*lsources)[nsFrust].m_fProjScale,
    vObjTrans,
    1.f, 
    Vec3d(0,0,0),
    m);
}
