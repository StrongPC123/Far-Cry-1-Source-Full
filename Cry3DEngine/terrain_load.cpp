////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_load.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: terrain loading
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "terrain_sector.h"
#include "objman.h"
#include "detail_grass.h"
#include "terrain_water.h"
#include "3dengine.h"

#ifdef PS2
#include "File.h"
#endif

CTerrain::CTerrain( )
{
  memset(this,0,sizeof(CTerrain)); // there is no virtual functions here

  m_fLodLFactor=1;

  m_nOldSectorsX = 0;
  m_nOldSectorsY =-1;

  m_pSHShore = GetRenderer()->EF_LoadShader("TerrainWaterBeach", eSH_World, EF_SYSTEM);
	m_pLowResTerrainShader = GetRenderer()->EF_LoadShader("TerrainLowLOD", eSH_World, EF_SYSTEM);
	m_matSecondPass.shaderItem.m_pShader = GetRenderer()->EF_LoadShader("TerrainDetailLayers", eSH_World, EF_SYSTEM);

  m_fShoreSize=2;

  m_pTexturePool = new CTexturePool( );
  
  m_vPrevCameraPos=SetMinBB();

	m_pViewCamera = &GetViewCamera();
  m_fTextureLodRatio=1.f;
	m_bOceanIsVisibe = true;
	m_nDefZoomTexId = 0;
}

void CTerrain::CloseTerrainTextureFile()
{
	if(m_fpTerrainTextureFile)
		GetSystem()->GetIPak()->FClose(m_fpTerrainTextureFile);
	m_fpTerrainTextureFile=0;
}

CTerrain::~CTerrain()
{
//  UnReg isterInAllSectors(0);

	if(m_arrSecInfoTable.m_nSize)
  for( int x=0; x<CTerrain::GetSectorsTableSize(); x++)
  for( int y=0; y<CTerrain::GetSectorsTableSize(); y++)
  {
//    m_arrSecInfoTable[x][y]->ReleaseSector();
    delete (m_arrSecInfoTable[x][y]);
    m_arrSecInfoTable[x][y]=0;
  }

  if(m_fpTerrainTextureFile)
    GetSystem()->GetIPak()->FClose(m_fpTerrainTextureFile);
  m_fpTerrainTextureFile=0;
  
  for(int i=0; i<MAX_SURFACE_TYPES_COUNT; i++)
  {
    GetRenderer()->DeleteLeafBuffer(m_DetailTexInfo[i].pVertBuff);
    m_DetailTexInfo[i].pVertBuff=0;
//    GetRenderer()->ReleaseIndexBuffer(&m_DetailTexInfo[i].m_Indices);
  //  m_DetailTexInfo[i].m_Indices.Reset();
  }
   
  if(m_pLowLodCoverMapTex)
    GetRenderer()->RemoveTexture(m_pLowLodCoverMapTex);
  m_pLowLodCoverMapTex=0;

  if(m_ucpTmpTexBuffer)
    delete [] m_ucpTmpTexBuffer;

  GetRenderer()->DeleteLeafBuffer(m_pLowResTerrainLeafBuffer);
  GetRenderer()->DeleteLeafBuffer(m_pReflectedTerrainLeafBuffer);  

  if(m_pRETerrainDetailTextureLayers)
    m_pRETerrainDetailTextureLayers->Release();
  m_pRETerrainDetailTextureLayers=0;

	// free detail textures
	for(int i=0; i<MAX_SURFACE_TYPES_COUNT; i++)
		if(m_DetailTexInfo[i].nTexID>=0)
			GetRenderer()->RemoveTexture(m_DetailTexInfo[i].nTexID);

  delete m_pDetailObjects;
  delete m_pWater;
//  delete m_pBugsManager;

  delete m_pTexturePool;

	GetPhysicalWorld()->SetHeightfieldData(0);
}

void CTerrain::SetDetailTextures(int nId, const char * szFileName, float fScaleX, float fScaleY, 
                                 uchar ucProjAxis, const char * szSurfName)
{
  if(nId>=0 && nId<MAX_SURFACE_TYPES_COUNT)
  {
    GetLog()->Log("  Layer %d: %s", nId, szFileName);

    ITexPic * pPic = GetRenderer()->EF_LoadTexture(szFileName,FT_NORESIZE,0,eTT_Base);
    m_DetailTexInfo[nId].nTexID = pPic->GetTextureID();

    m_DetailTexInfo[nId].fScaleX = fScaleX;
    m_DetailTexInfo[nId].fScaleY = fScaleY; 
    m_DetailTexInfo[nId].ucProjAxis = ucProjAxis;
    m_DetailTexInfo[nId].ucThisSurfaceTypeId = nId;

		CMatMan * pMatMan = Get3DEngine()->GetMatMan();
		char szMatName[256]="";
		sprintf(szMatName,"terrain.TerrainLayer%d", nId);
		m_DetailTexInfo[nId].pMatInfo = pMatMan->FindMatInfo(szMatName);
	}
  else
		Warning(0,0,"CTerrain::SetDetailTextures: LayerId is out fo range: %d: %s", nId, szFileName);
}

bool CTerrain::LoadTerrain(bool bEditorMode)
{
  float fStartTime = GetCurAsyncTimeSec();

  GetLog()->UpdateLoadingScreen("Loading terrain ... ");

  if(!LoadTerrainSettingsFromXML())
  {
		Warning(0,0,"CTerrain::LoadTerrain: Error loading heightmap settings from XML, default values used");

    m_nUnitSize    = 2;
		m_fInvUnitSize = 1.0f / m_nUnitSize;
    m_nTerrainSize = 2048; 
  }

  m_nSectorSize = 64;
  m_nSectorsTableSize = m_nTerrainSize/m_nSectorSize;

  while((128>>HMAP_BIT_SHIFT) != 128/CTerrain::GetHeightMapUnitSize())
    HMAP_BIT_SHIFT++;

  if(!LoadHighMap(GetLevelFilePath(HEIGHT_MAP_FILE_NAME), GetSystem()->GetIPak() ))
  {
    GetLog()->Log("CTerrain::LoadTerrain: Error loading %s", GetLevelFilePath(HEIGHT_MAP_FILE_NAME));
    return 0;
  }

  InitSectors(bEditorMode);
  
  // for phys engine
	primitives::heightfield hf;
	hf.Basis.SetIdentity33();
	hf.origin.zero();
	hf.step.x = hf.step.y = (float)CTerrain::GetHeightMapUnitSize();
	hf.size.x = hf.size.y = CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize();
	hf.stride.set(hf.size.y+1,1);
	hf.pflags = hf.pdata = &GetHMValue(0,0);
	hf.heightscale = TERRAIN_Z_RATIO;
	hf.typemask = hf.typehole = STYPE_BIT_MASK;
  GetPhysicalWorld()->SetHeightfieldData(&hf);

//  m_pMapPreviewTex     = GetRenderer()->EF_LoadTexture((char*)GetLevelFilePath("terrain\\map_preview.jpg"),FT_CLAMP|FT_NOMIPS|FT_DXT1,0,eTT_Base);  
  m_pLowLodCoverMapTex = GetRenderer()->EF_LoadTexture((char*)GetLevelFilePath("terrain\\cover_low.dds"),  FT_CLAMP,0,eTT_Base);
 
  m_pTerrainEf                          = GetRenderer()->EF_LoadShader("Terrain", eSH_World, EF_SYSTEM);
//  m_pTerrainZPassEf                     = GetRenderer()->EF_LoadShader("ZBuffPassVP", eSH_World, 0);
//  m_pTerrainVPEf                        = GetRenderer()->EF_LoadShader("TerrainVP", eSH_World, 0);
  m_pTerrainLightPassEf                 = GetRenderer()->EF_LoadShader("TerrainLightPass", eSH_World, EF_SYSTEM);
  m_pTerrainShadowPassEf                = GetRenderer()->EF_LoadShader("TerrainShadowPass", eSH_World, EF_SYSTEM);
  m_pTerrainEf_WithDefaultDetailTexture = GetRenderer()->EF_LoadShader("TerrainWithDefaultDetailTexture", eSH_World, EF_SYSTEM);
	m_pTerrainWithFog											=	GetRenderer()->EF_LoadShader("TerrainWithFog", eSH_World, EF_SYSTEM);
	m_pTerrainLayerEf											= GetRenderer()->EF_LoadShader("TerrainLayer",eSH_World,EF_SYSTEM);

//  m_pTerrainCausticsEf = GetRenderer()->EF_LoadShader("TerrainCaustics", eSH_World, 0);
//  if(m_pTerrainCausticsEf->GetFlags() & EF_NOTFOUND)
  //  m_pTerrainCausticsEf=0;

  m_pRETerrainDetailTextureLayers  = (CRETerrainDetailTextureLayers*)GetRenderer()->EF_CreateRE(eDATA_TerrainDetailTextureLayers);
  m_pTerrainDetailTextureLayersEff = GetRenderer()->EF_LoadShader("TerrainDetailTextureLayers", eSH_World, EF_SYSTEM);

  GetLog()->Log("Terrain was loaded in %.2f sec", GetCurAsyncTimeSec()-fStartTime );

  return true;
}

int __cdecl CTerrain__Cmp_CStatObjInstForLoading_Size(const void* v1, const void* v2)
{
	CStatObjInstForLoading * p1 = ((CStatObjInstForLoading*)v1);
	CStatObjInstForLoading * p2 = ((CStatObjInstForLoading*)v2);

	list2<StatInstGroup> & lstStaticTypes = ((C3DEngine*)Cry3DEngineBase::Get3DEngine())->GetObjManager()->m_lstStaticTypes;

	CStatObj * pStatObj1 = (p1->GetID()<lstStaticTypes.Count()) ? lstStaticTypes[p1->GetID()].GetStatObj() : 0;
	CStatObj * pStatObj2 = (p2->GetID()<lstStaticTypes.Count()) ? lstStaticTypes[p2->GetID()].GetStatObj() : 0;

	if(!pStatObj1)
		return 1;
	if(!pStatObj2)
		return -1;

	int nSecId1 = 0;
	{	// get pos
		Vec3d vCenter = Vec3d(p1->GetX(),p1->GetY(),p1->GetZ()) + (pStatObj1->GetBoxMin()+pStatObj1->GetBoxMax())*0.5f*p1->GetScale();
		// get sector ids
		int x = (int)(((vCenter.x)/CTerrain::GetSectorSize()));
		int y = (int)(((vCenter.y)/CTerrain::GetSectorSize()));
		// get obj bbox
		Vec3d vBMin = pStatObj1->GetBoxMin()*p1->GetScale();
		Vec3d vBMax = pStatObj1->GetBoxMax()*p1->GetScale();
		// if outside of the map, or too big - register in sector (0,0)
		if( x<0 || x>=CTerrain::GetSectorsTableSize() || y<0 || y>=CTerrain::GetSectorsTableSize() ||
			(vBMax.x - vBMin.x)>TERRAIN_SECTORS_MAX_OVERLAPPING*2 || (vBMax.y - vBMin.y)>TERRAIN_SECTORS_MAX_OVERLAPPING*2)
			x = y = 0;

		// get sector id
		nSecId1 = (x)*CTerrain::GetSectorsTableSize() + (y);
	}

	int nSecId2 = 0;
	{	// get pos
		Vec3d vCenter = Vec3d(p2->GetX(),p2->GetY(),p2->GetZ()) + (pStatObj2->GetBoxMin()+pStatObj2->GetBoxMax())*0.5f*p2->GetScale();
		// get sector ids
		int x = (int)(((vCenter.x)/CTerrain::GetSectorSize()));
		int y = (int)(((vCenter.y)/CTerrain::GetSectorSize()));
		// get obj bbox
		Vec3d vBMin = pStatObj2->GetBoxMin()*p2->GetScale();
		Vec3d vBMax = pStatObj2->GetBoxMax()*p2->GetScale();
		// if outside of the map, or too big - register in sector (0,0)
		if( x<0 || x>=CTerrain::GetSectorsTableSize() || y<0 || y>=CTerrain::GetSectorsTableSize() ||
			(vBMax.x - vBMin.x)>TERRAIN_SECTORS_MAX_OVERLAPPING*2 || (vBMax.y - vBMin.y)>TERRAIN_SECTORS_MAX_OVERLAPPING*2)
			x = y = 0;

		// get sector id
		nSecId2 = (x)*CTerrain::GetSectorsTableSize() + (y);
	}

	if(nSecId1 > nSecId2)
		return -1;
	if(nSecId1 < nSecId2)
		return 1;

	if(p1->GetScale()*pStatObj1->GetRadius() > p2->GetScale()*pStatObj2->GetRadius())
		return 1;
	if(p1->GetScale()*pStatObj1->GetRadius() < p2->GetScale()*pStatObj2->GetRadius())
		return -1;

	return 0;
}

int __cdecl CTerrain__Cmp_Int(const void* v1, const void* v2)
{
	if(*(uint*)v1 > *(uint*)v2)
		return 1;
	if(*(uint*)v1 < *(uint*)v2)
		return -1;

	return 0;
}

void CTerrain::LoadStatObjInstances()
{
  assert(this); if(!this) return;

  GetLog()->Log("Loading static object positions ...");

	//  RemoveAllStaticObjects();
  for( int x=0; x<CTerrain::GetSectorsTableSize(); x++)
    for( int y=0; y<CTerrain::GetSectorsTableSize(); y++)
      m_arrSecInfoTable[x][y]->Unload(true);

  // load static object positions list
  list2<CStatObjInstForLoading> static_objects;
  static_objects.Load(GetLevelFilePath("objects.lst"), GetSystem()->GetIPak());

	qsort(static_objects.GetElements(), static_objects.Count(), 
		sizeof(static_objects[0]), CTerrain__Cmp_CStatObjInstForLoading_Size);

  // put objects into sectors depending on object position and fill lstUsedCGFs
	list2<CStatObj*> lstUsedCGFs;
  for(int i=0; i<static_objects.Count(); i++)
  {
    float x       = static_objects[i].GetX();
    float y       = static_objects[i].GetY();
    float z       = static_objects[i].GetZ()>0 ? static_objects[i].GetZ() : GetZApr(x,y);
    int  nId      = static_objects[i].GetID();
    uchar ucBr    = static_objects[i].GetBrightness();
    float fScale  = static_objects[i].GetScale();

    if( nId>=0 && nId<m_pObjManager->m_lstStaticTypes.Count() && 
				fScale>0 && 
				x>=0 && x<CTerrain::GetTerrainSize() && y>=0 && y<CTerrain::GetTerrainSize() &&
				m_pObjManager->m_lstStaticTypes[nId].GetStatObj() )
    {
			if(m_pObjManager->m_lstStaticTypes[nId].GetStatObj()->GetRadius()*fScale < GetCVars()->e_vegetation_min_size)
				continue; // skip creation of very small objects

			if(lstUsedCGFs.Find(m_pObjManager->m_lstStaticTypes[nId].GetStatObj())<0)
				lstUsedCGFs.Add(m_pObjManager->m_lstStaticTypes[nId].GetStatObj());

      CStatObjInst * pEnt = (CStatObjInst*)((C3DEngine*)Get3DEngine())->CreateVegetation();
			pEnt->m_fScale = fScale;
      pEnt->m_vPos = Vec3d(x,y,z);
      if(!pEnt->m_pEntityRenderState)
        pEnt->m_pEntityRenderState = Get3DEngine()->MakeEntityRenderState();
      pEnt->SetStatObjGroupId(nId);
      pEnt->m_ucBright = ucBr;
      pEnt->m_vWSBoxMin = pEnt->m_vPos + m_pObjManager->m_lstStaticTypes[nId].GetStatObj()->GetBoxMin()*fScale;
      pEnt->m_vWSBoxMax = pEnt->m_vPos + m_pObjManager->m_lstStaticTypes[nId].GetStatObj()->GetBoxMax()*fScale;
      pEnt->Physicalize( );
    }
  }

	// release not used CGF's
	int nGroupsReleased=0;
	for(int i=0; i<m_pObjManager->m_lstStaticTypes.Count(); i++)
	{
		CStatObj * pStatObj = m_pObjManager->m_lstStaticTypes[i].GetStatObj();
		if(pStatObj && lstUsedCGFs.Find(pStatObj)<0)
		{
			Get3DEngine()->ReleaseObject(pStatObj);
			m_pObjManager->m_lstStaticTypes[i].pStatObj = NULL;
			nGroupsReleased++;
		}
	}

  GetLog()->LogPlus(" %d objects created, %d groups released", static_objects.Count(), nGroupsReleased);
}

// returns file path for current level
const char * CTerrain::GetLevelFilePath(const char * szFileName)
{
  return Get3DEngine()->GetLevelFilePath(szFileName);
}

void CTerrain::SortStaticInstancesBySize()
{
  // set max view distance and sort by size
  for( int x=0; x<CTerrain::GetSectorsTableSize(); x++)
  for( int y=0; y<CTerrain::GetSectorsTableSize(); y++)
    m_arrSecInfoTable[x][y]->SortStaticInstancesBySize(m_arrSecInfoTable[x][y]->m_pFogVolume);
}

void CTerrain::CheckUnload()
{
  m_nLoadedSectors=0;
  for( int x=0; x<CTerrain::GetSectorsTableSize(); x++)
  for( int y=0; y<CTerrain::GetSectorsTableSize(); y++)
    m_nLoadedSectors += m_arrSecInfoTable[x][y]->CheckUnload();
}

void CTerrain::GetStreamingStatus(int & nLoadedSectors, int & nTotalSectors)
{
  nLoadedSectors = m_nLoadedSectors;
  nTotalSectors = CTerrain::GetSectorsTableSize()*CTerrain::GetSectorsTableSize();
}

void CTerrain::InitFogVolumes()
{
	// reset fog volume pointer in all affected terrain sectors
	for(int x=0; x<CTerrain::GetSectorsTableSize(); x++)
	for(int y=0; y<CTerrain::GetSectorsTableSize(); y++)
	{
		CSectorInfo * pSecInfo = m_arrSecInfoTable[x][y];
//		if(pSecInfo)
	//		pSecInfo->m_pFogVolume = 0;
    SetSectorFogVolume(pSecInfo);
	}

  for(int v=0; v<m_lstFogVolumes.Count(); v++)
  {
    // register volume in renderer 
    int nVolumeID = m_lstFogVolumes[v].nRendererVolumeID = GetRenderer()->EF_RegisterFogVolume(
			m_lstFogVolumes[v].fMaxViewDist, m_lstFogVolumes[v].vBoxMax.z, m_lstFogVolumes[v].vColor, -1, m_lstFogVolumes[v].m_bCaustics);

    m_lstFogVolumes[v].nRendererVolumeID = nVolumeID;

    if(m_lstFogVolumes[v].bOcean)
      m_pWater->SetFogVolumrId(nVolumeID);
       /*
		// get 2d bounds in sectors array
		int min_x = (int)(((m_lstFogVolumes[v].vBoxMin.x - 1.f)/CTerrain::GetSectorSize()));
		int min_y = (int)(((m_lstFogVolumes[v].vBoxMin.y - 1.f)/CTerrain::GetSectorSize()));
		int max_x = (int)(((m_lstFogVolumes[v].vBoxMax.x + 1.f)/CTerrain::GetSectorSize()));
		int max_y = (int)(((m_lstFogVolumes[v].vBoxMax.y + 1.f)/CTerrain::GetSectorSize()));

		if( min_x<0 ) min_x = 0; else if( min_x>=CTerrain::GetSectorsTableSize() ) min_x = CTerrain::GetSectorsTableSize()-1;
		if( min_y<0 ) min_y = 0; else if( min_y>=CTerrain::GetSectorsTableSize() ) min_y = CTerrain::GetSectorsTableSize()-1;
		if( max_x<0 ) max_x = 0; else if( max_x>=CTerrain::GetSectorsTableSize() ) max_x = CTerrain::GetSectorsTableSize()-1;
		if( max_y<0 ) max_y = 0; else if( max_y>=CTerrain::GetSectorsTableSize() ) max_y = CTerrain::GetSectorsTableSize()-1;

		// set fog volume pointer in all affected sectors
		for(int x=min_x; x<=max_x; x++)
		for(int y=min_y; y<=max_y; y++)
		{
			CSectorInfo * pSecInfo = m_arrSecInfoTable[x][y];
      if(pSecInfo && pSecInfo->m_fMinZ < m_lstFogVolumes[v].vBoxMax.z )
				pSecInfo->m_pFogVolume = &m_lstFogVolumes[v];
		}*/
	}
}

void CTerrain::SetSectorFogVolume(CSectorInfo * pSecInfo)
{
  pSecInfo->m_pFogVolume = 0;
  for(int v=0; v<m_lstFogVolumes.Count(); v++)
  {
    if(pSecInfo && 
      pSecInfo->m_fMinZ < m_lstFogVolumes[v].vBoxMax.z &&
			m_lstFogVolumes[v].IntersectBBox(pSecInfo->m_vBoxMin,pSecInfo->m_vBoxMax) &&
      !m_lstFogVolumes[v].bIndoorOnly)
			if(!m_lstFogVolumes[v].bOcean || !GetCVars()->e_use_global_fog_in_fog_volumes)
    {
      pSecInfo->m_pFogVolume = &m_lstFogVolumes[v];
      break;
    }
  }
}
/*
void CTerrain::InitShadowHightTable(CObjManager * pObjManager)
{
  memset(m_arrShadowHight,0,sizeof(m_arrShadowHight));

  // calculate shadow high map
  for( int x=0; x<CTerrain::GetSectorsTableSize(); x++ )
  for( int y=0; y<CTerrain::GetSectorsTableSize(); y++ )
  {
    CSectorInfo * pSecInfo = m_arrSecInfoTable[x][y];
    if(!pSecInfo)
      GetConsole()->Exit("CTerrain::LoadStaticMap: !pSecInfo2");

    for( int i=0; i<pSecInfo->m_lstStatObjects.Count(); i++)
    {
      CStatObjInst * o = &pSecInfo->m_lstStatObjects[i];
      
      Vec3d vBoxMin, vBoxMax;
      pObjManager->GetStaticObjectBBox(o->m_nObjectTypeID, vBoxMin, vBoxMax);
      int nSizeX   = int(o->m_fScale*(vBoxMax.x-vBoxMin.x)*0.5f*0.33f);
      float fSizeZ = o->m_fScale*(vBoxMax.z-vBoxMin.z);

      for(int m=-nSizeX; m<=nSizeX; m++)
      {
        Vec3d vShadowPos = Vec3d(o->m_vPos.x+m, o->m_vPos.y, GetZApr(o->m_vPos.x, o->m_vPos.y) + fSizeZ);
        for(int n=0; n<=fSizeZ*4; n++)
        {
          vShadowPos+=Vec3d(0,1,-0.9f);
          if(vShadowPos.z < GetZApr(vShadowPos.x, vShadowPos.y))
            break;

          if( vShadowPos.x>=0 && vShadowPos.y>=0 && vShadowPos.x<CTerrain::GetTerrainSize() && vShadowPos.y<CTerrain::GetTerrainSize() )
            if(vShadowPos.z>m_arrShadowHight[int(vShadowPos.x)][int(vShadowPos.y)])
              m_arrShadowHight[int(vShadowPos.x)][int(vShadowPos.y)]=vShadowPos.z;
        }
      }
    }
  }

  // set brightness of objects
  for(     x=0; x<CTerrain::GetSectorsTableSize(); x++ )
  for( int y=0; y<CTerrain::GetSectorsTableSize(); y++ )
  {
    CSectorInfo * pSecInfo = m_arrSecInfoTable[x][y];
    if(!pSecInfo)
      GetConsole()->Exit("CTerrain::LoadStaticMap: !pSecInfo2");

    for( int i=0; i<pSecInfo->m_lstStatObjects.Count(); i++ )
    {
      CStatObjInst * o = &pSecInfo->m_lstStatObjects[i];
      o->m_bBright = true;

      int fSize = int(o->m_fMaxDist/OBJ_MAX_VIEW_DISTANCE_RATIO);
      if(m_arrShadowHight[int(o->m_vPos.x)][int(o->m_vPos.y)])
        if(m_arrShadowHight[int(o->m_vPos.x)][int(o->m_vPos.y)] > fSize/2 + o->m_vPos.z)
        o->m_bBright = 0;
    }
  }
}
*/

bool CTerrain::LoadTerrainSettingsFromXML()
{
	XDOM::IXMLDOMNodeListPtr pLevelInfoList;
	XDOM::IXMLDOMNodePtr pLevelInfoTag;
	XDOM::IXMLDOMDocumentPtr pDoc = GetSystem()->CreateXMLDocument();
	if(!pDoc->load(Get3DEngine()->GetLevelFilePath("LevelData.xml")))
    return 0;

	pLevelInfoList = pDoc->getElementsByTagName("LevelInfo");
  pLevelInfoList->reset();
  XDOM::IXMLDOMNodePtr pNode = pLevelInfoList->nextNode();

  { // GlobalWaterLevel 
    XDOM::IXMLDOMNodePtr pGlobalWaterLevel = pNode->getAttribute("WaterLevel");

    if(!pGlobalWaterLevel)
      return 0;

    char buff[32];
    strcpy(buff, pGlobalWaterLevel->getText());

    m_fGlobalWaterLevel = (float)atof(buff);
		if (m_fGlobalWaterLevel == 0)
			m_fGlobalWaterLevel = WATER_LEVEL_UNKNOWN;
  }

  { // HeightmapUnitSize
    XDOM::IXMLDOMNodePtr pHeightmapUnitSize = pNode->getAttribute("HeightmapUnitSize");

    if(!pHeightmapUnitSize)
      return 0;

    char buff[32];
    strcpy(buff, pHeightmapUnitSize->getText());

    m_nUnitSize = atol(buff);
		m_fInvUnitSize = 1.0f / m_nUnitSize;

    if( m_nUnitSize != 1 && m_nUnitSize != 2  && m_nUnitSize != 4 && 
        m_nUnitSize != 8 && m_nUnitSize != 16 && m_nUnitSize != 32 )
      return 0;
  }

  { // HeightmapSize
    XDOM::IXMLDOMNodePtr pHeightmapSize = pNode->getAttribute("HeightmapSize");

    if(!pHeightmapSize)
      return 0;

    char buff[32];
    strcpy(buff, pHeightmapSize->getText());

    m_nTerrainSize = atol(buff);

    if( m_nTerrainSize !=   64 && 
        m_nTerrainSize !=  128 && 
        m_nTerrainSize !=  256 && 
        m_nTerrainSize !=  512 && 
        m_nTerrainSize != 1024 && 
        m_nTerrainSize != 2048 && 
        m_nTerrainSize != 4096 )
      return 0;

    m_nTerrainSize *= m_nUnitSize;
  }
/*
  { // SectorsTableSize
    XDOM::IXMLDOMNodePtr pSectorsTableSize = pNode->getAttribute("SectorsTableSize");

    if(!pSectorsTableSize)
      return true; // use default if not found

    char buff[32];
    strcpy(buff, pSectorsTableSize->getText());

    m_nSectorsTableSize = atol(buff);

    if( m_nSectorsTableSize != 16 && m_nSectorsTableSize != 32  && m_nSectorsTableSize != 64 )
      return 0;
  }
  */
  return true;
}