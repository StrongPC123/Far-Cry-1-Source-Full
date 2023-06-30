////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   3dengine.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Implementation of I3DEngine interface methods
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "3dEngine.h"
#include "visareas.h"
#include "objman.h"
#include "terrain_water.h"
#include "CryStaticModel.h"

#include "partman.h"
#include "DecalManager.h"
#include "bflyes.h"
#include "rain.h"
#include "meshidx.h"
#include "detail_grass.h"
#include "StencilShadowEdgeDetector.h"						// CStencilShadowEdgeDetector
#include "StencilShadowConnectivityBuilder.h"			// CStencilShadowConnectivityBuilder
#include "watervolumes.h"

#include "LMCompStructures.h"
#include "LMSerializationManager2.h"

#include "brush.h"

ISystem * Cry3DEngineBase::m_pSys=0;
IRenderer * Cry3DEngineBase::m_pRenderer=0;
ITimer * Cry3DEngineBase::m_pTimer=0;
ILog * Cry3DEngineBase::m_pLog=0;
IPhysicalWorld * Cry3DEngineBase::m_pPhysicalWorld=0;
IConsole * Cry3DEngineBase::m_pConsole=0;
I3DEngine * Cry3DEngineBase::m_p3DEngine=0;
CVars * Cry3DEngineBase::m_pCVars=0;
ICryPak * Cry3DEngineBase::m_pCryPak=0;
int Cry3DEngineBase::m_nRenderStackLevel=0;
int Cry3DEngineBase::m_dwRecursionDrawFlags[2]={0,0};
int Cry3DEngineBase::m_nRenderFrameID = 0;
bool Cry3DEngineBase::m_bProfilerEnabled = false;
int Cry3DEngineBase::m_CpuFlags=0;
double Cry3DEngineBase::m_SecondsPerCycle=0;
float Cry3DEngineBase::m_fPreloadStartTime=0;
bool Cry3DEngineBase::m_bIgnoreFakeMaterialsInCGF = false;
bool Cry3DEngineBase::m_bEditorMode = false;
ESystemConfigSpec Cry3DEngineBase::m_configSpec = CONFIG_VERYHIGH_SPEC;
ESystemConfigSpec Cry3DEngineBase::m_LightConfigSpec = CONFIG_VERYHIGH_SPEC;


#define LAST_POTENTIALLY_VISIBLE_TIME 2

//////////////////////////////////////////////////////////////////////
C3DEngine::C3DEngine(ISystem	* pSystem)
{
  CXFile::SetIPack(pSystem->GetIPak());

  Cry3DEngineBase::m_pSys=pSystem;
  Cry3DEngineBase::m_pRenderer=pSystem->GetIRenderer();
  Cry3DEngineBase::m_pTimer=pSystem->GetITimer();
  Cry3DEngineBase::m_pLog=pSystem->GetILog();
  Cry3DEngineBase::m_pPhysicalWorld=pSystem->GetIPhysicalWorld();
  Cry3DEngineBase::m_pConsole=pSystem->GetIConsole();
  Cry3DEngineBase::m_p3DEngine=this;
  Cry3DEngineBase::m_pCryPak=pSystem->GetIPak();
  Cry3DEngineBase::m_pCVars=0;

  Cry3DEngineBase::m_CpuFlags=pSystem->GetCPUFlags();
  Cry3DEngineBase::m_SecondsPerCycle=pSystem->GetSecondsPerCycle();


#ifdef _DEBUG
#ifndef _XBOX
//  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // _crtBreakAlloc
  //	_controlfp( _EM_INEXACT,_MCW_EM );
#endif
#endif

	m_pShadowEdgeDetector=new CStencilShadowEdgeDetector();
	m_pConnectivityBuilder=new CStencilShadowConnectivityBuilder();
	m_pStaticConnectivityBuilder = new CStencilShadowStaticConnectivityBuilder();

  m_pRenderCallbackFunc = NULL;

  memset(m_SunObject, 0, sizeof(m_SunObject));  
  m_pBlurObj=0;
  m_pBlurObj=GetRenderer()->EF_GetObject(false, -1);
  m_pScreenObj=0;
  m_pScreenObj=GetRenderer()->EF_GetObject(false, -1);
	m_szLevelFolder[0]=0;

	m_nFlags=0;
  m_pRE2DQuad=0;
  m_pSHFullScreenQuad = NULL;
  m_pSHSky = NULL;//GetRenderer()->EF_LoadShader("InfRedGal", eSH_World, EF_SYSTEM);
  m_pTerrainWaterShader = m_pSunRoadShader = 0;
	m_nWaterBottomTexId=0;
  m_pSHLensFlares = GetRenderer()->EF_LoadShader("CryLight", eSH_World, EF_SYSTEM);
  m_vSunPosition = Vec3d(0, -10000.0f, 10000.0f);
  m_pSHDefault = GetRenderer()->EF_LoadShader("Default", eSH_World, EF_SYSTEM);

  m_pTerrain=0;	
	m_bEnabled=1;

	ITexPic * pPic = GetRenderer()->EF_LoadTexture("diskette.tga",0,0,eTT_Base);
  m_nStreamingIconTexID = pPic->GetTextureID();

	pPic = GetRenderer()->EF_LoadTexture("black.tga",0,0,eTT_Base);
  m_nBlackTexID = pPic->GetTextureID();

	ITexPic * pPicSpot = GetRenderer()->EF_LoadTexture("spot_shadow.tga",0,0,eTT_Base);
	m_nShadowSpotTexId = pPicSpot->GetTextureID();

  // create components
  m_pObjManager   = 0;//new CObjManager (m_pSystem);
	
  m_pPartManager = 0;

  m_pDecalManager     = 0;//new CDecalManager   (m_pSystem, this);
//  m_pBFManager        = new CBFManager      ();
  m_pRainManager      = new CRainManager    ();
  m_pVisAreaManager   = 0;
  m_pCVars            = new CVars();
  Cry3DEngineBase::m_pCVars = m_pCVars;

  // create REs
  m_pRESky              = (CRESky*)             GetRenderer()->EF_CreateRE(eDATA_Sky); m_pRESky->m_fAlpha = 1.f;
  //m_pREOutSpace         = (CREOutSpace * )      GetRenderer()->EF_CreateRE(eDATA_OutSpace);
  m_pREDummy            = (CREDummy*)           GetRenderer()->EF_CreateRE(eDATA_Dummy);
  m_pRETerrainParticles = (CRETerrainParticles*)GetRenderer()->EF_CreateRE(eDATA_TerrainParticles);
  m_pRE2DQuad           = (CRE2DQuad*)          GetRenderer()->EF_CreateRE(eDATA_2DQuad);
  m_pREScreenProcess    = (CREScreenProcess*)   GetRenderer()->EF_CreateRE(eDATA_ScreenProcess);

  m_pSHScreenTexMap     = GetRenderer()->EF_LoadShader("ScreenTexMap", eSH_World, EF_SYSTEM);
  m_pSHScreenProcess    = GetRenderer()->EF_LoadShader("ScreenProcess", eSH_World, EF_SYSTEM);
  m_pSHOutSpace         = GetRenderer()->EF_LoadShader("OutSpace", eSH_World, EF_SYSTEM);
  m_pSHFarTreeSprites   = GetRenderer()->EF_LoadShader("FarTreeSprites", eSH_World, EF_SYSTEM);
  m_pSHClearStencil     = GetRenderer()->EF_LoadShader("ClearStencil", eSH_World, EF_SYSTEM);
  m_pSHShadowMapGen     = GetRenderer()->EF_LoadShader("ShadowMapGen", eSH_World, EF_SYSTEM);
  m_pSHBinocularDistortMask = GetRenderer()->EF_LoadShader("BinocularDistortMask", eSH_World, EF_SYSTEM);
  m_pSHScreenDistort = GetRenderer()->EF_LoadShader("ScreenDistort", eSH_World, EF_SYSTEM);
  m_pSHSniperDistortMask = GetRenderer()->EF_LoadShader("SniperDistortMask", eSH_World, EF_SYSTEM);
  m_pSHRainMap          = GetRenderer()->EF_LoadShader("RainMap", eSH_World, EF_SYSTEM);

  m_pSHStencil          = GetRenderer()->EF_LoadShader("<Stencil>", eSH_World, EF_SYSTEM);
  m_pSHStencilState     = GetRenderer()->EF_LoadShader("StencilState", eSH_World, EF_SYSTEM);
  m_pSHStencilStateInv  = GetRenderer()->EF_LoadShader("StencilStateInv", eSH_World, EF_SYSTEM);

	m_pSHTerrainParticles = GetRenderer()->EF_LoadShader("TerrainParticles", eSH_World, EF_SYSTEM);

  m_pPhysMaterialEnumerator=0;

  m_fMaxViewDist = 1024;

  m_fWorldColorRatio = 1;
  m_vWorldColorConst(0.5f,0.5f,0.5f);

  m_fWaterBumpAmountX = 0.08f;
  m_fWaterBumpAmountY = 0.11f;
  m_fWaterBorderTranspRatio = m_fWaterReflectRatio = m_fWaterTranspRatio = 1.f;

  m_fSkyBoxAngle=0;
	m_fSkyBoxStretching=0;
	m_fSunHeightScale=0;
	m_pFogTopPlane=0;

	m_vWindForce(0,-8,0);

	m_pMatMan = new CMatMan();

	m_bShore = m_bOcean = true;
	m_pREShadowMapGenerator = (CREShadowMapGen*)GetRenderer()->EF_CreateRE(eDATA_ShadowMapGen);
  m_bTerrainLightMapGenError = false;

  m_vUnderWaterFogColor.Set(0.2f,0.35f,0.4f);
  m_fUnderWaterFogDistance = 16;
	m_bOceanCaustics = true;

	m_fFogFarDist = DEFAULT_ZMAX;

	m_nRealLightsNum=0;

	m_pSysSpec = GetConsole()->GetCVar( "sys_spec" );
  m_pLightQuality = GetConsole()->GetCVar("r_Quality_BumpMapping");
}

//////////////////////////////////////////////////////////////////////
C3DEngine::~C3DEngine()
{
	assert(IsHeapValid());

	ShutDown();

  delete m_pPartManager; 
  delete m_pDecalManager; 
//  delete m_pBFManager; 
  delete m_pRainManager;
  delete m_pVisAreaManager;
  delete m_pObjManager;
  delete m_pCVars;
	delete m_pMatMan; m_pMatMan=0;

  SAFE_RELEASE(m_pRESky);
  //m_pREOutSpace->Release();
  SAFE_RELEASE(m_pREDummy);
  SAFE_RELEASE(m_pRETerrainParticles);
  SAFE_RELEASE(m_pRE2DQuad);
	SAFE_RELEASE(m_pREShadowMapGenerator);  
	SAFE_RELEASE(m_pREScreenProcess);

  if (m_pSHLensFlares)
    m_pSHLensFlares->Release(true);
  if (m_pSHDefault)
    m_pSHDefault->Release(true);

  if (m_pSHScreenTexMap)
    m_pSHScreenTexMap->Release(true);
  if (m_pSHScreenProcess)
    m_pSHScreenProcess->Release(true);
  if (m_pSHOutSpace)
    m_pSHOutSpace->Release(true);
  if (m_pSHFarTreeSprites)
    m_pSHFarTreeSprites->Release(true);
  if (m_pSHClearStencil)
    m_pSHClearStencil->Release(true);
  if (m_pSHShadowMapGen)
    m_pSHShadowMapGen->Release(true);
  if (m_pSHBinocularDistortMask)
    m_pSHBinocularDistortMask->Release(true);
  if (m_pSHScreenDistort)
    m_pSHScreenDistort->Release(true);
  if (m_pSHSniperDistortMask)
    m_pSHSniperDistortMask->Release(true);
  if (m_pSHRainMap)
    m_pSHRainMap->Release(true);

  if (m_pSHStencil)
    m_pSHStencil->Release(true);
  if (m_pSHStencilState)
    m_pSHStencilState->Release(true);
  if (m_pSHStencilStateInv)
    m_pSHStencilStateInv->Release(true);

  if (m_pSHTerrainParticles)
    m_pSHTerrainParticles->Release(true);

  if (m_nStreamingIconTexID)
  {
    GetRenderer()->RemoveTexture(m_nStreamingIconTexID);
    m_nStreamingIconTexID = 0;
  }
  if (m_nShadowSpotTexId)
  {
    GetRenderer()->RemoveTexture(m_nShadowSpotTexId);
    m_nShadowSpotTexId = 0;
  }

	delete m_pShadowEdgeDetector;
	delete m_pConnectivityBuilder;
	delete m_pStaticConnectivityBuilder;

	GetLog()->Log ("\003----------------------------------------------------------------------");
	GetLog()->Log ("\003Cry3DEngine profile statistics:");
  GetLog()->Log ("\003I3DEngine::MakeObject() : %4.1f sec", CObjManager::m_dMakeObjectTime);
	GetLog()->Log ("\003I3DEngine::LoadLevel() : %4.1f sec", m_dLoadLevelTime);
	GetLog()->Log ("\003I3DEngine::LoadMaterial() : %4.1f sec", CObjManager::m_dCIndexedMesh__LoadMaterial);
	GetLog()->Log ("\003I3DEngine::UpdateCustomLightingSpritesAndShadowMaps() : %4.1f sec", CObjManager::m_dUpdateCustomLightingSpritesAndShadowMaps);	
	GetLog()->Log ("\003----------------------------------------------------------------------");
}

//////////////////////////////////////////////////////////////////////
bool C3DEngine::Init()
{	  	
  ShutDown();
 
	return  (true);
}

bool C3DEngine::IsCameraAnd3DEngineInvalid(const CCamera cam, const char * szCaller)
{
  const Vec3d & vCamPos = cam.GetPos();
  const Vec3d & vAngles = cam.GetAngles();
  const float fFov      = cam.GetFov();

	if (!m_pObjManager || !m_pDecalManager)
	{
		//GetLog()->Log("Warning: %s: Engine not initialized or level not loaded");
		return (true); 
	}

	if( m_fMaxViewDist<=0 || m_fFogFarDist<=0 ||
			!_finite(vCamPos.x) || !_finite(vCamPos.y) || !_finite(vCamPos.z) ||
      !_finite(vAngles.x) || !_finite(vAngles.y) || !_finite(vAngles.z) ||
      IsEquivalent(vCamPos,Vec3d(0,0,0),VEC_EPSILON) || vCamPos.z < -GetTerrainSize()*4 || vCamPos.z > GetTerrainSize()*4 ||
      fFov < 0.025f || fFov > gf_PI)
  {
    Warning(0,0,"%s: Camera undefined : Pos=(%.1f, %.1f, %.1f), Rot=(%.1f, %.1f, %.1f), Fov=%.1f, MaxViewDist=%.1f, FogFarDist=%.1f", 
      szCaller, 
      vCamPos.x, vCamPos.y, vCamPos.z, 
      vAngles.x, vAngles.y, vAngles.z,
      fFov, m_fMaxViewDist, m_fFogFarDist);
    return true;
  }

  return false;
}

void C3DEngine::Update()
{
	m_bProfilerEnabled = GetISystem()->GetIProfileSystem()->IsProfiling();
	m_configSpec = GetISystem()->GetConfigSpec();
  m_LightConfigSpec = (ESystemConfigSpec)GetCurrentLightSpec();

///	if(m_pVisAreaManager)
	//	m_pVisAreaManager->Preceche(m_pObjManager);
}

//////////////////////////////////////////////////////////////////////
void C3DEngine::UpdateScene(bool bAddStaticLights, bool bAlwaysAddSun)
{	
  if(IsCameraAnd3DEngineInvalid(GetViewCamera(), "C3DEngine::Update"))
    return;

  Vec3d vTerrainColor = GetSystem()->GetI3DEngine()->GetWorldColor();//*GetSystem()->GetI3DEngine()->GetWorldBrightnes();

  GetRenderer()->EF_SetWorldColor(vTerrainColor[0],vTerrainColor[1],vTerrainColor[2],1);

  if( bAlwaysAddSun || (GetCVars()->e_sun && IsOutdoorVisible() && m_pObjManager && m_pVisAreaManager) )
  {
	  // Add sun lsource
    CDLight DynLight;
//	  memset(&DynLight, 0, sizeof(CDLight));
	  DynLight.m_Origin = GetSunPosition();//Vec3d(0, -5000.0f, 5000.0f);
	  DynLight.m_fRadius  = 100000000;
	  DynLight.m_Color    = GetSunColor();
    DynLight.m_Color.r *= GetWorldColor().x;
    DynLight.m_Color.g *= GetWorldColor().y;
    DynLight.m_Color.b *= GetWorldColor().z;

	  DynLight.m_SpecColor= GetSunColor();
    DynLight.m_SpecColor.r *= GetWorldColor().x;
    DynLight.m_SpecColor.g *= GetWorldColor().y;
    DynLight.m_SpecColor.b *= GetWorldColor().z;

	  DynLight.m_Flags |= DLF_DIRECTIONAL | DLF_SUN | DLF_THIS_AREA_ONLY | DLF_LM | DLF_CASTSHADOW_MAPS;
		if(GetCVars()->e_sun_stencil)
			DynLight.m_Flags |= DLF_CASTSHADOW_VOLUME;

    for(int i=0; i<GetCVars()->e_sun; i++)
      AddDynamicLightSource(DynLight,(IEntityRender*)-1);
  }

	if(bAddStaticLights)
		UpdateStaticLightSources();

  LightSourcesDebug();

	if(m_pObjManager && m_pTerrain)
    m_pTerrain->SetCBuffer(m_pObjManager->m_pCoverageBuffer);

  m_bTerrainLightMapGenError = false;
}

//////////////////////////////////////////////////////////////////////
void C3DEngine::ShutDown(bool bEditorMode)
{
	if(GetRenderer() != GetSystem()->GetIRenderer())
		GetSystem()->Error("Renderer was deallocated before I3DEngine::ShutDown() call");

  // reinit console variables
//  delete m_pCVars;
//  m_pCVars = new CVars(m_pSystem);

  GetLog()->Log("Removing lights ...");
  for(int i=0; i<m_lstDynLights.Count(); i++)
  {
    CDLight * pLight = &m_lstDynLights[i];
    FreeLightSourceComponents(pLight);
  }
  DeleteAllStaticLightSources();

  GetLog()->Log("Deleting visareas ...");
  delete m_pVisAreaManager;
  m_pVisAreaManager = 0;

  GetLog()->Log("Deleting terrain ...");
//	list2<struct IEntityRender*> lstTerrainObjects;
//	if(m_pTerrain)
//		m_pTerrain->GetObjects(lstTerrainObjects);
  delete m_pTerrain;
  m_pTerrain=0;

  GetLog()->Log("ObjManager shutdown ...");
  delete m_pObjManager;
  m_pObjManager=0;

	GetRenderer()->DeleteLeafBuffer(m_pFogTopPlane);
	m_pFogTopPlane=0;

	SAFE_RELEASE(m_pSHSky);
}

//////////////////////////////////////////////////////////////////////
void C3DEngine::ActivateLight(const char *szName,bool bActivate)
{
	assert(0);
//	GetBuildingManager()->ActivateLight(szName,bActivate);
}

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4311 )
#endif

//////////////////////////////////////////////////////////////////////
void C3DEngine::SetCamera(const CCamera &cam, bool bToTheScreen) 
{ 
//	if(GetCVars()->e_hires_screenshoot)
//		return;

  if(IsCameraAnd3DEngineInvalid(cam, "C3DEngine::SetCamera"))
    return;

	GetViewCamera() = cam;

	// realtime skybox processing
  if(GetCVars()->e_out_space)
  {
	  const float fNearFarDistance = 64;

    if(bToTheScreen)
    {    
      GetViewCamera().SetZMin(DEFAULT_ZMIN);
      GetViewCamera().SetZMax(fNearFarDistance*1.5f);
    }
    else
    {
      GetViewCamera().SetZMin(fNearFarDistance*0.75f);
      GetViewCamera().SetZMax(DEFAULT_ZMAX);
      m_fMaxViewDist = DEFAULT_ZMAX;
    }
  }
  else if(GetViewCamera().GetZMax()>m_fMaxViewDist)
    GetViewCamera().SetZMax(m_fMaxViewDist);
											/*
	Vec3d vPosTest(106,120,0);
  vPosTest.z = GetWaterLevel(&vPosTest);
	GetRenderer()->Draw3dBBox(vPosTest-Vec3d(0.05f,0.05f,0.05f),vPosTest+Vec3d(0.05f,0.05f,0.05f));
												*/
	// under water camera effects
//  float fUnderWater = GetWaterLevel(&GetViewCamera().GetPos())-GetViewCamera().GetPos().z;
//  int nRecursionLevel = (int)GetRenderer()->EF_Query(EFQ_RecurseLevel);
//	float fZMin = GetViewCamera().GetZMin();
  //if(fUnderWater>(-fZMin) && nRecursionLevel==0)
  {
		// modify camera pos
	/*	if(fUnderWater>-0.04f) // if very close to the water - jump 5sm down
			GetViewCamera().SetPos(GetViewCamera().GetPos()-Vec3d(0,0,0.08f));

		// set z near
		if(fabs(fUnderWater)<fZMin)
			GetViewCamera().SetZMin(fZMin*0.2f);
*/
		// set animated fov
    /*if(fUnderWater>0)
		{
			if(fUnderWater>1) 
				fUnderWater=1;

			float k = 1.f+0.1f*(float)cry_sinf(2.f*GetTimer()->GetCurrTime())*fUnderWater;

//			CCamera cam = GetViewCamera();
			GetViewCamera().SetFov(gf_PI_DIV_2/(k*0.6f+0.4f));
			GetViewCamera().SetProjRatio((600.f/800.f)*k);    
		}*/
  }


	//GetViewCamera().Init(GetRenderer()->GetWidth(), GetRenderer()->GetHeight(), 
		//GetViewCamera().GetFov(), m_fMaxViewDist, GetViewCamera().GetProjRatio(), GetViewCamera().GetZMin());
	GetViewCamera().Init(GetRenderer()->GetWidth(), GetRenderer()->GetHeight(), 
		GetViewCamera().GetFov(), m_fMaxViewDist, 0, GetViewCamera().GetZMin());

  GetViewCamera().Update();

  GetRenderer()->SetCamera(GetViewCamera());

	if (m_pTerrain && bToTheScreen)
  {
    m_pObjManager->m_fZoomFactor = 0.05f + 0.95f*(RAD2DEG(GetViewCamera().GetFov())/90.f);
    m_pTerrain->m_fLodLFactor = m_pObjManager->m_fZoomFactor*m_pObjManager->m_fZoomFactor;    
  }
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

IStatObj * C3DEngine::MakeObject(const char * szFileName, const char * szGeomName, 
                                 EVertsSharing eVertsSharing, 
                                 bool bLoadAdditinalInfo, bool bKeepInLocalSpace)
{
  if(!szFileName || !szFileName[0])
  {
    Warning( 0,0,"I3DEngine::MakeObject: filename is not specified");
    return 0;
  }

  if(!m_pObjManager)
    m_pObjManager = new CObjManager (this);

  return m_pObjManager->MakeObject(szFileName, szGeomName, eVertsSharing, bLoadAdditinalInfo, bKeepInLocalSpace);
}

bool C3DEngine::ReleaseObject(IStatObj * pObject)
{
  if(!m_pObjManager || !pObject)
    return 0;

	if(!m_pObjManager->ReleaseObject((CStatObj*)pObject))
	{
		GetLog()->Log("Warning: I3DEngine::ReleaseObject: Attempt to release invalid IStatObj pointer");
		return false;
	}

  return true;
}

IStatObj* C3DEngine::MakeObject()
{
	return new CStatObj();
}

void C3DEngine::RegisterEntity( IEntityRender* pEntityRS )
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

//  if(strstr(pEntityRS->GetEntityClassName(),"Light"))
  //  int y=0;

#ifdef _DEBUG // crash test basically
  const char * szClass = pEntityRS->GetEntityClassName();
  const char * szName = pEntityRS->GetName();
  if(!szName[0] && !szClass[0])
    Warning(0,0,"C3DEngine::RegisterEntity: Entity undefined"); // do not register undefined objects
	
	if(!_finite(pEntityRS->GetPos().x) || !_finite(pEntityRS->GetPos().y) || !_finite(pEntityRS->GetPos().z))
	{
		Warning(0,0,"Warning: C3DEngine::RegisterEntity: Entity position undefined: %s", szName);
		return;
	}

#endif

  if(!GetCVars()->e_register_in_sectors)
    return; // speed test
/*
  ICryCharInstance * cmodel = pEntityRS->GetEntityCharacter(0);    
  if(cmodel && (cmodel->GetFlags() & CS_FLAG_DRAW_NEAR))
  {
    if(cmodel->GetFlags() & CS_FLAG_DRAW_MODEL)
      int t=0;
  }
*/
  if(!pEntityRS->GetEntityRS())
    return;

  pEntityRS->GetRenderBBox(pEntityRS->m_vWSBoxMin, pEntityRS->m_vWSBoxMax);
	assert(_finite(pEntityRS->m_vWSBoxMin.x) && _finite(pEntityRS->m_vWSBoxMin.y) && _finite(pEntityRS->m_vWSBoxMin.z));
	assert(_finite(pEntityRS->m_vWSBoxMax.x) && _finite(pEntityRS->m_vWSBoxMax.y) && _finite(pEntityRS->m_vWSBoxMax.z));
  pEntityRS->m_fWSRadius = GetDistance(pEntityRS->m_vWSBoxMin, pEntityRS->m_vWSBoxMax)*0.5f;

//  if(!pEntityRS->GetEntityRS()->pOcclState)
  //  pEntityRS->GetEntityRS()->pOcclState = new OcclusionTestClient;

//  if(pEntityRS->GetRndFlags()&ERF_SELECTED)
  //  int iww=0;

  if(!(pEntityRS->GetRndFlags()&ERF_OUTDOORONLY) && // do not find vis area if this flag is not set
    m_pVisAreaManager && m_pVisAreaManager->SetEntityArea(pEntityRS))
    return;

  assert(m_pTerrain == m_pObjManager->m_pTerrain);
  if(m_pObjManager)
    m_pObjManager->RegisterEntity(pEntityRS);
}

bool C3DEngine::UnRegisterEntity( IEntityRender* pEntityRS )
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

	if(!GetCVars()->e_register_in_sectors)
		return true; // speed test

#ifdef _DEBUG // crash test basically
	const char * szClass = pEntityRS->GetEntityClassName();
	const char * szName = pEntityRS->GetName();
	if(!szName[0] && !szClass[0])
		Warning(0,0,"C3DEngine::RegisterEntity: Entity undefined"); // do not register undefined objects
#endif

  bool bFound = false;

	if(m_pObjManager && m_pVisAreaManager)
		bFound |= m_pVisAreaManager->UnRegisterEntity(pEntityRS);

	if(m_pTerrain)
	{
		assert(m_pTerrain == m_pObjManager->m_pTerrain);
		if(m_pObjManager && m_pTerrain == m_pObjManager->m_pTerrain)
			bFound |= m_pObjManager->UnRegisterEntity(pEntityRS);
	}

/*
#ifdef _DEBUG

  if(pEntityRS->m_pSector || pEntityRS->m_pVisArea)
  {	
    const char * szName = pEntityRS->GetName();
    GetLog()->Log("Error: C3DEngine::UnRegisterEntity: Entity %s(ptr=%d) was not fully unregistered", szName, (int)pEntityRS);
    assert(0);
  }

  if((m_pTerrain && m_pTerrain->UnRegisterInAllSectors(pEntityRS)) || 
    (m_pVisAreaManager && m_pVisAreaManager->UnRegisterInAllSectors(pEntityRS)))
	{	
		const char * szName = pEntityRS->GetName();
    GetLog()->Log("Error: C3DEngine::UnRegisterEntity: Entity %s(ptr=%d) was found after unregistration", szName, (int)pEntityRS);
		assert(0);
	}

#endif
*/

  return bFound;
}

bool C3DEngine::UnRegisterInAllSectors(IEntityRender * pEntityRS) 
{ 
	if(m_pObjManager && m_pVisAreaManager)
		m_pVisAreaManager->UnRegisterEntity(pEntityRS);

  if(m_pTerrain) 
    return m_pTerrain->UnRegisterInAllSectors(pEntityRS); 

  return 0;
}

void C3DEngine::SpawnParticles( const ParticleParams & SpawnParticleParams )
{
  if(m_pPartManager)
    m_pPartManager->Spawn( SpawnParticleParams,m_fMaxViewDist, m_pObjManager);
}

void C3DEngine::CreateDecal( const CryEngineDecalInfo& DecalInfo )
{
	if(!GetCVars()->e_decals)
		return;

//	if(DecalInfo.nTid != 4276)
	//	return;

	if(GetCVars()->e_decals == 2)
		GetLog()->Log("Debug: C3DEngine::CreateDecal: Pos=(%.1f,%.1f,%.1f) Size=%.2f ObjName=%s nPartID=%d", DecalInfo.vPos.x, DecalInfo.vPos.y, DecalInfo.vPos.z, DecalInfo.fSize, DecalInfo.pDecalOwner ? DecalInfo.pDecalOwner->GetName() : "NULL", DecalInfo.nPartID);

	if(DecalInfo.pDecalOwner)
	if(DecalInfo.pDecalOwner->GetEntityCharacter(0) || DecalInfo.pDecalOwner->GetEntityCharacter(1))
	{
		ICryCharInstance* pChar = DecalInfo.pDecalOwner->GetEntityCharacter(0);
		if(pChar)
		{
			CryEngineDecalInfo DecalLCS = DecalInfo;

			Matrix44 matCharacter;
			matCharacter.SetIdentity();
			Vec3d vAngles = DecalInfo.pDecalOwner->GetAngles();

			matCharacter=matCharacter*Matrix33::CreateRotationX( DEG2RAD(-vAngles.x) );
			matCharacter=matCharacter*Matrix33::CreateRotationY( DEG2RAD(+vAngles.y) ); //IMPORTANT: radian-angle must be negated 
			matCharacter=matCharacter*Matrix33::CreateRotationZ( DEG2RAD(-vAngles.z) ); 
			matCharacter.SetTranslationOLD (DecalInfo.pDecalOwner->GetPos());

			Matrix44 matInvCharacter = GetInverted44(matCharacter);
			DecalLCS.vPos = matInvCharacter.TransformPointOLD(DecalInfo.vPos);
			DecalLCS.vHitDirection = matInvCharacter.TransformVectorOLD(DecalLCS.vHitDirection);

			pChar->CreateDecal(DecalLCS);
		}
	}

	// make decal on static entity component
	if(m_pDecalManager)
	{
		CryEngineDecalInfo DecalInfoFixed = DecalInfo;
		if(DecalInfo.pDecalOwner)
		{ // try to find at least one marked to be visible static component
			Matrix44 objMat, objMatInv;
			int nPartID = DecalInfo.nPartID;
			IStatObj * pEntObject = DecalInfo.pDecalOwner->GetEntityStatObj(nPartID, &objMat, true);	
			while(!pEntObject && nPartID<16)
			{ // find first visible entity component
				pEntObject = DecalInfo.pDecalOwner->GetEntityStatObj(++nPartID, &objMat, true);
			}

			if(!pEntObject)
				return;

			DecalInfoFixed.nPartID = nPartID;
		}

		m_pDecalManager->Spawn( DecalInfoFixed, m_fMaxViewDist, GetTerrain() );
	}
}

ICryCharInstance * C3DEngine::MakeCharacter(const char * cid_file_name, unsigned int dwFlags)
{
	ICryCharManager* pCharManager = GetSystem()->GetIAnimationSystem();
  assert(pCharManager);

	// NOTE: The returned class is not necessarily CryCharInstance. It can be AnimObject as well
  return pCharManager->MakeCharacter(cid_file_name, dwFlags);
}

void C3DEngine::RemoveCharacter(ICryCharInstance * pCryCharInstance)
{
	ICryCharManager* pCharManager = GetSystem()->GetIAnimationSystem();
  if(pCryCharInstance && pCharManager)
    pCharManager->RemoveCharacter(pCryCharInstance);
}
/*
float C3DEngine::GetDayTime(float fPrevDayTime) 
{ 
  if(fPrevDayTime)
  {
    float fDayTimeDiff = (GetCVars()->e_day_time - fPrevDayTime);
    while(fDayTimeDiff>12)
      fDayTimeDiff-=12;
    while(fDayTimeDiff<-12)
      fDayTimeDiff+=12;  
    fDayTimeDiff = (float)fabs(fDayTimeDiff);
    return fDayTimeDiff;
  }

  return GetCVars()->e_day_time; 
} */

void C3DEngine::SetWorldColorRatio(float fWorldColorRatio)
{ 
  m_fWorldColorRatio = fWorldColorRatio;
} 

float C3DEngine::GetWorldColorRatio()
{ 
  return m_fWorldColorRatio;
} 

Vec3d C3DEngine::GetWorldColor(bool bScaled) 
{ 
	if(bScaled)
	{
//*
	Vec3d	color;
		color = m_vWorldColorConst*m_fWorldColorRatio;
		if(color.x > 1.0f)
			color.x = 1.0f;
		if(color.y > 1.0f)
			color.y = 1.0f;
		if(color.z > 1.0f)
			color.z = 1.0f;
		return color;
//*/
//		return m_vWorldColorConst*m_fWorldColorRatio;
	}
	else
		return m_vWorldColorConst;
}

void C3DEngine::SetWorldColor(Vec3d vColor)
{
  m_vWorldColorConst = vColor;
}

void C3DEngine::SetOutdoorAmbientColor(Vec3d vColor)
{
	if (m_pObjManager)
		m_pObjManager->m_vOutdoorAmbientColor = vColor;
}

float C3DEngine::GetTerrainElevation(float x, float y)
{
  return m_pTerrain ? m_pTerrain->GetZApr(x, y) : 0;
}

float C3DEngine::GetTerrainZ(int x, int y)
{
  if(x<0 || y<0 || x>=CTerrain::GetTerrainSize() || y>=CTerrain::GetTerrainSize())
	{
//		GetLog()->Log("C3DEngine::GetTerrainZ: values out of range %d, %d, %d", (int)m_pTerrain, x, y);
		return BOTTOM_LEVEL;
	}
  return m_pTerrain ? m_pTerrain->GetZ(x, y) : 0;
}

int C3DEngine::GetHeightMapUnitSize()
{
  return CTerrain::GetHeightMapUnitSize();
}

int C3DEngine::GetTerrainSize()
{
  return CTerrain::GetTerrainSize();
}

float C3DEngine::GetMaxViewDist()
{
  return m_fMaxViewDist;
}
/*
bool C3DEngine::AddStaticObject(const char * szFileName, const Vec3d & vPos, const float fScale)
{
  if(!m_pTerrain)
    return 0;

  return m_pTerrain->AddStaticObject(szFileName, vPos, fScale, m_pObjManager);
}	*/

bool C3DEngine::AddStaticObject(int nObjectID, const Vec3d & vPos, const float fScale, uchar ucBright)
{
  if(!m_pTerrain)
    return 0;

  return m_pTerrain->AddStaticObject(nObjectID, vPos, fScale, m_pObjManager, ucBright);
}

bool C3DEngine::RemoveStaticObject(int nObjectID, const Vec3d & vPos)
{
  if(!m_pTerrain)
    return 0;

  return m_pTerrain->RemoveStaticObject(nObjectID, vPos, m_pObjManager);
}

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4311 )
#endif

bool C3DEngine::PhysicalizeStaticObject(void *pForeignData,int iForeignData,int iForeignFlags)
{
	if (!m_pTerrain || !m_pObjManager)
		return false;

	int ix=(int)pForeignData&0xFF, iy=(int)pForeignData>>8&0xFF, iobj=(int)pForeignData>>16&0xFFFF;
	return 0;//m_pObjManager->PhysicalizeStatObjInst( &m_pTerrain->m_arrSecInfoTable[ix][iy]->m_lstStatObjects[iobj], true );
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

void C3DEngine::RemoveAllStaticObjects()
{
  if(!m_pTerrain)
    return;

//  m_pObjManager->DeletePhysicalEntitys();

  m_pTerrain->RemoveAllStaticObjects();
}

int C3DEngine::GetTerrainSurfaceType(int x, int y)
{
  if(m_pTerrain)
    return m_pTerrain->GetSurfaceType(x,y);

  return -1;
}

void C3DEngine::SetTerrainSurfaceType(int x, int y, int nType)
{
  if(m_pTerrain)
    m_pTerrain->SetSurfaceType(x,y,nType);
}

void C3DEngine::SetTerainHightMapBlock(int x1, int y1, int nSizeX, int nSizeY, ushort * TerrainBlock, ushort nUpdateMask)
{
  if(m_pTerrain)
    m_pTerrain->SetTerainHightMapBlock(x1, y1, nSizeX, nSizeY, TerrainBlock, nUpdateMask);
}

int C3DEngine::LockTerrainSectorTexture(int nSectorOriginX, int nSectorOriginY, int & nTexDim)
{
	return m_pTerrain ? m_pTerrain->LockSectorTexture(nSectorOriginX, nSectorOriginY, nTexDim) : 0;
}

void C3DEngine::OnExplosion(Vec3d vPos, Vec3 vHitDir, float fRadius, int nTexID, bool bDeformTerrain)
{
  if(vPos.x<0 || vPos.x>=CTerrain::GetTerrainSize())
    return;
  if(vPos.y<0 || vPos.y>=CTerrain::GetTerrainSize())
    return;

//  m_pBFManager->KillBF(vPos, fRadius);

//  float fRadiusMinus = (vPos.z - GetTerrainZ((int)vPos.x,(int)vPos.y));
	//if(fRadiusMinus < -1)
    //return; // underground

//  fRadius -= fRadiusMinus;
  if(fRadius<=0)
    return;

	fRadius *= GetCVars()->e_explosion_scale;

//	if(vPos.z <= GetWaterLevel())
	//	GetRenderer()->EF_AddSplash(vPos, EST_Water, fRadius);

  //vPos+=Vec3d(1,1,0);
  for(int x=int(vPos.x-fRadius); x<=int(vPos.x+fRadius)+1; x+=CTerrain::GetHeightMapUnitSize())
  for(int y=int(vPos.y-fRadius); y<=int(vPos.y+fRadius)+1; y+=CTerrain::GetHeightMapUnitSize())
    if(m_pTerrain->GetHoleSafe(x,y))
      return;

  bool bGroundBurnedOut = m_pTerrain->IsBurnedOut((int)vPos.x,(int)vPos.y);
	float fHeight = (vPos.z - GetTerrainZ((int)vPos.x,(int)vPos.y));

  if(m_pObjManager && !bGroundBurnedOut && fHeight>0 && fHeight<fRadius)
    m_pTerrain->ExplodeTerrain(vPos,fRadius, m_pObjManager, bDeformTerrain);

	list2<IEntityRender*> lstStaticsAround;
	if(m_pTerrain)
		m_pTerrain->GetObjectsAround(vPos, fRadius, &lstStaticsAround);
	if(m_pVisAreaManager)
		m_pVisAreaManager->GetObjectsAround(vPos, fRadius, &lstStaticsAround);

	// make decals on the objects
	for(int i=0; i<lstStaticsAround.Count(); i++)
  {
		CryEngineDecalInfo DecalInfo;
		DecalInfo.vPos      = vPos;
		DecalInfo.vNormal   = Vec3d(0,0,1);
		DecalInfo.fSize     = fRadius;//*0.6666f;
		DecalInfo.fLifeTime = 120;
		DecalInfo.nTid      = nTexID;
		DecalInfo.fAngle		=	float(rand()%36000)*0.01f;
		DecalInfo.pDecalOwner = lstStaticsAround[i];
		DecalInfo.nPartID = 0;
//		DecalInfo.vHitDirection = ((vPos-GetViewCamera().GetPos()).GetNormalized()/*+vHitDir.GetNormalized()*/).GetNormalized();
		DecalInfo.bAdjustPos = false;
    CreateDecal( DecalInfo );
  }

	// make decal on the ground
	if(!bGroundBurnedOut && fHeight<fRadius && fHeight>-0.5)
	{
		CryEngineDecalInfo DecalInfo;
		DecalInfo.vPos      = vPos - Vec3d(0,0,fHeight);
		DecalInfo.vNormal   = Vec3d(0,0,1);
		DecalInfo.fSize     = fRadius-fHeight;
		DecalInfo.fLifeTime = 120;
		DecalInfo.nTid      = nTexID;
		DecalInfo.fAngle		=	float(rand()%36000)*0.01f;
		DecalInfo.bAdjustPos = false;
		CreateDecal( DecalInfo );
	}
}

void C3DEngine::SetSkyBox(const char * szSkyBoxShaderName)
{
  SAFE_RELEASE(m_pSHSky);
	m_pSHSky = szSkyBoxShaderName[0] ? GetRenderer()->EF_LoadShader(szSkyBoxShaderName, eSH_World) : NULL;
}
/*
int C3DEngine::GetMapPreviewTexID()
{
  return (m_pTerrain && m_pTerrain->m_pMapPreviewTex) ? m_pTerrain->m_pMapPreviewTex->GetTextureID() : 0;
} */

void C3DEngine::SetScreenShader(const char * szShaderName)
{ 
  if(szShaderName)
    m_pSHFullScreenQuad = GetRenderer()->EF_LoadShader(szShaderName, eSH_World, EF_SYSTEM);
  else
    m_pSHFullScreenQuad = NULL;
}

void C3DEngine::SetMaxViewDistance(float fMaxViewDistance)
{
  m_fMaxViewDist = fMaxViewDistance;
}

float C3DEngine::GetMaxViewDistance( )
{
	return	m_fMaxViewDist;
}

/*
void C3DEngine::SetFogParams(float fFogStart, float fFogEnd, Vec3d vFogColor)
{
  m_fFogNearDist = fFogStart; 
  m_fFogFarDist  = fFogEnd; 
  m_vFogColor    = vFogColor;
} */

void C3DEngine::SetFogColor(const Vec3d& vFogColor)
{
  m_vFogColor    = vFogColor;
}

void C3DEngine::SetFogStart(const float fFogStart)
{
  m_fFogNearDist = fFogStart; 
}

void C3DEngine::SetFogEnd(const float fFogEnd)
{
  m_fFogFarDist  = fFogEnd; 
}

Vec3d C3DEngine::GetFogColor( )
{
  return m_vFogColor;
}

float C3DEngine::GetFogStart( )
{
  return m_fFogNearDist;
}

float C3DEngine::GetFogEnd( )
{
  return m_fFogFarDist; 
}


/*
void C3DEngine::GetFogAndViewDistance(float & fFogStart, float & fFogEnd, Vec3d & vFogColor, float & fMaxViewDistance, bool bGetDefaultValues)
{
  if(bGetDefaultValues)
  {
    fFogStart        = m_fDefFogNearDist; 
    fFogEnd          = m_fDefFogFarDist ; 
    vFogColor        = m_vDefFogColor   ;
    fMaxViewDistance = m_fDefMaxViewDist;
  }
  else
  {
    fFogStart        = m_fFogNearDist; 
    fFogEnd          = m_fFogFarDist ; 
    vFogColor        = m_vFogColor   ;
    fMaxViewDistance = m_fMaxViewDist;
  }
} */

void C3DEngine::SetPhysMaterialEnumerator(IPhysMaterialEnumerator * pPhysMaterialEnumerator)
{
  m_pPhysMaterialEnumerator = pPhysMaterialEnumerator;
}

IPhysMaterialEnumerator * C3DEngine::GetPhysMaterialEnumerator()
{
  return m_pPhysMaterialEnumerator;
}
/*
IIndoorBase * C3DEngine::GetBuildingManager()
{
  return 0;
} */

/*bool C3DEngine::IsPointInShadow(Vec3d vPos)
{
  return m_pTerrain ? m_pTerrain->IsOnTheLight((int)vPos.x,(int)vPos.y)==0 : true;
} */

bool C3DEngine::IsPointInWater(Vec3d vPos)
{
  // TODO: test indoors aslo
  return vPos.z < GetWaterLevel(&vPos);
}

void C3DEngine::ApplyForceToEnvironment(Vec3d vPos, float fRadius, float fAmountOfForce)
{
  if(m_pTerrain)
    m_pTerrain->ApplyForceToEnvironment(vPos, fRadius, fAmountOfForce);
}

void BlurImage24(byte * pImage, int nSize, int nPassesNum)
{
#define DATA_TMP(_x,_y,_c) (pTemp [((_x)+nSize*(_y))*3+_c])
#define DATA_IMG(_x,_y,_c) (pImage[((_x)+nSize*(_y))*3+_c])

  byte * pTemp = new byte [nSize*nSize*3];
  
  for(int nPass=0; nPass<nPassesNum; nPass++)
  { 
    memcpy(pTemp,pImage,nSize*nSize*3);

    for(int x=1; x<nSize-1; x++)
    for(int y=1; y<nSize-1; y++)
    for(int c=0; c<3; c++)
    {
      float fVal = 0;
      fVal += DATA_TMP(x,y,c);
      fVal += DATA_TMP(x+1,y+1,c);
      fVal += DATA_TMP(x-1,y+1,c);
      fVal += DATA_TMP(x+1,y-1,c);
      fVal += DATA_TMP(x-1,y-1,c);
      DATA_IMG(x,y,c) = uchar(fVal*0.2f);
    }
  }

  delete [] pTemp;

#undef DATA_IMG
#undef DATA_TMP
}

bool C3DEngine::MakeSectorLightMap(int nSectorOriginX, int nSectorOriginY, uchar * pImage, int nImageSize)
{
	// reset to white
	if(pImage)
  for(int i=0; i<nImageSize*nImageSize*3; i++)
    pImage[i] = 255;

	if(!m_pTerrain)
	  return false;  

	// remove all lsources
	while(m_lstDynLights.Count())
	{
		CDLight *pLight = &m_lstDynLights[0];
		FreeLightSourceComponents(pLight);
		m_lstDynLights.Delete(0);
	}
	
	// add sun lsource into light manager
	UpdateScene(false,true); 

	// register sun in renderer
  GetRenderer()->EF_ClearLightsList();
	UpdateLightSources();
	PrepareLightSourcesForRendering();    

	// only sun should be
	int nRealLightsNum = ((C3DEngine*)Get3DEngine())->GetRealLightsNum();
	assert(nRealLightsNum==1);

  // increase frame id to help shadow map manager in renderer
  unsigned short * pPtr2FrameID = (unsigned short *)GetRenderer()->EF_Query(EFQ_Pointer2FrameID);
  if(pPtr2FrameID)
    (*pPtr2FrameID)++;

  if(nImageSize>GetRenderer()->GetWidth() || nImageSize>(float)GetRenderer()->GetHeight())
  {
    if(!m_bTerrainLightMapGenError)
      Warning( 0,0,
			"C3DEngine::MakeSectorLightMap: Requested image size is bigger than size of current renderer window."
			"Please select lower resolution for terrain texture.");
    m_bTerrainLightMapGenError = true;
    return false;  
  }

  if(!GetCVars()->e_shadow_maps)
  {
    m_bTerrainLightMapGenError = true;
    return false;  
  }

	GetLog()->Log("Building shadow map for sector %d-%d ... ", nSectorOriginX, nSectorOriginY);

	GetRenderer()->BeginFrame();

	{ 
		GetRenderer()->ResetToDefault();
		GetRenderer()->ClearDepthBuffer();
		GetRenderer()->ClearColorBuffer(Vec3d(1,1,1));
		GetRenderer()->SetClearColor(Vec3d(1,1,1));
		GetRenderer()->SetViewport(0,0,nImageSize,nImageSize);
		GetRenderer()->PushMatrix();
		GetRenderer()->Set2DMode(true,int(CTerrain::GetSectorSize()),int(CTerrain::GetSectorSize()));

		float fHalfPixelSize = (float(CTerrain::GetSectorSize())/nImageSize)*0.5f;
		GetRenderer()->TranslateMatrix(-(float)nSectorOriginX-fHalfPixelSize, -(float)nSectorOriginY-fHalfPixelSize, 0);

		{
			for(int x=0; x<CTerrain::GetSectorsTableSize(); x++)
			for(int y=0; y<CTerrain::GetSectorsTableSize(); y++)
			{
				CSectorInfo * pSecInfo = m_pTerrain->GetSecInfo(x*CTerrain::GetSectorSize(), y*CTerrain::GetSectorSize());
				if(pSecInfo)
				{
					pSecInfo->ReleaseHeightMapVertBuffer();
					pSecInfo->m_cGeometryMML=0;
					pSecInfo->m_fDistance=0;
				}
			}
		}

		list2<struct IEntityRender*> lstAllEntities;
		CSectorInfo * pSecInfo = m_pTerrain->GetSecInfo(nSectorOriginX, nSectorOriginY);
		if(pSecInfo)
		{
			// setup vertex container
			pSecInfo->m_cGeometryMML=0;
			pSecInfo->m_fDistance=0;

			// prepare terrain geometry
			GetRenderer()->EF_StartEf();  
			pSecInfo->RenderSector(GetIdentityCCObject());
			GetRenderer()->EF_EndEf3D(0);  

			GetRenderer()->ResetToDefault();
			GetRenderer()->ClearDepthBuffer();
			GetRenderer()->ClearColorBuffer(Vec3d(1,0,1));
			GetRenderer()->SetClearColor(Vec3d(1,1,1));

			// fill list with entities in this sector and neighbor sectors
			if(pSecInfo)
			{
				lstAllEntities.AddList(pSecInfo->m_lstEntities[STATIC_ENTITIES]);

				// get 2d bbox of sector
				Vec3d vSecBoxMin(	(float)pSecInfo->m_nOriginX,
					(float)pSecInfo->m_nOriginY,0);
				Vec3d vSecBoxMax(	(float)pSecInfo->m_nOriginX+CTerrain::GetSectorSize(),
					(float)pSecInfo->m_nOriginY+CTerrain::GetSectorSize(),512);

				// get 2d bounds in sectors array
				int min_x = (int)(((vSecBoxMin.x - 16.f)/CTerrain::GetSectorSize()));
				int min_y = (int)(((vSecBoxMin.y - 16.f)/CTerrain::GetSectorSize()));
				int max_x = (int)(((vSecBoxMax.x + 16.f)/CTerrain::GetSectorSize()));
				int max_y = (int)(((vSecBoxMax.y + 16.f)/CTerrain::GetSectorSize()));

				// limit bounds
				if(min_x<0) min_x=0; else if(min_x>=CTerrain::GetSectorsTableSize()) min_x=CTerrain::GetSectorsTableSize()-1;
				if(min_y<0) min_y=0; else if(min_y>=CTerrain::GetSectorsTableSize()) min_y=CTerrain::GetSectorsTableSize()-1;
				if(max_x<0) max_x=0; else if(max_x>=CTerrain::GetSectorsTableSize()) max_x=CTerrain::GetSectorsTableSize()-1;
				if(max_y<0) max_y=0; else if(max_y>=CTerrain::GetSectorsTableSize()) max_y=CTerrain::GetSectorsTableSize()-1;

				list2<CSectorInfo*> lstTmpSectors;
				lstTmpSectors.Add(m_pTerrain->m_arrSecInfoTable[0][0]);
				for(int x=min_x; x<=max_x && x>=0 && x<=CTerrain::GetTerrainSize(); x++)
				for(int y=min_y; y<=max_y && y>=0 && y<=CTerrain::GetTerrainSize(); y++)
				{
					CSectorInfo * pSectorInfo = m_pTerrain->m_arrSecInfoTable[x][y];
					if(pSectorInfo != m_pTerrain->m_arrSecInfoTable[0][0])
						lstTmpSectors.Add(pSectorInfo);
				}

				for(int s=0; s<lstTmpSectors.Count(); s++)
				{
					CSectorInfo * pSectorInfo = lstTmpSectors[s];
					for(int e=0; e<pSectorInfo->m_lstEntities[STATIC_ENTITIES].Count(); e++)
					{
						IEntityRender * pEnt = pSectorInfo->m_lstEntities[STATIC_ENTITIES][e];

						if(!(pEnt->GetRndFlags() & ERF_HIDDEN))
							if(pEnt->GetRndFlags() & ERF_CASTSHADOWINTOLIGHTMAP)
								if(!pEnt->GetEntityVisArea())
								{
									Vec3d vEntBoxMin,vEntBoxMax;
									pEnt->GetRenderBBox(vEntBoxMin,vEntBoxMax);
									Vec3d vRadius(pEnt->GetRenderRadius(),pEnt->GetRenderRadius(),pEnt->GetRenderRadius());
									if(Overlap::AABB_AABB(AABB(vEntBoxMin,vEntBoxMax), AABB(vSecBoxMin-vRadius,vSecBoxMax+vRadius))
										&& lstAllEntities.Find(pEnt)<0)
									lstAllEntities.Add(pEnt);
								}
					}
				}
			}
	
			GetLog()->LogPlus("%d casters found ... ", lstAllEntities.Count());

			GetRenderer()->ResetToDefault();
			GetRenderer()->ClearDepthBuffer();
			GetRenderer()->ClearColorBuffer(Vec3d(1,0,1));
			GetRenderer()->SetClearColor(Vec3d(1,1,1));

			m_pObjManager->DrawAllShadowsOnTheGroundInSector(&lstAllEntities);  
		}

		pSecInfo->m_cGeometryMML = 0;
		pSecInfo->m_pCurShader = m_pTerrain->m_pTerrainEf;

		GetRenderer()->Set2DMode(false,CTerrain::GetSectorSize(),CTerrain::GetSectorSize());

		GetRenderer()->PopMatrix();

		if(lstAllEntities.Count())
		{
			GetLog()->LogPlus("reading frame buffer ... ");

			if(pImage)
				GetRenderer()->ReadFrameBuffer(pImage, nImageSize, nImageSize, true, false);  

			bool bImageFound=false;
			if(pImage)
			for(int i=1; i<nImageSize*nImageSize*3; i++)
				if(pImage[i]!=pImage[i-1])
					bImageFound = true;

			if(bImageFound && pImage)
			{
				GetLog()->LogPlus("blur image ... ");
				BlurImage24(pImage, nImageSize, 1);
			}
			
/*			{
				char sFileName[128];
				snprintf(sFileName,128,"lmap-%d-%d.tga",nSectorOriginX, nSectorOriginY);
				if(bImageFound)
					GetRenderer()->SaveTga(pImage,24,nImageSize,nImageSize,sFileName,0);
				else
					remove(sFileName);
			}*/
		}								
	}
  
	GetRenderer()->Update();

	GetLog()->LogPlus("done");

  return true;  
}

float C3DEngine::GetDistanceToSectorWithWater()
{
	CSectorInfo * pSectorInfo = NULL;
	if(m_pTerrain)
		pSectorInfo = m_pTerrain->GetSecInfo(GetViewCamera().GetPos());

  return (pSectorInfo && (m_pTerrain->m_fDistanceToSectorWithWater > 0.1f))
    ? m_pTerrain->m_fDistanceToSectorWithWater : max(GetViewCamera().GetPos().z - GetWaterLevel(),0.1f);
}

void C3DEngine::SetSkyBoxAlpha(float fAlpha)
{
  if(m_pRESky)
    m_pRESky->m_fAlpha = fAlpha;
}


void C3DEngine::SetBFCount(int nCount)
{
	Warning(0,0,"C3DEngine::SetBFCount: butterflies are not supported by 3dengine anymore, plz use boids entity instead");
//  if(m_pBFManager)
  //  m_pBFManager->SetCount(nCount);
}

int C3DEngine::GetBFCount()
{
  return 0;//m_pBFManager ? m_pBFManager->GetCount() : 0;
}

void C3DEngine::SetGrasshopperCGF( int nSlot, IStatObj * pStatObj ) 
{
	GetLog()->Log("Warning: I3DEngine::SetGrasshopperCGF: Feature not supported anymore");
//  if(m_pTerrain && m_pTerrain->m_pBugsManager)
  //  m_pTerrain->m_pBugsManager->SetCGF( nSlot, pStatObj );
}

void C3DEngine::SetGrasshopperCount(int nCount)
{
	GetLog()->Log("Warning: I3DEngine::SetGrasshopperCount: Feature not supported anymore");
//  if(m_pTerrain && m_pTerrain->m_pBugsManager)
  //  m_pTerrain->m_pBugsManager->SetCount(nCount);
}

int C3DEngine::GetGrasshopperCount()
{
	GetLog()->Log("Warning: I3DEngine::GetGrasshopperCount: Feature not supported anymore");
  return 0;//(m_pTerrain && m_pTerrain->m_pBugsManager) ? m_pTerrain->m_pBugsManager->GetCount() : 0;
}

Vec3d C3DEngine::GetOutdoorAmbientColor()
{
  return m_pObjManager ? m_pObjManager->m_vOutdoorAmbientColor : Vec3d(0,0,0);
}

Vec3d C3DEngine::GetSunColor()
{
  return m_pObjManager ? m_pObjManager->m_vSunColor : Vec3d(0,0,0);
}

Vec3d C3DEngine::GetSunPosition(bool bMoveUp) 
{ 
  if(bMoveUp)
  {
    Vec3d vSunPosition = m_vSunPosition;
    vSunPosition.Normalize();
    vSunPosition.z+=0.5f;
    vSunPosition*=10000.0f;
    return vSunPosition;
  }

  return m_vSunPosition; 
}

Vec3d C3DEngine::GetAmbientColorFromPosition(const Vec3d & vPos, float fRadius) 
{
	if(m_pObjManager)
	{
		if(CVisArea * pVisArea =	(CVisArea *)m_pVisAreaManager->GetVisAreaFromPos(vPos))
			return pVisArea->m_vAmbColor+pVisArea->m_vDynAmbColor; // indooor

		return m_pObjManager->m_vOutdoorAmbientColor;
	}
	
	return Vec3d(0,0,0); // undefined outdoor
}

uint C3DEngine::GetLightMaskFromPosition(const Vec3d & vPos, float fRadius) 
{
//	if(CVisArea * pVisArea =	(CVisArea *)m_pVisAreaManager->GetVisAreaFromPos(vPos))
	//	return pVisArea->m_vAmbColor; // indooor

  if(!m_pTerrain)
    return 0;

  if(fRadius > (float)CTerrain::GetSectorSize())
    fRadius = (float)CTerrain::GetSectorSize();

  CSectorInfo *pSectorInfo0 = m_pTerrain->GetSecInfo(vPos);
	CSectorInfo *pSectorInfo1=0,*pSectorInfo2=0,*pSectorInfo3=0,*pSectorInfo4=0;
	if (fRadius != 0)
	{
		pSectorInfo1 = m_pTerrain->GetSecInfo(vPos+Vec3d( fRadius,       0,0));
	  pSectorInfo2 = m_pTerrain->GetSecInfo(vPos+Vec3d(       0, fRadius,0));
		pSectorInfo3 = m_pTerrain->GetSecInfo(vPos+Vec3d(-fRadius,       0,0));
		pSectorInfo4 = m_pTerrain->GetSecInfo(vPos+Vec3d(       0,-fRadius,0));
	}

  uint nMask = 0;
	if(GetRenderer()->EF_GetHeatVision())
	{
		nMask |= pSectorInfo0 ? pSectorInfo0->m_nDynLightMaskNoSun : 0;
		nMask |= pSectorInfo1 ? pSectorInfo1->m_nDynLightMaskNoSun : 0;
		nMask |= pSectorInfo2 ? pSectorInfo2->m_nDynLightMaskNoSun : 0;
		nMask |= pSectorInfo3 ? pSectorInfo3->m_nDynLightMaskNoSun : 0;
		nMask |= pSectorInfo4 ? pSectorInfo4->m_nDynLightMaskNoSun : 0;
	}
	else
	{
		nMask |= pSectorInfo0 ? pSectorInfo0->m_nDynLightMask : 0;
		nMask |= pSectorInfo1 ? pSectorInfo1->m_nDynLightMask : 0;
		nMask |= pSectorInfo2 ? pSectorInfo2->m_nDynLightMask : 0;
		nMask |= pSectorInfo3 ? pSectorInfo3->m_nDynLightMask : 0;
		nMask |= pSectorInfo4 ? pSectorInfo4->m_nDynLightMask : 0;
	}

	assert(nMask>=0);

	if(!nMask)
	{
    nMask = GetFullLightMask();
    //		int nTmp = GetCVars()->e_max_entity_lights;
    //	GetCVars()->e_max_entity_lights = 8;
    CheckDistancesToLightSources(nMask, vPos, fRadius);
    //GetCVars()->e_max_entity_lights = nTmp;
	}

  return nMask;
}

bool C3DEngine::IsBoxVisibleOnTheScreen(const Vec3d & vBoxMin, const Vec3d & vBoxMax, OcclusionTestClient * pOcclusionTestClient )
{
	// frustum test
  if(!GetViewCamera().IsAABBVisibleFast( AABB(vBoxMin,vBoxMax) ))
    return false;

	// check visibility of outdoor sectors where this box is
  // get 2d bounds in sectors array
  int min_x = (int)(((vBoxMin.x - 1.f)/CTerrain::GetSectorSize()));
  int min_y = (int)(((vBoxMin.y - 1.f)/CTerrain::GetSectorSize()));
  int max_x = (int)(((vBoxMax.x + 1.f)/CTerrain::GetSectorSize()));
  int max_y = (int)(((vBoxMax.y + 1.f)/CTerrain::GetSectorSize()));

  if(min_x<0) min_x=0;
  if(min_y<0) min_y=0;
	if(max_x>=CTerrain::GetSectorsTableSize()) max_x = CTerrain::GetSectorsTableSize()-1;
	if(max_y>=CTerrain::GetSectorsTableSize()) max_y = CTerrain::GetSectorsTableSize()-1;

	// this test is valid only if entire bbox is in the map
  if( min_x>=0 && max_x<CTerrain::GetSectorsTableSize() || 
			min_y>=0 && max_y<CTerrain::GetSectorsTableSize() )
  { // if some sector was visible in last 5 sec - box is visible
		for(int x=min_x; x<=max_x; x++)
		for(int y=min_y; y<=max_y; y++)
		{	// if some sector was visible in last 5 sec - box is visible
			if(!GetCVars()->e_terrain || 
				(GetTimer()->GetCurrTime() - m_pTerrain->m_arrSecInfoTable[x][y]->GetLastTimeUsed()) < 3.f)
			{
				if( pOcclusionTestClient )
				{ // test terrain and objects (cbuffer) occlusion
					float fDist = GetDistance( (vBoxMin + vBoxMax)*0.5f, GetViewCamera().GetPos());
					if( m_pObjManager->IsBoxOccluded(vBoxMin,vBoxMax,fDist,pOcclusionTestClient) )
						return false;
				}

				return true; // sector is visible
			}
		}

		return false; // no sectors was rendered in last seconds
  }

	return true;
}

bool C3DEngine::IsSphereVisibleOnTheScreen(const Vec3d & vPos, const float fRadius, OcclusionTestClient * pOcclusionTestClient )
{
	return IsBoxVisibleOnTheScreen(
		vPos - Vec3d(fRadius,fRadius,fRadius), 
		vPos + Vec3d(fRadius,fRadius,fRadius), 
		pOcclusionTestClient );
}

IEntityRenderState * C3DEngine::MakeEntityRenderState()
{
	return new IEntityRenderState;
}

void C3DEngine::FreeEntityRenderState(IEntityRender * pEntityRnd)
{
//  GetLog()->Log("vlad: C3DEngine::FreeEntityRenderState: %d, %s", (int)(IEntityRender*)pEntityRnd, pEntityRnd->GetName());

	/*
	LightMapInfo * pLightMapInfo = (LightMapInfo *)pEntityRnd->GetLightMapInfo();
	if(pLightMapInfo && pLightMapInfo->pLMTCBuffer)
		GetRenderer()->ReleaseBuffer(pLightMapInfo->pLMTCBuffer);
	*/
	// TODO: Delete data here, or at a more appropiate place 
	// (currently done in CBrush::~CBrush(), hope that is 'a more appropiate place')

	RemoveEntityLightSources(pEntityRnd);

  if(m_pDecalManager)
    m_pDecalManager->OnEntityDeleted(pEntityRnd);

  if(m_pPartManager)
    m_pPartManager->OnEntityDeleted(pEntityRnd);

	GetRenderer()->OnEntityDeleted(pEntityRnd);

/*
#ifdef _DEBUG
  UnRegisterInAllSectors(pEntityRnd);
#else*/
  UnRegisterEntity(pEntityRnd);
//#endif

  IEntityRenderState * pEntRendState = pEntityRnd->GetEntityRS();

	if(!pEntRendState)
		return;

  if(pEntRendState->pShadowMapInfo)
	{
		pEntRendState->pShadowMapInfo->Release(pEntityRnd->GetEntityRenderType(), GetRenderer());
		pEntRendState->pShadowMapInfo = NULL;
	}

  // check that was really unregistered
/*  if(pEntityRnd->m_pSector)
    for(int t=0; t<2; t++)
	{
	  for(int e=0; e<pEntityRnd->m_pSector->m_lstEntities[t].Count(); e++)
		{
			if(pEntityRnd->m_pSector->m_lstEntities[t][e]->GetEntityRS() == pEntRendState)
			{
				assert(0); // UnRegisterInAllSectors do this already
				pEntityRnd->m_pSector->m_lstEntities[t].Delete(e);
			}
		}

		pEntityRnd->m_pSector->m_lstEntities[t].Delete(pEntityRnd);
	}										 

  pEntityRnd->m_pSector = 0;
  */

	// delete occlusion client
/*	for(int i=0; i<2; i++)
	if(pEntRendState->pOcclState && pEntRendState->pOcclState->arrREOcclusionQuery[i])
	{
		pEntRendState->pOcclState->arrREOcclusionQuery[i]->Release();
		pEntRendState->pOcclState->arrREOcclusionQuery[i] = 0;
	}*/

//  delete pEntRendState->pOcclState;
  //pEntRendState->pOcclState = 0;

  //todo: is it needed?
//	pEntityRnd->m_pVisArea = 0;

	delete pEntRendState;
	pEntityRnd->GetEntityRS()=0;
}

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

//////////////////////////////////////////////////////////
// checks whether the given file is a character file
// returns true if it is
//////////////////////////////////////////////////////////
bool C3DEngine::IsCharacterFile(const char * szCGFileName)
{
  FILE * f = GetSystem()->GetIPak()->FOpen(szCGFileName,"rb");
  if(!f)
    return false;

  //read the file header
  FILE_HEADER fh;
  int res = GetSystem()->GetIPak()->FRead(&fh,sizeof(fh),1,f);
  if(res!=1)
  {
    GetSystem()->GetIPak()->FClose(f);
    return false;
  }

  if(fh.Version != GeomFileVersion || fh.FileType != FileType_Geom) 
  {
    GetSystem()->GetIPak()->FClose(f);
    return false;
  }

  //read the chunk table
  GetSystem()->GetIPak()->FSeek(f,fh.ChunkTableOffset,SEEK_SET);
  int n_chunks;
  res = GetSystem()->GetIPak()->FRead(&n_chunks,sizeof(n_chunks),1,f);
  if(res!=1)
  {
    GetSystem()->GetIPak()->FClose(f);
    return false;
  }

  CHUNK_HEADER * arrChunks = new CHUNK_HEADER [n_chunks];
  res = GetSystem()->GetIPak()->FRead(arrChunks,sizeof(CHUNK_HEADER),n_chunks,f);
  if(res!=n_chunks)
  {
		delete [] arrChunks;
    GetSystem()->GetIPak()->FClose(f);
    return false;
  }

  for(int i=0;i<n_chunks;i++)
  {
    switch (arrChunks[i].ChunkType)
    {
      case ChunkType_BoneNameList:
				delete [] arrChunks;
				GetSystem()->GetIPak()->FClose (f);
				return true;
		}
	}

	delete [] arrChunks;
	GetSystem()->GetIPak()->FClose(f);
	return false;
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

//////////////////////////////////////////////////////////////////////////
IParticleEmitter* C3DEngine::CreateParticleEmitter()
{
	if (m_pPartManager)
    return m_pPartManager->CreateEmitter();
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void C3DEngine::DeleteParticleEmitter(IParticleEmitter * pPartEmitter)
{
	assert(pPartEmitter);
	if (m_pPartManager)
		m_pPartManager->DeleteEmitter(pPartEmitter);
}

const char * C3DEngine::GetLevelFilePath(const char * szFileName)
{
  strcpy(m_sGetLevelFilePathTmpBuff, m_szLevelFolder);
  strcat(m_sGetLevelFilePathTmpBuff, szFileName);
	return m_sGetLevelFilePathTmpBuff;
}

ushort * C3DEngine::GetUnderWaterSmoothHMap(int & nDimensions)
{
  return m_pTerrain ? m_pTerrain->GetUnderWaterSmoothHMap(nDimensions) : 0;
}

void C3DEngine::MakeUnderWaterSmoothHMap(int nWaterUnitSize)
{
  if(m_pTerrain)
    m_pTerrain->MakeUnderWaterSmoothHMap(nWaterUnitSize);
}

void C3DEngine::SetTerrainBurnedOut(int x, int y, bool bBurnedOut)
{
  if(m_pTerrain)
  {
    m_pTerrain->SetBurnedOut(x, y, bBurnedOut);

    // update vert buffers
    CSectorInfo * pInfo = m_pTerrain->GetSecInfo(x,y);
    if(pInfo)
      pInfo->ReleaseHeightMapVertBuffer();
  }
}

bool C3DEngine::IsTerrainBurnedOut(int x, int y)
{
  return m_pTerrain ? m_pTerrain->IsBurnedOut(x, y) : 0;
}

void C3DEngine::UpdateDetailObjects() 
{ 
  if(m_pTerrain && m_pTerrain->m_pDetailObjects)
    m_pTerrain->m_pDetailObjects->UpdateGrass(); 
}

int C3DEngine::GetTerrainSectorSize()
{
  return m_pTerrain ? m_pTerrain->GetSectorSize() : 0;
}

void C3DEngine::AddWaterSplash (Vec3d vPos, eSplashType eST, float fForce, int Id=-1)
{
  GetRenderer()->EF_AddSplash(vPos, eST, fForce, Id);
}

void C3DEngine::EnableHeatVision(bool bEnable)
{
	GetRenderer()->EF_EnableHeatVision(bEnable);
}

void C3DEngine::ActivatePortal(const Vec3d &vPos, bool bActivate, IEntityRender *pEntity)
{
	m_pVisAreaManager->ActivatePortal(vPos, bActivate, pEntity);
}

int C3DEngine::GetTerrainTextureDim()
{
	return m_pTerrain ? m_pTerrain->GetTerrainTextureDim() : 0;
}

bool C3DEngine::SetStatInstGroup(int nGroupId, const IStatInstGroup & siGroup)
{
	m_pObjManager->m_lstStaticTypes.PreAllocate(1024,1024);

	if(nGroupId<0 || nGroupId>=m_pObjManager->m_lstStaticTypes.Count())
		return false;

	m_pObjManager->m_lstStaticTypes[nGroupId].pStatObj			= (CStatObj*)siGroup.pStatObj;
  if(siGroup.pStatObj)
    siGroup.pStatObj->CheckValidVegetation();

  m_pObjManager->m_lstStaticTypes[nGroupId].bHideability	= siGroup.bHideability;
	m_pObjManager->m_lstStaticTypes[nGroupId].bPhysNonColl	= siGroup.bPhysNonColl;
	m_pObjManager->m_lstStaticTypes[nGroupId].fBending			= siGroup.fBending;
	m_pObjManager->m_lstStaticTypes[nGroupId].bCastShadow   = siGroup.bCastShadow;
	m_pObjManager->m_lstStaticTypes[nGroupId].bRecvShadow   = siGroup.bRecvShadow;
	m_pObjManager->m_lstStaticTypes[nGroupId].bPrecShadow   = siGroup.bPrecShadow;

	m_pObjManager->m_lstStaticTypes[nGroupId].bUseAlphaBlending						= siGroup.bUseAlphaBlending;
//	m_pObjManager->m_lstStaticTypes[nGroupId].bTakeBrightnessFromLightBit = siGroup.bTakeBrightnessFromLightBit;
	m_pObjManager->m_lstStaticTypes[nGroupId].fSpriteDistRatio						= siGroup.fSpriteDistRatio;
	m_pObjManager->m_lstStaticTypes[nGroupId].fShadowDistRatio						= siGroup.fShadowDistRatio;
	m_pObjManager->m_lstStaticTypes[nGroupId].fMaxViewDistRatio						= siGroup.fMaxViewDistRatio;

	m_pObjManager->m_lstStaticTypes[nGroupId].fBrightness									= siGroup.fBrightness;
	m_pObjManager->m_lstStaticTypes[nGroupId].bUpdateShadowEveryFrame     = siGroup.bUpdateShadowEveryFrame;
//	m_pObjManager->m_lstStaticTypes[nGroupId].fAmbScale										= siGroup.fAmbScale;
	m_pObjManager->m_lstStaticTypes[nGroupId].nSpriteTexRes								= siGroup.nSpriteTexRes;	
  m_pObjManager->m_lstStaticTypes[nGroupId].pMaterial										= siGroup.pMaterial;	
  m_pObjManager->m_lstStaticTypes[nGroupId].fBackSideLevel							= siGroup.fBackSideLevel;	
	m_pObjManager->m_lstStaticTypes[nGroupId].bCalcLighting               = siGroup.bCalcLighting;
	m_pObjManager->m_lstStaticTypes[nGroupId].bUseSprites						      = siGroup.bUseSprites;
	m_pObjManager->m_lstStaticTypes[nGroupId].bFadeSize										= siGroup.bFadeSize;

	m_pObjManager->m_lstStaticTypes[nGroupId].SetRndFlags();

	return true;
}

bool C3DEngine::GetStatInstGroup(int nGroupId, IStatInstGroup & siGroup)
{
	if(nGroupId<0 || nGroupId>=m_pObjManager->m_lstStaticTypes.Count())
		return false;

	siGroup.pStatObj			= m_pObjManager->m_lstStaticTypes[nGroupId].pStatObj;
	siGroup.bHideability	= m_pObjManager->m_lstStaticTypes[nGroupId].bHideability;
	siGroup.bPhysNonColl	= m_pObjManager->m_lstStaticTypes[nGroupId].bPhysNonColl;
	siGroup.fBending			= m_pObjManager->m_lstStaticTypes[nGroupId].fBending;
	siGroup.bCastShadow   = m_pObjManager->m_lstStaticTypes[nGroupId].bCastShadow;
	siGroup.bRecvShadow   = m_pObjManager->m_lstStaticTypes[nGroupId].bRecvShadow;
	siGroup.bPrecShadow   = m_pObjManager->m_lstStaticTypes[nGroupId].bPrecShadow;

	siGroup.bUseAlphaBlending						= m_pObjManager->m_lstStaticTypes[nGroupId].bUseAlphaBlending;
//	siGroup.bTakeBrightnessFromLightBit = m_pObjManager->m_lstStaticTypes[nGroupId].bTakeBrightnessFromLightBit;
	siGroup.fSpriteDistRatio						= m_pObjManager->m_lstStaticTypes[nGroupId].fSpriteDistRatio;
	siGroup.fShadowDistRatio						= m_pObjManager->m_lstStaticTypes[nGroupId].fShadowDistRatio;
	siGroup.fMaxViewDistRatio						= m_pObjManager->m_lstStaticTypes[nGroupId].fMaxViewDistRatio;

	siGroup.fBrightness									= m_pObjManager->m_lstStaticTypes[nGroupId].fBrightness;
	siGroup.bUpdateShadowEveryFrame			= m_pObjManager->m_lstStaticTypes[nGroupId].bUpdateShadowEveryFrame;
//	siGroup.fAmbScale										= m_pObjManager->m_lstStaticTypes[nGroupId].fAmbScale;
	siGroup.nSpriteTexRes								=	m_pObjManager->m_lstStaticTypes[nGroupId].nSpriteTexRes;	
	siGroup.pMaterial										= m_pObjManager->m_lstStaticTypes[nGroupId].pMaterial;
  siGroup.fBackSideLevel              = m_pObjManager->m_lstStaticTypes[nGroupId].fBackSideLevel;	
  siGroup.bCalcLighting               = m_pObjManager->m_lstStaticTypes[nGroupId].bCalcLighting;
  siGroup.bUseSprites                 = m_pObjManager->m_lstStaticTypes[nGroupId].bUseSprites;

	return true;
}

void C3DEngine::GetMemoryUsage(class ICrySizer * pSizer)
{
	if (!pSizer->Add(*this))
		return; // we've already added this object

	{
	  SIZER_COMPONENT_NAME(pSizer, "Particles");
		if(m_pPartManager)
			m_pPartManager->GetMemoryUsage(pSizer);
	}

	pSizer->AddContainer(m_lstDynLights);
	
	{
//	  SIZER_COMPONENT_NAME(pSizer, "BFlies");
	//	pSizer->AddObject(m_pBFManager, m_pBFManager->GetMemoryUsage());
	}

	if(m_pConnectivityBuilder)
	{
	  SIZER_COMPONENT_NAME(pSizer, "EdgeConnectivityBuilder");
		m_pConnectivityBuilder->GetMemoryUsage(pSizer);
	}

	pSizer->AddObject(m_pCVars, sizeof(CVars));

	if(m_pDecalManager)
	{
	  SIZER_COMPONENT_NAME(pSizer, "DecalManager");
		m_pDecalManager->GetMemoryUsage(pSizer);
		pSizer->AddObject(m_pDecalManager, sizeof(*m_pDecalManager));
	}

	if(m_pShadowEdgeDetector)
	{
	  SIZER_COMPONENT_NAME(pSizer, "ShadowEdgeDetector");
		m_pShadowEdgeDetector->GetMemoryUsage(pSizer);
		pSizer->AddObject(m_pShadowEdgeDetector, sizeof(*m_pShadowEdgeDetector));
	}

	if(m_pObjManager)
	{
	  SIZER_COMPONENT_NAME(pSizer, "ObjManager");
		pSizer->AddObject(m_pObjManager, sizeof(*m_pObjManager) + m_pObjManager->GetMemoryUsage(pSizer));
	}
	
	if (m_pTerrain)
	{
	  SIZER_COMPONENT_NAME(pSizer, "Terrain");
		m_pTerrain->GetMemoryUsage(pSizer);
	}

	if (m_pVisAreaManager)
	{
	  SIZER_COMPONENT_NAME(pSizer, "VisAreas");
		m_pVisAreaManager->GetMemoryUsage(pSizer);
	}
}

float C3DEngine::GetWaterLevel(IEntityRender * pEntityRender, Vec3d * pvFlowDir)
{
//	const char * pName = pEntityRender->GetEntityClassName();
  if (pEntityRender && m_pObjManager && m_pObjManager->m_pCWaterVolumes && m_pObjManager->m_pTerrain)
  { 
		IPhysicalEntity *pent = pEntityRender->GetPhysics();
		pe_params_bbox pbb;
		Vec3d pos;
		if (pent && pent->GetParams(&pbb))
			pos = (pbb.BBox[0]+pbb.BBox[1])*0.5f;
		else
			pos = pEntityRender->GetPos();
    float fWaterVolumeLevel = m_pObjManager->m_pCWaterVolumes->GetWaterVolumeLevelFor2DPoint(pos, pvFlowDir);
    return max(fWaterVolumeLevel, pEntityRender->GetEntityVisArea() ? WATER_LEVEL_UNKNOWN : m_pObjManager->m_pTerrain->GetWaterLevel());
  }

  if(m_pObjManager && m_pObjManager->m_pTerrain && pEntityRender && !pEntityRender->GetEntityVisArea())
    return m_pObjManager->m_pTerrain->GetWaterLevel();

  return WATER_LEVEL_UNKNOWN;
}

float C3DEngine::GetWaterLevel(const Vec3d * pvPos, Vec3d * pvFlowDir)
{
  IVisArea * pArea = (pvPos && m_pVisAreaManager) ? m_pVisAreaManager->GetVisAreaFromPos(*pvPos) : 0;

	if(m_pObjManager && pvPos && m_pObjManager->m_pCWaterVolumes && m_pObjManager->m_pTerrain)
	{
		float fWaterVolumeLevel = m_pObjManager->m_pCWaterVolumes->GetWaterVolumeLevelFor2DPoint(*pvPos, pvFlowDir);
    return max(fWaterVolumeLevel, pArea ? WATER_LEVEL_UNKNOWN : m_pObjManager->m_pTerrain->GetWaterLevel());
	}

	if(m_pObjManager && m_pObjManager->m_pTerrain && !pArea)
		return m_pObjManager->m_pTerrain->GetWaterLevel();

	return WATER_LEVEL_UNKNOWN;
}

IWaterVolume * C3DEngine::CreateWaterVolume()
{
	if(!m_pObjManager->m_pCWaterVolumes)
		m_pObjManager->m_pCWaterVolumes = new CWaterVolumeManager();

	if(m_pObjManager && m_pObjManager->m_pCWaterVolumes)
		return m_pObjManager->m_pCWaterVolumes->CreateWaterVolume();

	return 0;
}

void C3DEngine::DeleteWaterVolume(IWaterVolume * pWaterVolume)
{
	if(m_pObjManager && m_pObjManager->m_pCWaterVolumes)
		m_pObjManager->m_pCWaterVolumes->DeleteWaterVolume(pWaterVolume);
}

IWaterVolume * C3DEngine::FindWaterVolumeByName(const char * szName)
{
	if(m_pObjManager && m_pObjManager->m_pCWaterVolumes)
		return m_pObjManager->m_pCWaterVolumes->FindWaterVolumeByName(szName);

	return 0;
}

IVisArea * C3DEngine::CreateVisArea()
{
	return m_pObjManager ? m_pVisAreaManager->CreateVisArea() : 0;
}

void C3DEngine::DeleteVisArea(IVisArea * pVisArea)
{
  if(!m_pVisAreaManager->IsValidVisAreaPointer((CVisArea*)pVisArea))
  {
    Warning( 0,0,"I3DEngine::DeleteVisArea: Invalid VisArea pointer");
    return;
  }
	if(m_pObjManager)
	{
		((CVisArea*)pVisArea)->UnmakeAreaBrush();

		list2<IEntityRender*> lstEntitiesInArea;
    lstEntitiesInArea.AddList(((CVisArea*)pVisArea)->m_lstEntities[DYNAMIC_ENTITIES]);
    lstEntitiesInArea.AddList(((CVisArea*)pVisArea)->m_lstEntities[STATIC_ENTITIES]);

		for(int i=0; i<lstEntitiesInArea.Count(); i++)
			Get3DEngine()->UnRegisterEntity(lstEntitiesInArea[i]);

    assert(((CVisArea*)pVisArea)->m_lstEntities[STATIC_ENTITIES].Count()==0);
    assert(((CVisArea*)pVisArea)->m_lstEntities[DYNAMIC_ENTITIES].Count()==0);

		m_pVisAreaManager->DeleteVisArea((CVisArea*)pVisArea);

		for(int i=0; i<lstEntitiesInArea.Count(); i++)
			Get3DEngine()->RegisterEntity(lstEntitiesInArea[i]);

		if(m_pObjManager->m_pCWaterVolumes)
			m_pObjManager->m_pCWaterVolumes->UpdateWaterVolumeVisAreas();
	}
}

void C3DEngine::UpdateVisArea(IVisArea * pVisArea, 
															const Vec3d * pPoints, int nCount, 
															const char * szName, float fHeight, 
															const Vec3d & vAmbientColor, 
															bool bAfectedByOutLights,
                              bool bSkyOnly, const Vec3 & vDynAmbientColor,
															float fViewDistRatio, bool bDoubleSide, bool bUseDeepness, bool bUseInIndoors )
{
	if(!m_pObjManager)
    return;

  CVisArea * pArea = (CVisArea*)pVisArea;

	GetLog()->Log("C3DEngine::UpdateVisArea: %s", szName);

  Vec3d vTotalBoxMin = pArea->m_vBoxMin;
  Vec3d vTotalBoxMax = pArea->m_vBoxMax;

  m_pVisAreaManager->UpdateVisArea((CVisArea*)pVisArea, pPoints, nCount, szName, fHeight, vAmbientColor, bAfectedByOutLights, bSkyOnly, m_pTerrain, vDynAmbientColor, fViewDistRatio, bDoubleSide, bUseDeepness, bUseInIndoors);

  if(pArea->m_lstEntities[STATIC_ENTITIES].Count() || pArea->m_lstEntities[DYNAMIC_ENTITIES].Count())
  {
    // merge old and new bboxes
    vTotalBoxMin.CheckMin(pArea->m_vBoxMin);
    vTotalBoxMax.CheckMax(pArea->m_vBoxMax);
  }
  else
  {
    vTotalBoxMin = pArea->m_vBoxMin;
    vTotalBoxMax = pArea->m_vBoxMax;
  }

//  if(strstr(szName, "VisArea8"))
  //  int y=0;

	m_pObjManager->ReregisterEntitiesInArea(vTotalBoxMin - Vec3d(8,8,8), vTotalBoxMax + Vec3d(8,8,8));

	if(m_pObjManager->m_pCWaterVolumes)
		m_pObjManager->m_pCWaterVolumes->UpdateWaterVolumeVisAreas();
}

int C3DEngine::GetFogVolumeIdFromBBox(const Vec3d & vBoxMin, const Vec3d & vBoxMax)
{
	for(int f=0; f<m_pTerrain->m_lstFogVolumes.Count(); f++)
	{
		const Vec3d & vVolMin = m_pTerrain->m_lstFogVolumes[f].vBoxMin;
		const Vec3d & vVolMax = m_pTerrain->m_lstFogVolumes[f].vBoxMax;

		if(vVolMax.x>vBoxMin.x && vBoxMax.x>vVolMin.x)
		if(vVolMax.y>vBoxMin.y && vBoxMax.y>vVolMin.y)
		if(vVolMax.z>vBoxMin.z && vBoxMax.z>vVolMin.z)
		{
			return m_pTerrain->m_lstFogVolumes[f].nRendererVolumeID;
		}
	}

	return 0;
}

void C3DEngine::ResetParticlesAndDecals( )
{
	if(m_pPartManager)
		m_pPartManager->Reset();

	if(m_pDecalManager)
		m_pDecalManager->Reset();

	if(GetSystem()->GetIAnimationSystem())
		GetSystem()->GetIAnimationSystem()->ClearDecals();
}

IEntityRender * C3DEngine::CreateEntityRender()
{
	CBrush * pBrush = new CBrush();
	m_pObjManager->m_lstBrushContainer.Add(pBrush);
	return pBrush;
}

IEntityRender * C3DEngine::CreateVegetation()
{
  CStatObjInst * pVeget = new CStatObjInst();
  m_pObjManager->m_lstVegetContainer.Add(pVeget);
  return pVeget;
}

void C3DEngine::DeleteEntityRender(IEntityRender * pEntityRender)
{
	UnRegisterEntity(pEntityRender);
	m_pObjManager->m_lstBrushContainer.Delete((CBrush*)pEntityRender);
	m_pObjManager->m_lstVegetContainer.Delete((CStatObjInst*)pEntityRender);
	delete pEntityRender;
}

void C3DEngine::DrawRain()
{
	Vec3d vWindDir = m_vWindForce;
	vWindDir.z = -16.f - GetCVars()->e_rain_amount*16.f;
	if(m_pRainManager && m_pTerrain)
		m_pRainManager->Render(m_pTerrain, m_vFogColor, m_pObjManager, m_pPartManager, vWindDir);
}

void C3DEngine::SetRainAmount( float fAmount )
{
	GetCVars()->e_rain_amount = max(min(1.f,fAmount),0);
}

void C3DEngine::SetWindForce( const Vec3d & vWindForce )
{
	m_vWindForce = vWindForce;
}

float C3DEngine::GetAmbientLightAmountForEntity(IEntityRender * pEntity)
{
  Vec3d vColor(0,0,0);
  if(pEntity && pEntity->GetEntityVisArea())
    vColor = ((CVisArea*)pEntity->GetEntityVisArea())->m_vAmbColor; 
  else if(m_pObjManager)
    vColor = m_pObjManager->m_vOutdoorAmbientColor;

  vColor.x *= m_vWorldColorConst.x;
  vColor.y *= m_vWorldColorConst.y;
  vColor.z *= m_vWorldColorConst.z;

  return (vColor.x + vColor.y + vColor.z)*0.3333f;
}

float C3DEngine::GetLightAmountForEntity(IEntityRender * pEntity, bool bOnlyVisibleLights)
{
	Vec3d vLightAmount(0,0,0);
	uint dwDLightMask = (uint)-1; // check all lights
	CheckDistancesToLightSources(dwDLightMask, pEntity->GetPos(), 1, pEntity, 16, 0, 0, &vLightAmount);
	float fLightAmount = (vLightAmount.x+vLightAmount.y+vLightAmount.z)*0.233f;
	return min(1.f,fLightAmount);
}

IVisArea * C3DEngine::GetVisAreaFromPos(const Vec3d &vPos)
{
	if(m_pObjManager && m_pVisAreaManager)
		return m_pVisAreaManager->GetVisAreaFromPos(vPos);

	return 0;
}

bool C3DEngine::IsVisAreasConnected(IVisArea * pArea1, IVisArea * pArea2, int nMaxRecursion, bool bSkipDisabledPortals)
{
	if (pArea1==pArea2)
		return (true);	// includes the case when both pArea1 & pArea2 are NULL (totally outside)
										// not considered by the other checks
	if (!pArea1 || !pArea2)
		return (false); // avoid a crash - better to put this check only
										// here in one place than in all the places where this function is called

	nMaxRecursion *= 2; // include portals since portals are the areas

	if(m_pObjManager && m_pVisAreaManager)
		return ((CVisArea*)pArea1)->FindVisArea((CVisArea*)pArea2, nMaxRecursion, bSkipDisabledPortals);

	return false;
}

#ifdef GAMECUBE

#define CRY_GCRENDER_API __declspec(export)

#ifdef __cplusplus
extern "C" {
#endif

	typedef void (*voidfunctionptr) (void); // ptr to function returning void
	__declspec(section ".init") extern voidfunctionptr _ctors[];
	__declspec(section ".init") extern voidfunctionptr _dtors[];

	void _prolog(void);
	void _epilog(void);
	void _unresolved(void);

#ifdef __cplusplus
}
#endif


//_prolog is the entrypoint to the module
// Called by the main application after the module is linked
// This executes the static constructors for the module.
// User code starts here.
CRY_GCRENDER_API void  _prolog(void) {
	//  call static initializers
	OSReport("Call Static Initializers\n");
	voidfunctionptr *constructor;
	for (constructor = _ctors; *constructor; constructor++) {
		(*constructor)();
	}
}

// _epilog is called before the the module is unlinked
// This executes the static destructors for the module.
CRY_GCRENDER_API void _epilog(void) {
	voidfunctionptr *destructor;
	for (destructor = _dtors; *destructor; destructor++) {
		(*destructor)();
	}
}

// _unresolved is called if a module attempts
// to call a symbol that is not present in another module.
// If _unresolved is not specified, the branch to the symbol
// will instead branch back to itself.
CRY_GCRENDER_API void _unresolved(void) {
	u32     i;
	u32*    p;
	OSReport("\nError: Unlinked function called in module %s.\n", __FILE__);
	OSReport("Address:      Back Chain    LR Save\n");
	for (i = 0, p = (u32*) OSGetStackPointer(); // get current sp
		p && (u32) p != 0xffffffff && i++ < 16;
		p = (u32*) *p)                         // get caller sp
	{
		OSReport("0x%08x:   0x%08x    0x%08x\n", p, p[0], p[1]);
	}
	OSReport("\n");
}

#endif //GAMECUBE

bool C3DEngine::IsOutdoorVisible()
{
	if(m_pVisAreaManager)
		return m_pVisAreaManager->IsOutdoorAreasVisible();

	return false;
}

void C3DEngine::EnableOceanRendering(bool bOcean, bool bShore)
{
	m_bOcean = bOcean;
  m_bShore = bShore;
}

IMatInfo * C3DEngine::CreateMatInfo()
{
	return m_pMatMan ? m_pMatMan->CreateMatInfo() : 0;
}

void C3DEngine::DeleteMatInfo(IMatInfo * pMatInfo)
{
	m_pMatMan->DeleteMatInfo(pMatInfo);
}

void C3DEngine::RenameMatInfo( IMatInfo *pMtl,const char *sNewName )
{
	m_pMatMan->RenameMatInfo(pMtl,sNewName);
}

IMatInfo* C3DEngine::FindMaterial( const char *sMaterialName )
{
	return m_pMatMan->FindMatInfo( sMaterialName );
}
//////////////////////////////////////////////////////////////////////////


ILMSerializationManager * C3DEngine::CreateLMSerializationManager()
{
	return new CLMSerializationManager2();
}

bool C3DEngine::IsOutdoorsVisible()
{
	return (m_pObjManager && m_pVisAreaManager) ? 
		m_pVisAreaManager->IsOutdoorAreasVisible() : false;
}

bool C3DEngine::IsTerainHightMapModifiedByGame()
{
  return m_pTerrain ? m_pTerrain->m_bHightMapModified : 0;
}

bool C3DEngine::IsPotentiallyVisible(IEntityRender * pEntityRender, float fAdditionalRadius )
{
  if(!pEntityRender->GetEntityRS())
    return false;

  IVisArea * pVisArea = (IVisArea *)pEntityRender->m_pVisArea;

  if(pVisArea)
  { // object is in indoors - test vis of current visarea
    if ((GetFrameID() - pVisArea->GetVisFrameId())<2)
      return true; // visible

    if (fAdditionalRadius > 0)
    { // test neighbours
      IVisArea * Areas[64];
      int nCount = pVisArea->GetVisAreaConnections(Areas,64);
      for (int i=0; i<nCount; i++)
        if((GetFrameID() - Areas[i]->GetVisFrameId())<2)
          return true; // visible
    }

    return false; // no areas visible
  }
  else if(m_pTerrain)
  { // object is in outdoors - test vis of terrain sectors
		if (!GetCVars()->e_terrain)
			return false;
      
    Vec3d vBoxMin,vBoxMax;
    pEntityRender->GetRenderBBox(vBoxMin,vBoxMax);

    // check visibility of outdoor sectors where this box is
    // get 2d bounds in sectors array
    // todo: use ready list of terrain sectors from entity itself
    float fSizePlus = 8.f + fAdditionalRadius;
		float fInvSectorSize = 1.0f / CTerrain::GetSectorSize();
    int min_x = fastftol_positive( (vBoxMin.x - fSizePlus)*fInvSectorSize );
    int min_y = fastftol_positive( (vBoxMin.y - fSizePlus)*fInvSectorSize );
    int max_x = fastftol_positive( (vBoxMax.x + fSizePlus)*fInvSectorSize );
    int max_y = fastftol_positive( (vBoxMax.y + fSizePlus)*fInvSectorSize );

    // can not test outside of the map
    // this test is valid only if entire bbox is in the map
    if(min_x<0) return true;
    if(min_y<0) return true;
    if(max_x>=CTerrain::GetSectorsTableSize()) return true;
    if(max_y>=CTerrain::GetSectorsTableSize()) return true;

//    OcclusionTestClient * pOcclusionTestClient = 0;
		float currTime = GetTimer()->GetCurrTime();

    // if some sector was visible in last 2 sec - box is visible
    for(int x=min_x; x<=max_x; x++)
    for(int y=min_y; y<=max_y; y++)
    {
      if ((currTime - m_pTerrain->m_arrSecInfoTable[x][y]->GetLastTimeRendered()) < LAST_POTENTIALLY_VISIBLE_TIME)
      {
/*        if( pOcclusionTestClient )
        { // test terrain and objects occlusion
          float fDist = GetDistance( (vBoxMin + vBoxMax)*0.5f, GetViewCamera().GetPos());
          if( m_pObjManager->IsBoxOccluded(vBoxMin,vBoxMax,fDist,pOcclusionTestClient) )
            return false;
        }*/

        return true; // sector is visible
      }
    }

    return false; // no sectors was rendered in last seconds
  }
  else
    return false; // no outdoors present
}

IEdgeConnectivityBuilder *C3DEngine::GetNewConnectivityBuilder( void )
{
	assert(m_pConnectivityBuilder);

	m_pConnectivityBuilder->Reinit();

	return(m_pConnectivityBuilder);
}

//! creates a connectivity object that can be used to deserialize the connectivity data
IStencilShadowConnectivity *C3DEngine::NewConnectivity()
{
	return new CStencilShadowConnectivity();
}

IEdgeConnectivityBuilder *C3DEngine::GetNewStaticConnectivityBuilder( void )
{
	assert(m_pStaticConnectivityBuilder);

	m_pStaticConnectivityBuilder->Reinit();

	return(m_pStaticConnectivityBuilder);
}


IEdgeDetector *C3DEngine::GetEdgeDetector( void )
{
	return(m_pShadowEdgeDetector);
}

void C3DEngine::UpdateBeaches()
{
  if(m_pTerrain)
    m_pTerrain->InitBeaches(true);
}

void C3DEngine::RestoreTerrainFromDisk()
{
  if(m_pTerrain && m_pObjManager)
  {
    m_pTerrain->ResetTerrainVertBuffers();
    m_pTerrain->LoadHighMap(GetLevelFilePath(HEIGHT_MAP_FILE_NAME), GetSystem()->GetIPak());
//    m_pTerrain->LoadStatObjInstances();
//    m_pObjManager->CreatePhysicalEntitys();
  }

  ResetParticlesAndDecals( );
}

void C3DEngine::CheckPhysicalized(const Vec3d & vBoxMin, const Vec3d & vBoxMax)
{
  if(!GetCVars()->e_stream_areas)
    return;

  CVisArea * pVisArea = (CVisArea *)GetVisAreaFromPos((vBoxMin+vBoxMax)*0.5f);
  if(pVisArea)
  { // indoor
    pVisArea->CheckPhysicalized();
    IVisArea * arrConnections[16];
    int nConections = pVisArea->GetRealConnections(arrConnections,16);
    for(int i=0; i<nConections; i++)
      ((CVisArea*)arrConnections[i])->CheckPhysicalized();
  }
  else
  { // outdoor
    CSectorInfo * arrSecInfo[] = 
    {
      m_pTerrain->GetSecInfo(int(vBoxMin.x), int(vBoxMin.y)),
      m_pTerrain->GetSecInfo(int(vBoxMax.x), int(vBoxMin.y)),
      m_pTerrain->GetSecInfo(int(vBoxMin.x), int(vBoxMax.y)),
      m_pTerrain->GetSecInfo(int(vBoxMax.x), int(vBoxMax.y))
    };

    for(int i=0; i<4; i++)
      if(arrSecInfo[i])
        arrSecInfo[i]->CheckPhysicalized();
  }
}

void C3DEngine::CheckMemoryHeap()
{
	assert (IsHeapValid());
}

void C3DEngine::RecompileBeaches()
{
	if(m_pTerrain)
		m_pTerrain->InitBeaches(true);
}

float C3DEngine::GetObjectsLODRatio()
{
	return GetCVars()->e_obj_lod_ratio;
}

float C3DEngine::GetObjectsViewDistRatio()
{
	return GetCVars()->e_obj_view_dist_ratio;
}

float C3DEngine::GetObjectsMinViewDist()
{
	return GetCVars()->e_obj_min_view_dist;
}

bool C3DEngine::SetMaterialFloat( char * szMatName, int nSubMatId, int nTexSlot, char * szParamName, float fValue )
{
	IMatInfo * pMatInfo = FindMaterial( szMatName );
	if(!pMatInfo)
	{ Warning(0,0,"I3DEngine::SetMaterialFloat: Material not found: %s", szMatName); return false; }

	if(nSubMatId>0)
	{
		if(nSubMatId<=pMatInfo->GetSubMtlCount())
			pMatInfo = pMatInfo->GetSubMtl(nSubMatId-1);
		else
		{ 
			Warning(0,0,"I3DEngine::SetMaterialFloat: SubMaterial not found: %s, SubMatId: %d", szMatName, nSubMatId); 
			return false; 
		}
	}

	SEfResTexture * pTex = pMatInfo->GetShaderItem().m_pShaderResources->m_Textures[nTexSlot];
	if(!pTex)
	{ Warning(0,0,"I3DEngine::SetMaterialFloat: Texture slot not found: %s, TexSlot: %d", szMatName, nTexSlot); return false; }

	char szM_Name[128] = "";

	if(strncmp(szParamName,"m_", 2))
		strncpy(szM_Name,"m_",sizeof(szM_Name));

	strncat(szM_Name,szParamName,sizeof(szM_Name));

	return pTex->m_TexModificator.SetMember(szM_Name,fValue);
}

void C3DEngine::CloseTerrainTextureFile()
{
	if(m_pTerrain)
		m_pTerrain->CloseTerrainTextureFile();
}

//////////////////////////////////////////////////////////////////////////
IParticleEffect* C3DEngine::CreateParticleEffect()
{
	if (m_pPartManager)
		return m_pPartManager->CreateEffect();
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void C3DEngine::DeleteParticleEffect( IParticleEffect* pEffect )
{
	if (m_pPartManager)
		m_pPartManager->RemoveEffect( pEffect );
}

//////////////////////////////////////////////////////////////////////////
IParticleEffect* C3DEngine::FindParticleEffect( const char *sEffectName )
{
	if (m_pPartManager)
		return m_pPartManager->FindEffect( sEffectName );
	return 0;
}

int C3DEngine::GetLoadedObjectCount()
{
	int nObjectsLoaded = m_pObjManager ? m_pObjManager->GetLoadedObjectCount() : 0;
	
	if(GetSystem()->GetIAnimationSystem())
	{
		ICryCharManager::Statistics AnimStats;
		GetSystem()->GetIAnimationSystem()->GetStatistics(AnimStats);
		nObjectsLoaded += AnimStats.numAnimObjectModels + AnimStats.numCharModels;
	}

	return nObjectsLoaded;
}

void C3DEngine::DeleteEntityDecals(IEntityRender * pEntity)
{
	if(m_pDecalManager && pEntity)
		m_pDecalManager->OnEntityDeleted(pEntity);
}

void C3DEngine::OnLevelLoaded()
{
	if(GetCVars()->e_precache_level)
	{
		UpdateLoadingScreen("\003Precaching level ... ");
/*
		CCamera cam = GetViewCamera();
		cam.SetPos(Vec3d(0,0,16));
		cam.SetAngle(Vec3d(0,0,-90));
		Get3DEngine()->SetCamera(cam);

		GetRenderer()->BeginFrame();*/

		if(m_pObjManager && m_pVisAreaManager)
			m_pVisAreaManager->Preceche(m_pObjManager);

		//GetRenderer()->Update();

		UpdateLoadingScreenPlus("\003done");
	}
/*
	for(int x=0; x<C3DEngine::GetTerrainSize(); x+=GetTerrainSectorSize())
	for(int y=0; y<C3DEngine::GetTerrainSize(); y+=GetTerrainSectorSize())
	{
		int nPosStride=0, nVertCount=0;
		const char * pVerts = (const char *)C3DEngine::GetShoreGeometry(nPosStride, nVertCount, x, y);
		for(int v=0 ;v<nVertCount && pVerts; v++)
		{
			Vec3d vPos = *((Vec3d*)&pVerts[nPosStride*v]);
			assert(pVerts);
		}
	}*/
}

void C3DEngine::DeleteDecalsInRange( Vec3d vBoxMin, Vec3d vBoxMax, bool bDeleteBigTerrainDecals)
{
	if(m_pDecalManager)
		m_pDecalManager->DeleteDecalsInRange(vBoxMin, vBoxMax, bDeleteBigTerrainDecals);
}

const void * C3DEngine::GetShoreGeometry(int & nPosStride, int & nVertCount, int nSectorX, int nSectorY)
{
	if(!this || !m_pTerrain)
		return 0;

	return m_pTerrain->GetShoreGeometry(nPosStride, nVertCount, nSectorX, nSectorY);
}


void C3DEngine::LockCGFResources()
{
	if(m_pObjManager)
		m_pObjManager->m_bLockCGFResources = true;
}

void C3DEngine::UnlockCGFResources()
{
	if(m_pObjManager)
	{
		m_pObjManager->m_bLockCGFResources = false;
		m_pObjManager->FreeNotUsedCGFs();
	}
}

void IEntityRenderState::ShadowMapInfo::Release(enum EERType eEntType, struct IRenderer * pRenderer)
{
	if(pShadowMapCasters)
	{
		pShadowMapCasters->Clear();

		delete pShadowMapCasters;
		pShadowMapCasters = 0;
	}

	if(	pShadowMapFrustumContainer && eEntType != eERType_Vegetation ) // vegetations share same frustum allocated in CStatObj
	{
		delete pShadowMapFrustumContainer->m_LightFrustums.Get(0)->pEntityList;
		pShadowMapFrustumContainer->m_LightFrustums.Get(0)->pEntityList=0;

		delete pShadowMapFrustumContainer->m_LightFrustums.Get(0)->pModelsList;
		pShadowMapFrustumContainer->m_LightFrustums.Get(0)->pModelsList=0;

		delete pShadowMapFrustumContainer;
		pShadowMapFrustumContainer=0;
	}

	if(	pShadowMapFrustumContainerPassiveCasters && eEntType != eERType_Vegetation ) // vegetations share same frustum allocated in CStatObj
	{
		delete pShadowMapFrustumContainerPassiveCasters->m_LightFrustums.Get(0)->pEntityList;
		pShadowMapFrustumContainerPassiveCasters->m_LightFrustums.Get(0)->pEntityList=0;

		delete pShadowMapFrustumContainerPassiveCasters->m_LightFrustums.Get(0)->pModelsList;
		pShadowMapFrustumContainerPassiveCasters->m_LightFrustums.Get(0)->pModelsList=0;

		delete pShadowMapFrustumContainerPassiveCasters;
		pShadowMapFrustumContainerPassiveCasters=0;
	}

	if(pShadowMapLeafBuffersList)
	{
		for(int i=0; i<pShadowMapLeafBuffersList->Count(); i++)
		{
			if(pShadowMapLeafBuffersList->GetAt(i))
			{
				pRenderer->DeleteLeafBuffer(pShadowMapLeafBuffersList->GetAt(i));
				pShadowMapLeafBuffersList->GetAt(i)=0;
			}
		}
		delete pShadowMapLeafBuffersList;
		pShadowMapLeafBuffersList=0;
	}

	delete this;
}
