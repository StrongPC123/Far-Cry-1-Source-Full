////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjmandraw.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Draw static objects (vegetations)
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "objman.h"
#include "visareas.h"
#include "terrain_sector.h"
#include "3dengine.h"
#include "cbuffer.h"

bool CObjManager::IsBoxOccluded( const Vec3d & vBoxMin, const Vec3d & vBoxMax, float fDistance, OcclusionTestClient * pOcclTestVars )
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

  assert(pOcclTestVars);
  assert(fDistance>=0);

  if(m_nRenderStackLevel)
    return pOcclTestVars->bLastResult; // return result of base level test

  if(pOcclTestVars->nLastVisibleFrameID < GetFrameID()-1)
  { // if was invisible last frames (because of frustum culling)
    pOcclTestVars->ucOcclusionByObjectsFrames = 0; // force to test this frame

		if(GetCVars()->e_terrain_occlusion_culling!=2)
	    pOcclTestVars->ucOcclusionByTerrainFrames = 0; // force to test this frame
  }

  { // test occlusion by objects
    if(!pOcclTestVars->ucOcclusionByObjectsFrames && fDistance>COVERAGEBUFFER_OCCLUSION_TESTERS_MIN_DISTANCE && GetCVars()->e_cbuffer)
    { // less distance do not work because objects are sorted wrong
      if(!m_pCoverageBuffer->IsBBoxVisible(vBoxMin,vBoxMax))
      {
        pOcclTestVars->ucOcclusionByObjectsFrames = 0; // force to test next frame
        pOcclTestVars->bLastResult = true;
        return true;
      }
    }

    pOcclTestVars->ucOcclusionByObjectsFrames += 4; // skip testing next frame
  }
/*
	{ // test occl by antiportals
		if(GetVisAreaManager()->IsOccludedByOcclVolumes(vBoxMin,vBoxMax))
			return true;
	}
*/
  { // test occlusion by terrain
    Vec3d vTopMax = vBoxMax; 
    Vec3d vTopMin = vBoxMin; vTopMin.z = vTopMax.z;

    const Vec3d & vCamPos = GetViewCamera().GetPos();

		vTopMax.CheckMin(Vec3d((float)CTerrain::GetTerrainSize(),(float)CTerrain::GetTerrainSize(),1024.f));
		vTopMin.CheckMax(Vec3d(0,0,-1024.f));

		vTopMin.CheckMin(Vec3d((float)CTerrain::GetTerrainSize(),(float)CTerrain::GetTerrainSize(),1024.f));
		vTopMax.CheckMax(Vec3d(0,0,-1024.f));

    if( !pOcclTestVars->ucOcclusionByTerrainFrames && GetCVars()->e_terrain_occlusion_culling )
    {    
      int nMaxTestsToScip = (GetVisAreaManager()->m_pCurPortal) ? 3 : 10000;

			// precision in meters for this object
			float fMaxStep = fDistance*GetCVars()->e_terrain_occlusion_culling_precision;

      if( (fMaxStep < (vTopMax.x - vTopMin.x)*2.f || fMaxStep < (vTopMax.y - vTopMin.y)*2.f) && 
//				fDistance<700 &&  
				vBoxMin.x != vBoxMax.x && vBoxMin.y != vBoxMax.y )
      {
        bool bOccluded = true;

        // todo: debug this
        float dx = (vTopMax.x - vTopMin.x)*0.99999f;
        while(dx>fMaxStep)
          dx*=0.5f;

        float dy = (vTopMax.y - vTopMin.y)*0.99999f;
        while(dy>fMaxStep)
          dy*=0.5f;

        // todo: test only borders
        for(float x=vTopMin.x; x<=vTopMax.x; x+=dx)
        for(float y=vTopMin.y; y<=vTopMax.y; y+=dy)
        {
          if(!m_pTerrain->IsPointOccludedByTerrain(Vec3d(x, y, vTopMax.z), fDistance, vCamPos, nMaxTestsToScip))
          {
            bOccluded = false;
            x=y=1000000;//break
          }
        }

        if(bOccluded)
        {
					if(GetCVars()->e_terrain_occlusion_culling!=2)
	          pOcclTestVars->ucOcclusionByTerrainFrames = 0; // force to test next frame

          pOcclTestVars->bLastResult = true;
          return true;
        }
      }
      else
      {
				Vec3d vTopMid = (vTopMin+vTopMax)*0.5f; 
        if( m_pTerrain->IsPointOccludedByTerrain(vTopMid, fDistance,vCamPos, nMaxTestsToScip))
        {
					if(GetCVars()->e_terrain_occlusion_culling!=2)
						pOcclTestVars->ucOcclusionByTerrainFrames = 0; // force to test next frame
          
					pOcclTestVars->bLastResult = true;
          return true;
        }
      }
    }

		if(pOcclTestVars->ucOcclusionByTerrainFrames == 0) // && GetCVars()->e_terrain_occlusion_culling==3)
		{ // randomize test time, do this only here otherwice ucOcclusionByTerrainFrames may never become 0 and next test will not happend
			static int arrRnd[16] = 
			{ 
				rand()%4+1, rand()%4+1, rand()%4+1, rand()%4+1, 
				rand()%4+1, rand()%4+1, rand()%4+1, rand()%4+1, 
				rand()%4+1, rand()%4+1, rand()%4+1, rand()%4+1, 
				rand()%4+1, rand()%4+1, rand()%4+1, rand()%4+1
			};
			
			static int nRndCounter = 0;
			nRndCounter++;
			if(nRndCounter>=16)
				nRndCounter=0;

			pOcclTestVars->ucOcclusionByTerrainFrames = arrRnd[nRndCounter]*4;
		}
		else
			pOcclTestVars->ucOcclusionByTerrainFrames += 4; // skip testing next 64 frames
  }

/*	const Vec3d & vCamPos = GetViewCamera().GetPos();

	bool bCameraInBBox = 
		vCamPos.x >= (vBoxMin.x-0.05f) && vCamPos.y >= (vBoxMin.y-0.05f) && vCamPos.z >= (vBoxMin.z-0.05f) &&
		vCamPos.x <= (vBoxMax.x+0.05f) && vCamPos.y <= (vBoxMax.y+0.05f) && vCamPos.z <= (vBoxMax.z+0.05f);

	// Make test only if camera outside of the box and if feature supported
	if(fDistance>1 && !bCameraInBBox && GetCVars()->e_hw_occlusion_culling_objects && (GetRenderer()->GetFeatures() & RFT_OCCLUSIONTEST) )
	{
		// construct RE if needed
		for(int i=0; i<2; i++)
		{
			if(!pOcclTestVars->arrREOcclusionQuery[i])
				pOcclTestVars->arrREOcclusionQuery[i] = 
				(CREOcclusionQuery *)GetRenderer()->EF_CreateRE(eDATA_OcclusionQuery);

			pOcclTestVars->arrREOcclusionQuery[i]->m_vBoxMin = vBoxMin;
			pOcclTestVars->arrREOcclusionQuery[i]->m_vBoxMax = vBoxMax;
		}

		int nSlotId = GetFrameID()&1;
		GetRenderer()->EF_AddEf(0, pOcclTestVars->arrREOcclusionQuery[nSlotId], m_pShaderOcclusionQuery, 0);

		if(GetCVars()->e_hw_occlusion_culling_objects>1)
		{
			float fVis = (pOcclTestVars->arrREOcclusionQuery[!nSlotId]->m_nVisSamples >= 2);
			float fCol[] = {fVis,!fVis,1,1};
			GetRenderer()->DrawLabelEx((vBoxMin + vBoxMax)*0.5f,6, fCol, false, true, "%s", 
				fVis ? "V" : "N");
		}

		if( 
			pOcclTestVars->arrREOcclusionQuery[0]->m_nVisSamples < 2 &&
			pOcclTestVars->arrREOcclusionQuery[1]->m_nVisSamples < 2 &&
			1 >= abs( pOcclTestVars->arrREOcclusionQuery[0]->m_nCheckFrame - 
								pOcclTestVars->arrREOcclusionQuery[1]->m_nCheckFrame ) &&
			GetFrameID()-2 <= pOcclTestVars->arrREOcclusionQuery[0]->m_nDrawFrame &&
			GetFrameID()-2 <= pOcclTestVars->arrREOcclusionQuery[1]->m_nDrawFrame )
    { // return true only if last test is not older than 1 frame
      pOcclTestVars->bLastResult = true;
      return true;
    }
	}*/

  pOcclTestVars->bLastResult = false;
  pOcclTestVars->nLastVisibleFrameID = GetFrameID();

  return false;
}

void CObjManager::PrefechObjects()
{
	for (ObjectsMap::iterator it = m_lstLoadedObjects.begin(); it != m_lstLoadedObjects.end(); ++it)
	{
		CStatObj * pStatObj = (*it);
		SRendParams params;
		params.nDLightMask = 1;
		GetRenderer()->EF_StartEf();
		for(int i=0; i<3; i++)
			pStatObj->Render(params,Vec3(zero),i);
		GetRenderer()->EF_EndEf3D(0);
	}
}
