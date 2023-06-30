/*=============================================================================
  ShaderComponents.cpp : implementation of the common Shaders components.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "I3DEngine.h"
#include "CryHeaders.h"

#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#include <io.h>
#elif defined(LINUX)
#endif



//=================================================================================================

TArray<SArrayPointer *> SArrayPointer::m_Arrays;

void *SArrayPointer_Vertex::m_pLastPointer;
int SArrayPointer_Vertex::m_nFrameCreateBuf;

bool SArrayPointer_Vertex::mfCompile(char *scr, SShader *ef)
{
  eDst = eDstPointer_Vert;
  char *type = strtok(NULL, " ,");
  NumComponents = atol(type);

  type = strtok(NULL, " ,");
  if (!stricmp(type, "byte"))
    Type = GL_UNSIGNED_BYTE;
  else
    Type = GL_FLOAT;

  type = strtok(NULL, " ,");
  ePT = gRenDev->m_cEF.mfParseSrcPointer(type, ef);

  return true;
}


void *SArrayPointer_Normal::m_pLastPointer;
int SArrayPointer_Normal::m_nFrameCreateBuf;

bool SArrayPointer_Normal::mfCompile(char *scr, SShader *ef)
{
  eDst = eDstPointer_Normal;
  char *type = strtok(NULL, " ,");

  if (!stricmp(type, "byte"))
    Type = GL_BYTE;
  else
    Type = GL_FLOAT;

  type = strtok(NULL, " ,");
  ePT = gRenDev->m_cEF.mfParseSrcPointer(type, ef);

  return true;
}


void *SArrayPointer_Texture::m_pLastPointer[8];
int SArrayPointer_Texture::m_nFrameCreateBuf[8];

bool SArrayPointer_Texture::mfCompile(char *scr, SShader *ef)
{
  eDst = eDstPointer_Tex0;
  char *type = strtok(NULL, " ,");
  NumComponents = atol(type);

  type = strtok(NULL, " ,");
  Type = GL_FLOAT;

  type = strtok(NULL, " ,");

  char *ns = strtok(NULL, " ,");
  if (ns)
    Stage = atoi(ns);
  ePT = gRenDev->m_cEF.mfParseSrcPointer(type, ef);

  return true;
}

void *SArrayPointer_Color::m_pLastPointer;
int SArrayPointer_Color::m_nFrameCreateBuf;

bool SArrayPointer_Color::mfCompile(char *scr, SShader *ef)
{
  eDst = eDstPointer_Color;
  char *type = strtok(NULL, " ,");
  NumComponents = atol(type);

  type = strtok(NULL, " ,");
  if (!stricmp(type, "byte"))
    Type = GL_UNSIGNED_BYTE;
  else
    Type = GL_FLOAT;

  type = strtok(NULL, " ,");
  ePT = gRenDev->m_cEF.mfParseSrcPointer(type, ef);

  return true;
}

void *SArrayPointer_SecColor::m_pLastPointer;
int SArrayPointer_SecColor::m_nFrameCreateBuf;

bool SArrayPointer_SecColor::mfCompile(char *scr, SShader *ef)
{
  eDst = eDstPointer_SecColor;
  char *type = strtok(NULL, " ,");
  NumComponents = atol(type);

  type = strtok(NULL, " ,");
  if (!stricmp(type, "byte"))
    Type = GL_UNSIGNED_BYTE;
  else
    Type = GL_FLOAT;

  type = strtok(NULL, " ,");
  ePT = gRenDev->m_cEF.mfParseSrcPointer(type, ef);

  return true;
}

void CShader::mfCompileArrayPointer(TArray<SArrayPointer *>& Pointers, char *scr, SShader *ef)
{
  SArrayPointer *p;
  char *type = strtok(scr, " ,");
  if (!stricmp(type, "Verts") || !strnicmp(type, "Vertex", 6))
  {
    SArrayPointer_Vertex ap;
    if (ap.mfCompile(scr, ef))
    {
      p = SArrayPointer::AddNew(ap);
      Pointers.AddElem(p);
    }
  }
  else
  if (!strnicmp(type, "Texture", 7))
  {
    SArrayPointer_Texture ap;
    ap.Stage = atol(&type[7]);
    if (ap.mfCompile(scr, ef))
    {
      p = SArrayPointer::AddNew(ap);
      Pointers.AddElem(p);
    }
  }
  else
  if (!strnicmp(type, "Color", 5))
  {
    SArrayPointer_Color ap;
    if (ap.mfCompile(scr, ef))
    {
      p = SArrayPointer::AddNew(ap);
      Pointers.AddElem(p);
    }
  }
  else
  if (!stricmp(type, "SecColor"))
  {
    SArrayPointer_SecColor ap;
    if (ap.mfCompile(scr, ef))
    {
      p = SArrayPointer::AddNew(ap);
      Pointers.AddElem(p);
    }
  }
  else
  if (!strnicmp(type, "Normal", 6))
  {
    SArrayPointer_Normal ap;
    if (ap.mfCompile(scr, ef))
    {
      p = SArrayPointer::AddNew(ap);
      Pointers.AddElem(p);
    }
  }
  else
  {
    Warning( 0,0,"Warning: Unknown array pointer type '%s' int effector '%s'\n", type, ef->m_Name.c_str());
  }
}


SArrayPointer *SArrayPointer::AddNew(SArrayPointer& New)
{
  SArrayPointer *p;
  for (int i=0; i<m_Arrays.Num(); i++)
  {
    p = m_Arrays[i];
    if (New == *p)
      return p;
  }
  p = NULL;
  switch (New.eDst)
  {
    case eDstPointer_Vert:
      p = new SArrayPointer_Vertex;
      break;
    case eDstPointer_Normal:
      p = new SArrayPointer_Normal;
      break;
    case eDstPointer_Tex0:
      p = new SArrayPointer_Texture;
      break;
    case eDstPointer_Color:
      p = new SArrayPointer_Color;
      break;
    case eDstPointer_SecColor:
      p = new SArrayPointer_SecColor;
      break;
  }
  if (p)
  {
    *p = New;
    m_Arrays.AddElem(p);
  }
  return p;
}

ESrcPointer CShader::mfParseSrcPointer(char *parms, SShader *ef)
{
  ESrcPointer ePT = eSrcPointer_Vert;

  if (!strnicmp(parms, "Color", 5))
    ePT = eSrcPointer_Color;
  else
  if (!strnicmp(parms, "SecColor", 8))
    ePT = eSrcPointer_SecColor;
  else
  if (!strnicmp(parms, "Vertex", 6))
    ePT = eSrcPointer_Vert;
  else
  if (!stricmp(parms, "Texture0"))
    ePT = eSrcPointer_Tex;
  else
  if (!stricmp(parms, "Texture1") || !stricmp(parms, "TextureLM"))
    ePT = eSrcPointer_TexLM;
  else
  if (!strnicmp(parms, "Normal", 6))
    ePT = eSrcPointer_Normal;
  else
  if (!strnicmp(parms, "Binormal", 8))
    ePT = eSrcPointer_Binormal;
  else
  if (!strnicmp(parms, "TNormal", 7))
    ePT = eSrcPointer_TNormal;
  else
  if (!strnicmp(parms, "Tangent", 7))
    ePT = eSrcPointer_Tangent;
  else
  if (!strnicmp(parms, "LightVector", 11))
    ePT = eSrcPointer_LightVector;
  else
  if (!strnicmp(parms, "TerrainLightVector", 18))
    ePT = eSrcPointer_LightVector_Terrain;
  else
  if (!strnicmp(parms, "NormLightVector", 15))
    ePT = eSrcPointer_NormLightVector;
  else
  if (!strnicmp(parms, "Refract", 7))
    ePT = eSrcPointer_Refract;
  else
  if (!stricmp(parms, "Project"))
    ePT = eSrcPointer_Project;
  else
  if (!stricmp(parms, "ProjectTexture"))
    ePT = eSrcPointer_ProjectTexture;
  else
  if (!strnicmp(parms, "HalfAngle", 9))
    ePT = eSrcPointer_HalfAngleVector;
  else
  if (!strnicmp(parms, "TerrainHalfAngle", 16))
    ePT = eSrcPointer_HalfAngleVector_Terrain;
  else
  if (!strnicmp(parms, "Attenuation", 11))
    ePT = eSrcPointer_Attenuation;
  else
  if (!stricmp(parms, "LAttenuationSpec0"))
    ePT = eSrcPointer_LAttenuationSpec0;
  else
  if (!stricmp(parms, "LAttenuationSpec1"))
    ePT = eSrcPointer_LAttenuationSpec1;
  else
  if (!stricmp(parms, "TerrainLAttenuationSpec0"))
    ePT = eSrcPointer_LAttenuationSpec0_Terrain;
  else
  if (!stricmp(parms, "TerrainLAttenuationSpec1"))
    ePT = eSrcPointer_LAttenuationSpec1_Terrain;
  else
  if (!stricmp(parms, "Detail"))
    ePT = eSrcPointer_Detail;
  else
  if (!stricmp(parms, "ProjectAttenFromCamera"))
    ePT = eSrcPointer_ProjectAttenFromCamera;
  else
  {
    Warning( 0,0,"Warning: Unknown Pointer type '%s' in Shader '%s'\n", parms, ef->m_Name.c_str());
  }

  return ePT;
}


//=================================================================================
// Components

TArray<SParamComp *> SParamComp::m_ParamComps;
vec4_t SParam::m_sFVals;

bool CShader::mfGetParmComps(int comp, SParam *vpp, char *name, char *params, SShader *ef)
{
  if (comp >= 4 || comp < 0)
    return false;
  if (!vpp)
    return false;
  if (!name || !name[0])
  {
    SParamComp_Const p;
    p.m_Val = shGetFloat(params);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "Wave"))
  {
    SParamComp_Wave p;
    gRenDev->m_cEF.mfCompileWaveForm(&p.m_WF, params);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "ObjWaveX"))
  {
    SParamComp_ObjWave p;
    p.m_bObjX = true;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  if (!stricmp(name, "ObjWaveY"))
  {
    SParamComp_ObjWave p;
    p.m_bObjX = false;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "FromRE", 6))
  {
    SParamComp_FromRE p;
    if (strlen(name) > 6)
      p.m_Offs = atol(&name[7]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "REColor", 7))
  {
    SParamComp_REColor p;
    p.m_Offs = atol(&name[8]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "ObjRefrFactor"))
  {
    SParamComp_ObjRefrFactor p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "FromObject", 10))
  {
    SParamComp_FromObject p;
    p.m_Offs = atol(&name[11]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "Time", 4))
  {
    SParamComp_Time p;
    char s[64];
    float sc;
    int n = sscanf(name, "%s %f", s, &sc);
    if (n == 2)
      p.m_Scale = sc;
    else
      p.m_Scale = 1;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "Distance", 4))
  {
    SParamComp_Distance p;
    char s[64];
    float sc;
    int n = sscanf(name, "%s %f", s, &sc);
    if (n == 2)
      p.m_Scale = sc;
    else
      p.m_Scale = 1;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "MinDistance"))
  {
    SParamComp_MinDistance p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "MinLightDistance"))
  {
    SParamComp_MinLightDistance p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "Random", 4))
  {
    SParamComp_Random p;
    char s[64];
    float sc;
    int n = sscanf(name, "%s %f", s, &sc);
    if (n == 2)
      p.m_Scale = sc;
    else
      p.m_Scale = 1;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "SunDirect", 9))
  {
    SParamComp_SunDirect p;
    p.m_Offs = atol(&name[10]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "SunFlarePos", 11))
  {
    SParamComp_SunFlarePos p;
    p.m_Offs = atol(&name[12]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "LightPos", 8))
  {
    SParamComp_LightPos p;
    p.m_Offs = atol(&name[9]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "LightBright", 11))
  {
    SParamComp_LightBright p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "LightDirectFactor", 17))
  {
    SParamComp_LightDirectFactor p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "LightIntens", 11))
  {
    SParamComp_LightIntens p;
    p.m_bInv = 0;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "InvLightIntens", 13))
  {
    SParamComp_LightIntens p;
    p.m_bInv = 1;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "LightsNum"))
  {
    SParamComp_LightsNum p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "OSLightPos", 10))
  {
    SParamComp_OSLightPos p;
    p.m_Offs = atol(&name[11]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "SLightPos", 9))
  {
    SParamComp_SLightPos p;
    p.m_Offs = atol(&name[10]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "SCameraPos", 10))
  {
    SParamComp_SCameraPos p;
    p.m_Offs = atol(&name[11]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "LightOcclusion", 14))
  {
    SParamComp_LightOcclusion p;
    p.m_Offs = atol(&name[15]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "VolFogColor", 11))
  {
    SParamComp_VolFogColor p;
    p.m_Offs = atol(&name[12]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "VolFogDensity", 13))
  {
    SParamComp_VolFogDensity p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "CameraAngle", 11))
  {
    SParamComp_CameraAngle p;
    char s[64];
    char op;
    char sign[8];
    int offs;
    float oper;
    int n = sscanf(name, "%s %s %d %c %f", s, sign, &offs, &op, &oper);
    if (!strnicmp(sign, "neg", 3))
      p.m_Sign = -1;
    else
      p.m_Sign = 1;
    p.m_Offs = offs;
    if (n == 5)
    {
      if (op == '+')
        p.m_Op = 1;
      else
      if (op == '*')
        p.m_Op = 2;
      else
      if (op == '/')
        p.m_Op = 3;
      else
      if (op == '-')
        p.m_Op = 4;
      else
        p.m_Op = 0;
      p.m_Operand = oper;
    }
    else
      p.m_Op = 0;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "CameraPos", 9))
  {
    SParamComp_CameraPos p;
    char s[64];
    char op;
    char sign[8];
    int offs;
    float oper;
    int n = sscanf(name, "%s %s %d %c %f", s, sign, &offs, &op, &oper);
    if (!strnicmp(sign, "neg", 3))
      p.m_Sign = -1;
    else
      p.m_Sign = 1;
    p.m_Offs = offs;
    if (n == 5)
    {
      if (op == '+')
        p.m_Op = 1;
      else
      if (op == '*')
        p.m_Op = 2;
      else
      if (op == '/')
        p.m_Op = 3;
      else
      if (op == '-')
        p.m_Op = 4;
      else
        p.m_Op = 0;
      p.m_Operand = oper;
    }
    else
      p.m_Op = 0;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "ObjPos", 6))
  {
    SParamComp_ObjPos p;
    char s[64];
    char op;
    char sign[8];
    int offs;
    float oper;
    int n = sscanf(name, "%s %s %d %c %f", s, sign, &offs, &op, &oper);
    if (!strnicmp(sign, "neg", 3))
      p.m_Sign = -1;
    else
      p.m_Sign = 1;
    p.m_Offs = offs;
    if (n == 5)
    {
      if (op == '+')
        p.m_Op = 1;
      else
      if (op == '*')
        p.m_Op = 2;
      else
      if (op == '/')
        p.m_Op = 3;
      else
      if (op == '-')
        p.m_Op = 4;
      else
        p.m_Op = 0;
      p.m_Operand = oper;
    }
    else
      p.m_Op = 0;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "OSCameraPos", 11))
  {
    SParamComp_OSCameraPos p;
    char s[64];
    char op;
    char sign[8];
    int offs;
    float oper;
    int n = sscanf(name, "%s %s %d %c %f", s, sign, &offs, &op, &oper);
    if (!strnicmp(sign, "neg", 3))
      p.m_Sign = -1;
    else
      p.m_Sign = 1;
    p.m_Offs = offs;
    if (n == 5)
    {
      if (op == '+')
        p.m_Op = 1;
      else
      if (op == '*')
        p.m_Op = 2;
      else
      if (op == '/')
        p.m_Op = 3;
      else
      if (op == '-')
        p.m_Op = 4;
      else
        p.m_Op = 0;
      p.m_Operand = oper;
    }
    else
      p.m_Op = 0;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "SunColor", 8))
  {
    SParamComp_SunColor p;
    p.m_Offs = atol(&name[9]);
    char s[64];
    float mult = 1.0f;
    int n = sscanf(name, "%s %f", s, &mult);
    p.m_Mult = mult;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "WorldColor", 10))
  {
    SParamComp_WorldColor p;
    p.m_Offs = atol(&name[11]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "WaterLevel"))
  {
    SParamComp_WaterLevel p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "WorldObjColor", 13))
  {
    SParamComp_WorldColor p;
    p.m_Offs = atol(&name[14]) | 0x80000000;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "ObjVal", 6))
  {
    SParamComp_ObjVal p;
    p.m_Offs = atol(&name[7]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "GeomCenter", 10))
  {
    SParamComp_GeomCenter p;
    p.m_Offs = atol(&name[11]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "Bending", 7))
  {
    SParamComp_Bending p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "LightColor", 10))
  {
    SParamComp_LightColor p;
    p.m_Offs = atol(&name[11]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "DiffuseColor", 12))
  {
    SParamComp_DiffuseColor p;
    p.m_Offs = atol(&name[13]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "SpecularPower"))
  {
    SParamComp_SpecularPower p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "ObjColor", 8))
  {
    SParamComp_ObjColor p;
    p.m_Offs = atol(&name[9]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "SpecLightColor", 14))
  {
    SParamComp_SpecLightColor p;
    p.m_Offs = atol(&name[15]);
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "AmbLightColor", 13))
  {
    SParamComp_AmbLightColor p;
    p.m_Offs = atol(&name[14]);
    char s[64];
    float mult;
    int n = sscanf(name, "%s %f", s, &mult);
    if (n == 2)
      p.m_fMul = mult;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "FlashBangBrightness"))
  {
    SParamComp_FlashBangBrightness p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "ScreenSize", 10))
  {
    SParamComp_ScreenSize p;
    char s[64];
    char op;
    char sign[8];
    int offs;
    float oper;
    int n = sscanf(name, "%s %s %d %c %f", s, sign, &offs, &op, &oper);
    if (!strnicmp(sign, "neg", 3))
      p.m_Sign = -1;
    else
      p.m_Sign = 1;
    p.m_Offs = offs;
    if (n == 5)
    {
      if (op == '+')
        p.m_Op = 1;
      else
        if (op == '*')
          p.m_Op = 2;
        else
          if (op == '/')
            p.m_Op = 3;
          else
            if (op == '-')
              p.m_Op = 4;
            else
              p.m_Op = 0;
      p.m_Operand = oper;
    }
    else
      p.m_Op = 0;

    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "DofFocalParams", 14))
  {
    SParamComp_DofFocalParams p;
    char s[64];
    char op;
    char sign[8];
    int offs;
    float oper;
    int n = sscanf(name, "%s %s %d %c %f", s, sign, &offs, &op, &oper);
    if (!strnicmp(sign, "neg", 3))
      p.m_Sign = -1;
    else
      p.m_Sign = 1;
    p.m_Offs = offs;
    if (n == 5)
    {
      if (op == '+')
        p.m_Op = 1;
      else
        if (op == '*')
          p.m_Op = 2;
        else
          if (op == '/')
            p.m_Op = 3;
          else
            if (op == '-')
              p.m_Op = 4;
            else
              p.m_Op = 0;
      p.m_Operand = oper;
    }
    else
      p.m_Op = 0;

    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "BumpAmount", 10) || !strnicmp(name, "BumpScale", 9))
  {
    SParamComp_BumpAmount p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "ObjScale"))
  {
    SParamComp_ObjScale p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "FogStart"))
  {
    SParamComp_Fog p;
    p.m_Type = 1;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "FogEnd"))
  {
    SParamComp_Fog p;
    p.m_Type = 2;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!stricmp(name, "FogRange"))
  {
    SParamComp_Fog p;
    p.m_Type = 3;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "HeatFactor", 10))
  {
    SParamComp_HeatFactor p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  else
  if (!strnicmp(name, "Opacity", 7))
  {
    SParamComp_Opacity p;
    vpp->m_Comps[comp] = SParamComp::mfAdd(&p);
    return true;
  }
  Warning( 0,0,"Warning: Unknown component type '%s' for params section in Shader '%s'\n", name, ef->m_Name.c_str());
  return false;
}

bool CShader::mfCompilePlantsTMoving(char *scr, SShader *ef, TArray<SParam>* Params, int reg)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  int comp = 0;

  enum {eWaveX = 1, eWaveY};
  static tokenDesc commands[] =
  {
    {eWaveX, "WaveX"},
    {eWaveY, "WaveY"},

    {0,0}
  };

  SParamComp_PlantsTMoving pr;

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;

    switch (cmd)
    {
      case eWaveX:
        gRenDev->m_cEF.mfCompileWaveForm(&pr.m_WFX, params);
        break;

      case eWaveY:
        gRenDev->m_cEF.mfCompileWaveForm(&pr.m_WFY, params);
        break;
    }
  }

  SParam vpp;
  for (int i=0; i<2; i++)
  {
    memset(&vpp, 0, sizeof(vpp));
    vpp.m_Reg = reg+i;
    for (int j=0; j<4; j++)
    {
      pr.m_Offs = j | (i<<4);
      vpp.m_Comps[j] = SParamComp::mfAdd(&pr);
    }
    Params->AddElem(vpp);
  }

  return true;
}

#define GL_CONSTANT_COLOR0_NV               0x852A
#define GL_CONSTANT_COLOR1_NV               0x852B
#define GL_OFFSET_TEXTURE_2D_MATRIX_NV      0x86E1

void CShader::mfCompileParamComps(SParam *pr, char *scr, SShader *ef)
{
  TArray<SParam> Parms;
  mfCompileParam(scr, ef, &Parms);
  if (Parms.Num())
  {
    *pr = Parms[0];
  }
}

bool CShader::mfCompileCGParam(char *scr, SShader *ef, TArray<SCGParam4f>* Params)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eName=1};
  static tokenDesc commands[] =
  {
    {eName, "Name"},

    {0,0}
  };

  SCGParam4f pr;
  pr.m_eCGParamType = ECGP_Float4;
  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;

    switch (cmd)
    {
      case eName:
        {
          if (data)
          {
            pr.m_Name = data;
            TArray<SParam> TempParms;
            mfCompileParam(scr, ef, &TempParms);
            if (TempParms.Num() == 1)
            {
              pr.m_nComponents = 1;
              pr.m_Flags |= (TempParms[0].m_Flags & ~PF_CGTYPE);
              pr.m_Comps[0] = TempParms[0].m_Comps[0];
              pr.m_Comps[1] = TempParms[0].m_Comps[1];
              pr.m_Comps[2] = TempParms[0].m_Comps[2];
              pr.m_Comps[3] = TempParms[0].m_Comps[3];
              Params->AddElem(pr);
            }
            else
            if (TempParms.Num() > 1)
            {
              for (int i=0; i<TempParms.Num(); i++)
              {
                if (i)
                {
                  pr.m_nComponents = 0;
                  pr.m_Name.mfClear();
                }
                else
                  pr.m_nComponents = TempParms.Num();
                pr.m_Flags |= (TempParms[i].m_Flags & ~PF_CGTYPE);
                pr.m_Comps[0] = TempParms[i].m_Comps[0];
                pr.m_Comps[1] = TempParms[i].m_Comps[1];
                pr.m_Comps[2] = TempParms[i].m_Comps[2];
                pr.m_Comps[3] = TempParms[i].m_Comps[3];
                Params->AddElem(pr);
              }
            }
          }
        }
        break;
    }
    break;
  }

  return false;
}

void CShader::mfCompileParamMatrix(char *scr, SShader *ef, SParamComp_Matrix *pcm)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  int comp = 0;

  TArray<SMatrixTransform> *List = &pcm->m_Transforms;

  enum {eIdentity = 1, eTranslate, eRotateX, eRotateY, eRotateZ, eRotate_XY, eRotate_XZ, eRotate_YZ, eScale, eMatrix};
  static tokenDesc commands[] =
  {
    {eIdentity, "Identity"},
    {eTranslate, "Translate"},
    {eRotateX, "RotateX"},
    {eRotateY, "RotateY"},
    {eRotateZ, "RotateZ"},
    {eRotate_XY, "Rotate_XY"},
    {eRotate_XZ, "Rotate_XZ"},
    {eRotate_YZ, "Rotate_YZ"},
    {eScale, "Scale"},
    {eMatrix, "Matrix"},

    {0,0}
  };

  int Matr = 0;
  int Stage = 0;
  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;

    switch (cmd)
    {
      case eIdentity:
        {
          SMatrixTransform_Identity mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;

      case eMatrix:
        {
          SMatrixTransform_Matrix mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          int n = List->Num();
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          for (int i=0; i<Params.Num(); i++)
          {
            mt.m_Params[i] = Params[i];
          }
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
      case eTranslate:
        {
          SMatrixTransform_Translate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;

      case eScale:
        {
          SMatrixTransform_Scale mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;

      case eRotateX:
        {
          SMatrixTransform_Rotate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          mt.m_Offs = 1;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
      case eRotateY:
        {
          SMatrixTransform_Rotate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          mt.m_Offs = 2;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
      case eRotateZ:
        {
          SMatrixTransform_Rotate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          mt.m_Offs = 4;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
      case eRotate_XY:
        {
          SMatrixTransform_Rotate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          mt.m_Offs = 3;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
      case eRotate_XZ:
        {
          SMatrixTransform_Rotate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          mt.m_Offs = 5;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
      case eRotate_YZ:
        {
          SMatrixTransform_Rotate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          mt.m_Offs = 6;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
    }
  }
}

bool CShader::mfCompileParam(char *scr, SShader *ef, TArray<SParam>* Params)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  int comp = 0;

  enum {eReg = 1, eAll, eUser, eUsrTime, eComp, eObjMatrix, eInvObjMatrix, eObjInvMatrix, eTranspObjInvMatrix, eTranspInvObjMatrix, eTranspObjMatrix, eCameraMatrix, eInvCameraMatrix, eCameraInvMatrix, eTranspCameraInvMatrix, eTranspInvCameraMatrix, eTranspCameraMatrix, eTexViewProjMatrix, eTranspTexViewProjMatrix, eWaterProjMatrix, eTranspWaterProjMatrix, eLightMatrix, eTranspLightMatrix, eFromRE0, eFromRE1, eFromRE2, eFromRE3, eTempMatr0, eTempMatr1, eTempMatr2, eTempMatr3, eTemp2Matr0, eTemp2Matr1, eTemp2Matr2, eTemp2Matr3, ePlantsTMoving, eMatrix, eVFogMatrix, eFogMatrix, eFogEnterMatrix, eTranspMatrix, eEnvColor, eLightsColor, eSpecLightsColor, eLightsIntens, eOSLightsPos, eLightsType, eOcclusionMasks};
  static tokenDesc commands[] =
  {
    {eReg, "Reg"},
    {eAll, "All"},
    {eComp, "Comp"},

    {ePlantsTMoving, "PlantsTMoving"},

    {eFromRE0, "FromRE0"},
    {eFromRE1, "FromRE1"},
    {eFromRE2, "FromRE2"},
    {eFromRE3, "FromRE3"},

    {eTempMatr0, "TempMatr0"},
    {eTempMatr1, "TempMatr1"},
    {eTempMatr2, "TempMatr2"},
    {eTempMatr3, "TempMatr3"},

    {eTemp2Matr0, "Temp2Matr0"},
    {eTemp2Matr1, "Temp2Matr1"},
    {eTemp2Matr2, "Temp2Matr2"},
    {eTemp2Matr3, "Temp2Matr3"},

    {eUser, "User"},
    {eUsrTime, "UsrTime"},

    {eObjMatrix, "ObjMatrix"},
    {eInvObjMatrix, "InvObjMatrix"},
    {eObjInvMatrix, "ObjInvMatrix"},
    {eTranspObjMatrix, "TranspObjMatrix"},
    {eTranspInvObjMatrix, "TranspInvObjMatrix"},
    {eTranspObjInvMatrix, "TranspObjInvMatrix"},

    {eCameraMatrix, "CameraMatrix"},
    {eInvCameraMatrix, "InvCameraMatrix"},
    {eCameraInvMatrix, "CameraInvMatrix"},
    {eTranspCameraMatrix, "TranspCameraMatrix"},
    {eTranspInvCameraMatrix, "TranspInvCameraMatrix"},
    {eTranspCameraInvMatrix, "TranspCameraInvMatrix"},

    {eEnvColor, "EnvColor"},
    {eMatrix, "Matrix"},
    {eTranspMatrix, "TranspMatrix"},

    {eLightMatrix, "LightMatrix"},
    {eTranspLightMatrix, "TranspLightMatrix"},

    {eWaterProjMatrix, "WaterProjMatrix"},
    {eTranspWaterProjMatrix, "TranspWaterProjMatrix"},

    {eTexViewProjMatrix, "TexViewProjMatrix"},
    {eTranspTexViewProjMatrix, "TranspTexViewProjMatrix"},

    {eFogMatrix, "FogMatrix"},
    {eVFogMatrix, "VFogMatrix"},
    {eFogEnterMatrix, "FogEnterMatrix"},

    {eLightsColor, "LightsColor"},
    {eSpecLightsColor, "SpecLightsColor"},
    {eLightsIntens, "LightsIntens"},
    {eLightsType, "LightsType"},
    {eOSLightsPos, "OSLightsPos"},
    {eOcclusionMasks, "OcclusionMasks"},

    {0,0}
  };

  SParam vpp;
  memset(&vpp, 0, sizeof(vpp));
  int n = Params->Num();

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;

    switch (cmd)
    {
      case eReg:
        {
          if (!data || !data[0])
          {
            Warning( 0,0,"Warning: Missing Reg parameter for 'Params' section in Shader '%s'\n", ef->m_Name.c_str());
            break;
          }
        }
        if (!name || !name[0])
          vpp.m_Reg = shGetInt(data);
        else
        {
          if (!stricmp(name, "ConstColor0") || !stricmp(name, "GL_CONSTANT_COLOR0_NV"))
            vpp.m_Reg = GL_CONSTANT_COLOR0_NV;
          else
          if (!stricmp(name, "ConstColor1") || !stricmp(name, "GL_CONSTANT_COLOR1_NV"))
            vpp.m_Reg = GL_CONSTANT_COLOR1_NV;
          else
          if (!stricmp(name, "GL_OFFSET_TEXTURE_2D_MATRIX_NV") || !stricmp(name, "OFFSET_TEXTURE_2D_MATRIX_NV"))
            vpp.m_Reg = GL_OFFSET_TEXTURE_2D_MATRIX_NV;
          else
          {
            Warning( 0,0,"Warning: Unknown component type '%s' in Shader '%s'\n", name, ef->m_Name.c_str());
            break;
          }
        }
        break;

      case eAll:
        {
          for (int i=comp; i<4; i++)
          {
            mfGetParmComps(i, &vpp, name, params, ef);
          }
        }
        break;

      case eLightsColor:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Flags |= PF_CANMERGED;
            vpp.m_Reg = reg+i;
            SParamComp_LightsColor pc;
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eSpecLightsColor:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Flags |= PF_CANMERGED;
            vpp.m_Reg = reg+i;
            SParamComp_SpecLightsColor pc;
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eOcclusionMasks:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Flags |= PF_CANMERGED;
            vpp.m_Reg = reg+i;
            SParamComp_LightOcclusions pc;
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eOSLightsPos:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Flags |= PF_CANMERGED;
            vpp.m_Reg = reg+i;
            SParamComp_OSLightsPos pc;
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eLightsIntens:
        {
          int reg = vpp.m_Reg;
          memset(&vpp, 0, sizeof(vpp));
          vpp.m_Flags |= PF_CANMERGED;
          vpp.m_Reg = reg;
          SParamComp_LightsIntens pc;
          vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
          Params->AddElem(vpp);
        }
        break;

      case eLightsType:
        {
          int reg = vpp.m_Reg;
          memset(&vpp, 0, sizeof(vpp));
          vpp.m_Flags |= PF_CANMERGED;
          vpp.m_Reg = reg;
          SParamComp_LightsType pc;
          vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
          Params->AddElem(vpp);
        }
        break;

      case eEnvColor:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<6; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Flags |= PF_CANMERGED;
            vpp.m_Reg = reg+i;
            SParamComp_EnvColor pc;
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eFogMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<2; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Flags |= PF_CANMERGED;
            vpp.m_Reg = reg+i;
            SParamComp_FogMatrix pc;
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eVFogMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<2; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Flags |= PF_CANMERGED;
            vpp.m_Reg = reg+i;
            SParamComp_VFogMatrix pc;
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eFogEnterMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<2; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Flags |= PF_CANMERGED;
            vpp.m_Reg = reg+i;
            SParamComp_FogEnterMatrix pc;
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eMatrix:
        {
          int reg = vpp.m_Reg;
          memset(&vpp, 0, sizeof(vpp));
          vpp.m_Flags |= PF_CANMERGED;
          SParamComp_Matrix pc;
          mfCompileParamMatrix(params, ef, &pc);
          for (int i=0; i<4; i++)
          {
            pc.m_Offs = i;
            vpp.m_Reg = reg+i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eTranspMatrix:
        {
          int reg = vpp.m_Reg;
          memset(&vpp, 0, sizeof(vpp));
          vpp.m_Flags |= PF_CANMERGED;
          SParamComp_Matrix pc;
          mfCompileParamMatrix(params, ef, &pc);
          for (int i=0; i<4; i++)
          {
            pc.m_Offs = i | 0x40000000;
            vpp.m_Reg = reg+i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eObjMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_ObjMatrix pc;
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eCameraMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            for (int j=0; j<4; j++)
            {
              SParamComp_CameraMatrix pc;
              pc.m_Offs = j | (i<<4);
              vpp.m_Comps[j] = SParamComp::mfAdd(&pc);
            }
            Params->AddElem(vpp);
          }
        }
        break;

      case eInvObjMatrix:
      case eObjInvMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_ObjMatrix pc;
            pc.m_Offs = (0x80000000 | i);
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eInvCameraMatrix:
      case eCameraInvMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            for (int j=0; j<4; j++)
            {
              SParamComp_CameraMatrix pc;
              pc.m_Offs = (0x80000000 | j | (i<<4));
              vpp.m_Comps[j] = SParamComp::mfAdd(&pc);
            }
            Params->AddElem(vpp);
          }
        }
        break;

      case eTranspObjMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_ObjMatrix pc;
            pc.m_Offs = (0x40000000 | i);
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eTranspCameraMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            for (int j=0; j<4; j++)
            {
              SParamComp_CameraMatrix pc;
              pc.m_Offs = (0x40000000 | j | (i<<4));
              vpp.m_Comps[j] = SParamComp::mfAdd(&pc);
            }
            Params->AddElem(vpp);
          }
        }
        break;

      case eTranspInvObjMatrix:
      case eTranspObjInvMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_ObjMatrix pc;
            pc.m_Offs = (0xc0000000 | i);
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eTranspInvCameraMatrix:
      case eTranspCameraInvMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            for (int j=0; j<4; j++)
            {
              SParamComp_CameraMatrix pc;
              pc.m_Offs = (0xc0000000 | j | (i<<4));
              vpp.m_Comps[j] = SParamComp::mfAdd(&pc);
            }
            Params->AddElem(vpp);
          }
        }
        break;

      case eWaterProjMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            for (int j=0; j<4; j++)
            {
              SParamComp_WaterProjMatrix pc;
              pc.m_Offs = j | (i<<4);
              vpp.m_Comps[j] = SParamComp::mfAdd(&pc);
            }
            Params->AddElem(vpp);
          }
        }
        break;

      case eTranspWaterProjMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            for (int j=0; j<4; j++)
            {
              SParamComp_WaterProjMatrix pc;
              pc.m_Offs = (0x40000000 | j | (i<<4));
              vpp.m_Comps[j] = SParamComp::mfAdd(&pc);
            }
            Params->AddElem(vpp);
          }
        }
        break;

      case eTexViewProjMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_TexProjMatrix pc;
            if (data)
              pc.m_Stage = atoi(data);
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eTranspTexViewProjMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_TexProjMatrix pc;
            if (data)
              pc.m_Stage = atoi(data);
            pc.m_Offs = (0x40000000 | i);
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eLightMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_LightMatrix pc;
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case eTranspLightMatrix:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_LightMatrix pc;
            pc.m_Offs = i | 0x40000000;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
        }
        break;

      case ePlantsTMoving:
        mfCompilePlantsTMoving(params, ef, Params, vpp.m_Reg);
        break;

      case eUser:
      case eUsrTime:
        {
          if (comp < 4)
          {
            int reg = vpp.m_Reg;
            char name[128];
            strcpy(name, data);
            strlwr(name);
            int i;
            for (i=0; i<ef->m_PublicParams.Num(); i++)
            {
              if (!strncmp(ef->m_PublicParams[i].m_Name, name, strlen(ef->m_PublicParams[i].m_Name)))
                break;
            }
            if (i == ef->m_PublicParams.Num())
              iLog->Log("WARNING: Coudn't find declaration of the public parameter '%s' for shader '%s'\n", name, ef->m_Name.c_str());
            //else
            {
              SParamComp_User pr;
              pr.m_Name = name;
              if (cmd == eUsrTime)
                pr.m_bTimeModulate = true;
              else
                pr.m_bTimeModulate = false;
              vpp.m_Comps[comp] = SParamComp::mfAdd(&pr);
              comp++;
            }
          }
        }
        break;

      case eFromRE0:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            for (int j=0; j<4; j++)
            {
              SParamComp_FromRE pc;
              pc.m_Offs = j + (i*4);
              vpp.m_Comps[j] = SParamComp::mfAdd(&pc);
            }
            Params->AddElem(vpp);
          }
        }
        break;

      case eFromRE1:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            for (int j=0; j<4; j++)
            {
              SParamComp_FromRE pc;
              pc.m_Offs = (j + (i*4))+16*1;
              vpp.m_Comps[j] = SParamComp::mfAdd(&pc);
            }
            Params->AddElem(vpp);
          }
        }
        break;

      case eFromRE2:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            for (int j=0; j<4; j++)
            {
              SParamComp_FromRE pc;
              pc.m_Offs = (j + (i*4))+16*2;
              vpp.m_Comps[j] = SParamComp::mfAdd(&pc);
            }
            Params->AddElem(vpp);
          }
        }
        break;

      case eFromRE3:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            for (int j=0; j<4; j++)
            {
              SParamComp_FromRE pc;
              pc.m_Offs = (j + (i*4))+16*3;
              vpp.m_Comps[j] = SParamComp::mfAdd(&pc);
            }
            Params->AddElem(vpp);
          }
        }
        break;

      case eTempMatr0:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_TempMatr pc;
            pc.m_MatrID0 = 0;
            pc.m_MatrID1 = atol(&scr[1]);
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
          scr += 3;
        }
        break;

      case eTempMatr1:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_TempMatr pc;
            pc.m_MatrID0 = 1;
            pc.m_MatrID1 = atol(&scr[1]);
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
          scr += 3;
        }
        break;

      case eTempMatr2:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_TempMatr pc;
            pc.m_MatrID0 = 2;
            pc.m_MatrID1 = atol(&scr[1]);
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
          scr += 3;
        }
        break;

      case eTempMatr3:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<4; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_TempMatr pc;
            pc.m_MatrID0 = 3;
            pc.m_MatrID1 = atol(&scr[1]);
            pc.m_Offs = i;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
          scr += 3;
        }
        break;

      case eTemp2Matr0:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<2; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_TempMatr pc;
            pc.m_MatrID0 = 0;
            pc.m_MatrID1 = atol(&scr[1]);
            pc.m_Offs = i+2;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
          scr += 3;
        }
        break;

      case eTemp2Matr1:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<2; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_TempMatr pc;
            pc.m_MatrID0 = 1;
            pc.m_MatrID1 = atol(&scr[1]);
            pc.m_Offs = i+2;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
          scr += 3;
        }
        break;

      case eTemp2Matr2:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<2; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_TempMatr pc;
            pc.m_MatrID0 = 2;
            pc.m_MatrID1 = atol(&scr[1]);
            pc.m_Offs = i+2;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
          scr += 3;
        }
        break;

      case eTemp2Matr3:
        {
          int reg = vpp.m_Reg;
          for (int i=0; i<2; i++)
          {
            memset(&vpp, 0, sizeof(vpp));
            vpp.m_Reg = reg+i;
            vpp.m_Flags |= PF_CANMERGED;
            SParamComp_TempMatr pc;
            pc.m_MatrID0 = 3;
            pc.m_MatrID1 = atol(&scr[1]);
            pc.m_Offs = i+2;
            vpp.m_Comps[0] = SParamComp::mfAdd(&pc);
            Params->AddElem(vpp);
          }
          scr += 3;
        }
        break;

      case eComp:
        if (comp < 4)
        {
          mfGetParmComps(comp, &vpp, name, params, ef);
          comp++;
        }
        break;
    }
  }
  if (n == Params->Num())
    Params->AddElem(vpp);
  for (int i=n; i<Params->Num(); i++)
  {
    Params->Get(i).mfOptimize();
  }

  return true;
}



SParamComp *SParamComp::mfAdd(SParamComp *pPC)
{
  for (int i=0; i<m_ParamComps.Num(); i++)
  {
    SParamComp *p = m_ParamComps[i];
    if (p->m_eType == pPC->m_eType && p->mfIsEqual(pPC))
      return p;
  }
  SParamComp *pc = NULL;
  switch (pPC->m_eType)
  {
    case EParamComp_BumpAmount:
      pc = new SParamComp_BumpAmount;
      break;
    case EParamComp_PlantsTMoving:
      pc = new SParamComp_PlantsTMoving;
      break;
    case EParamComp_EnvColor:
      pc = new SParamComp_EnvColor;
      break;
    case EParamComp_ObjMatrix:
      pc = new SParamComp_ObjMatrix;
      break;
    case EParamComp_ObjScale:
      pc = new SParamComp_ObjScale;
      break;
    case EParamComp_CameraMatrix:
      pc = new SParamComp_CameraMatrix;
      break;
    case EParamComp_User:
      pc = new SParamComp_User;
      break;
    case EParamComp_Matrix:
      pc = new SParamComp_Matrix;
      break;
    case EParamComp_LightMatrix:
      pc = new SParamComp_LightMatrix;
      break;
    case EParamComp_FogMatrix:
      pc = new SParamComp_FogMatrix;
      break;
    case EParamComp_VFogMatrix:
      pc = new SParamComp_VFogMatrix;
      break;
    case EParamComp_FogEnterMatrix:
      pc = new SParamComp_FogEnterMatrix;
      break;
    case EParamComp_WaterProjMatrix:
      pc = new SParamComp_WaterProjMatrix;
      break;
    case EParamComp_WaterLevel:
      pc = new SParamComp_WaterLevel;
      break;
    case EParamComp_TexProjMatrix:
      pc = new SParamComp_TexProjMatrix;
      break;
    case EParamComp_Const:
      pc = new SParamComp_Const;
      break;
    case EParamComp_Wave:
      pc = new SParamComp_Wave;
      break;
    case EParamComp_ObjWave:
      pc = new SParamComp_ObjWave;
      break;
    case EParamComp_SunDirect:
      pc = new SParamComp_SunDirect;
      break;
    case EParamComp_SunFlarePos:
      pc = new SParamComp_SunFlarePos;
      break;
    case EParamComp_LightPos:
      pc = new SParamComp_LightPos;
      break;
    case EParamComp_LightIntens:
      pc = new SParamComp_LightIntens;
      break;
    case EParamComp_LightDirectFactor:
      pc = new SParamComp_LightDirectFactor;
      break;
    case EParamComp_LightBright:
      pc = new SParamComp_LightBright;
      break;
    case EParamComp_OSLightPos:
      pc = new SParamComp_OSLightPos;
      break;
    case EParamComp_SLightPos:
      pc = new SParamComp_SLightPos;
      break;
    case EParamComp_SCameraPos:
      pc = new SParamComp_SCameraPos;
      break;
    case EParamComp_LightOcclusion:
      pc = new SParamComp_LightOcclusion;
      break;
    case EParamComp_LightOcclusions:
      pc = new SParamComp_LightOcclusions;
      break;
    case EParamComp_VolFogColor:
      pc = new SParamComp_VolFogColor;
      break;
    case EParamComp_FlashBangBrightness:
      pc = new SParamComp_FlashBangBrightness;
      break;
    case EParamComp_ScreenSize:
      pc = new SParamComp_ScreenSize;
      break;
    case EParamComp_DofFocalParams:
      pc = new SParamComp_DofFocalParams;
      break;
    case EParamComp_VolFogDensity:
      pc = new SParamComp_VolFogDensity;
      break;
    case EParamComp_FromRE:
      pc = new SParamComp_FromRE;
      break;
    case EParamComp_REColor:
      pc = new SParamComp_REColor;
      break;
    case EParamComp_TempMatr:
      pc = new SParamComp_TempMatr;
      break;
    case EParamComp_ObjRefrFactor:
      pc = new SParamComp_ObjRefrFactor;
      break;
    case EParamComp_FromObject:
      pc = new SParamComp_FromObject;
      break;
    case EParamComp_WorldColor:
      pc = new SParamComp_WorldColor;
      break;
    case EParamComp_SunColor:
      pc = new SParamComp_SunColor;
      break;
    case EParamComp_ObjVal:
      pc = new SParamComp_ObjVal;
      break;
    case EParamComp_GeomCenter:
      pc = new SParamComp_GeomCenter;
      break;
    case EParamComp_Bending:
      pc = new SParamComp_Bending;
      break;
    case EParamComp_ObjColor:
      pc = new SParamComp_ObjColor;
      break;
    case EParamComp_LightColor:
      pc = new SParamComp_LightColor;
      break;
    case EParamComp_DiffuseColor:
      pc = new SParamComp_DiffuseColor;
      break;
    case EParamComp_SpecularPower:
      pc = new SParamComp_SpecularPower;
      break;
    case EParamComp_SpecLightColor:
      pc = new SParamComp_SpecLightColor;
      break;
    case EParamComp_AmbLightColor:
      pc = new SParamComp_AmbLightColor;
      break;
    case EParamComp_CameraAngle:
      pc = new SParamComp_CameraAngle;
      break;
    case EParamComp_CameraPos:
      pc = new SParamComp_CameraPos;
      break;
    case EParamComp_ObjPos:
      pc = new SParamComp_ObjPos;
      break;
    case EParamComp_OSCameraPos:
      pc = new SParamComp_OSCameraPos;
      break;
    case EParamComp_ClipPlane:
      pc = new SParamComp_ClipPlane;
      break;
    case EParamComp_Time:
      pc = new SParamComp_Time;
      break;
    case EParamComp_Distance:
      pc = new SParamComp_Distance;
      break;
    case EParamComp_MinDistance:
      pc = new SParamComp_MinDistance;
      break;
    case EParamComp_MinLightDistance:
      pc = new SParamComp_MinLightDistance;
      break;
    case EParamComp_Random:
      pc = new SParamComp_Random;
      break;
    case EParamComp_Fog:
      pc = new SParamComp_Fog;
      break;
    case EParamComp_HeatFactor:
      pc = new SParamComp_HeatFactor;
      break;
    case EParamComp_Opacity:
      pc = new SParamComp_Opacity;
      break;
    case EParamComp_MatrixTCG:
      pc = new SParamComp_MatrixTCG;
      break;
    case EParamComp_MatrixTCM:
      pc = new SParamComp_MatrixTCM;
      break;

    case EParamComp_LightsColor:
      pc = new SParamComp_LightsColor;
      break;

    case EParamComp_SpecLightsColor:
      pc = new SParamComp_SpecLightsColor;
      break;

    case EParamComp_OSLightsPos:
      pc = new SParamComp_OSLightsPos;
      break;

    case EParamComp_LightsIntens:
      pc = new SParamComp_LightsIntens;
      break;

    case EParamComp_LightsType:
      pc = new SParamComp_LightsType;
      break;

    case EParamComp_LightsNum:
      pc = new SParamComp_LightsNum;
      break;
  }
  if (!pc)
  {
    Warning( 0,0,"Warning: Unknown component type %d (ignored)\n", pPC->m_eType);
  }
  else
  {
    assert (pc->m_eType == pPC->m_eType);

    pc->mfCopy(pPC);
    m_ParamComps.AddElem(pc);
  }
  return pc;
}

float SParamComp_Time::mfGet()
{  
  float fTime=0.0f;

  if(gRenDev->m_bPauseTimer && gRenDev->m_fPrevTime==-1.0f)
  {
    gRenDev->m_fPrevTime=gRenDev->m_RP.m_RealTime * m_Scale;
  }
  else
  if(gRenDev->m_bPauseTimer)
  {
    fTime=gRenDev->m_fPrevTime;
  }
  else    
  {
    gRenDev->m_fPrevTime=-1.0f;
    fTime = gRenDev->m_RP.m_RealTime * m_Scale;
  }
    
  return fTime;
}

float SParamComp_Distance::mfGet()
{
  CCamera cam = gRenDev->GetCamera();
  Vec3d Delta = cam.GetPos() - gRenDev->m_RP.m_pCurObject->GetTranslation();
  float fDist = Delta.Length();
  float fFov = cam.GetFov();
  float fFactor = fFov / (PI/2.0f);

  return fDist * m_Scale * fFactor;
}

float SParamComp_Random::mfGet()
{
  float fRand = RandomNum() * m_Scale;

  return fRand;
}

float SParamComp_Wave::mfGet()
{
  return SEvalFuncs::EvalWaveForm(&m_WF);
}

float SParamComp_ObjWave::mfGet()
{
  CCObject *obj = gRenDev->m_RP.m_pCurObject;
  if (!obj)
    return 0;
  SWaveForm2 *wf;
  if (m_bObjX)
  {
    if (obj->m_NumWFX <= 0)
      return 0;
    else
      wf = &CCObject::m_Waves[obj->m_NumWFX];
  }
  else
  {
    if (obj->m_NumWFY <= 0)
      return 0;
    else
      wf = &CCObject::m_Waves[obj->m_NumWFY];
  }
  return SEvalFuncs::EvalWaveForm(wf);
}
void SParamComp_ObjWave::mfGet4f(vec4_t v)
{
  CCObject *obj = gRenDev->m_RP.m_pCurObject;
  if (obj->m_NumWFX <= 0)
    v[0] = 0;
  else
    v[0] = SEvalFuncs::EvalWaveForm(&CCObject::m_Waves[obj->m_NumWFX]);
  if (obj->m_NumWFY <= 0)
    v[1] = 0;
  else
    v[1] = SEvalFuncs::EvalWaveForm(&CCObject::m_Waves[obj->m_NumWFY]);
  v[2] = gRenDev->m_RP.m_pCurObject->m_fBending;
  v[3] = 1;
}

float SParamComp_WaterLevel::mfGet()
{
  return iSystem->GetI3DEngine()->GetWaterLevel();
}

float SParamComp_CameraPos::mfGet()
{
  Vec3d pos = gRenDev->GetCamera().GetPos();
  float v = pos[m_Offs] * m_Sign;
  switch (m_Op)
  {
    case 1:
      return v + m_Operand;
    case 2:
      return v * m_Operand;
    case 3:
      return v / m_Operand;
    case 4:
      return v - m_Operand;
  }
  return v;
}
void SParamComp_CameraPos::mfGet4f(vec4_t v)
{
  Vec3d pos = gRenDev->GetCamera().GetPos();
  v[0] = pos.x;
  v[1] = pos.y;
  v[2] = pos.z;
  v[3] = 1.0f;
}

float SParamComp_ObjPos::mfGet()
{
  if (!gRenDev->m_RP.m_pCurObject)
    return 0;
  Vec3d pos = gRenDev->m_RP.m_pCurObject->GetTranslation();
  float v = pos[m_Offs] * m_Sign;
  switch (m_Op)
  {
    case 1:
      return v + m_Operand;
    case 2:
      return v * m_Operand;
    case 3:
      return v / m_Operand;
    case 4:
      return v - m_Operand;
  }
  return v;
}

float SParamComp_OSCameraPos::mfGet()
{
  Vec3d p = gRenDev->GetCamera().GetPos();
  Vec3d pos;
  Matrix44& im = gRenDev->m_RP.m_pCurObject->GetInvMatrix();
  TransformPosition(pos, p, im);
  float v = pos[m_Offs] * m_Sign;
  switch (m_Op)
  {
    case 1:
      return v + m_Operand;
    case 2:
      return v * m_Operand;
    case 3:
      return v / m_Operand;
    case 4:
      return v - m_Operand;
  }
  return v;
}

void SParamComp_OSCameraPos::mfGet4f(vec4_t v)
{
  Vec3d p = gRenDev->GetCamera().GetPos();
  Vec3d pos;
  Matrix44& im = gRenDev->m_RP.m_pCurObject->GetInvMatrix();
  TransformPosition(pos, p, im);
  v[0] = pos.x * m_Sign;
  v[1] = pos.y * m_Sign;
  v[2] = pos.z * m_Sign;
  switch (m_Op)
  {
    case 1:
      v[0] += m_Operand;
      v[1] += m_Operand;
      v[2] += m_Operand;
      break;
    case 2:
      v[0] *= m_Operand;
      v[1] *= m_Operand;
      v[2] *= m_Operand;
      break;
    case 3:
      v[0] /= m_Operand;
      v[1] /= m_Operand;
      v[2] /= m_Operand;
      break;
    case 4:
      v[0] -= m_Operand;
      v[1] -= m_Operand;
      v[2] -= m_Operand;
      break;
  }
  v[3] = 1.0f;
}

float SParamComp_WorldColor::mfGet()
{
  float fc;
  if (gRenDev->m_RP.m_PersFlags & RBPF_DRAWNIGHTMAP)
    return 1.0f;
  if (m_Offs & 0x80000000)
  {
    int offs = m_Offs & ~0x80000000;
    fc = gRenDev->m_WorldColor[offs];
    if (gRenDev->m_RP.m_pCurObject)
      fc *= gRenDev->m_RP.m_pCurObject->m_Color[offs];
  }
  else
    fc = gRenDev->m_WorldColor[m_Offs];
  return fc;
}
void SParamComp_WorldColor::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  if (rd->m_RP.m_PersFlags & RBPF_DRAWNIGHTMAP)
  {
    v[0] = v[1] = v[2] = v[3] = 1.0f;
    return;
  }

  if (m_Offs & 0x80000000)
  {
    v[0] = rd->m_WorldColor[0];
    v[1] = rd->m_WorldColor[1];
    v[2] = rd->m_WorldColor[2];
    v[3] = rd->m_RP.m_fCurOpacity;
    if (rd->m_RP.m_pCurObject)
    {
      v[0] *= rd->m_RP.m_pCurObject->m_Color[0];
      v[1] *= rd->m_RP.m_pCurObject->m_Color[1];
      v[2] *= rd->m_RP.m_pCurObject->m_Color[2];
      v[3] *= rd->m_RP.m_pCurObject->m_Color[3];
    }
  }
  else
  {
    v[0] = rd->m_WorldColor[0];
    v[1] = rd->m_WorldColor[1];
    v[2] = rd->m_WorldColor[2];
    v[3] = rd->m_RP.m_fCurOpacity;
  }
}


float SParamComp_SunColor::mfGet()
{
  return iSystem->GetI3DEngine()->GetSunColor()[m_Offs] * m_Mult;
}

void SParamComp_SunColor::mfGet4f(vec4_t v)
{
  Vec3d sc = iSystem->GetI3DEngine()->GetSunColor();
  v[0] = sc.x * m_Mult;
  v[1] = sc.y * m_Mult;
  v[2] = sc.z * m_Mult;
  v[3] = 1.0f;
}

float SParamComp_ObjVal::mfGet()
{
  float fc = gRenDev->m_RP.m_pCurObject->m_TempVars[m_Offs];
  //fc = 1.0f;
  return fc;
}

float SParamComp_Bending::mfGet()
{
  if (!gRenDev->m_RP.m_pCurObject)
    return 0;
  float fc = gRenDev->m_RP.m_pCurObject->m_fBending;
  return fc;
}

float SParamComp_LightColor::mfGet()
{
  float fc = gRenDev->m_RP.m_pCurLight->m_Color[m_Offs];
  if (gRenDev->m_RP.m_pCurLightMaterial)
  {
    fc *= gRenDev->m_RP.m_pCurLightMaterial->Front.m_Diffuse[m_Offs];
  }
  fc = min(fc, 1.0f);
  return fc;
}

void SParamComp_LightColor::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  if (!rd->m_RP.m_pCurLight)
    return;
  v[0] = rd->m_RP.m_pCurLight->m_Color[0];
  v[1] = rd->m_RP.m_pCurLight->m_Color[1];
  v[2] = rd->m_RP.m_pCurLight->m_Color[2];
  v[3] = rd->m_RP.m_fCurOpacity * rd->m_RP.m_pCurObject->m_Color.a;
  if (SLightMaterial *mt=rd->m_RP.m_pCurLightMaterial)
  {
    v[0] *= mt->Front.m_Diffuse[0];
    v[1] *= mt->Front.m_Diffuse[1];
    v[2] *= mt->Front.m_Diffuse[2];
    v[3] *= mt->Front.m_Diffuse[3];
  }
  if (!(rd->m_RP.m_PersFlags & RBPF_HDR))
  {
    float fMax = max(max(v[0], v[1]), v[2]);
    if (fMax > 1.1f)
    {
      float fIMax = 1.0f / fMax;
      v[0] *= fIMax;
      v[1] *= fIMax;
      v[2] *= fIMax;
    }
  }
}
void SParamComp_LightsColor::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  SLightPass *lp = &rd->m_RP.m_LPasses[rd->m_RP.m_nCurLightPass];
  if (m_Offs >= lp->nLights)
  {
    v[0] = v[1] = v[2] = v[3] = 0;
    return;
  }
  CDLight *dl = lp->pLights[m_Offs];
  v[0] = dl->m_Color[0];
  v[1] = dl->m_Color[1];
  v[2] = dl->m_Color[2];
  v[3] = rd->m_RP.m_fCurOpacity * rd->m_RP.m_pCurObject->m_Color.a;
  if (SLightMaterial *mt=rd->m_RP.m_pCurLightMaterial)
  {
    v[0] *= mt->Front.m_Diffuse[0];
    v[1] *= mt->Front.m_Diffuse[1];
    v[2] *= mt->Front.m_Diffuse[2];
    v[3] *= mt->Front.m_Diffuse[3];
  }
  if (!(rd->m_RP.m_PersFlags & RBPF_HDR))
  {
    float fMax = max(max(v[0], v[1]), v[2]);
    if (fMax > 1.1f)
    {
      float fIMax = 1.0f / fMax;
      v[0] *= fIMax;
      v[1] *= fIMax;
      v[2] *= fIMax;
    }
  }
}

float SParamComp_SpecularPower::mfGet()
{
  float fc = 8.0f;
  if (gRenDev->m_RP.m_pCurLightMaterial)
    fc = gRenDev->m_RP.m_pCurLightMaterial->Front.m_SpecShininess;
  return fc;
}

float SParamComp_DiffuseColor::mfGet()
{
  float fc = 1.0f;
  if (gRenDev->m_RP.m_pCurLightMaterial)
  {
    fc = gRenDev->m_RP.m_pCurLightMaterial->Front.m_Diffuse[m_Offs];
  }
  fc = min(fc, 1.0f);
  return fc;
}
void SParamComp_DiffuseColor::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  v[0] = 1.0f;
  v[1] = 1.0f;
  v[2] = 1.0f;
  v[3] = rd->m_RP.m_fCurOpacity * rd->m_RP.m_pCurObject->m_Color.a;
  if (SLightMaterial *mt=rd->m_RP.m_pCurLightMaterial)
  {
    v[0] *= mt->Front.m_Diffuse[0];
    v[1] *= mt->Front.m_Diffuse[1];
    v[2] *= mt->Front.m_Diffuse[2];
    v[3] *= mt->Front.m_Diffuse[3];
  }
  v[0] = min(v[0], 1.0f);
  v[1] = min(v[1], 1.0f);
  v[2] = min(v[2], 1.0f);
}

float SParamComp_ObjColor::mfGet()
{
  float fc = 1.0f;
  if (gRenDev->m_RP.m_pCurObject)
    fc *= gRenDev->m_RP.m_pCurObject->m_Color[m_Offs];
  return fc;
}

float SParamComp_SpecLightColor::mfGet()
{
  float fc = gRenDev->m_RP.m_pCurLight->m_SpecColor[m_Offs];
  if (gRenDev->m_RP.m_pCurLightMaterial)
    fc *= gRenDev->m_RP.m_pCurLightMaterial->Front.m_Specular[m_Offs];
  fc = min(fc, 1.0f);
  return fc;
}
void SParamComp_SpecLightColor::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;

  v[0] = rd->m_RP.m_pCurLight->m_SpecColor[0];
  v[1] = rd->m_RP.m_pCurLight->m_SpecColor[1];
  v[2] = rd->m_RP.m_pCurLight->m_SpecColor[2];
  v[3] = rd->m_RP.m_fCurOpacity * gRenDev->m_RP.m_pCurObject->m_Color.a;
  if (SLightMaterial *mt=rd->m_RP.m_pCurLightMaterial)
  {
    v[0] *= mt->Front.m_Specular[0];
    v[1] *= mt->Front.m_Specular[1];
    v[2] *= mt->Front.m_Specular[2];
  }
  if (!(rd->m_RP.m_PersFlags & RBPF_HDR))
  {
    float fMax = max(max(v[0], v[1]), v[2]);
    if (fMax > 1.1f)
    {
      float fIMax = 1.0f / fMax;
      v[0] *= fIMax;
      v[1] *= fIMax;
      v[2] *= fIMax;
    }
  }
}
void SParamComp_SpecLightsColor::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  SLightPass *lp = &rd->m_RP.m_LPasses[rd->m_RP.m_nCurLightPass];
  if (m_Offs >= lp->nLights)
  {
    v[0] = v[1] = v[2] = v[3] = 0;
    return;
  }
  CDLight *dl = lp->pLights[m_Offs];
  v[0] = dl->m_SpecColor[0];
  v[1] = dl->m_SpecColor[1];
  v[2] = dl->m_SpecColor[2];
  v[3] = rd->m_RP.m_fCurOpacity * rd->m_RP.m_pCurObject->m_Color.a;
  if (SLightMaterial *mt=rd->m_RP.m_pCurLightMaterial)
  {
    v[0] *= mt->Front.m_Specular[0];
    v[1] *= mt->Front.m_Specular[1];
    v[2] *= mt->Front.m_Specular[2];
  }
  if (!(rd->m_RP.m_PersFlags & RBPF_HDR))
  {
    float fMax = max(max(v[0], v[1]), v[2]);
    if (fMax > 1.1f)
    {
      float fIMax = 1.0f / fMax;
      v[0] *= fIMax;
      v[1] *= fIMax;
      v[2] *= fIMax;
    }
  }
}

float SParamComp_AmbLightColor::mfGet()
{
  float fc = 1.0f;
  if (gRenDev->m_RP.m_PersFlags & RBPF_DRAWNIGHTMAP)
    return fc;
  if (gRenDev->m_RP.m_pCurObject)
    fc *= gRenDev->m_RP.m_pCurObject->m_AmbColor[m_Offs];
  fc *= m_fMul;
  if (gRenDev->m_RP.m_pCurLightMaterial)
  {
    if (!gRenDev->m_RP.m_pCurObject || !(gRenDev->m_RP.m_pCurObject->m_ObjFlags & FOB_IGNOREMATERIALAMBIENT))
      fc *= gRenDev->m_RP.m_pCurLightMaterial->Front.m_Ambient[m_Offs];

		fc *= gRenDev->m_RP.m_pCurLightMaterial->Front.m_Diffuse[m_Offs];			// diffuse is used like filter so it affects ambient as well
  }
  fc = min(fc, 1.0f);

  return fc;
}

void SParamComp_AmbLightColor::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  CCObject *pObj = rd->m_RP.m_pCurObject;
  v[0] = v[1] = v[2] = 1.0f;
  v[3] = rd->m_RP.m_fCurOpacity * pObj->m_Color.a;
  if (rd->m_RP.m_PersFlags & RBPF_DRAWNIGHTMAP)
    return;
  v[0] *= pObj->m_AmbColor[0] * m_fMul;
  v[1] *= pObj->m_AmbColor[1] * m_fMul;
  v[2] *= pObj->m_AmbColor[2] * m_fMul;
  if (SLightMaterial *mt=rd->m_RP.m_pCurLightMaterial)
  {
    if (!(pObj->m_ObjFlags & FOB_IGNOREMATERIALAMBIENT))
    {
      v[0] *= mt->Front.m_Ambient[0];
      v[1] *= mt->Front.m_Ambient[1];
      v[2] *= mt->Front.m_Ambient[2];
    }

		// diffuse is used like filter so it affects ambient as well
  	v[0] *= mt->Front.m_Diffuse[0];
    v[1] *= mt->Front.m_Diffuse[1];
    v[2] *= mt->Front.m_Diffuse[2];
  }
  v[0] = min(v[0], 1.0f);
  v[1] = min(v[1], 1.0f);
  v[2] = min(v[2], 1.0f);
}

float SParamComp_CameraAngle::mfGet()
{
  Vec3d angs = gRenDev->GetCamera().GetAngles();
  float v = angs[m_Offs] * m_Sign;
  switch (m_Op)
  {
    case 1:
      return v + m_Operand;
    case 2:
      return v * m_Operand;
    case 3:
      return v / m_Operand;
    case 4:
      return v - m_Operand;
  }
  return v;
}

float SParamComp_SunDirect::mfGet()
{
  return gRenDev->m_RP.m_SunDir[m_Offs];
}

float SParamComp_SunFlarePos::mfGet()
{
  return iSystem->GetI3DEngine()->GetSunPosition(false)[m_Offs];
}

float SParamComp_LightPos::mfGet()
{
  return gRenDev->m_RP.m_pCurLight->m_Origin[m_Offs];
}

float SParamComp_LightIntens::mfGet()
{
  if (gRenDev->m_RP.m_nCurLightParam >= 0)
  {
    if (!m_bInv)
      return gRenDev->m_RP.m_pActiveDLights[gRenDev->m_RP.m_nCurLightParam]->m_fRadius;
    else
      return 1.0f / gRenDev->m_RP.m_pActiveDLights[gRenDev->m_RP.m_nCurLightParam]->m_fRadius;
  }

  // Approximated reciprocal square root
  float fRadius;
  if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
  {
    float fLen = gRenDev->m_RP.m_pCurObject->m_Matrix(0,0)*gRenDev->m_RP.m_pCurObject->m_Matrix(0,0) + gRenDev->m_RP.m_pCurObject->m_Matrix(0,1)*gRenDev->m_RP.m_pCurObject->m_Matrix(0,1) + gRenDev->m_RP.m_pCurObject->m_Matrix(0,2)*gRenDev->m_RP.m_pCurObject->m_Matrix(0,2);
    unsigned int *n1 = (unsigned int *)&fLen;
    unsigned int n = 0x5f3759df - (*n1 >> 1);
    float *n2 = (float *)&n;
    float fISqrt = (1.5f - (fLen * 0.5f) * *n2 * *n2) * *n2;
    fRadius = gRenDev->m_RP.m_pCurLight->m_fRadius * fISqrt;
  }
  else
    fRadius = gRenDev->m_RP.m_pCurLight->m_fRadius;

  if (!m_bInv)
    return fRadius;
  else
    return 1.0f / fRadius; //gRenDev->m_RP.m_pCurLight->m_fRadius;
}
void SParamComp_LightsIntens::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  int i;

  SLightPass *lp = &rd->m_RP.m_LPasses[rd->m_RP.m_nCurLightPass];
  float fISqrt;
  if (rd->m_RP.m_ObjFlags & FOB_TRANS_MASK)
  {
    float fLen = rd->m_RP.m_pCurObject->m_Matrix(0,0)*rd->m_RP.m_pCurObject->m_Matrix(0,0) + rd->m_RP.m_pCurObject->m_Matrix(0,1)*rd->m_RP.m_pCurObject->m_Matrix(0,1) + rd->m_RP.m_pCurObject->m_Matrix(0,2)*rd->m_RP.m_pCurObject->m_Matrix(0,2);
    unsigned int *n1 = (unsigned int *)&fLen;
    unsigned int n = 0x5f3759df - (*n1 >> 1);
    float *n2 = (float *)&n;
    fISqrt = (1.5f - (fLen * 0.5f) * *n2 * *n2) * *n2;
  }
  else
    fISqrt = 1.0f;
  for (i=0; i<4; i++)
  {
    if (i >= lp->nLights)
      v[i] = 0;
    else
    {
      CDLight *dl = lp->pLights[i];
      float fRadius = dl->m_fRadius * fISqrt;
      v[i] = 1 / fRadius;
    }
  }
}

void SParamComp_LightsType::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  int i;
  SLightPass *lp = &rd->m_RP.m_LPasses[rd->m_RP.m_nCurLightPass];

  for (i=0; i<4; i++)
  {
    if (i >= lp->nLights)
      v[i] = 0;
    else
    {
      CDLight *dl = lp->pLights[i];
      if (dl->m_Flags & DLF_DIRECTIONAL)
        v[i] = 0;
      else
      if (dl->m_Flags & DLF_POINT)
        v[i] = 1;
      else
      if (dl->m_Flags & DLF_PROJECT)
        v[i] = 2;
      else
        v[i] = 0;
    }
  }
}

float SParamComp_LightsNum::mfGet(void)
{
  SLightPass *lp = &gRenDev->m_RP.m_LPasses[gRenDev->m_RP.m_nCurLightPass];
  return (float)lp->nLights;
}

void SParamComp_LightIntens::mfGet4f(vec4_t v)
{
  if (!gRenDev->m_RP.m_pCurLight)
    return;

  // Approximated reciprocal square root
  float fRadius;
  if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
  {
    float fLen = gRenDev->m_RP.m_pCurObject->m_Matrix(0,0)*gRenDev->m_RP.m_pCurObject->m_Matrix(0,0) + gRenDev->m_RP.m_pCurObject->m_Matrix(0,1)*gRenDev->m_RP.m_pCurObject->m_Matrix(0,1) + gRenDev->m_RP.m_pCurObject->m_Matrix(0,2)*gRenDev->m_RP.m_pCurObject->m_Matrix(0,2);
    unsigned int *n1 = (unsigned int *)&fLen;
    unsigned int n = 0x5f3759df - (*n1 >> 1);
    float *n2 = (float *)&n;
    float fISqrt = (1.5f - (fLen * 0.5f) * *n2 * *n2) * *n2;

    fRadius = gRenDev->m_RP.m_pCurLight->m_fRadius * fISqrt;
  }
  else
    fRadius = gRenDev->m_RP.m_pCurLight->m_fRadius;
  v[0] = fRadius;
  v[1] = 1.0f/fRadius;
  v[2] = 0.5f;
  v[3] = 0;
}

float SParamComp_LightBright::mfGet()
{
  float fLuminance;
  if (gRenDev->m_RP.m_nCurLightParam >= 0)
    fLuminance = ((gRenDev->m_RP.m_pActiveDLights[gRenDev->m_RP.m_nCurLightParam]->m_Color.r * 0.3f) + (gRenDev->m_RP.m_pActiveDLights[gRenDev->m_RP.m_nCurLightParam]->m_Color.g * 0.59f) + (gRenDev->m_RP.m_pActiveDLights[gRenDev->m_RP.m_nCurLightParam]->m_Color.b * 0.11f));
  else
    fLuminance = ((gRenDev->m_RP.m_pCurLight->m_Color.r * 0.3f) + (gRenDev->m_RP.m_pCurLight->m_Color.g * 0.59f) + (gRenDev->m_RP.m_pCurLight->m_Color.b * 0.11f));
  return fLuminance;
}

float SParamComp_LightDirectFactor::mfGet()
{
  float fFactor;
  if (gRenDev->m_RP.m_nCurLightParam >= 0)
    fFactor = gRenDev->m_RP.m_pActiveDLights[gRenDev->m_RP.m_nCurLightParam]->m_fDirectFactor;
  else
    fFactor = gRenDev->m_RP.m_pCurLight->m_fDirectFactor;
  return fFactor;
}

float SParamComp_FromRE::mfGet()
{
  if (!gRenDev->m_RP.m_pRE || !gRenDev->m_RP.m_pRE->m_CustomData)
    return 0;

  float *data;
  if (gRenDev->m_RP.m_pRE && gRenDev->m_RP.m_pRE->m_CustomData)
    data = (float *)gRenDev->m_RP.m_pRE->m_CustomData;
  else
    data = (float *)gRenDev->m_RP.m_RECustomData;
  int Offs;
  if (m_Offs >= 0)
    Offs = m_Offs;
  else
    Offs = gRenDev->m_RP.m_pRE->m_nCountCustomData;
  float val = data[Offs];
  gRenDev->m_RP.m_pRE->m_nCountCustomData++;
  return val;
}

float SParamComp_REColor::mfGet()
{
  float *data;
  if (gRenDev->m_RP.m_pRE)
    data = (float *)&gRenDev->m_RP.m_pRE->m_Color[0];
  else
    data = (float *)&gRenDev->m_RP.m_REColor[0];
  float val = data[m_Offs];
  return val;
}

void SParamComp_TempMatr::mfGet4f(vec4_t v)
{
  float *data = &gRenDev->m_cEF.m_TempMatrices[m_MatrID0][m_MatrID1].GetData()[m_Offs*4];
  v[0] = data[0];
  v[1] = data[1];
  v[2] = data[2];
  v[3] = data[3];
}

float SParamComp_FromObject::mfGet()
{
  if (!gRenDev->m_RP.m_pCurObject || !gRenDev->m_RP.m_pCurObject->m_CustomData)
    return 0;

  float *data = (float *)gRenDev->m_RP.m_pCurObject->m_CustomData;
  float val = data[m_Offs];
  return val;
}

float SParamComp_User::mfGet()
{
  int nMat = -1;
  CRenderer *rd = gRenDev;
  if (rd->m_RP.m_pRE)
    nMat = rd->m_RP.m_pRE->mfGetMatId();
  float fParm = -9999.0f;
  if (rd->m_RP.m_pCurObject && rd->m_RP.m_pCurObject->m_ShaderParams)
  {
    fParm = SShaderParam::GetFloat(m_Name.c_str(), rd->m_RP.m_pCurObject->m_ShaderParams, nMat);
    if (fParm == -9999.0f && nMat != -1)
      fParm = SShaderParam::GetFloat(m_Name.c_str(), rd->m_RP.m_pCurObject->m_ShaderParams, -1);
  }
  if (fParm == -9999.0f)
  {
    SRenderShaderResources *pRS;
    if (pRS = rd->m_RP.m_pShaderResources)
    {
      fParm = SShaderParam::GetFloat(m_Name.c_str(), &pRS->m_ShaderParams, -1);
      if ((pRS->m_ResFlags & MTLFLAG_USEGLOSSINESS) && m_Name == rd->m_RP.m_Name_SpecularExp)
      {
        if (pRS->m_LMaterial)
          fParm = pRS->m_LMaterial->Front.m_SpecShininess;
      }
    }
    if (fParm == -9999.0f)
      fParm = SShaderParam::GetFloat(m_Name.c_str(), &rd->m_RP.m_pShader->m_PublicParams, -1);
  }
  if (fParm != -9999.0f && m_Name == rd->m_RP.m_Name_SpecularExp)
    rd->m_RP.m_fCurSpecShininess = fParm;

  if (m_bTimeModulate)
    fParm *= rd->m_RP.m_RealTime;
  return fParm;
}

void SParamComp_ObjMatrix::mfGet4f(vec4_t v)
{
  int i = m_Offs & 0xf;
  Matrix44 m;
  if (m_Offs & 0x80000000)
  {
    Matrix44 *im;
    if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
      im = &gRenDev->m_RP.m_pCurObject->GetInvMatrix();
    else
    {
      m.SetIdentity();
      im = &m;
    }
    if (!(m_Offs & 0x40000000))
    {
      v[0] = (*im)[i][0];
      v[1] = (*im)[i][1];
      v[2] = (*im)[i][2];
      v[3] = (*im)[i][3];
    }
    else
    {
      v[0] = (*im)[0][i];
      v[1] = (*im)[1][i];
      v[2] = (*im)[2][i];
      v[3] = (*im)[3][i];
    }
  }
  else
  {
    Matrix44 *im;
    if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
      im = &gRenDev->m_RP.m_pCurObject->m_Matrix;
    else
    {
      m.SetIdentity();
      im = &m;
    }
    if (!(m_Offs & 0x40000000))
    {
      v[0] = (*im)[i][0];
      v[1] = (*im)[i][1];
      v[2] = (*im)[i][2];
      v[3] = (*im)[i][3];
    }
    else
    {
      v[0] = (*im)[0][i];
      v[1] = (*im)[1][i];
      v[2] = (*im)[2][i];
      v[3] = (*im)[3][i];
    }
  }
}

float SParamComp_ObjScale::mfGet()
{
  if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
    return gRenDev->m_RP.m_pCurObject->GetScaleX();
  else
    return 1;
}

float SParamComp_CameraMatrix::mfGet()
{
  int i = m_Offs & 0xf;
  int j = (m_Offs >> 4) & 0xf;
  if (m_Offs & 0x80000000)
  {
    Matrix44 im = gRenDev->m_CameraMatrix;
    im.Invert44();
    if (m_Offs & 0x40000000)
      return im[i][j];
    else
      return im[j][i];
  }
  if (m_Offs & 0x40000000)
    return gRenDev->m_CameraMatrix[i][j];
  else
    return gRenDev->m_CameraMatrix[j][i];
}

SShader *SParamComp_Matrix::m_pLastShader;
Matrix44 SParamComp_Matrix::m_LastMatrix;
int SParamComp_Matrix::m_Frame;
void SParamComp_Matrix::mfGet4f(vec4_t v)
{
  int i;
  if (m_pLastShader != gRenDev->m_RP.m_pShader || m_Frame != gRenDev->m_RP.m_RenderFrame)
  {
    static Vec3d sV;
    m_pLastShader = gRenDev->m_RP.m_pShader;
    m_Frame = gRenDev->m_RP.m_RenderFrame;
    m_LastMatrix.SetIdentity();
    for (i=0; i<m_Transforms.Num(); i++)
    {
      SMatrixTransform *mt = &m_Transforms[i];
      mt->mfSet(m_LastMatrix);
    }
  }
  i = m_Offs & 0xf;
  if (m_Offs & 0x40000000)
  {
    for (int j=0; j<4; j++)
    {
      v[j] = m_LastMatrix[j][i];
    }
  }
  else
  {
    for (int j=0; j<4; j++)
    {
      v[j] = m_LastMatrix[i][j];
    }
  }
}
int SParamComp_Matrix::Size()
{
  int nSize = sizeof(SParamComp_Matrix);
  nSize += m_Transforms.GetSize() * sizeof(SMatrixTransform);
  return nSize;
}

static int sLastObjFrame;
static Vec3d sLastLPos;
static Vec3d sLastLAngles;
static float sLastLFrust;
DEFINE_ALIGNED_DATA_STATIC( Matrix44, m, 16 );

void SParamComp_LightMatrix::mfGet4f(vec4_t v)
{
  int i = m_Offs & 0xf;
  CDLight *dl = gRenDev->m_RP.m_pCurLight;
  if (dl && dl->m_pLightImage)
  {
    if (!i)
    {
			if (gRenDev->m_RP.m_FrameObject!=sLastObjFrame || !IsEquivalent(dl->m_Origin,sLastLPos) || !IsEquivalent(dl->m_ProjAngles,sLastLAngles) || dl->m_fLightFrustumAngle != sLastLFrust)
      {
        sLastObjFrame = gRenDev->m_RP.m_FrameObject;
        sLastLPos = dl->m_Origin;
        sLastLFrust = dl->m_fLightFrustumAngle;
        sLastLAngles = dl->m_ProjAngles;
        if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
        {
          CCObject *obj = gRenDev->m_RP.m_pCurObject;
          mathMatrixMultiply(m.GetData(), dl->m_TextureMatrix.GetData(), obj->m_Matrix.GetData(), g_CpuFlags);
        }
        else
          m = dl->m_TextureMatrix;
      }
    }
    if (!(m_Offs & 0x40000000))
    {
      v[0] = m(i,0);
      v[1] = m(i,1);
      v[2] = m(i,2);
      v[3] = m(i,3);
    }
    else
    {
      v[0] = m(0,i);
      v[1] = m(1,i);
      v[2] = m(2,i);
      v[3] = m(3,i);
    }
  }
}

void SParamComp_VFogMatrix::mfGet4f(vec4_t v)
{
  gRenDev->m_RP.m_FlagsPerFlush |= RBSI_FOGVOLUME;
  Plane plane11;
  SMFog *fb = gRenDev->m_RP.m_pFogVolume;
  if (!fb)
    return;
  float fDot   = fb->m_Normal.Dot(gRenDev->m_RP.m_ViewOrg) - fb->m_Dist;
  float fSmooth;
  if (plane11.d < -0.5f)
    fSmooth = 1.0f;
  else
    fSmooth = 0.1f;

  if (!m_Offs)
  {
    if (fb->m_FogInfo.m_WaveFogGen.m_eWFType)
    {
      float f = SEvalFuncs::EvalWaveForm(&fb->m_FogInfo.m_WaveFogGen);

      fb->m_fMaxDist = f;
    }

    float intens = fb->m_fMaxDist;
    if (intens <= 0)
      intens = 1.0f;
    intens = -0.25f / intens;
    Plane plane00;
    plane00.n.x = intens*gRenDev->m_CameraMatrix(0,2);
    plane00.n.y = intens*gRenDev->m_CameraMatrix(1,2);
    plane00.n.z = intens*gRenDev->m_CameraMatrix(2,2);
    plane00.d   = intens*gRenDev->m_CameraMatrix(3,2);
    if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
    {
      Plane p = TransformPlane2(gRenDev->m_RP.m_pCurObject->m_Matrix, plane00);
      v[0] = p.n.x;
      v[1] = p.n.y;
      v[2] = p.n.z;
      v[3] = p.d+0.5f;
    }
    else
    {
      v[0] = plane00.n.x;
      v[1] = plane00.n.y;
      v[2] = plane00.n.z;
      v[3] = plane00.d+0.5f;
    }
  }
  else
  {
    Plane plane10;
    plane10.n.x = fb->m_Normal.x * fSmooth;
    plane10.n.y = fb->m_Normal.y * fSmooth;
    plane10.n.z = fb->m_Normal.z * fSmooth;
    plane10.d     = -(fb->m_Dist) * fSmooth;
    if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
    {
      Plane p = TransformPlane2(gRenDev->m_RP.m_pCurObject->m_Matrix, plane10);
      v[0] = p.n.x;
      v[1] = p.n.y;
      v[2] = p.n.z;
      v[3] = p.d+0.5f;
    }
    else
    {
      v[0] = plane10.n.x;
      v[1] = plane10.n.y;
      v[2] = plane10.n.z;
      v[3] = plane10.d+0.5f;
    }
  }
}

void SParamComp_FogMatrix::mfGet4f(vec4_t v)
{
  if (!m_Offs)
  {
    SMFog *fb = gRenDev->m_RP.m_pFogVolume;
    if (!fb)
      return;
    if (fb->m_FogInfo.m_WaveFogGen.m_eWFType)
    {
      float f = SEvalFuncs::EvalWaveForm(&fb->m_FogInfo.m_WaveFogGen);

      fb->m_fMaxDist = f;
    }
    float intens = fb->m_fMaxDist;
    if (intens <= 0)
      intens = 1.0f;
    intens = -0.25f / intens;
    Plane plane00;
    plane00.n.x = intens*gRenDev->m_CameraMatrix(0,2);
    plane00.n.y = intens*gRenDev->m_CameraMatrix(1,2);
    plane00.n.z = intens*gRenDev->m_CameraMatrix(2,2);
    plane00.d   = intens*gRenDev->m_CameraMatrix(3,2);
    if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
    {
      Plane p = TransformPlane2(gRenDev->m_RP.m_pCurObject->m_Matrix, plane00);
      v[0] = p.n.x;
      v[1] = p.n.y;
      v[2] = p.n.z;
      v[3] = p.d+0.5f;
    }
    else
    {
      v[0] = plane00.n.x;
      v[1] = plane00.n.y;
      v[2] = plane00.n.z;
      v[3] = plane00.d+0.5f;
    }
  }
  else
  {
    v[0] = v[1] = v[2] = 0;
    v[3] = 0.49f;
  }
}

void SParamComp_FogEnterMatrix::mfGet4f(vec4_t v)
{
  if (!m_Offs)
  {
    gRenDev->m_RP.m_FlagsPerFlush |= RBSI_FOGVOLUME;
    SMFog *fb = gRenDev->m_RP.m_pFogVolume;
    if (!fb)
      return;
    Plane plane11;
    plane11.n.x = 0;
    plane11.n.y = 0;
    plane11.n.z = 0;
    plane11.d     = fb->m_Normal.Dot(gRenDev->m_RP.m_ViewOrg) - fb->m_Dist;

    float fSmooth;
    if (plane11.d < -0.5f)
      fSmooth = 1.0f;
    else
      fSmooth = 0.1f;

    plane11.d *= fSmooth;

    if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
    {
      Plane p = TransformPlane2(gRenDev->m_RP.m_pCurObject->m_Matrix, plane11);
      v[0] = p.n.x;
      v[1] = p.n.y;
      v[2] = p.n.z;
      v[3] = p.d+0.5f;
    }
    else
    {
      v[0] = plane11.n.x;
      v[1] = plane11.n.y;
      v[2] = plane11.n.z;
      v[3] = plane11.d+0.5f;
    }
  }
  else
  {
    SMFog *fb = gRenDev->m_RP.m_pFogVolume;
    if (!fb)
      return;
    Plane plane10;
    float f = fb->m_Normal.Dot(gRenDev->m_RP.m_ViewOrg) - fb->m_Dist;
    float fSmooth;
    if (f < -0.5f)
      fSmooth = 1.0f;
    else
      fSmooth = 0.1f;
    plane10.n.x = fb->m_Normal.x * fSmooth;
    plane10.n.y = fb->m_Normal.y * fSmooth;
    plane10.n.z = fb->m_Normal.z * fSmooth;
    plane10.d     = -(fb->m_Dist) * fSmooth;
    if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
    {
      Plane p = TransformPlane2(gRenDev->m_RP.m_pCurObject->m_Matrix, plane10);
      v[0] = p.n.x;
      v[1] = p.n.y;
      v[2] = p.n.z;
      v[3] = p.d+0.5f;
    }
    else
    {
      v[0] = plane10.n.x;
      v[1] = plane10.n.y;
      v[2] = plane10.n.z;
      v[3] = plane10.d+0.5f;
    }
  }
}

float SParamComp_VolFogColor::mfGet()
{
  if (!gRenDev->m_RP.m_pFogVolume)
    return 0;
  return gRenDev->m_RP.m_pFogVolume->m_Color[m_Offs];
}

float SParamComp_VolFogDensity::mfGet()
{
  if (!gRenDev->m_RP.m_pFogVolume)
    return 0;
  return gRenDev->m_RP.m_pFogVolume->m_fMaxDist;
}


float SParamComp_WaterProjMatrix::mfGet()
{
  int i = m_Offs & 0xf;
  int j = (m_Offs >> 4) & 0xf;
  if (m_Offs & 0x40000000)
    return gRenDev->m_RP.m_WaterProjMatrix(i,j);
  return gRenDev->m_RP.m_WaterProjMatrix(j,i);
}

float SParamComp_BumpAmount::mfGet()
{
  if (gRenDev->m_RP.m_pShaderResources && gRenDev->m_RP.m_pShaderResources->m_Textures[EFTT_BUMP])
  {
    return (float)gRenDev->m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_Amount / 255.0f;
  }
  return 1.0f;
}

float SParamComp_ObjRefrFactor::mfGet()
{
  float fRefract = -1000.0f;
  if (gRenDev->m_RP.m_pCurObject)
    fRefract = gRenDev->m_RP.m_pCurObject->m_fRefract;

  //fRefract = 1.0f;
  return fRefract;
}

float SParamComp_GeomCenter::mfGet()
{
  if (gRenDev->m_RP.m_pRE)
  {
    Vec3d Pos;
    gRenDev->m_RP.m_pRE->mfCenter(Pos, gRenDev->m_RP.m_pCurObject);
    return Pos[m_Offs];
  }
  return 0;
}

float SParamComp_HeatFactor::mfGet()
{
  if (gRenDev->m_RP.m_pCurObject)
    return gRenDev->m_RP.m_pCurObject->m_fHeatFactor;

  return 1.0f;
}

float SParamComp_Opacity::mfGet()
{
  return gRenDev->m_RP.m_fCurOpacity * gRenDev->m_RP.m_pCurObject->m_Color.a;
}

void SParamComp_ClipPlane::mfGet4f(vec4_t v)
{
  if (gRenDev->m_RP.m_bClipPlaneRefract && gRenDev->m_RP.m_pShader->m_eSort == eS_Terrain)
  {
    v[0] = gRenDev->m_RP.m_CurClipPlane.m_Normal.x;
    v[1] = gRenDev->m_RP.m_CurClipPlane.m_Normal.y;
    v[2] = gRenDev->m_RP.m_CurClipPlane.m_Normal.z;
    v[3] = gRenDev->m_RP.m_CurClipPlane.m_Dist + 0.5f;
  }
  else
  {
    Plane plSrc;
		plSrc.Set(gRenDev->m_RP.m_CurClipPlane.m_Normal, gRenDev->m_RP.m_CurClipPlane.m_Dist);

    if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
    {
		  Plane p = TransformPlane2(gRenDev->m_RP.m_pCurObject->m_Matrix, plSrc);
      v[0] = p.n.x;
      v[1] = p.n.y;
      v[2] = p.n.z;
      v[3] = p.d;
    }
    else
    {
      v[0] = plSrc.n.x;
      v[1] = plSrc.n.y;
      v[2] = plSrc.n.z;
      v[3] = plSrc.d;
    }
  }
}

float SParamComp_OSLightPos::mfGet()
{
  Vec3d p;
  if (gRenDev->m_RP.m_nCurLightParam >= 0)
    p = gRenDev->m_RP.m_pActiveDLights[gRenDev->m_RP.m_nCurLightParam]->m_Origin;
  else
    p = gRenDev->m_RP.m_pCurLight->m_Origin;
  Vec3d pos;
  TransformPosition(pos, p, gRenDev->m_RP.m_pCurObject->GetInvMatrix());

  return pos[m_Offs];
}
void SParamComp_OSLightPos::mfGet4f(vec4_t v)
{
  if (!gRenDev->m_RP.m_pCurLight)
  {
    v[0] = v[1] = v[2] = v[3] = 0;
    return;
  }
  Vec3d p;
  if (gRenDev->m_RP.m_nCurLightParam >= 0)
    p = gRenDev->m_RP.m_pActiveDLights[gRenDev->m_RP.m_nCurLightParam]->m_Origin;
  else
    p = gRenDev->m_RP.m_pCurLight->m_Origin;
  Vec3d pos;
  if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
    TransformPosition(pos, p, gRenDev->m_RP.m_pCurObject->GetInvMatrix());
  else
    pos = p;
  v[0] = pos.x;
  v[1] = pos.y;
  v[2] = pos.z;
  v[3] = 1.0f;
}
void SParamComp_OSLightsPos::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  SLightPass *lp = &rd->m_RP.m_LPasses[rd->m_RP.m_nCurLightPass];
  if (m_Offs >= lp->nLights)
  {
    v[0] = v[1] = v[2] = v[3] = 0;
    return;
  }
  CDLight *dl = lp->pLights[m_Offs];
  Vec3 p = dl->m_Origin;
  Vec3 pos;
  if (rd->m_RP.m_ObjFlags & FOB_TRANS_MASK)
    TransformPosition(pos, p, rd->m_RP.m_pCurObject->GetInvMatrix());
  else
    pos = p;
  v[0] = pos.x;
  v[1] = pos.y;
  v[2] = pos.z;

  float fDist = 0;
  Vec3 vCenterRE;
  if (rd->m_RP.m_pRE)
  {
    Vec3d vMins, vMaxs;
    rd->m_RP.m_pRE->mfGetBBox(vMins, vMaxs);
    vCenterRE = (vMins + vMaxs) * 0.5f;
    vCenterRE += rd->m_RP.m_pCurObject->GetTranslation();
  }
  else
  {
    vCenterRE = Vec3(0,0,0);
  }
  fDist = max(1.0f, GetSquaredDistance(vCenterRE, dl->m_Origin));
  fDist = cryISqrtf(fDist);
  v[3] = fDist;
}

float SParamComp_SLightPos::mfGet()
{
  Vec3d p;
  if (gRenDev->m_RP.m_nCurLightParam >= 0)
    p = gRenDev->m_RP.m_pActiveDLights[gRenDev->m_RP.m_nCurLightParam]->m_Origin;
  else
    p = gRenDev->m_RP.m_pCurLight->m_Origin;
  Vec3d pos;
  TransformPosition(pos, p, gRenDev->m_RP.m_pCurObject->GetInvMatrix());

  return pos[m_Offs];
}
void SParamComp_SLightPos::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  if (!rd->m_RP.m_pCurLight)
  {
    v[0] = v[1] = v[2] = v[3] = 0;
    return;
  }
  Vec3d p;
  if (rd->m_RP.m_nCurLightParam >= 0)
    p = rd->m_RP.m_pActiveDLights[rd->m_RP.m_nCurLightParam]->m_Origin;
  else
    p = rd->m_RP.m_pCurLight->m_Origin;
  Vec3d pos;
  if (rd->m_RP.m_ObjFlags & FOB_TRANS_MASK)
    TransformPosition(pos, p, rd->m_RP.m_pCurObject->GetInvMatrix());
  else
    pos = p;
  v[0] = pos.x;
  v[1] = pos.y;
  v[2] = pos.z;

  float fDist = 0;
  Vec3 vCenterRE;
  if (rd->m_RP.m_pRE)
  {
    Vec3d vMins, vMaxs;
    rd->m_RP.m_pRE->mfGetBBox(vMins, vMaxs);
    vCenterRE = (vMins + vMaxs) * 0.5f;
    vCenterRE += rd->m_RP.m_pCurObject->GetTranslation();
  }
  else
  {
    vCenterRE = Vec3(0,0,0);
  }
  fDist = max(1.0f, GetSquaredDistance(vCenterRE, rd->m_RP.m_pCurLight->m_Origin));
  fDist = cryISqrtf(fDist);
  v[3] = fDist;
}

float SParamComp_SCameraPos::mfGet()
{
  Vec3 p = gRenDev->GetCamera().GetPos();
  Vec3 pos;
  Matrix44& im = gRenDev->m_RP.m_pCurObject->GetInvMatrix();
  TransformPosition(pos, p, im);
  return pos[m_Offs];
}
void SParamComp_SCameraPos::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  Vec3 p = rd->GetCamera().GetPos();
  Vec3 pos;
  Matrix44& im = rd->m_RP.m_pCurObject->GetInvMatrix();
  if (rd->m_RP.m_ObjFlags & FOB_TRANS_MASK)
    TransformPosition(pos, p, im);
  else
    pos = p;
  v[0] = pos.x;
  v[1] = pos.y;
  v[2] = pos.z;

  float fDist = 0;
  if (gRenDev->m_RP.m_pRE)
  {
    fDist = gRenDev->m_RP.m_pRE->mfMinDistanceToCamera(gRenDev->m_RP.m_pCurObject);
    fDist = 1.0f / fDist;
  }
  v[3] = fDist;
}


float SParamComp_MinDistance::mfGet()
{
  float fDist = 0;
  if (gRenDev->m_RP.m_pRE)
  {
    fDist = gRenDev->m_RP.m_pRE->mfMinDistanceToCamera(gRenDev->m_RP.m_pCurObject);
    fDist = 1.0f / fDist;
  }
  return fDist;
}
float SParamComp_MinLightDistance::mfGet()
{
  float fDist = 0;
  Vec3 vCenterRE;
  if (gRenDev->m_RP.m_pRE)
  {
    Vec3d vMins, vMaxs;
    gRenDev->m_RP.m_pRE->mfGetBBox(vMins, vMaxs);
    vCenterRE = (vMins + vMaxs) * 0.5f;
    vCenterRE += gRenDev->m_RP.m_pCurObject->GetTranslation();
  }
  else
  {
    vCenterRE = Vec3(0,0,0);
  }
  fDist = max(1.0f, GetSquaredDistance(vCenterRE, gRenDev->m_RP.m_pCurLight->m_Origin));
  fDist = cryISqrtf(fDist);
  return fDist;
}


float SParamComp_LightOcclusion::mfGet()
{
  if ((gRenDev->m_RP.m_pCurObject->m_OcclLights[m_Offs]-1) == gRenDev->m_RP.m_nCurLight)
    return 1.0f;
  return 0;
}
void SParamComp_LightOcclusion::mfGet4f(vec4_t v)
{
  v[0] = v[1] = v[2] = v[3] = 0;
  int i;
  for (i=0; i<4; i++)
  {
    if ((gRenDev->m_RP.m_pCurObject->m_OcclLights[i]-1) == gRenDev->m_RP.m_nCurLight)
    {
      v[i] = 1.0f;
      break;
    }
  }
}

void SParamComp_LightOcclusions::mfGet4f(vec4_t v)
{
  v[0] = v[1] = v[2] = v[3] = 0;
  SLightPass *lp = &gRenDev->m_RP.m_LPasses[gRenDev->m_RP.m_nCurLightPass];
  if (m_Offs >= lp->nLights)
    return;
  CDLight *dl = lp->pLights[m_Offs];
  int i;
  for (i=0; i<4; i++)
  {
    if ((gRenDev->m_RP.m_pCurObject->m_OcclLights[i]-1) == dl->m_Id)
    {
      v[i] = 1.0f;
      break;
    }
  }
}

float SParamComp_PlantsTMoving::m_LastAX = -2000;
float SParamComp_PlantsTMoving::m_LastAY = -2000;
Matrix44    SParamComp_PlantsTMoving::m_Matrix;


void SParamComp_PlantsTMoving::mfGet4f(vec4_t v)
{
  static int sPrevObjFrame;

  if (sPrevObjFrame != gRenDev->m_RP.m_FrameObject)
  {
    sPrevObjFrame = gRenDev->m_RP.m_FrameObject;

    float ax, ay;
    if (gRenDev->m_RP.m_pCurObject && gRenDev->m_RP.m_pRE && gRenDev->m_RP.m_pRE->mfGetType() == eDATA_OcLeaf)
    {
      CREOcLeaf *pRE = (CREOcLeaf *)gRenDev->m_RP.m_pRE;
      float fiHeight = 1.0f / ((pRE->m_pBuffer->m_vBoxMax.z - pRE->m_pBuffer->m_vBoxMin.z) * gRenDev->m_RP.m_pCurObject->GetScaleZ());
      SWaveForm wfx = m_WFX;
      SWaveForm wfy = m_WFY;
      Vec3d vobjPos;
      if (gRenDev->m_RP.m_ObjFlags & FOB_TRANS_MASK)
        vobjPos = gRenDev->m_RP.m_pCurObject->GetTranslation();
      else
        vobjPos = Vec3(0,0,0);
      wfx.m_Freq = fiHeight/8.0f+0.2f;
      wfx.m_Phase = vobjPos.x/8.0f;
      wfy.m_Freq = fiHeight/7.0f+0.2f;
      wfy.m_Phase = vobjPos.y/8.0f;
      ax = SEvalFuncs::EvalWaveForm(&wfx);
      ay = SEvalFuncs::EvalWaveForm(&wfy);
    }
    else
    {
      ax = SEvalFuncs::EvalWaveForm(&m_WFX);
      ay = SEvalFuncs::EvalWaveForm(&m_WFY);
    }
    if (ax != m_LastAX || ay != m_LastAY)
    {
      m_LastAX = ax;
      m_LastAY = ay;
			SParamComp_PlantsTMoving::m_Matrix	=	GetTranslationMat(Vec3d(0.5f, 1.0f, 0))*Matrix33::CreateRotationZ(ax / 180.0f * PI)*Matrix33::CreateRotationY( -ay / 180.0f * PI ); //IMPORTANT: radian-angle must be negated ;
			SParamComp_PlantsTMoving::m_Matrix	=	GetTranslationMat(Vec3d(-0.5f, -1.0f, 0))*SParamComp_PlantsTMoving::m_Matrix;
		}
  }
  int j = (m_Offs >> 4) & 0xf;
  v[0] = SParamComp_PlantsTMoving::m_Matrix(0,j);
  v[1] = SParamComp_PlantsTMoving::m_Matrix(1,j);
  v[2] = SParamComp_PlantsTMoving::m_Matrix(2,j);
  v[3] = SParamComp_PlantsTMoving::m_Matrix(3,j);
}


void SParamComp_TexProjMatrix::mfGet4f(vec4_t v)
{
  static int nLastObjFrame=-1;
  static Vec3d pLastPos;
  static Vec3d pLastAngs;
  static STexPic *pLastTex;
  static Matrix44 m;

  SShaderPass *sl = gRenDev->m_RP.m_CurrPass;
  int tmu = m_Stage & 0xf;
  if (tmu == 0xf)
    tmu = gRenDev->m_TexMan->m_CurStage;
  if (sl->m_TUnits.Num() <= tmu)
    return;
  STexPic *tp = sl->m_TUnits[tmu].m_TexPic;

  int i = m_Offs & 0xf;

  if (!tp || !tp->m_Matrix)
  {
    if (tp->m_Bind == TO_ENVIRONMENT_TEX)
    {
      bool bReflect = false;
      if ((gRenDev->m_RP.m_pShader->m_Flags3 & (EF3_CLIPPLANE_FRONT | EF3_REFLECTION)))
        bReflect = true;
      SEnvTexture *cm = NULL;
      Vec3d Angs = gRenDev->GetCamera().GetAngles();
      Vec3d Pos = gRenDev->GetCamera().GetPos();
      cm = gRenDev->m_cEF.mfFindSuitableEnvTex(Pos, Angs, true, 0, false, gRenDev->m_RP.m_pShader, gRenDev->m_RP.m_pShaderResources, gRenDev->m_RP.m_pCurObject, bReflect, gRenDev->m_RP.m_pRE);
      if (!cm || !cm->m_Tex->m_Matrix)
        return;
      tp = cm->m_Tex;
    }
  }
  if(!tp->m_Matrix)
    return;

	if (gRenDev->m_RP.m_FrameObject != nLastObjFrame || pLastTex != tp || (!IsEquivalent(gRenDev->GetCamera().GetAngles(),pLastAngs,VEC_EPSILON)) || (!IsEquivalent(gRenDev->GetCamera().GetPos(),pLastPos,VEC_EPSILON)))
  {
    pLastTex = tp;
    pLastPos = gRenDev->GetCamera().GetPos();
    pLastAngs = gRenDev->GetCamera().GetAngles();
    nLastObjFrame = gRenDev->m_RP.m_FrameObject;
    if (gRenDev->m_RP.m_pCurObject->m_ObjFlags & FOB_TRANS_MASK)
      m = gRenDev->m_RP.m_pCurObject->m_Matrix * *(Matrix44 *)tp->m_Matrix;
    else
      memcpy(&m(0,0), tp->m_Matrix, 4*4*sizeof(float));
  }

  if (m_Offs & 0x40000000)
  {
    v[0] = m(0,i);
    v[1] = m(1,i);
    v[2] = m(2,i);
    v[3] = m(3,i);
  }
  else
  {
    v[0] = m(i,0);
    v[1] = m(i,1);
    v[2] = m(i,2);
    v[3] = m(i,3);
  }
}

void SParamComp_MatrixTCG::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  SEfResTexture *pRT = gRenDev->m_RP.m_ShaderTexResources[m_Stage];
  if (!pRT)
    return;  // should never happen
  v[0] = pRT->m_TexModificator.m_TexGenMatrix(m_Row, 0);
  v[1] = pRT->m_TexModificator.m_TexGenMatrix(m_Row, 1);
  v[2] = pRT->m_TexModificator.m_TexGenMatrix(m_Row, 2);
  v[3] = pRT->m_TexModificator.m_TexGenMatrix(m_Row, 3);
}

void SParamComp_MatrixTCM::mfGet4f(vec4_t v)
{
  CRenderer *rd = gRenDev;
  SEfResTexture *pRT = gRenDev->m_RP.m_ShaderTexResources[m_Stage];
  if (!pRT)
    return;  // should never happen
  v[0] = pRT->m_TexModificator.m_TexMatrix(0, m_Row);
  v[1] = pRT->m_TexModificator.m_TexMatrix(1, m_Row);
  v[2] = pRT->m_TexModificator.m_TexMatrix(2, m_Row);
  v[3] = pRT->m_TexModificator.m_TexMatrix(3, m_Row);
}

// <<FIXME>> tiago: remove reduntant stuff

float SParamComp_FlashBangBrightness::mfGet()
{
  CRendElement *pRE = gRenDev->m_RP.m_pRE;
  if (!pRE || pRE->mfGetType() != eDATA_FlashBang)
    return 0;
  CREFlashBang *pREFB = (CREFlashBang *)pRE;

  return 1;
}

float SParamComp_ScreenSize::mfGet()
{
  if (!gRenDev->m_RP.m_pCurObject)
    return 0;

  int iTempX, iTempY, iWidth, iHeight;
  gRenDev->GetViewport(&iTempX, &iTempY, &iWidth, &iHeight);

  switch (m_Op)
    {
    case 1:
      return (float) iWidth;
    case 2:
      return (float) iHeight;
    case 3:
      return (float) iWidth;
    case 4:
      return (float) iHeight;
    }
  Vec3d pos((float) iWidth, (float) iHeight, 1.0f);
  return pos[m_Offs]*m_Sign;
}

float SParamComp_DofFocalParams::mfGet()
{
  if (!gRenDev->m_RP.m_pCurObject)
    return 0;

  switch (m_Op)
  {
  case 1:
    if(CRenderer::CV_r_doffocalsource==0)
    {
      return (float) CRenderer::CV_r_doffocaldist;
    }
    else
    if(CRenderer::CV_r_doffocalsource==1)
    {
      return (float) CRenderer::CV_r_doffocaldist_tag;
    }
    else
    if(CRenderer::CV_r_doffocalsource==2)
    {
      return (float) CRenderer::CV_r_doffocaldist_entity;    
    }
    else
    {
      return 0;
    }
  case 2:
    return (float) CRenderer::CV_r_doffocalareacurr;
  case 3:
    return (float) 1.0f;
  case 4:
    return (float) 1.0f;
  }

  float fCurrFocalDist=0;
  if(CRenderer::CV_r_doffocalsource==0)
  {
    fCurrFocalDist=CRenderer::CV_r_doffocaldist;
  }
  else
  if(CRenderer::CV_r_doffocalsource==1)
  {
    fCurrFocalDist=CRenderer::CV_r_doffocaldist_tag;
  }
  else
  if(CRenderer::CV_r_doffocalsource==2)
  {
    fCurrFocalDist=CRenderer::CV_r_doffocaldist_entity;    
  }

  Vec3d pos(fCurrFocalDist, (float) CRenderer::CV_r_doffocalareacurr, 1.0f);
  return pos[m_Offs]*m_Sign;
}

void SParamComp_EnvColor::mfGet4f(vec4_t v)
{
  if (!gRenDev->m_RP.m_pCurEnvTexture)
  {
    SEnvTexture *cm = NULL;
		Vec3 vTrans( gRenDev->m_RP.m_pCurObject->GetTranslation() );
    cm = gRenDev->m_cEF.mfFindSuitableEnvLCMap( vTrans, true, 0, 0);
    if (cm)
      gRenDev->m_RP.m_pCurEnvTexture = cm;
  }
  if (!gRenDev->m_RP.m_pCurEnvTexture)
  {
    v[0] = v[1] = v[2] = v[3] = 1.0f;
    return;
  }
  v[0] = gRenDev->m_RP.m_pCurEnvTexture->m_EnvColors[m_Offs].bcolor[0] / 255.0f;
  v[1] = gRenDev->m_RP.m_pCurEnvTexture->m_EnvColors[m_Offs].bcolor[1] / 255.0f;
  v[2] = gRenDev->m_RP.m_pCurEnvTexture->m_EnvColors[m_Offs].bcolor[2] / 255.0f;
  v[3] = gRenDev->m_RP.m_pCurEnvTexture->m_EnvColors[m_Offs].bcolor[3] / 255.0f;
}
