/*=============================================================================
  PS2_Shaders.cpp : PS2 specific effectors/shaders functions implementation.
  Copyright 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "NULL_Renderer.h"
#include "I3dengine.h"


#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

//============================================================================


void CShader::mfCompileVarsPak(char *scr, TArray<CVarCond>& Vars, SShader *ef)
{
  char *var;
  char *val;

  while ((shGetVar (&scr, &var, &val)) >= 0)
  {
    if (!var)
      continue;

    ICVar *vr = iConsole->GetCVar(var);
    if (!vr)
    {
      iLog->Log("Warning: Couldn't find console variable '%s' in shader '%s'\n", var, ef->m_Name.c_str());
      continue;
    }
    float v = shGetFloat(val);
    CVarCond vc;
    vc.m_Var = vr;
    vc.m_Val = v;
    Vars.AddElem(vc);
  }
}

bool CShader::mfCompileHWShadeLayer(SShader *ef, char *scr, TArray<SShaderPassHW>& Layers)
{
  return true;
}

void CShader::mfCompileLayers(SShader *ef, char *scr, TArray<SShaderPassHW>& Layers, EShaderPassType eType)
{
}

void CShader::mfCompileHWConditions(SShader *ef, char *scr, SShaderTechnique *hs, int Id)
{
}

SShaderTechnique *CShader::mfCompileHW(SShader *ef, char *scr, int Id)
{
  return NULL;
}

//===================================================================

//====================================================================

SGenTC *SGenTC_NormalMap::mfCopy()
{
  return NULL;
}

bool SGenTC_NormalMap::mfSet(bool bEnable)
{
  return true;
}


void SGenTC_NormalMap::mfCompile(char *params, SShader *ef)
{
}

SGenTC *SGenTC_ReflectionMap::mfCopy()
{
  return NULL;
}

bool SGenTC_ReflectionMap::mfSet(bool bEnable)
{
  return true;
}

void SGenTC_ReflectionMap::mfCompile(char *params, SShader *ef)
{
}

SGenTC *SGenTC_ObjectLinear::mfCopy()
{
  return NULL;
}

bool SGenTC_ObjectLinear::mfSet(bool bEnable)
{
  return true;
}

void SGenTC_ObjectLinear::mfCompile(char *scr, SShader *ef)
{
}

SGenTC *SGenTC_EyeLinear::mfCopy()
{
  return NULL;
}

bool SGenTC_EyeLinear::mfSet(bool bEnable)
{
  return true;
}

void SGenTC_EyeLinear::mfCompile(char *scr, SShader *ef)
{
}

SGenTC *SGenTC_SphereMap::mfCopy()
{
  return NULL;
}

bool SGenTC_SphereMap::mfSet(bool bEnable)
{
  return true;
}

void SGenTC_SphereMap::mfCompile(char *params, SShader *ef)
{
}


SGenTC *SGenTC_EmbossMap::mfCopy()
{
  return NULL;
}


bool SGenTC_EmbossMap::mfSet(bool bEnable)
{
  return true;
}

void SGenTC_EmbossMap::mfCompile(char *params, SShader *ef)
{
}

bool CShader::mfCompileTexGen(char *name, char *params, SShader *ef, SShaderTexUnit *ml)
{
  return true;
}

//====================================================================
// Matrix operations


void CShader::mfCompileMatrixOp(TArray<SMatrixTransform>* List, char *scr, char *nmMat, SShader *ef)
{
}

void SMatrixTransform_LightCMProject::mfSet(bool bSet)
{
}
void SMatrixTransform_LightCMProject::mfSet(Matrix44& matr)
{
}


void SMatrixTransform_Identity::mfSet(bool bSet)
{
}
void SMatrixTransform_Identity::mfSet(Matrix44& matr)
{
  matr.SetIdentity();
}

void SMatrixTransform_Translate::mfSet(bool bSet)
{
}
void SMatrixTransform_Translate::mfSet(Matrix44& matr)
{
}

void SMatrixTransform_Scale::mfSet(bool bSet)
{
}
void SMatrixTransform_Scale::mfSet(Matrix44& matr)
{
}

void SMatrixTransform_Matrix::mfSet(bool bSet)
{
}
void SMatrixTransform_Matrix::mfSet(Matrix44& matr)
{
}

void SMatrixTransform_Rotate::mfSet(bool bSet)
{
}
void SMatrixTransform_Rotate::mfSet(Matrix44& matr)
{
}

//====================================================================
// Array pointers for PS2
//================================================================

void SArrayPointer_Vertex::mfSet(int Id)
{
}

//=========================================================================================

void SArrayPointer_Normal::mfSet(int Id)
{
}

//=========================================================================================

void SArrayPointer_Texture::mfSet(int Id)
{
}

//=========================================================================================

void SArrayPointer_Color::mfSet(int Id)
{
}

//=========================================================================================

void SArrayPointer_SecColor::mfSet(int Id)
{
}

//=========================================================================================

float SParamComp_Fog::mfGet()
{
  return 0;
}

