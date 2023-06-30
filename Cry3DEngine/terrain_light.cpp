////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_light.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: generate geometry for hmap light pass
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "terrain_sector.h"

bool CTerrain::RenderAreaLeafBuffers(Vec3d vPos, float fRadius, 
                                     int nDynLMask, 
                                     CLeafBuffer ** arrLightLeafBuffers, 
                                     int nMaxLeafBuffersNum,
                                     CCObject * pObj, IShader * pShader,
                                     bool bRecalcLeafBuffers,
																		 const char * szComment,
                                     IShader * pEffStencilTest,
																		 int nShaderSortOrder,
																		 ShadowMapFrustum * pFrustum,
																		 Vec3d * pvFrustPos, float fFrustScale)
{                                                          	
  //if(m_nRenderStackLevel)
  //  return;

  if(!_finite(vPos.x) || !_finite(vPos.y) || !_finite(fRadius))
  {
    Warning( 0,0,"Warning: CTerrain::RenderAreaLeafBuffers: undefined light source position" );
    return false;
  }

  // should be alligned already
  assert(vPos.x  == float(int(vPos.x  + 0.5f*CTerrain::GetHeightMapUnitSize())/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize()));
  assert(vPos.y  == float(int(vPos.y  + 0.5f*CTerrain::GetHeightMapUnitSize())/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize()));
  assert(fRadius == float(int(fRadius + 0.5f*CTerrain::GetHeightMapUnitSize())/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize()));

  // get 2d bounds in sectors array
  int min_x = (int)(((vPos.x - fRadius)/CTerrain::GetSectorSize()));
  int min_y = (int)(((vPos.y - fRadius)/CTerrain::GetSectorSize()));
  int max_x = (int)(((vPos.x + fRadius)/CTerrain::GetSectorSize()));
  int max_y = (int)(((vPos.y + fRadius)/CTerrain::GetSectorSize()));

  // limit bounds
  if( min_x<0 ) min_x = 0; else if( min_x>=CTerrain::GetSectorsTableSize() ) min_x = CTerrain::GetSectorsTableSize()-1;
  if( min_y<0 ) min_y = 0; else if( min_y>=CTerrain::GetSectorsTableSize() ) min_y = CTerrain::GetSectorsTableSize()-1;
  if( max_x<0 ) max_x = 0; else if( max_x>=CTerrain::GetSectorsTableSize() ) max_x = CTerrain::GetSectorsTableSize()-1;
  if( max_y<0 ) max_y = 0; else if( max_y>=CTerrain::GetSectorsTableSize() ) max_y = CTerrain::GetSectorsTableSize()-1;

  // make leaf buffers
  int i=0; bool bREAdded = false;
  for(int x=min_x; x<=max_x; x++)
  for(int y=min_y; y<=max_y; y++)
  {
    if(i>=nMaxLeafBuffersNum)
      break;

		CSectorInfo * pSectorInfo = m_arrSecInfoTable[x][y];
		arrLightLeafBuffers[i] = pSectorInfo->MakeSubAreaLeafBuffer(vPos, fRadius, arrLightLeafBuffers[i], pShader, bRecalcLeafBuffers,szComment,pFrustum, pvFrustPos, fFrustScale);

		if(arrLightLeafBuffers[i])
		{
			// render
			pObj->m_Matrix.SetIdentity();
			pObj->m_DynLMMask = nDynLMask;

			if(arrLightLeafBuffers[i]->m_NumIndices)
			{
				arrLightLeafBuffers[i]->SetRECustomData((m_arrSecInfoTable[x][y]->m_arrTexOffsets));
				arrLightLeafBuffers[i]->AddRE(pObj, 0, EFSLIST_STENCIL | nShaderSortOrder, pEffStencilTest);
				bREAdded = true;
			}
		}

		i++;
  }

	return bREAdded;
}
