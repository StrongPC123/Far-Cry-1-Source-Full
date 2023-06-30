////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjshadow.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: shadow maps
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StatObj.h"
#include "../RenderDll/Common/shadow_renderer.h"

void CStatObj::PrepareShadowMaps(const Vec3d & obj_pos, ShadowMapLightSource * pLSource)
{
	if(!GetCVars()->e_shadow_maps)
		return;

  // reset frustums
  int f;
  for(f=0; f<pLSource->m_LightFrustums.Count(); f++)
  { delete pLSource->m_LightFrustums[f].pModelsList; pLSource->m_LightFrustums[f].pModelsList=0; }
  pLSource->m_LightFrustums.Reset();

  { // define new frustum
    ShadowMapFrustum new_lof;
//    memset(&new_lof,0,sizeof(ShadowMapFrustum));

    new_lof.pModelsList = new list2<IStatObj*>;
    new_lof.pModelsList->PreAllocate(128);
    
    list2<IStatObj*> so;
    so.Add(this);

    ShadowMapFrustum * lof = GetRenderer()->MakeShadowMapFrustum(&new_lof, pLSource, obj_pos+GetCenter()/*Vec3d(0,0,GetCenterZ())*/, &so, EST_DEPTH_BUFFER);

		lof->pOwnerGroup = this;

    if(lof)
    {
      pLSource->m_LightFrustums.Add(*lof);
    }

    // this lists are in another object now
    new_lof.pModelsList=0;
    new_lof.pEntityList=0;
  }
 
  int nTexSize = GetCVars()->e_max_shadow_map_size;
  while(nTexSize > GetRadius()*200)
    nTexSize/=2;

  if(nTexSize<16)
    nTexSize=16; // in case of error

  pLSource->m_LightFrustums[f].nTexSize = nTexSize;

  // make depth textures
  //for( f=0; f<pLSource->m_LightFrustums.Count(); f++)
  //  GetRenderer()->PrepareDepthMap(&pLSource->m_LightFrustums[f], true);
}

void CStatObj::MakeShadowMaps(const Vec3d vSunPos)
{
  if(m_pSMLSource)
  {
    for(int f=0; f<m_pSMLSource->m_LightFrustums.Count(); f++)
    { 
      if(m_pSMLSource->m_LightFrustums[f].depth_tex_id)
      {
				// shadow maps can be deleted only on smaps pool destruction
//        GetRenderer()->RemoveTexture(m_pSMLSource->m_LightFrustums[f].depth_tex_id);
        m_pSMLSource->m_LightFrustums[f].depth_tex_id=0;
      }
    }
    m_pSMLSource->m_LightFrustums.Reset();
  }

  delete m_pSMLSource;
  m_pSMLSource = new ShadowMapLightSource;
  memset(m_pSMLSource,0,sizeof(ShadowMapLightSource));

  m_pSMLSource->fRadius = 500000;  

  m_pSMLSource->vSrcPos = GetNormalized(vSunPos)*10000;//0;

  if(GetCVars()->e_shadow_maps)
    PrepareShadowMaps(Vec3d(0,0,0), m_pSMLSource);
}
