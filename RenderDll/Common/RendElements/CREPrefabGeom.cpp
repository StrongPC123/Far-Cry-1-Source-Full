/*=============================================================================
	CREPrefabGeom.cpp : implementation of wall geometry RE for the editor.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#include "RenderPCH.h"
#include "RendElement.h"

//===============================================================

CRendElement *CREPrefabGeom::mfCopyConstruct(void)
{
  CREPrefabGeom *cp = new CREPrefabGeom;
  *cp = *this;
  return cp;
}

bool CREPrefabGeom::mfCompile(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eModel=1};
  static tokenDesc commands[] =
  {
    {eModel, "Model"},

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
      case eModel:
        mModel = NULL;//(CModelCgf *)CComModel::mfForName(data);
        break;
    }
  }

  if (mModel)
    return true;
  return false;
}

void CREPrefabGeom::mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);
  
  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_RendNumVerts = 0;
}

bool CREPrefabGeom::mfDraw(SShader *ef, SShaderPass *sl)
{
  //if (mModel)
  //  mModel->mfDraw(ef, bCamera);
  return true;
}