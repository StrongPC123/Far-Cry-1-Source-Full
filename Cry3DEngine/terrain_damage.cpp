////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_damage.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: terrain deformations
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "terrain_sector.h"
#include "detail_grass.h"
#include "objman.h"

void CTerrain::ExplodeTerrain(Vec3d vExploPos, float fExploRadius, CObjManager * pObjManager, bool bDeformTerrain)
{
  // calc area
  int x1=int(vExploPos.x-fExploRadius-CTerrain::GetHeightMapUnitSize());
  int y1=int(vExploPos.y-fExploRadius-CTerrain::GetHeightMapUnitSize());
  int x2=int(vExploPos.x+fExploRadius+CTerrain::GetHeightMapUnitSize());
  int y2=int(vExploPos.y+fExploRadius+CTerrain::GetHeightMapUnitSize());

  int nUnitSize = GetHeightMapUnitSize();

  x1=x1/nUnitSize*nUnitSize;
  x2=x2/nUnitSize*nUnitSize;
  y1=y1/nUnitSize*nUnitSize;
  y2=y2/nUnitSize*nUnitSize;

  // limits
  if(x1<0) x1=0;
  if(y1<0) y1=0;
  if(x2>=CTerrain::GetTerrainSize()) x2=CTerrain::GetTerrainSize()-1;
  if(y2>=CTerrain::GetTerrainSize()) y2=CTerrain::GetTerrainSize()-1;

  fExploRadius -= (float)fabs(vExploPos.z - GetZ(int((x1+x2)/2.f),int((y1+y2)/2.f)));

  if(fExploRadius<0.125f)
    return; // to hight

  // get near sectors
  list2<CSectorInfo*> lstNearSecInfos;
	for(int x=x1; x<=x2; x++)
	for(int y=y1; y<=y2; y++)
	{
		CSectorInfo * pInfo = GetSecInfo(x,y);
		if(pInfo && lstNearSecInfos.Find(pInfo)<0)
			lstNearSecInfos.Add(pInfo);
	}

	{
		CSectorInfo * pInfo = GetSecInfo(0,0);
		if(pInfo && lstNearSecInfos.Find(pInfo)<0)
			lstNearSecInfos.Add(pInfo);
	}

	// remove small objects around
  int s;
  for( s=0; s<lstNearSecInfos.Count(); s++)
  {
    CSectorInfo * pSecInfo = lstNearSecInfos[s];
    for(int i=0; i<pSecInfo->m_lstEntities[STATIC_ENTITIES].Count(); i++)
    {
			IEntityRender * pEntityRender =	pSecInfo->m_lstEntities[STATIC_ENTITIES][i];
			Vec3d vEntBoxMin,vEntBoxMax;
			pEntityRender->GetBBox(vEntBoxMin,vEntBoxMax);
			float fEntRadius = vEntBoxMin.GetDistance(vEntBoxMax)*0.5f;
			Vec3d vEntCenter = (vEntBoxMin+vEntBoxMax)*0.5;
			float fDist = GetDistance(vExploPos,vEntCenter);
      if(fDist < fExploRadius+fEntRadius &&
				Overlap::Sphere_AABB(Sphere(vExploPos,fExploRadius), AABB(vEntBoxMin,vEntBoxMax)))
			{
				if(fDist>=fExploRadius)
				{ // 
					Matrix44 objMat;
					CStatObj * pStatObj = (CStatObj *)pEntityRender->GetEntityStatObj(0,&objMat);
					if(!pStatObj)
						continue;
					objMat.Invert44();
					Vec3d vOSExploPos = objMat.TransformPointOLD(vExploPos);

					Vec3d vScaleTest(0,0,1.f);
					vScaleTest = objMat.TransformVectorOLD(vScaleTest);
					float fObjScaleInv = vScaleTest.len();

					if(!pStatObj->IsSphereOverlap(Sphere(vOSExploPos,fExploRadius*fObjScaleInv)))
						continue;
				}

				if(1)
				{
					if(pEntityRender->GetEntityRenderType() == eERType_Vegetation && fEntRadius < fExploRadius)
					{ // remove this object
						pSecInfo->m_lstEntities[STATIC_ENTITIES].Delete(i);
						delete pEntityRender;
						i--;
					}
					else
					{ // if something was imposible to destroy - disable deformation
						bDeformTerrain = false;
					}
				}
			}
    }
  }

	if(GetCVars()->e_decals == 2)
		UpdateLoadingScreen("CTerrain::ExplodeTerrain: %s", bDeformTerrain ? "Yes" : "No");

	// find min max of result
	float fResultZMin, fResultZMax;
	fResultZMin = fResultZMax = GetHMValue(x1,y1)*TERRAIN_Z_RATIO;

  // modify hightfield
  for(int x=x1; x<=x2; x+=CTerrain::GetHeightMapUnitSize())
  for(int y=y1; y<=y2; y+=CTerrain::GetHeightMapUnitSize())
  {
    ushort & sValue = GetHMValue(x,y);

    float fDamage = (fExploRadius - GetDistance(vExploPos,Vec3d((float)x,(float)y,GetZ(x,y))))/fExploRadius;
    if(fDamage<0)
      continue;

		if(sValue*TERRAIN_Z_RATIO > fResultZMax)
			fResultZMax = sValue*TERRAIN_Z_RATIO;

    // remember flags
    int flags = sValue & (INFO_BITS_MASK);

    if(bDeformTerrain && sValue > GetWaterLevel()*256+0.25f*256) // do not change hmap if there are trees here
    {
      sValue -= short(fDamage*2/TERRAIN_Z_RATIO);
      if(sValue < GetWaterLevel()*256+0.25f*256)
        sValue = short(GetWaterLevel()*256+0.25f*256);
    }

    if(fDamage>0.5f)
      flags |= EXPLO_BIT_MASK;

    // restore flags
    sValue = (sValue & ~(INFO_BITS_MASK)) | flags ;

    m_bHightMapModified = true;
  
		if(sValue*TERRAIN_Z_RATIO < fResultZMin)
			fResultZMin = sValue*TERRAIN_Z_RATIO;
	}

  // update terrain video buffers
  for(s=0; s<lstNearSecInfos.Count(); s++)
  {
    CSectorInfo * pSecInfo = lstNearSecInfos[s];
    if(pSecInfo)
    {
      pSecInfo->ReleaseHeightMapVertBuffer();
      pSecInfo->SetMinMaxMidZ();
    }
  }
  
  // update detail texture layers
  m_nDetailTexFocusX=m_nDetailTexFocusY=-CTerrain::GetTerrainSize();
  if(m_pDetailObjects)
    m_pDetailObjects->UpdateGrass();

	// delete decals what can not be correctly updated
	Get3DEngine()->DeleteDecalsInRange( Vec3d((float)x1,(float)y1,fResultZMin), Vec3d((float)x2,(float)y2,fResultZMax+1.f), false );
}

void CTerrain::GetObjectsAround(Vec3d vExploPos, float fExploRadius, list2<IEntityRender*> * pEntList)
{
	assert(pEntList);

	// calc area
	int x1=int(vExploPos.x-fExploRadius-TERRAIN_SECTORS_MAX_OVERLAPPING);
	int y1=int(vExploPos.y-fExploRadius-TERRAIN_SECTORS_MAX_OVERLAPPING);
	int x2=int(vExploPos.x+fExploRadius+TERRAIN_SECTORS_MAX_OVERLAPPING);
	int y2=int(vExploPos.y+fExploRadius+TERRAIN_SECTORS_MAX_OVERLAPPING);

	int nUnitSize = GetHeightMapUnitSize();

	x1=x1/nUnitSize*nUnitSize;
	x2=x2/nUnitSize*nUnitSize;
	y1=y1/nUnitSize*nUnitSize;
	y2=y2/nUnitSize*nUnitSize;

	// limits
	if(x1<0) x1=0;
	if(y1<0) y1=0;
	if(x2>=CTerrain::GetTerrainSize()) x2=CTerrain::GetTerrainSize()-nUnitSize;
	if(y2>=CTerrain::GetTerrainSize()) y2=CTerrain::GetTerrainSize()-nUnitSize;

	// get near outdoor sectors
	list2<CSectorInfo*> lstNearSecInfos;
	for(int x=x1; x<=x2; x+=nUnitSize)
	for(int y=y1; y<=y2; y+=nUnitSize)
	{
		CSectorInfo * pInfo = GetSecInfo(x,y);
		if(pInfo && lstNearSecInfos.Find(pInfo)<0)
			lstNearSecInfos.Add(pInfo);
	}

	{ // add 0,0 outdoor sector
		CSectorInfo * pInfo = GetSecInfo(0,0);
		if(pInfo && lstNearSecInfos.Find(pInfo)<0)
			lstNearSecInfos.Add(pInfo);
	}

	// find static objects around
	for( int s=0; s<lstNearSecInfos.Count(); s++)
	{
		CSectorInfo * pSecInfo = lstNearSecInfos[s];
		for(int i=0; i<pSecInfo->m_lstEntities[STATIC_ENTITIES].Count(); i++)
		{
			IEntityRender * pEntityRender =	pSecInfo->m_lstEntities[STATIC_ENTITIES][i];
			Vec3d vEntBoxMin, vEntBoxMax;
			pEntityRender->GetBBox(vEntBoxMin, vEntBoxMax);
			if(Overlap::Sphere_AABB(Sphere(vExploPos,fExploRadius), AABB(vEntBoxMin,vEntBoxMax)))
				if(pEntList->Find(pEntityRender)<0)
					pEntList->Add(pEntityRender);
		}
	}
}