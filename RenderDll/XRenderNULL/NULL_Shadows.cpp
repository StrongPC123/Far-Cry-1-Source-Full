//////////////////////////////////////////////////////////////////////
//
//  Crytek CryENGINE Source code
//  
//  File: PS2_Shadows.cpp
//  Description: Implementation of the shadow maps using PS2 renderer API
//  shadow map calculations
//
//  History:
//  -Jan 31,2001:Created by Vladimir Kajain
//
//////////////////////////////////////////////////////////////////////

#include "RenderPCH.h"
#include "NULL_Renderer.h"
#include "../Common/shadow_renderer.h"

#include "I3dengine.h"

// render depth/shadow map into texture from light source position
void CNULLRenderer::PrepareDepthMap(ShadowMapFrustum * lof, bool make_new_tid)
{
}

void CNULLRenderer::SetupShadowOnlyPass(int Num, ShadowMapFrustum * pFrustum, Vec3d * vShadowTrans, const float fShadowScale, Vec3d vObjTrans, float fObjScale, const Vec3d vObjAngles, Matrix44 * pObjMat)
{
}

void CNULLRenderer::OnEntityDeleted(IEntityRender * pEntityRender)
{
}

void CNULLRenderer::DrawAllShadowsOnTheScreen()
{
}
