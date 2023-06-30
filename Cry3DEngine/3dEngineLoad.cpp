////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   3dengineload.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Level loading
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "3dEngine.h"
#include "objman.h"
#include "visareas.h"
#include "terrain_water.h"
#include "CryStaticModel.h"
#include "partman.h"
#include "DecalManager.h"
#include "bflyes.h"
#include "detail_grass.h"
#include "rain.h"
#include <IXMLDOM.h>
#include "watervolumes.h"
#include "brush.h"
#include "LMCompStructures.h"

#define LEVEL_DATA_FILE "LevelData.xml"
#define CUSTOM_MATERIALS_FILE "Materials.xml"
#define PARTICLES_FILE "particles.lst"
#define EFFECTS_FOLDER "Effects"
#define SHARED_PARTICLES_EXPLOSIONS "Explosions"
#define SHARED_PARTICLES_WATER "Water"
#define SHARED_PARTICLES_SMOKE "Smoke"
#define SHARED_PARTICLES_BLOOD "Blood"
#define SHARED_PARTICLES_BULLET "Bullet"
#define SHARED_PARTICLES_MISC "Misc"
#define SHARED_PARTICLES_FIRE "Fire"

double C3DEngine::m_dLoadLevelTime = 0;

//////////////////////////////////////////////////////////////////////////
void C3DEngine::ClearRenderResources( bool bEditorMode )
{
//  GetLog()->Log("\003*** Clearing render resources ***");

//	GetSystem()->GetIAnimationSystem()->	;

//  ShutDown(false);

  //if(GetRenderer())
  //  GetRenderer()->FreeResources(FRR_SHADERS | FRR_TEXTURES | FRR_RESTORE);

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	if(m_pVisAreaManager)
	{
		UpdateLoadingScreen("Deleting VisAreaManager ...");
		delete m_pVisAreaManager;
		UpdateLoadingScreenPlus(" Ok");
		m_pVisAreaManager=0;
	}

	if (m_pObjManager)
	{
		SAFE_DELETE( m_pObjManager->m_pCWaterVolumes );
	}

	// delete terrain
	if(m_pTerrain)
	{
		UpdateLoadingScreen("Deleting terrain ...");
		SAFE_DELETE( m_pTerrain );
		UpdateLoadingScreenPlus(" Ok");
	}

	// free vegetations
	if(!bEditorMode)
	{
		if (m_pObjManager)
			m_pObjManager->UnloadVegetations();
	}

	if (m_pPartManager)
	{
		m_pPartManager->Reset();
		if (!bEditorMode)
		{
			SAFE_DELETE( m_pPartManager );
		}
	}

	SAFE_DELETE( m_pDecalManager );

	// Load Materials Library from LevelData.xml
	if (!bEditorMode)
	{
		SAFE_DELETE( m_pMatMan );
		m_pMatMan = new CMatMan();
	}

	//////////////////////////////////////////////////////////////////////////
	// Purge destroyed physics entities.
	//////////////////////////////////////////////////////////////////////////
	GetSystem()->GetIPhysicalWorld()->PurgeDeletedEntities();

	// print warnings
	if (m_pObjManager)
		m_pObjManager->CheckObjectLeaks();

	GetRenderer()->PreLoad();
}

//////////////////////////////////////////////////////////////////////////
void C3DEngine::SetLevelPath( const char * szFolderName )
{
	// make folder path
	assert(strlen(szFolderName) < 1024);
	strcpy( m_szLevelFolder,szFolderName );
	if (strlen(m_szLevelFolder) > 0)
	{
		if (m_szLevelFolder[strlen(m_szLevelFolder)-1] != '/')
			strcat( m_szLevelFolder,"/" );
	}
}

//////////////////////////////////////////////////////////////////////////
bool C3DEngine::LoadLevel(const char * szFolderName, const char * szMissionName, bool bEditorMode)
{
	AUTO_PROFILE_SECTION(GetTimer(), m_dLoadLevelTime);

	m_bIgnoreFakeMaterialsInCGF = !bEditorMode;
	m_bEditorMode = bEditorMode;

  GetRenderer()->MakeCurrent();
  
  if(!szFolderName || !szFolderName[0])
  { Warning( 0,0,"C3DEngine::LoadLevel: Level name is not specified"); return 0; }

	if(!szMissionName || !szMissionName[0])
	{ Warning( 0,0,"C3DEngine::LoadLevel: Mission name is not specified"); }

  char szMissionNameBody[256] = "NoMission";
  if(!szMissionName)
    szMissionName = szMissionNameBody;

	SetLevelPath( szFolderName );

	if(!bEditorMode)
  { // check is LevelData.xml file exist
    char sMapFileName[_MAX_PATH];
    strcpy(sMapFileName,m_szLevelFolder);
    strcat(sMapFileName,LEVEL_DATA_FILE);
    if(!CXFile::IsFileExist(sMapFileName))
    { UpdateLoadingScreen("Error: Level not found: %s", sMapFileName); return 0; }
  }

  if(!m_pObjManager)
    m_pObjManager = new CObjManager (this);

  if(!bEditorMode)
  {
		if(m_pVisAreaManager)
		{
			UpdateLoadingScreen("Deleting VisAreaManager ...");
	    delete m_pVisAreaManager;
			UpdateLoadingScreenPlus(" Ok");
			m_pVisAreaManager=0;
		}
  }

  if(!m_pVisAreaManager)
    m_pVisAreaManager = new CVisAreaManager( );

	assert (GetSystem()->GetIAnimationSystem());

	if(!bEditorMode)
	{
		int nCellSize = 4;
		GetPhysicalWorld()->SetupEntityGrid(2,vectorf(0,0,0), // this call will destroy all physicalized stuff
			CTerrain::GetTerrainSize()/nCellSize,CTerrain::GetTerrainSize()/nCellSize,(float)nCellSize,(float)nCellSize);
	}

	// cache prev terrain objects
	list2<struct IEntityRender*> lstTerrainObjects;
	if(bEditorMode && m_pTerrain)
		m_pTerrain->GetObjects(&lstTerrainObjects);

	// delete terrain
	if(m_pTerrain)
	{
		UpdateLoadingScreen("Deleting terrain ...");
		delete m_pTerrain; m_pTerrain=0;
		UpdateLoadingScreenPlus(" Ok");
	}

	// free vegetations
	if(!bEditorMode)
		m_pObjManager->UnloadVegetations();

  // print level name into console
  char header[512];
  snprintf(header, sizeof(header),
    "\001---------------------- Loading level %s, mission %s ------------------------------------",
    szFolderName, szMissionName);
  header[100]=0;
  UpdateLoadingScreen(header);

	// print warnings
	m_pObjManager->CheckObjectLeaks();

	// create terrain
  m_pTerrain = new CTerrain( );
	m_pObjManager->m_pTerrain = m_pTerrain;
  if(m_pTerrain && !m_pTerrain->LoadTerrain(bEditorMode))
  { // terrain not found
    delete m_pTerrain;
    m_pTerrain = 0;  
  }

	// restore prev terrain objects
	for(int i=0; i<lstTerrainObjects.Count(); i++)
	{
		lstTerrainObjects[i]->m_pSector = 0;
		Get3DEngine()->RegisterEntity(lstTerrainObjects[i]);
	}

	lstTerrainObjects.Reset();

  // 
  m_pObjManager->m_lstVegetContainer.Reset();
  m_pObjManager->m_lstBrushContainer.Reset();

	//////////////////////////////////////////////////////////////////////////
	// Load LevelData.xml File.
	XmlNodeRef levelDataRoot = GetSystem()->LoadXmlFile( GetLevelFilePath(LEVEL_DATA_FILE) );
	//////////////////////////////////////////////////////////////////////////

	// Load Materials Library from LevelData.xml
	if (!bEditorMode)
	{
		delete m_pMatMan;
		m_pMatMan = new CMatMan();
		m_pMatMan->LoadMaterialsLibrary( GetLevelFilePath(CUSTOM_MATERIALS_FILE),levelDataRoot );
	}

	{ // recreate particles and decals
		if(m_pPartManager)
			m_pPartManager->Reset();
		else
			m_pPartManager = new CPartManager();

		delete m_pDecalManager; 
		m_pDecalManager     = new CDecalManager   (this);
	}

	LoadParticleEffects( levelDataRoot,bEditorMode );

  // load leveldata.xml
  m_pTerrainWaterShader=m_pSunRoadShader=0;
	m_nWaterBottomTexId=0;
  LoadEnvironmentSettingsFromXML(szMissionName, bEditorMode, 0, true);

  // init water if not initialized already (if no mission was found)
  if(m_pTerrain && !m_pTerrain->m_pWater)
    m_pTerrain->InitTerrainWater(bEditorMode,
      m_pTerrainWaterShader, m_nWaterBottomTexId, m_pSunRoadShader, 
			m_fWaterTranspRatio, m_fWaterReflectRatio, 
			m_fWaterBumpAmountX, m_fWaterBumpAmountY, m_fWaterBorderTranspRatio);

  m_pObjManager->m_pTerrain = m_pTerrain;
/*	if (!m_pTerrain) 
	{
		// [marco] Vlad check why it crashes here
		CryError("3dengine:Cannot Load terrain - report this to Vladimir Kajalin");
	}*/
	
	if(m_pTerrain)
		m_pTerrain->SetObjManager(m_pObjManager);

	m_pVisAreaManager->SetupFogVolumes(m_pTerrain);
	if(m_pObjManager->m_pCWaterVolumes)
		m_pObjManager->m_pCWaterVolumes->InitWaterVolumes();

  if(!bEditorMode)
  {
    // load static object bodies
    m_pObjManager->LoadVegetationModels(szMissionName,bEditorMode);

		// load lsources used for lightmaps
#if !defined(LINUX64)//fails due to different size of CDLight (pointers have 8 bytes)
		LoadStaticLightSources(GetLevelFilePath("StatLights.dat"));
#endif
    // bushes bodies and instances
    m_pObjManager->LoadBrushes();

    // merge brushes grouped in the editor
    m_pObjManager->MergeBrushes();

    // load positions of static objects, update sectors bboxes
    m_pTerrain->LoadStatObjInstances();
  }

  // recreate bugs
/*	if(m_pTerrain)
	{
		delete m_pTerrain->m_pBugsManager;
		m_pTerrain->m_pBugsManager = new CBugsManager( );
	}*/

  // physicalise instances of static objects and buildings
//  m_pObjManager->CreatePhysicalEntitys();

	for(int i=0; i<m_pObjManager->m_lstBrushContainer.Count(); i++)
	{
    if (!(m_pObjManager->m_lstBrushContainer[i]->m_dwRndFlags & ERF_MERGED))
    {
  		Get3DEngine()->UnRegisterEntity(m_pObjManager->m_lstBrushContainer[i]);
	  	Get3DEngine()->RegisterEntity(m_pObjManager->m_lstBrushContainer[i]);
    }
  }

  for(int i=0; i<m_pObjManager->m_lstVegetContainer.Count(); i++)
  {
    Get3DEngine()->UnRegisterEntity(m_pObjManager->m_lstVegetContainer[i]);
    Get3DEngine()->RegisterEntity(m_pObjManager->m_lstVegetContainer[i]);
  }

	if(!bEditorMode)
	{
		m_pTerrain->SortStaticInstancesBySize();
		m_pVisAreaManager->SortStaticInstancesBySize();
	}

  if(GetCVars()->e_stream_areas)
  { // all data can be streamed from disk now
    for(int i=0; i<m_pObjManager->m_lstBrushContainer.Count(); i++)
      delete m_pObjManager->m_lstBrushContainer[i];
    m_pObjManager->m_lstBrushContainer.Reset();

    for(int i=0; i<m_pObjManager->m_lstVegetContainer.Count(); i++)
      delete m_pObjManager->m_lstVegetContainer[i];
    m_pObjManager->m_lstVegetContainer.Reset();
  }
  else
  {
    // from now objects will be stored in the sectors
    m_pObjManager->m_lstVegetContainer.Reset();
    m_pObjManager->m_lstBrushContainer.Reset();
  }

	// restore game state
	SetRainAmount(0);
	EnableOceanRendering(true,true);
	SetSkyBoxAlpha(1.f);
	m_pObjManager->m_bLockCGFResources = false;

  return (true);
}

void C3DEngine::LoadTerrainSurfacesFromXML(void * _pDoc)
{
  UpdateLoadingScreen("Loading terrain detail textures ...");

	XDOM::IXMLDOMNodeListPtr pDetTexTagList;
	XDOM::IXMLDOMNodePtr pDetTexTag;
	XDOM::IXMLDOMDocumentPtr pDoc = _pDoc ? *((XDOM::IXMLDOMDocumentPtr*)_pDoc) : (XDOM::IXMLDOMDocumentPtr)0;

	if(!_pDoc)
	{ // load current LevelData if pDoc not specified
		pDoc = GetSystem()->CreateXMLDocument();
		if(!pDoc->load(Get3DEngine()->GetLevelFilePath(LEVEL_DATA_FILE)))
			return;
	}

	pDetTexTagList = pDoc->getElementsByTagName("SurfaceTypes");
	if (pDetTexTagList)
	{
		pDetTexTagList->reset();
		pDetTexTag = pDetTexTagList->nextNode();
		XDOM::IXMLDOMNodeListPtr pDetTexList;
		pDetTexList = pDetTexTag->getElementsByTagName("SurfaceType");
		if (pDetTexList)
		{
			pDetTexList->reset();
			XDOM::IXMLDOMNodePtr pDetTex;
      int nId = 0;
			while (pDetTex = pDetTexList->nextNode())
			{
				XDOM::IXMLDOMNodePtr pDetailTextureName = pDetTex->getAttribute("DetailTexture");
				XDOM::IXMLDOMNodePtr pScaleX = pDetTex->getAttribute("DetailScaleX");
				XDOM::IXMLDOMNodePtr pScaleY = pDetTex->getAttribute("DetailScaleY");
				XDOM::IXMLDOMNodePtr pProjAxis = pDetTex->getAttribute("ProjAxis");
        XDOM::IXMLDOMNodePtr pSurfaceName = pDetTex->getAttribute("Name");

        if(pDetailTextureName) if(pScaleX) if(pScaleY) if(pSurfaceName)
				{
          m_pTerrain->SetDetailTextures(nId, pDetailTextureName->getText(), 
            (float)atof(pScaleX->getText()), (float)atof(pScaleY->getText()),
            pProjAxis ? (pProjAxis->getText())[0] : 0, pSurfaceName->getText());
        }

        nId++;
      }
    }

		// recreate detail objects
		delete m_pTerrain->m_pDetailObjects;
		m_pTerrain->m_pDetailObjects = new CDetailGrass(pDetTexTagList);
  }

  m_pTerrain->InitDetailTextureLayers();

  UpdateLoadingScreenPlus("  ok");
}  

void C3DEngine::LoadFogVolumesFromXML(XDOM::IXMLDOMDocumentPtr pDoc)
{
	if(!m_pTerrain)
		return;

  m_pTerrain->m_lstFogVolumes.Clear();

  { // make hardcoded volume for ocean
    VolumeInfo volumeInfo;
    volumeInfo.vBoxMax = Vec3d( 1000000,  1000000, m_pTerrain->GetWaterLevel());
    volumeInfo.vBoxMin = Vec3d(-1000000, -1000000, -1000000);
//    volumeInfo.pShader = 0;
    volumeInfo.pShader = GetRenderer()->EF_LoadShader("FogLayer", eSH_World, EF_SYSTEM);
    volumeInfo.vColor  = m_vUnderWaterFogColor;
    volumeInfo.fMaxViewDist = m_fUnderWaterFogDistance;
    volumeInfo.bOcean = true;
		volumeInfo.m_bCaustics = m_bOceanCaustics;

    // add volume to list
    m_pTerrain->m_lstFogVolumes.Add(volumeInfo);
  }

	XDOM::IXMLDOMNodeListPtr pNodeTagList;
	XDOM::IXMLDOMNodePtr pNodeTag;
	pNodeTagList = pDoc->getElementsByTagName("Objects");
	if (pNodeTagList)
	{
		pNodeTagList->reset();
		pNodeTag = pNodeTagList->nextNode();
		XDOM::IXMLDOMNodeListPtr pNodeList;
		pNodeList = pNodeTag->getElementsByTagName("Object");
		if (pNodeList)
		{
			pNodeList->reset();
			XDOM::IXMLDOMNodePtr pNode;
			while (pNode = pNodeList->nextNode())
			{
				XDOM::IXMLDOMNodePtr pName = pNode->getAttribute("Type");
				if (pName)
				{
					if (strstr(pName->getText(),"FogVolume"))
          {
            XDOM::IXMLDOMNodePtr pAttr;
            VolumeInfo volumeInfo;
            pAttr = pNode->getAttribute("Pos");
            if(pAttr)
            {
              Vec3d vPos = StringToVector(pAttr->getText());

              // get properties
//              XDOM::IXMLDOMNodeListPtr pObjectsTagList = pNode->getElementsByTagName("Properties");
  //            if(pObjectsTagList)
              {
                XDOM::IXMLDOMNodePtr pAttr1,pAttr2,pAttr3,pAttr4,pAttr5;

                pAttr1 = pNode->getAttribute("ViewDistance");
                if(pAttr1)
                  volumeInfo.fMaxViewDist = (float)atof(pAttr1->getText());

                pAttr1 = pNode->getAttribute("Width");
                pAttr2 = pNode->getAttribute("Length");
                pAttr3 = pNode->getAttribute("Height");
                pAttr4 = pNode->getAttribute("Shader");
                pAttr5 = pNode->getAttribute("Color");

                if(pAttr1!=0 && pAttr2!=0 && pAttr3!=0 && pAttr4!=0 && pAttr5!=0) 
                {
                  Vec3d vSize((float)atof(pAttr1->getText()), (float)atof(pAttr2->getText()), (float)atof(pAttr3->getText()));
                  volumeInfo.vBoxMax = vPos + Vec3d(vSize.x*0.5f, vSize.y*0.5f, vSize.z);
                  volumeInfo.vBoxMin = vPos - Vec3d(vSize.x*0.5f, vSize.y*0.5f, 0);
									volumeInfo.pShader = (char*)pAttr4->getText()[0] ? GetRenderer()->EF_LoadShader((char*)pAttr4->getText(), eSH_World, EF_SYSTEM) : GetRenderer()->EF_LoadShader("FogLayer", eSH_World, EF_SYSTEM);
                  volumeInfo.vColor  = StringToVector(pAttr5->getText());

                  // add volume to list
                  m_pTerrain->m_lstFogVolumes.Add(volumeInfo);
                }
              }
            }
          }
        }
			}
		}
	}
}

void C3DEngine::LoadEnvironmentSettingsFromXML(const char * szMissionName, bool bEditorMode, const char * szMissionXMLString, bool bUpdateLightingOnVegetations)
{
  if(!m_pTerrain)
  {
    Warning( 0,0,"Calling C3DEngine::LoadEnvironmentSettingsFromXML while level is not loaded");
    return;
  }

  GetRenderer()->MakeCurrent();

	// if xml string specified - load settings just from this string
	if(szMissionXMLString)
	{
		XDOM::IXMLDOMDocumentPtr pMissionDoc;
		pMissionDoc=GetSystem()->CreateXMLDocument();

		// add empty <Mission> tag
		char * szMissionXMLStringWithMissionTag = new char [strlen(szMissionXMLString)+256];
		strcpy(szMissionXMLStringWithMissionTag, "<Mission>\n");
		strcat(szMissionXMLStringWithMissionTag, szMissionXMLString);
		strcat(szMissionXMLStringWithMissionTag, "</Mission>");

		if (pMissionDoc->loadXML(szMissionXMLStringWithMissionTag))
    {
      LoadMissionSettingsFromXML(pMissionDoc, bEditorMode, bUpdateLightingOnVegetations);
      // load fog	volumes (mission shared data)
//      LoadFogVolumesFromXML(pMissionDoc);
  //    m_pTerrain->InitFogVolumes();
    }

		delete [] szMissionXMLStringWithMissionTag;
		return;
	}

  // load environment settings
	XDOM::IXMLDOMDocumentPtr pDoc = GetSystem()->CreateXMLDocument();

  // set default values
	m_vFogColor(1,1,1);
  m_fDefMaxViewDist = m_fMaxViewDist = 1024;
  m_vDefFogColor    = m_vFogColor;
  m_fDefFogNearDist = m_fFogNearDist=50;
  m_fDefFogFarDist  = m_fFogFarDist = 1500;

	//char buff[128];
//	GetCurrentDirectory(128, buff);

	if(pDoc->load(Get3DEngine()->GetLevelFilePath(LEVEL_DATA_FILE)))
  {
    // load detail textures	(mission shared data)
		LoadTerrainSurfacesFromXML(&pDoc);

    // mission environment
		if (szMissionName && szMissionName[0])
		{
			XDOM::IXMLDOMNodeListPtr pMissionTagList;
			XDOM::IXMLDOMNodePtr pMissionTag;
			pMissionTagList = pDoc->getElementsByTagName("Missions");
			if (pMissionTagList)
			{
				pMissionTagList->reset();
				pMissionTag = pMissionTagList->nextNode();
				XDOM::IXMLDOMNodeListPtr pMissionList;
				pMissionList = pMissionTag->getElementsByTagName("Mission");
				if (pMissionList)
				{
					pMissionList->reset();
					XDOM::IXMLDOMNodePtr pMission;
					while (pMission = pMissionList->nextNode())
          {
            XDOM::IXMLDOMNodePtr pName = pMission->getAttribute("Name");
            if (pName)
            {
              if (!stricmp(pName->getText(),szMissionName))
              { // get mission XML file name and open mission file
                XDOM::IXMLDOMNodePtr pMissionFileName = pMission->getAttribute("File");
                if (pMissionFileName)
                {
                  XDOM::IXMLDOMDocumentPtr pMissionDoc;
                  pMissionDoc=GetSystem()->CreateXMLDocument();
                  if (pMissionDoc->load(Get3DEngine()->GetLevelFilePath(pMissionFileName->getText())))
									{
										LoadMissionSettingsFromXML(pMissionDoc, bEditorMode, bUpdateLightingOnVegetations);

										// load fog	volumes (mission shared data)
										LoadFogVolumesFromXML(pMissionDoc);
										m_pTerrain->InitFogVolumes();

										if(!bEditorMode)
										{
											if(!m_pObjManager->m_pCWaterVolumes)
												m_pObjManager->m_pCWaterVolumes = new CWaterVolumeManager( );
											m_pObjManager->m_pCWaterVolumes->LoadWaterVolumesFromXML(pMissionDoc);

											m_pVisAreaManager->LoadVisAreaShapeFromXML(pMissionDoc);
										}

										m_pVisAreaManager->LoadVisAreaBoxFromXML(pMissionDoc);
									}
                  break;
                }
              }
            }
          }
				}
			}
		}
		else
			Warning(0,0,"C3DEngine::LoadEnvironmentSettingsFromXML: Mission name is not defined");
  }
}

char * C3DEngine::GetXMLAttribText(XDOM::IXMLDOMNode * pInputNode, const char * szLevel0,const char * szLevel1,const char * szLevel2,const char * szDefaultValue)
{
  static char szResText[128];
  
  strncpy(szResText,szDefaultValue,128);

  XDOM::IXMLDOMNodeListPtr pObjectsTagList = pInputNode->getElementsByTagName(szLevel0);
  if (!pObjectsTagList) 
    return szResText;

  pObjectsTagList->reset();
  XDOM::IXMLDOMNodePtr pObjectsTag = pObjectsTagList->nextNode(); 

  XDOM::IXMLDOMNodeListPtr pNodes = pObjectsTag->getElementsByTagName(szLevel1);
  if(pNodes)
  {
	  XDOM::IXMLDOMNodePtr pNode;
	  pNodes->reset();
	  while(pNode = pNodes->nextNode())
	  {
      XDOM::IXMLDOMNodePtr pAttr = pNode->getAttribute(szLevel2);
      if(pAttr)
        strncpy(szResText, pAttr->getText(), sizeof(szResText));
      break;
    }
  }
   
  return szResText;
}

void C3DEngine::LoadMissionSettingsFromXML(XDOM::IXMLDOMNode *pInputNode, bool bEditorMode, bool bUpdateLightingOnVegetations)
{
  Vec3d vColor = StringToVector(GetXMLAttribText(pInputNode,"Environment","Fog","Color","255,255,255"));
  m_vDefFogColor[0] = m_vFogColor[0] = vColor[0]/255.f;
  m_vDefFogColor[1] = m_vFogColor[1] = vColor[1]/255.f;
  m_vDefFogColor[2] = m_vFogColor[2] = vColor[2]/255.f;
  GetRenderer()->SetClearColor(m_vFogColor);

  // fog distance
  m_fDefFogNearDist = m_fFogNearDist= (float)atol(GetXMLAttribText(pInputNode,"Environment","Fog","Start","64"));
  m_fDefFogFarDist  = m_fFogFarDist = (float)atol(GetXMLAttribText(pInputNode,"Environment","Fog","End","1500"));

  // max view distance
  m_fDefMaxViewDist = m_fMaxViewDist = (float)atol(GetXMLAttribText(pInputNode,"Environment","Fog","ViewDistance","1024"));

  // Shaders
  char szSkyBoxShaderName[128];
  strncpy(szSkyBoxShaderName, GetXMLAttribText(pInputNode, "Environment", "Shaders", "SkyBox", "InfRedGal"), sizeof(szSkyBoxShaderName));
	SAFE_RELEASE(m_pSHSky);
	m_pSHSky = szSkyBoxShaderName[0] ? GetRenderer()->EF_LoadShader(szSkyBoxShaderName, eSH_World) : NULL;

  // set terrain water, sun road and bottom shaders
  char szTerrainWaterShaderName[256]="";
  strncpy(szTerrainWaterShaderName,       GetXMLAttribText(pInputNode,"Environment","Shaders","Water",        "terrainwater"),      sizeof(szTerrainWaterShaderName));

  m_pTerrainWaterShader = szTerrainWaterShaderName[0] ? GetRenderer()->EF_LoadShader(szTerrainWaterShaderName, eSH_World, EF_SYSTEM) : 0;
  
  char szSunRoadShaderName[256]="";
  strncpy(szSunRoadShaderName,            GetXMLAttribText(pInputNode,"Environment","Shaders","SunWaterRefl", "BumpSunGlow"),       sizeof(szSunRoadShaderName));
	m_pSunRoadShader = szSunRoadShaderName[0] ? GetRenderer()->EF_LoadShader(szSunRoadShaderName, eSH_World, EF_SYSTEM) : 0;

	// load water bottom texture
	char szWaterBottomTexName[256]="";
  strncpy(szWaterBottomTexName, GetXMLAttribText(pInputNode,"Environment","Ocean","BottomTexture", "terrain/water/oceanfloorcolor.bmp"),sizeof(szWaterBottomTexName));	
	ITexPic * pPic = GetRenderer()->EF_LoadTexture(szWaterBottomTexName,FT_NOREMOVE,0,eTT_Base);
	m_nWaterBottomTexId = pPic->GetTextureID();

	// load default zoom texture
	char szDefZoomTexName[256]="";
	strncpy(szDefZoomTexName, GetXMLAttribText(pInputNode, "Environment", "HeightMap", "DefaultZoomTexture", ""),sizeof(szDefZoomTexName));	
	if(szDefZoomTexName[0])
	{
		pPic = GetRenderer()->EF_LoadTexture(szDefZoomTexName,FT_NOREMOVE,0,eTT_Base);
		m_pTerrain->m_nDefZoomTexId = pPic->GetTextureID();
	}
	else
		m_pTerrain->m_nDefZoomTexId = 0;

  // Ocean
  m_pTerrain->m_fShoreSize  = (float)atof(GetXMLAttribText(pInputNode,"Environment","Ocean","ShoreSize",          "2.0"));
  m_fWaterTranspRatio       = (float)atof(GetXMLAttribText(pInputNode,"Environment","Ocean","SurfaceTranspRatio", "1.0"));
  m_fWaterReflectRatio      = (float)atof(GetXMLAttribText(pInputNode,"Environment","Ocean","SurfaceReflectRatio","1.0"));
  m_fWaterBumpAmountX       = (float)atof(GetXMLAttribText(pInputNode,"Environment","Ocean","SurfaceBumpAmountX", "0.08"));
  m_fWaterBumpAmountY       = (float)atof(GetXMLAttribText(pInputNode,"Environment","Ocean","SurfaceBumpAmountY", "0.12"));
  m_fWaterBorderTranspRatio = (float)atof(GetXMLAttribText(pInputNode,"Environment","Ocean","BorderTranspRatio",  "0.5"));
  m_vUnderWaterFogColor  = StringToVector(GetXMLAttribText(pInputNode,"Environment","Ocean","FogColor","51,90,102"))/255;
  m_fUnderWaterFogDistance  = (float)atof(GetXMLAttribText(pInputNode,"Environment","Ocean","FogDistance", "8"));
	m_bOceanCaustics					= atol(GetXMLAttribText(pInputNode,"Environment","Ocean","Caustics", "1"))==1;
  
	m_fSkyBoxAngle = (float)atof(GetXMLAttribText(pInputNode,"Environment","EnvState","SkyBoxAngle","0.0"));
	m_fSunHeightScale = (float)atof(GetXMLAttribText(pInputNode,"Environment","EnvState","SunHeightScale","1.0"));
	m_fSkyBoxStretching = (float)atof(GetXMLAttribText(pInputNode,"Environment","EnvState","SkyBoxStretching","1.0"));

  m_pTerrain->InitTerrainWater(bEditorMode, m_pTerrainWaterShader, m_nWaterBottomTexId, m_pSunRoadShader, m_fWaterTranspRatio, m_fWaterReflectRatio, m_fWaterBumpAmountX, m_fWaterBumpAmountY, m_fWaterBorderTranspRatio);

  char szLensFlaresShaderName[128];
  strncpy(szLensFlaresShaderName,         GetXMLAttribText(pInputNode,"Environment","Shaders","SunLensFlares","CryLight"),            sizeof(szLensFlaresShaderName));
	if(szLensFlaresShaderName[0])
		m_pSHLensFlares = GetRenderer()->EF_LoadShader(szLensFlaresShaderName, eSH_World, EF_SYSTEM);
	else
		m_pSHLensFlares = NULL;

  char szShoreShaderName[128];
  strncpy(szShoreShaderName,  GetXMLAttribText(pInputNode,"Environment","Shaders","Shore",        "terrainwaterbeach"),   sizeof(szShoreShaderName));
	m_pTerrain->m_pSHShore = szShoreShaderName[0] ? GetRenderer()->EF_LoadShader(szShoreShaderName, eSH_World, EF_SYSTEM) : 0;

  // State
  vColor = StringToVector(GetXMLAttribText(pInputNode,"Environment","EnvState","EnvColor","128,128,128"));
  m_vWorldColorConst = vColor/255.f;

//  GetCVars()->e_rain_amount = strstr(GetXMLAttribText(pInputNode,"Environment","EnvState","Rain",""),"True")!=0;

//  GetCVars()->e_bflyes = GetCVars()->e_bflyes && strstr(GetXMLAttribText(pInputNode,"Environment","EnvState","BFlyes",""),"True")!=0;

  // set sun position
  m_vSunPosition = StringToVector(GetXMLAttribText(pInputNode,"Environment","Lighting","SunVector","0,5,-5"));
  m_vSunPosition *= -1.0f;
  m_vSunPosition.Normalize();
  
  float x=m_vSunPosition.x;
  m_vSunPosition.x = m_vSunPosition.y;
  m_vSunPosition.y=x;
  
  if(m_vSunPosition.x == 0 && m_vSunPosition.y == 0)
    m_vSunPosition = Vec3d(5,5,10000);
  else
    m_vSunPosition = GetNormalized(m_vSunPosition)*10000;

  m_pObjManager->m_vOutdoorAmbientColor = 
    StringToVector(GetXMLAttribText(pInputNode,"Environment","EnvState","OutdoorAmbientColor","64,64,64"))/255;

  m_pObjManager->m_vSunColor = 
    StringToVector(GetXMLAttribText(pInputNode,"Environment","EnvState","SunColor","128,128,128"))/255;

	if(!GetISystem()->IsDedicated())
	  m_pObjManager->UpdateCustomLighting( GetNormalized(GetSunPosition()) );

  // get wind
  m_pObjManager->m_fWindForce = (float)atof(GetXMLAttribText(pInputNode,"Environment","EnvState","WindForce","0.15"));

  // get terrain lods
  float fGeometryLodRatio = (float)atof(GetXMLAttribText(pInputNode,"Environment","HeightMap","GeometryLodRatio","1.0"));
	if(GetCVars()->e_cgf_load_lods == 0 && fGeometryLodRatio>1.f) // ised only for very high spec
		fGeometryLodRatio = 1.f+(fGeometryLodRatio-1.f)*0.5f;
	ICVar * pCVar =	GetConsole()->GetCVar("e_terrain_lod_ratio");
	if(pCVar)
		pCVar->Set(fGeometryLodRatio);

  m_pTerrain->m_fTextureLodRatio  = (float)atof(GetXMLAttribText(pInputNode,"Environment","HeightMap","TextureLodRatio", "1.0"));
}

//////////////////////////////////////////////////////////////////////////
void C3DEngine::LoadParticleEffects( XmlNodeRef &levelDataRoot,bool bEditorMode )
{
	if (!m_pPartManager)
		return;

	m_pPartManager->LoadSharedParticleLibrary( EFFECTS_FOLDER,SHARED_PARTICLES_EXPLOSIONS );
	m_pPartManager->LoadSharedParticleLibrary( EFFECTS_FOLDER,SHARED_PARTICLES_WATER );
	m_pPartManager->LoadSharedParticleLibrary( EFFECTS_FOLDER,SHARED_PARTICLES_SMOKE );
	m_pPartManager->LoadSharedParticleLibrary( EFFECTS_FOLDER,SHARED_PARTICLES_BLOOD );
	m_pPartManager->LoadSharedParticleLibrary( EFFECTS_FOLDER,SHARED_PARTICLES_BULLET );
	m_pPartManager->LoadSharedParticleLibrary( EFFECTS_FOLDER,SHARED_PARTICLES_MISC );
	m_pPartManager->LoadSharedParticleLibrary( EFFECTS_FOLDER,SHARED_PARTICLES_FIRE );
	
	if (levelDataRoot)
		m_pPartManager->LoadParticlesLibs( EFFECTS_FOLDER,levelDataRoot );

	CCryFile file;
	if (file.Open( GetLevelFilePath(PARTICLES_FILE),"rb" ))
	{
		m_pPartManager->LoadParticles( file );
	}
}