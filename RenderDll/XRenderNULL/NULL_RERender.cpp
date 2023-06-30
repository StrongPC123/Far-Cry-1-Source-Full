/*=============================================================================
  PS2_RERender.cpp : implementation of the Rendering RenderElements pipeline.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "NULL_Renderer.h"
#include "I3dengine.h"

//#include "../cry3dengine/StatObj.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

//=======================================================================

void CREFlareGeom::mfCheckVis(CFColor &col, CCObject *obj)
{
}

//#include "..\common\shadow_renderer.h"

///////////////////////////////////////////////////////////////////

CREOcclusionQuery::~CREOcclusionQuery()
{
}

void CREOcclusionQuery::mfReset()
{
}

bool CREScreenProcess::mfDrawLowSpec(SShader *ef, SShaderPass *sfm)
{
  return 1;
}

bool CREOcclusionQuery::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  return true;
}

bool CREOcLeaf::mfPreDraw(SShaderPass *sl)
{
  return true;
}

bool CREGlare::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  return true;
}

bool CREOcLeaf::mfDraw(SShader *ef, SShaderPass *sl)
{
  return (true);
}

///////////////////////////////////////////////////////////////////
void CREOcLeaf::mfEndFlush(void)
{
}

//===============================================================================

void CRE2DQuad::mfPrepare()
{
}

bool CRE2DQuad::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  return 1;
}
//===============================================================================

bool CRETempMesh::mfDraw(SShader *ef, SShaderPass *sl)
{
  return true;
}
bool CRETempMesh::mfPreDraw(SShaderPass *sl)
{
  return true;
}
void CRETempMesh::mfReset()
{
}

//=========================================================================================

bool CREFlare::mfCheckVis(CCObject *obj)
{
  return false;
}

void CREFlare::mfDrawFlares(SShader *ef, CFColor &col)
{
}

void CREFlare::mfDrawCorona(SShader *ef, CFColor &col)
{
}

bool CREFlare::mfDraw(SShader *ef, SShaderPass *sfm)
{
  return true;
}

bool CREFlashBang::mfDraw(SShader *ef, SShaderPass *sfm)
{   
  return true;
}
void CREFlashBang::mfPrepare()
{   
}

// screen process 
bool CREScreenProcess:: mfDraw(SShader *ef, SShaderPass *sfm)
{
  return 1;
}

//=====================================================================================

bool CREClearStencil::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  return true;
}

bool CRETriMeshShadow::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  return true;
}

bool CREHDRProcess::mfDraw(SShader *ef, SShaderPass *sfm)
{
  return true;
}