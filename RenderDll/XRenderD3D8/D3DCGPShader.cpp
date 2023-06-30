/*=============================================================================
  GLCGPShader.cpp : OpenGL cg pixel shaders support.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "stdafx.h"
#include "GL_Renderer.h"
#include "GLCGPShader.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

CPShader *CCGPShader_GL::m_LastPS;

//=======================================================================

void CCGPShader_GL::mfReset()
{
  mfDel();
}

CCGPShader_GL::~CCGPShader_GL()
{
  int i;

  if (m_Script)
  {
    delete [] m_Script;
    m_Script = NULL;
  }
  for (i=0; i<m_Params.Num(); i++)
  {
    SCGParam *pr = &m_Params[i];
    SAFE_DELETE_ARRAY(pr->m_ParamName);
  }
  m_Params.Free();
  m_Params.Free();
  mfDel();
}

void CCGPShader_GL::mfCompileParam4f(char *scr, SShader *ef, TArray<SCGParam4f> *Params)
{
  guard(CCGPShader_GL::mfCompileParam4f);

  gcEf.mfCompileCGParam(scr, ef, Params);

  unguard;
}

bool CCGPShader_GL::mfCompile(char *scr, SShader *ef)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  guard(CCGPShader_GL::mfCompile);

  enum {eScript=1, eParam4f, eParamStateMatrix, ePointer, eNoFog};
  static tokenDesc commands[] =
  {
    {eScript, "Script"},
    {eParam4f, "Param4f"},

    {0,0}
  };

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
      case eParam4f:
        mfCompileParam4f(params, ef, &m_Params);
        break;

      case eScript:
        {
          int len = strlen(data) + 1;
          m_Script = new char [len];
          memcpy(m_Script, data, len);
        }
        break;
    }
  }

  unguard;

  return 1;
}

void CCGPShader_GL::mfSetVariables(TArray<SCGParam4f>* Vars)
{
  int i;

  if (!Vars)
    return;

  for (i=0; i<Vars->Num(); i++)
  {
    SCGParam4f *p = &Vars->Get(i);
    if (!p->m_pBind)
    {
      char str[128];
      strcpy(str, "main.");
      strcat(str, p->m_ParamName);
      p->m_pBind = cgGetBindByName(m_CGProgram, str);
      if (p->m_pBind == NULL)
      {
        iLog->Log("Couldn't find parameter '%s' in CG program '%s' (%s)", str, m_Name);
        p->m_pBind = (void *)(-1);
        continue;
      }
    }
    if ((int)p->m_pBind == -1)
      continue;
    float *v = p->mfGet();
    mfParameter4f((cgBindIter *)p->m_pBind, v);
  }
}

bool CCGPShader_GL::mfActivate()
{
  if (!m_CGProgram)
  {
    mfLoad(m_Script);
    mfUnbind();
  }

  return true;
}

bool CCGPShader_GL::mfSet(bool bStat, SShadeLayerHW *slw)
{
  if (!bStat)
  {
    if (CRenderer::CV_r_log == 4)
      gRenDev->Logv(SRendItem::m_RecurseLevel, "--- Reset CGPShader \"%s\"\n", m_Name);
    mfDisable();

    return true;
  }

  if (CRenderer::CV_r_log == 4)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "--- Set CGPShader \"%s\"\n", m_Name);

  if (m_CGProgram < 0)
    return false;

  if (!m_CGProgram)
  {
    if (!mfActivate())
      return false;
    m_LastPS = NULL;
  }

  if (m_Frame != gRenDev->GetFrameID())
  {
    m_Frame = gRenDev->GetFrameID();
    gRenDev->m_RP.m_PS.m_NumCGPShaders++;
  }
  if (m_LastPS != this)
  {
    m_LastPS = this;
    mfBind();
  }
  mfEnable();
  mfSetVariables(&m_Params);
  if (slw)
    mfSetVariables(slw->m_CGPSParams);

  return true;
}
