////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terran_edit.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: add/remove static objects, modify hmap (used by editor)
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "terrain_sector.h"
#include "StatObj.h"
#include "objman.h"
#include "detail_grass.h"
#include "3dengine.h"

bool CTerrain::AddStaticObject(int nObjectID, const Vec3d & vPos, const float fScale, CObjManager * pObjManager, uchar ucBright)
{
  if( vPos.x<=0 || vPos.y<=0 || vPos.x>=CTerrain::GetTerrainSize() || vPos.y>=CTerrain::GetTerrainSize() || fScale<=0 )
		return 0;

  if(nObjectID<0 || nObjectID >= pObjManager->m_lstStaticTypes.Count())
  { // object id is out of range 
		/*assert(0); */return 0;
	}

	//int ix=int(vPos.x)/CTerrain::GetSectorSize(), iy=int(vPos.y)/CTerrain::GetSectorSize();
//  CSectorInfo * info = m_arrSecInfoTable[ix][iy];
/*
  CStatObjInst so;
  so.Init( vPos, nObjectID, 1, fScale );
//  so.m_fDistance = so.m_fMaxDist = pObjManager->GetXYRadius(so.m_nObjectTypeID)*so.m_fScale*STAT_OBJ_MAX_VIEW_DISTANCE_RATIO;  
  so.m_ucBright = ucBright;
  
	if (pObjManager->PhysicalizeStatObjInst( &so ))
	{
		pe_params_foreign_data pfd;
		pfd.pForeignData = (void*)(info->m_lstStatObjects.Count()<<16|iy<<8|ix);
		so.m_pPhysEnt->SetParams(&pfd);
	}

	RegisterStatObjInstanceShadow( pObjManager, so );

  info->m_lstStatObjects.Add(so);

	SortSectorInstancesBySizeAndInitMaxViewDist(info);

  // update physics
  pe_params_foreign_data pfd;
  for(int j=0;j<info->m_lstStatObjects.Count();j++)	
    if (info->m_lstStatObjects[j].m_pPhysEnt)
    {
      info->m_lstStatObjects[j].m_pPhysEnt->GetParams(&pfd);
      pfd.pForeignData = (void*)((int)pfd.pForeignData&0x0FFFF | j<<16);
      info->m_lstStatObjects[j].m_pPhysEnt->SetParams(&pfd);
    }
  */

//  {
    CStatObjInst * pEnt = (CStatObjInst*)((C3DEngine*)Get3DEngine())->CreateVegetation();
    pEnt->m_fScale = fScale;
    pEnt->m_vPos = vPos;
    if(!pEnt->m_pEntityRenderState)
      pEnt->m_pEntityRenderState = Get3DEngine()->MakeEntityRenderState();
    pEnt->SetStatObjGroupId(nObjectID);
    pEnt->m_ucBright = ucBright;

		if(m_pObjManager->m_lstStaticTypes[nObjectID].GetStatObj())
		{
			pEnt->m_vWSBoxMin = pEnt->m_vPos + m_pObjManager->m_lstStaticTypes[nObjectID].GetStatObj()->GetBoxMin()*fScale;
			pEnt->m_vWSBoxMax = pEnt->m_vPos + m_pObjManager->m_lstStaticTypes[nObjectID].GetStatObj()->GetBoxMax()*fScale;
		}
		else
		{
			Warning(0,0,"I3DEngine::AddStaticObject: Attempt to add object of undefined type");
			return 0;
		}

    pEnt->Physicalize( );
//    pEnt->SetMaxViewDist();
    Get3DEngine()->RegisterEntity(pEnt);
  //}

	// update sector bbox
	//Vec3d vObjBoxMin, vObjBoxMax;
//	pObjManager->GetStaticObjectBBox(so.m_nObjectTypeID, vObjBoxMin, vObjBoxMax);

//	float fObjMaxZ = pEnt->m_vWSBoxMax.z;
//	if(info->m_vBoxMax.z < fObjMaxZ)
	//	info->m_vBoxMax.z = fObjMaxZ;

  return true;
}

void CTerrain::RemoveAllStaticObjects()
{
  for( int x=0; x<CTerrain::GetSectorsTableSize(); x++)
  for( int y=0; y<CTerrain::GetSectorsTableSize(); y++)
  if(m_arrSecInfoTable[x][y])
  {
    m_arrSecInfoTable[x][y]->Unload(true);
  }
}

bool CTerrain::RemoveStaticObject(int nObjectID, const Vec3d & vPos, CObjManager * pObjManager)
{
	int nRange = GetSectorSize();
	list2<CSectorInfo*> lstProcessedSectors;
  for(int a=-nRange; a<=nRange; a+=nRange/2)
  for(int b=-nRange; b<=nRange; b+=nRange/2)
  {
    int x = (int)vPos.x+a;
    int y = (int)vPos.y+b;

    if( x<0 || y<0 || x>=CTerrain::GetTerrainSize() || y>=CTerrain::GetTerrainSize() )
		  continue;

    CSectorInfo * info = m_arrSecInfoTable[x/CTerrain::GetSectorSize()][y/CTerrain::GetSectorSize()];

		if(lstProcessedSectors.Find(info)<0)
		{
			info->Unload(true, vPos);
			lstProcessedSectors.Add(info);
		}
  }

	m_arrSecInfoTable[0][0]->Unload(true, vPos);
    
  /*for(int n=0; n<info->m_lstStatObjects.Count(); n++)
  {
    Vec3d vPosSameZ = vPos;
		vPosSameZ.z = info->m_lstStatObjects[n].m_vPos.z;
		if( GetSquaredDistance(info->m_lstStatObjects[n].m_vPos,vPosSameZ) < 0.01f )
			if( nObjectID<0 || info->m_lstStatObjects[n].m_nObjectTypeID==nObjectID )
    {
      {
	      CStatObjInst * pStatObjInst = info->m_lstStatObjects.Get(n);
	      if(pStatObjInst->m_pPhysEnt)
          GetSystem()->GetIPhysicalWorld()->DestroyPhysicalEntity(pStatObjInst->m_pPhysEnt);
        pStatObjInst->m_pPhysEnt=0;
      }

			RegisterStatObjInstanceShadow( pObjManager, info->m_lstStatObjects[n], true );

      info->m_lstStatObjects.Delete(n);

			pe_params_foreign_data pfd;
			for(int j=n;j<info->m_lstStatObjects.Count();j++)	
				if (info->m_lstStatObjects[j].m_pPhysEnt)
				{
					info->m_lstStatObjects[j].m_pPhysEnt->GetParams(&pfd);
					pfd.pForeignData = (void*)((int)pfd.pForeignData&0x0FFFF | j<<16);
					info->m_lstStatObjects[j].m_pPhysEnt->SetParams(&pfd);
				}
      n--;
    }
  }*/

  return true;
}

void CTerrain::SetSurfaceType(int x, int y, int nType)
{
  assert(CTerrain::GetHeightMapUnitSize()==2);

  nType &= STYPE_BIT_MASK;

  if(x>0 && y>0 && x<CTerrain::GetTerrainSize() && y<CTerrain::GetTerrainSize())
  {
    GetHMValue(x,y) &= ~STYPE_BIT_MASK;
    GetHMValue(x,y) |= nType;

    CSectorInfo * info = GetSectorFromPoint(x,y);
    if(nType == STYPE_HOLE && info)
			info->m_bHasHoles = true;

    if(info)
      info->ReleaseHeightMapVertBuffer();
  }
}

void CTerrain::SetTerainHightMapBlock(int x1, int y1, int nSizeX, int nSizeY, ushort * TerrainBlock, ushort nUpdateMask)
{
  int nHmapSize = CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize();
  if( x1<0 || y1<0 || x1+nSizeX>nHmapSize || y1+nSizeY>nHmapSize )
  {
		Warning(0,0,"CTerrain::SetTerainHightMapBlock: (x1,y1) values out of range");
    return;
  }

  for(int x=x1; x<x1+nSizeX; x++)
  for(int y=y1; y<y1+nSizeY; y++)
  {
		ushort & nValue = GetHMValue(x*CTerrain::GetHeightMapUnitSize(),y*CTerrain::GetHeightMapUnitSize());

		nValue &= (~nUpdateMask); // clear bits
		nValue |= (TerrainBlock[(x-x1)*nSizeY + (y-y1)] & nUpdateMask); // set bits

    // update vert buffers
    CSectorInfo * info = m_arrSecInfoTable[x*CTerrain::GetHeightMapUnitSize()/CTerrain::GetSectorSize()][y*CTerrain::GetHeightMapUnitSize()/CTerrain::GetSectorSize()];

//		if(nUpdateMask & ~STYPE_BIT_MASK)
			info->ReleaseHeightMapVertBuffer();

    // set hole flag for sector
    if((GetHMValue(x*CTerrain::GetHeightMapUnitSize(),y*CTerrain::GetHeightMapUnitSize()) & STYPE_BIT_MASK) == STYPE_HOLE)
			info->m_bHasHoles = true;

		// update bounds
		float fElev = TERRAIN_Z_RATIO*(nValue & (~INFO_BITS_MASK));
    
		if(info->m_fMaxZ < fElev)
			info->m_fMaxZ = fElev;
    
		if(info->m_vBoxMax.z < fElev)
			info->m_vBoxMax.z = fElev;
													/*
		info->m_fMaxZ = max(info->m_fMaxZ, fElev);
    info->m_vBoxMax.z = max(info->m_vBoxMax.z, fElev);*/
  }

  // update detail texture and grass
  m_nDetailTexFocusX=m_nDetailTexFocusY=-CTerrain::GetTerrainSize();
  if(m_pDetailObjects)
    m_pDetailObjects->UpdateGrass();

  GetRenderer()->DeleteLeafBuffer(m_pLowResTerrainLeafBuffer);
  m_pLowResTerrainLeafBuffer=0;
  GetRenderer()->DeleteLeafBuffer(m_pReflectedTerrainLeafBuffer);  
  m_pReflectedTerrainLeafBuffer=0;

  m_bHightMapModified = false;
}
																		 /*
void CTerrain::SetTerainSectorTexture(int nSectorOroginX, int nSectorOroginY, unsigned char * pTexData, int nSizeOffTexData)
{
  if( 
    nSectorOroginX/CTerrain::GetSectorSize() < 0 || 
    nSectorOroginY/CTerrain::GetSectorSize() < 0 || 
    nSectorOroginX/CTerrain::GetSectorSize() >= CTerrain::GetSectorsTableSize() || 
    nSectorOroginY/CTerrain::GetSectorSize() >= CTerrain::GetSectorsTableSize() )
  {
    GetLog()->Log("Error: CTerrain::SetTerainSectorTexture: (nSectorOroginX, nSectorOroginY) values out of range");
    return;
  }

  CSectorInfo * info = m_arrSecInfoTable[nSectorOroginX/CTerrain::GetSectorSize()][nSectorOroginY/CTerrain::GetSectorSize()];
  info->UpdateSectorTexture(pTexData, nSizeOffTexData);
}																			 */

int CTerrain::LockSectorTexture(int nSectorOriginX, int nSectorOriginY, int & nTexDim)
{
  if( 
    nSectorOriginX/CTerrain::GetSectorSize() < 0 || 
    nSectorOriginY/CTerrain::GetSectorSize() < 0 || 
    nSectorOriginX/CTerrain::GetSectorSize() >= CTerrain::GetSectorsTableSize() || 
    nSectorOriginY/CTerrain::GetSectorSize() >= CTerrain::GetSectorsTableSize() )
  {
    Warning(0,0,"CTerrain::LockSectorTexture: (nSectorOriginX, nSectorOriginY) values out of range");
    return 0;
  }

	// disable optimizations of far sectors
	GetCVars()->e_terrain_merge_far_sectors = 0;

  CSectorInfo * info = m_arrSecInfoTable[nSectorOriginX/CTerrain::GetSectorSize()][nSectorOriginY/CTerrain::GetSectorSize()];
	return info->LockSectorTexture(nTexDim);
}

void CTerrain::ResetTerrainVertBuffers()
{
  for(int x=0; x<m_arrSecInfoTable.m_nSize; x++)
  for(int y=0; y<m_arrSecInfoTable.m_nSize; y++)
  {
    CSectorInfo * pInfo = m_arrSecInfoTable[x][y];
  	pInfo->ReleaseHeightMapVertBuffer();
  }
}