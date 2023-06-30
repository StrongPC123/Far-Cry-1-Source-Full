/*=============================================================================
  D3DShadows.cpp : shadows support.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honich Andrey

=============================================================================*/

#include "stdafx.h"
#include "DriverD3D8.h"
#include "../Common/Shadow_Renderer.h"

// draw grid and project depth map on it
/*void CD3D8Renderer::DrawShadowGrid(const Vec3d & pos, const Vec3d & Scale, ShadowMapFrustum*lf, bool translate_projection, float alpha, IndexedVertexBuffer* pVertexBuffer, float anim_angle)
{
}	*/

void CD3D8Renderer::SetupShadowOnlyPass(int Num, ShadowMapFrustum * pFrustum, Vec3d * vShadowTrans, const float fShadowScale, Vec3d vObjTrans, float fObjScale, const Vec3d vObjAngles, Matrix44 * pObjMat)
{  
}

// Make 8-bit identity texture that maps (s)=(z) to [0,255]/255.
int CD3D8Renderer::MakeShadowIdentityTexture()
{ 
  return 0;
}

// setup projection texgen
void CD3D8Renderer::ConfigShadowTexgen(int Num, int rangeMap, ShadowMapFrustum * pFrustum, float * pLightFrustumMatrix, float * pLightViewMatrix)
{
}


