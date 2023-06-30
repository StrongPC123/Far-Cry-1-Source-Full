////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   3denginerender.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: rendering
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "3dEngine.h"
#include "objman.h"
#include "visareas.h"
#include "terrain_water.h"
#include "partman.h"
#include "DecalManager.h"
#include "bflyes.h"
#include "rain.h"
#include "../RenderDll/Common/RendElements/CREScreenCommon.h"

#include "cbuffer.h"

#include "watervolumes.h"

////////////////////////////////////////////////////////////////////////////////////////
// RenderScene
////////////////////////////////////////////////////////////////////////////////////////

void C3DEngine::Draw()
{
	m_bProfilerEnabled = GetISystem()->GetIProfileSystem()->IsProfiling();

	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	if (!m_bEnabled)
	{
		return;
	}
#if !defined(LINUX)
	if(GetCVars()->e_sleep)
		Sleep(GetCVars()->e_sleep);
#endif
	GetTimer()->MeasureTime("EnterDraw");

	m_nRenderFrameID = GetRenderer()->GetFrameID();

  if(IsCameraAnd3DEngineInvalid(GetViewCamera(), "C3DEngine::Draw"))
    return;

	UpdateScene();

  Get3DEngine()->CheckPhysicalized(GetViewCamera().GetPos()-Vec3d(2,2,2),GetViewCamera().GetPos()+Vec3d(2,2,2));

	if(GetCVars()->e_hires_screenshoot)
	{
		if(GetRenderer()->GetType()==R_GL_RENDERER)
			MakeHiResScreenShot();
		else
		{
			GetLog()->Log("\003e_hires_screenshoot is suported only in OpenGL");
			GetCVars()->e_hires_screenshoot=0;
		}
	}

	m_pObjManager->m_fMaxViewDistanceScale = 1.f;

	if (GetSystem()->IsDedicated())
		RenderScene(0);
	 else
	 {
		// bad solution - only to get CryVision working properly
		if(*((bool*)m_pREScreenProcess->mfGetParameter(SCREENPROCESS_NIGHTVISION, SCREENPROCESS_ACTIVE)))
		{
			static ICVar *pCVVolFog=GetConsole()->GetCVar("r_VolumetricFog");
			int e_shadow_maps=GetCVars()->e_shadow_maps;					GetCVars()->e_shadow_maps=0;
      int e_stencil_shadows=GetCVars()->e_stencil_shadows;	GetCVars()->e_stencil_shadows=0;
			int r_VolumetricFog=pCVVolFog->GetIVal();							pCVVolFog->Set(0);

			RenderScene(-1);

			GetCVars()->e_shadow_maps=e_shadow_maps;
			GetCVars()->e_stencil_shadows=e_stencil_shadows;
			pCVVolFog->Set(r_VolumetricFog);
		}
		else
		{
			RenderScene(-1);
		}
	 }



	CaptureFrameBufferToFile();

  GetRenderer()->EF_ClearLightsList();

	// fog addons
	//GetRenderer()->ResetToDefault();
/*	m_pTerrain->m_pWater->DrawUnderWaterFog(
    m_arrUnderWaterPlaneColor[0], 
    m_arrUnderWaterPlaneColor[1], 
    m_arrUnderWaterPlaneColor[2], 
    m_arrUnderWaterPlaneColor[3]);*/

	// streaming state
	if(GetCVars()->e_terrain_debug)
  if (CStatObj::m_fStreamingTimePerFrame>0 ||
    (IsOutdoorVisible() && m_pTerrain && m_pTerrain->IsStreamingNow() && GetCVars()->e_terrain && !GetCVars()->e_hires_screenshoot))
	{
		//GetRenderer()->ResetToDefault();
    GetRenderer()->Draw2dImage(800-8,0,8,8,m_nStreamingIconTexID);
	}
  
	// black quad for simple shadow volumes
//  if(GetCVars()->e_stencil_shadows)
  //{
//    GetRenderer()->ResetToDefault();
  //  GetRenderer()->DrawTransparentQuad2D(0.5);
//  }

	// c-buffer debug info
  if(m_pObjManager && m_pObjManager->m_pCoverageBuffer && GetCVars()->e_cbuffer>=2)
  {
    m_pObjManager->m_pCoverageBuffer->DrawDebug(GetCVars()->e_cbuffer-1);
    GetTimer()->MeasureTime("3CBDebug");
  }

  GetTimer()->MeasureTime("3DrawEnd");  
/*
#ifdef _DEBUG
#ifdef WIN32
	if(GetCVars()->e_portals>1)
	{
		Sleep(20);
		GetTimer()->MeasureTime("3Sleep(20)");  
	}
#endif
#endif
*/
  // Unload old stuff
  if(GetCVars()->e_stream_areas)
  {
    if(m_pTerrain)
      m_pTerrain->CheckUnload();
    if(m_pVisAreaManager)
      m_pVisAreaManager->CheckUnload();
  }

	if(m_pObjManager && GetCVars()->e_stream_cgf)
		m_pObjManager->CheckUnload();
/*
	CSectorInfo * pSectorInfo = m_pTerrain ? m_pTerrain->GetSecInfo(GetViewCamera().GetPos()) : 0;
	uchar * pImage = new uchar[256*256*4];
	if(pSectorInfo)
		MakeSectorLightMap(pSectorInfo->m_nOriginX, pSectorInfo->m_nOriginY, pImage, 256);
	delete [] pImage;*/
}

void C3DEngine::RenderScene(unsigned int dwDrawFlags)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );
  if (!m_pTerrain)
    return;

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4311 )
#endif
  
	// it's correct only before StartEf()
  int nRecursionLevel = (int)GetRenderer()->EF_Query(EFQ_RecurseLevel);
	assert(nRecursionLevel>=0);
	m_pObjManager->m_nRenderStackLevel = m_pTerrain->m_nRenderStackLevel = nRecursionLevel;
	if(m_pObjManager->m_nRenderStackLevel<0 || m_pObjManager->m_nRenderStackLevel>1)
		return;

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

	GetRenderer()->EF_StartEf();  
  GetTimer()->MeasureTime("3StartEf");

////////////////////////////////////////////////////////////////////////////////////////
// Add lsources to the renderer and register into sectors
////////////////////////////////////////////////////////////////////////////////////////

  //if(dwDrawFlags & DLD_ADD_LIGHTSOURCES)
  { // Add lsources to the renderer and register into sectors
    GetRenderer()->EF_ClearLightsList();
		UpdateLightSources();
		PrepareLightSourcesForRendering();    
		GetTimer()->MeasureTime("3LSources");
	}

	// prepare coverage buffer
  m_pObjManager->m_pCoverageBuffer->BeginFrame(GetViewCamera());

////////////////////////////////////////////////////////////////////////////////////////
// Add render elements for indoor
////////////////////////////////////////////////////////////////////////////////////////

  CCamera InitialCam = GetViewCamera();
//	m_lstPortalCamera.Clear();
//	m_bCameraInsideBuilding = false;
  
  m_pDecalManager->SubmitLightHolesToRenderer();		  

////////////////////////////////////////////////////////////////////////////////////////
// Add render elements for outdoor
////////////////////////////////////////////////////////////////////////////////////////

	{
		for(int i=0; i<MAX_LIGHTS_NUM; i++)
			m_pObjManager->m_lstLightEntities[i].Clear();
	}

//	for(int i=0; i<MAX_LIGHTS_NUM; i++)
	//	m_pObjManager->m_lstShadowEntities[i].Clear();

	CStatObj::m_fStreamingTimePerFrame-=CGF_STREAMING_MAX_TIME_PER_FRAME;
  if(CStatObj::m_fStreamingTimePerFrame<0)
    CStatObj::m_fStreamingTimePerFrame=0;
  if(CStatObj::m_fStreamingTimePerFrame>0.25f)
    CStatObj::m_fStreamingTimePerFrame=0.25f;

  assert(Cry3DEngineBase::m_nRenderStackLevel>=0 && Cry3DEngineBase::m_nRenderStackLevel<=1);
  m_pObjManager->m_dwRecursionDrawFlags[Cry3DEngineBase::m_nRenderStackLevel] = dwDrawFlags;

	m_pVisAreaManager->DrawOcclusionAreasIntoCBuffer(m_pObjManager->m_pCoverageBuffer);
	GetTimer()->MeasureTime("3OcclAreas");

	// draw entities inside vis areas
	m_pVisAreaManager->Render(m_pObjManager);
	GetTimer()->MeasureTime("3VisAreas");

	if(m_pObjManager)
		m_pObjManager->PreloadNearObjects();

	if(GetCVars()->e_stream_preload_textures && Cry3DEngineBase::m_nRenderStackLevel==0)
	{
		m_fPreloadStartTime = GetCurTimeSec();
		bool bPreloadOutdoor = m_pVisAreaManager->PreloadResources();
		GetTimer()->MeasureTime("3TexPrelInd");

//    for(int p=0; p<16; p++)
		if(bPreloadOutdoor && m_pTerrain->PreloadResources() && m_pSHSky)
		{
			FRAME_PROFILER( "Renderer::EF_PrecacheResource", GetSystem(), PROFILE_RENDERER );
			GetRenderer()->EF_PrecacheResource(m_pSHSky, 0, 1.f, 0);
		}
		GetTimer()->MeasureTime("3TexPrelOut");
	}

/*
  // precache tarrain data if camera was teleported more than 32 meters
  if(m_pTerrain->m_nRenderStackLevel==0)
  {
    if(GetDistance(m_pTerrain->m_vPrevCameraPos, GetViewCamera().GetPos()) > 32)
      m_pTerrain->PreCacheArea(GetViewCamera().GetPos(), GetViewCamera().GetZMax()*1.5f);
    m_pTerrain->m_vPrevCameraPos = GetViewCamera().GetPos();
  }
  */

	// clear sprites list
	if(m_nRenderStackLevel>=0 && m_nRenderStackLevel<=1)
	{
		list2<CStatObjInst*> * pList = &m_pObjManager->m_lstFarObjects[m_nRenderStackLevel];
		pList->Clear();
	}
	else
		assert(0);

  CCamera prevCam = GetViewCamera();
  if (IsOutdoorVisible())  	
	{	
		if (m_pVisAreaManager->m_lstOutdoorPortalCameras.Count()	&& 
			(m_pVisAreaManager->m_pCurArea || m_pVisAreaManager->m_pCurPortal))
      GetViewCamera() = m_pVisAreaManager->m_lstOutdoorPortalCameras[0];

		if(m_pVisAreaManager->IsOutdoorAreasVisible())
		{
			if(GetCVars()->e_out_space)
				RenderOutSpace();
			else
				RenderSkyBox(m_pSHSky);
		}

		// terrain and tree sprites
    m_pTerrain->m_bCameraInsideBuilding = (m_pVisAreaManager->m_pCurArea || m_pVisAreaManager->m_pCurPortal);

		if(m_pVisAreaManager->IsOutdoorAreasVisible())
			m_pTerrain->RenderTerrain(m_pObjManager, dwDrawFlags);
		GetTimer()->MeasureTime("3Terrain");

//		m_pObjManager->m_lstStatShadowsBuffer.Clear();
		m_pObjManager->m_fWindForce = m_vWindForce.Length()/53.33f;
//		if(m_pVisAreaManager->IsOutdoorAreasVisible())
	//    m_pTerrain->RenderStaticObjects(m_pObjManager);      
		//GetTimer()->MeasureTime("3statobj");

		// draw terrain water and beach
		if(	m_pVisAreaManager->IsOutdoorAreasVisible() && 
			(	dwDrawFlags & DLD_TERRAIN_WATER) && 
				m_pTerrain->m_fDistanceToSectorWithWater>=0 && 
				m_pTerrain->GetWaterLevel() && m_pTerrain->m_bOceanIsVisibe && 
				m_bOcean)
		{
			Vec3d vBoxMin(-2048.f,-2048.f,-2048.f);
			Vec3d vBoxMax(GetTerrainSize()+2048.f,GetTerrainSize()+2048.f,m_pTerrain->GetWaterLevel());
			if(GetViewCamera().IsAABBVisible_exact(AABB(vBoxMin,vBoxMax)))
				m_pTerrain->RenderTerrainWater(m_bShore);
		}

    if(m_pTerrain && m_pTerrain->m_pWater)
      m_pTerrain->m_pWater->SetLastFov(GetViewCamera().GetFov());

		GetTimer()->MeasureTime("3Wtr&Bch");

//		if(m_pObjManager->m_pCWaterVolumes && m_pVisAreaManager->IsOutdoorAreasVisible())
//			m_pObjManager->m_pCWaterVolumes->RenderWaterVolumes();
//		GetTimer()->MeasureTime("3WaterVol");

		// draw sprites at far distance
    if(dwDrawFlags & DLD_FAR_SPRITES)
			m_pObjManager->RenderFarObjects();
		GetTimer()->MeasureTime("3Far");

		// render entities not registered in sectors
		m_pTerrain->RenderEntitiesOutOfTheMap(m_pObjManager);
		GetTimer()->MeasureTime("3EntOut");

	  // render terrain ground
		if(dwDrawFlags & DLD_TERRAIN_FULLRES)// && !m_pTerrain->m_nRenderStackLevel)
			m_pTerrain->DrawVisibleSectors();
		else
	    m_pTerrain->RenderReflectedTerrain();
		GetTimer()->MeasureTime("3TerVisSec");

		// unload far and old sectors
		if(!m_pTerrain->m_nRenderStackLevel)
			m_pTerrain->UnloadOldSectors(m_fMaxViewDist);
		GetTimer()->MeasureTime("3UnlOldSecs");

		if((dwDrawFlags & DLD_TERRAIN_LIGHT) && m_pTerrain && 
			m_pVisAreaManager->IsOutdoorAreasVisible())
		{ // render terrain light passes
			FRAME_PROFILER( "TerrainLightPasses", GetSystem(), PROFILE_RENDERER );
			for (int nLightId=0; nLightId<MAX_LIGHTS_NUM &&	nLightId<m_nRealLightsNum/*m_lstDynLights.Count()*/; nLightId++)    
			{
				if(m_lstDynLights[nLightId].m_Id>=0	&& !(m_lstDynLights[nLightId].m_Flags & DLF_DIRECTIONAL))
				{
					if(m_lstDynLights[nLightId].m_Flags & DLF_CASTSHADOW_VOLUME)
						m_pObjManager->m_lstLightEntities[nLightId].Add((IEntityRender*)-1);
					else if(m_pVisAreaManager->IsOutdoorAreasVisible())
          {
            if( (!(m_lstDynLights[nLightId].m_Flags & DLF_DIRECTIONAL)) &&
              IsSphereVisibleOnTheScreen(m_lstDynLights[nLightId].m_Origin, m_lstDynLights[nLightId].m_fRadius, 0))
              m_pTerrain->RenderDLightOnHeightMap(&m_lstDynLights[nLightId]);
          }
				}
			}
			GetTimer()->MeasureTime("3TerLSrc");
		}
  }
  else if(m_pVisAreaManager->IsSkyVisible())
    RenderSkyBox(m_pSHSky);

  GetViewCamera() = prevCam;

	// water volumes
	if(m_pObjManager->m_pCWaterVolumes && !m_pObjManager->m_nRenderStackLevel)
		m_pObjManager->m_pCWaterVolumes->RenderWaterVolumes(m_pVisAreaManager->IsOutdoorAreasVisible());
	GetTimer()->MeasureTime("3WaterVol");

	// render/generate shadow maps from entities
	m_pObjManager->RenderEntitiesShadowMapsOnTerrain(false, m_pREShadowMapGenerator);
	GetTimer()->MeasureTime("3EntSMaps");

  if(GetCVars()->e_stencil_shadows)
  {
		m_pObjManager->DrawEntitiesLightPass();
		GetTimer()->MeasureTime("3EntLPass");
  }

	// render terrain detail textures
	if(m_pVisAreaManager->IsOutdoorAreasVisible())
		if(dwDrawFlags & DLD_DETAIL_TEXTURES)
			m_pTerrain->RenderDetailTextures(m_fFogNearDist,m_fFogFarDist);
	GetTimer()->MeasureTime("3TerDetTex");

  GetViewCamera() = InitialCam;
     
  m_pDecalManager->Render();
	GetTimer()->MeasureTime("3Decals");		

  if(dwDrawFlags & DLD_PARTICLES)
	{
	  RenderTerrainParticles();
    m_pPartManager->Render(m_pObjManager, m_pTerrain);    
		GetTimer()->MeasureTime("3Part3d");
	}

	if (m_pRenderCallbackFunc && GetCVars()->e_entities && !m_pObjManager->m_nRenderStackLevel)
	if(m_pObjManager && m_pVisAreaManager->IsOutdoorAreasVisible())
  { 
		FRAME_PROFILER( "RenderScene::m_pRenderCallbackFunc", GetSystem(), PROFILE_EDITOR );
    m_pRenderCallbackFunc(m_pRenderCallbackParams);
		GetTimer()->MeasureTime("3RendCB");
  }

  RenderVolumeFogTopPlane();
	GetTimer()->MeasureTime("3UW&FP");    

	if(m_pSHFullScreenQuad && !m_pObjManager->m_nRenderStackLevel)
	{
		DrawFullScreenQuad(m_pSHFullScreenQuad, false, eS_HeatVision);		
		GetTimer()->MeasureTime("32dquad");
	}

	if(GetCVars()->e_rain_amount>1)
		GetCVars()->e_rain_amount=1;
	else if(GetCVars()->e_rain_amount<0)
		GetCVars()->e_rain_amount=0;

	if(GetCVars()->e_rain_amount && (m_pVisAreaManager->IsOutdoorAreasVisible()))
	{
		bool bCameraInOutdoors = !m_pVisAreaManager->m_pCurArea && !(m_pVisAreaManager->m_pCurPortal && m_pVisAreaManager->m_pCurPortal->m_lstConnections.Count()>1);
		if(bCameraInOutdoors)
			DrawFullScreenQuad(m_pSHRainMap, false);
	}

//-----------------------------------------------------------------------

  ProcessScreenEffects();

	SetupDistanceFog();

	if(!m_pTerrain->m_nRenderStackLevel)
		SetupClearColor();

	IRenderer * ppp = GetRenderer();
	{
		FRAME_PROFILER( "Renderer::EF_EndEf3D", GetSystem(), PROFILE_RENDERER );

		if(!m_nRenderStackLevel)
		{
			if(GetCVars()->e_widescreen)
			{ // set view port to 16/9 mode
				int nVPSizeX = GetRenderer()->GetWidth();
				int nVPSizeY = GetRenderer()->GetWidth()*9/16;
				int nOffsetY = (GetRenderer()->GetHeight()-nVPSizeY)/2;
				int nOffsetX = 0;
				if(nOffsetY<0)
				{
					nOffsetY = 0;
					nVPSizeX = GetRenderer()->GetHeight()*16/9;
					nVPSizeY = GetRenderer()->GetHeight();
					nOffsetX = (GetRenderer()->GetWidth()-nVPSizeX)/2;
				}

				// draw2 black textures
				GetRenderer()->SetViewport(0,0,GetRenderer()->GetWidth(),GetRenderer()->GetHeight());
				GetRenderer()->SetState(GS_NODEPTHTEST);

				float fRatioX = 800.f/GetRenderer()->GetWidth();
				float fRatioY = 600.f/GetRenderer()->GetHeight();
				if(nOffsetY>0)
				{
					GetRenderer()->Draw2dImage(0,0,nVPSizeX*fRatioX,(600-(nVPSizeY*fRatioY))/2,m_nBlackTexID);
					GetRenderer()->Draw2dImage(0,(nOffsetY+nVPSizeY)*fRatioY,nVPSizeX*fRatioX,(600-(nVPSizeY*fRatioY))/2,m_nBlackTexID);
				}
				else if(nOffsetX>0)
				{
					GetRenderer()->Draw2dImage(0,0,(800-(nVPSizeX*fRatioX))/2,nVPSizeY*fRatioY,m_nBlackTexID);
					GetRenderer()->Draw2dImage((nOffsetX+nVPSizeX)*fRatioX,0,(800-(nVPSizeX*fRatioX))/2,nVPSizeY*fRatioY,m_nBlackTexID);
				}

				// set viewport
				GetRenderer()->SetViewport(nOffsetX,nOffsetY,nVPSizeX,nVPSizeY);
			}
			else
				GetRenderer()->SetViewport(0,0,GetRenderer()->GetWidth(),GetRenderer()->GetHeight());
		}

		GetRenderer()->EF_EndEf3D(SHDF_ALLOWHDR | SHDF_SORT);
	}
	GetRenderer()->EnableFog(false);
	//GetRenderer()->EF_ClearLightsList();
	if(!m_pObjManager->m_nRenderStackLevel)
		GetTimer()->MeasureTime("3EndEf");
  /*
  // example for wemade
  // setup vertices
  SColorVert arrVerts[4];
  Vec3d vPos(142,46,20);
  Vec3d right(1,0,0);
  Vec3d up(0,1,0);
  UCol ucResCol;
  ucResCol.dcolor= (DWORD)-1;
  arrVerts[0].vert = (-right-up) + vPos;
  arrVerts[0].dTC[0] = 1; 
  arrVerts[0].dTC[1] = 0;
  arrVerts[0].color = ucResCol;
  arrVerts[1].vert = ( right-up) + vPos;
  arrVerts[1].dTC[0] = 1; 
  arrVerts[1].dTC[1] = 1;
  arrVerts[1].color = ucResCol;
  arrVerts[2].vert = ( right+up) + vPos;
  arrVerts[2].dTC[0] = 0; 
  arrVerts[2].dTC[1] = 1;
  arrVerts[2].color = ucResCol;
  arrVerts[3].vert = (-right+up) + vPos;
  arrVerts[3].dTC[0] = 0; 
  arrVerts[3].dTC[1] = 0;
  arrVerts[3].color = ucResCol;

  // make indices
  byte arrIndices[6] = {0,1,2,0,2,3};

  // get shader
  IShader * pShader = GetRenderer()->EF_LoadShader("ParticleLight", eSH_World);
  int nTexId = GetRenderer()->LoadTexture("detail");

  // now render
  // make object
  CCObject * pOb = GetRenderer()->EF_GetObject(true);		
  pOb->m_Trans(0,0,0);
  pOb->m_Angs(0,0,0);				
  pOb->m_AmbColor = Col_White;
  pOb->m_RenderState = GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
  pOb->m_NumCM = nTexId;

  GetRenderer()->EF_StartEf();  
  GetRenderer()->EF_AddSpriteToScene(pShader->GetID(), 4, arrVerts, pOb, arrIndices, 6);
  GetRenderer()->EF_EndEf3D(true);
              
    */
/*  {
    // i use static in this example only allow to have initialization and rendering in one place
    static CLeafBuffer * pLeafBuffer = 0;
    if(!pLeafBuffer)
    {
      const int nChunksNum = 16;
      struct_VERTEX_FORMAT_P3F tmp;
      list2<struct_VERTEX_FORMAT_P3F> lstVertices; 
      list2<ushort> lstIndices;
      list2<ushort> lstChunkFirstIdxId;

      // contruct some geometry
      for(float r1=0, r2=1.f; r2<=nChunksNum; r1+=1.f, r2+=1.f)
      {
        lstChunkFirstIdxId.Add(lstIndices.Count());

        for(int i=0; i<=360; i+=30)
        {
          float rad = (i) * (gf_PI/180);

          tmp.x = Fsin(rad)*(r2);
          tmp.y = Fcos(rad)*(r2);
          tmp.z = 0;

          lstIndices.Add(lstVertices.Count());
          lstVertices.Add(tmp);

          tmp.x = Fsin(rad)*(r1);
          tmp.y = Fcos(rad)*(r1);
          tmp.z = 0;

          lstIndices.Add(lstVertices.Count());
          lstVertices.Add(tmp);
        }
      }

      lstChunkFirstIdxId.Add(lstIndices.Count());

      // make leaf buffer
      pLeafBuffer = GetRenderer()->CreateLeafBufferInitialized(
        lstVertices.GetElements(), lstVertices.Count(), VERTEX_FORMAT_P3F, 
        lstIndices.GetElements(), lstIndices.Count(), R_PRIMV_MULTI_STRIPS,
        "VolFogTopCircle", eBT_Dynamic,nChunksNum, 0x1000);

      // make shader
      IShader * pShader = GetRenderer()->EF_LoadShader("WaterVolume",eSH_World);

      // define chunks
      for(int i=0; i<nChunksNum; i++)
      {
        pLeafBuffer->SetChunk(pShader,
          lstIndices[lstChunkFirstIdxId[i]],lstVertices.Count()/(lstChunkFirstIdxId.Count()-1),
          lstChunkFirstIdxId[i],lstChunkFirstIdxId[i+1]-lstChunkFirstIdxId[i], i);
      }
    }

    // render
    CCObject * pObject = GetRenderer()->EF_GetObject(true);
    Vec3d vPos(142,46,20);
    pObject->m_Trans = vPos;
    pObject->m_Scale = Vec3d(0.1f,0.1f,0.1f);

    GetRenderer()->EF_StartEf();
    pLeafBuffer->AddRenderElements(pObject);
    GetRenderer()->EF_EndEf3D(true);
  }  */

  /*
list2<struct_VERTEX_FORMAT_P3F_COL4UB> lstVert;
list2<unsigned short> lstIdx;
static CLeafBuffer * pLeafBuffer = NULL;
static IShader* pTempShader = NULL;

int offset = 2;
struct_VERTEX_FORMAT_P3F_COL4UB center;
center.x = 142;
center.y = 46;
center.z = 20;
center.r = 255;
center.g = 255;
center.b = 255;
center.a = 255;
struct_VERTEX_FORMAT_P3F_COL4UB temp[3];

temp[0] = center;
temp[0].x -= offset;
temp[0].z -= offset;

temp[1] = center;
temp[1].z += offset;

temp[2] = center;
temp[2].x += offset;
temp[2].z -= offset;

lstVert.Add( temp[0]);
lstVert.Add( temp[1]);
lstVert.Add( temp[2]);

lstIdx.Add(0);
lstIdx.Add(1);
lstIdx.Add(2);

static int minit=1;
if(minit)
{
  minit=0;
  pTempShader= GetSystem()->GetIRenderer()->EF_LoadShader("WaterVolume", eSH_World, 0);

  pLeafBuffer = GetSystem()->GetIRenderer()->CreateLeafBufferInitialized(
    &temp[0], lstVert.Count(), VERTEX_FORMAT_P3F_COL4UB,
    lstIdx.GetElements(), lstIdx.Count(),
    R_PRIMV_TRIANGLES,"TerrainSector");

  pLeafBuffer->SetChunk(pTempShader,0,3,0,3);
  pLeafBuffer->SetShader(pTempShader);
}

//In Rendering part
{

  //In this position, want to change some vertex pos.

    //and want to render. Below is Possible?

  CCObject * pObject = GetRenderer()->EF_GetObject(true);
//  Vec3d vPos(142,46,20);
  //pObject->m_Trans = vPos;
  //pObject->m_Scale = Vec3d(0.1f,0.1f,0.1f);
//  pObject->m_NumCM = 4096;

//  pLeafBuffer->UpdateVertices()

  GetSystem()->GetIRenderer()->EF_StartEf();
  pLeafBuffer->AddRenderElements();
  GetSystem()->GetIRenderer()->EF_EndEf3D(true);
}
*/

	m_pObjManager->m_nRenderStackLevel = m_pTerrain->m_nRenderStackLevel = 0;

  // Light info is not valid after C3Dengine::Render()
//  m_lstDynLights.Clear();
}





void C3DEngine::RenderSkyBox(IShader *pSH)
{    
	// skybox
  if (pSH && m_pRESky && GetCVars()->e_sky_box)
  {
    CCObject * pObj = GetRenderer()->EF_GetObject(true, -1);
		pObj->m_Matrix.SetTranslationMat(GetViewCamera().GetPos());
		pObj->m_Matrix = Matrix33::CreateRotationZ(gf_DEGTORAD*m_fSkyBoxAngle)*pObj->m_Matrix;
    pObj->m_ObjFlags |= FOB_TRANS_TRANSLATE | FOB_TRANS_ROTATE;
		if(!m_pObjManager->m_nRenderStackLevel)
		{
			pObj->m_nScissorX1 = GetViewCamera().m_ScissorInfo.x1;
			pObj->m_nScissorY1 = GetViewCamera().m_ScissorInfo.y1;
			pObj->m_nScissorX2 = GetViewCamera().m_ScissorInfo.x2;
			pObj->m_nScissorY2 = GetViewCamera().m_ScissorInfo.y2;

			if(m_pVisAreaManager->m_lstIndoorActiveOcclVolumes.Count())
			{
				Matrix44 matInv = pObj->m_Matrix;
				matInv.Invert44();
				Vec3d vPos;

				for(int i=0; i<MAX_SKY_OCCLAREAS_NUM; i++)
				{
					if(i<m_pVisAreaManager->m_lstIndoorActiveOcclVolumes.Count())
					{
						vPos = matInv.TransformPointOLD(m_pVisAreaManager->m_lstIndoorActiveOcclVolumes[i]->m_arrvActiveVerts[0]);
						m_pRESky->m_arrvPortalVerts[i][0].xyz = vPos;
						m_pRESky->m_arrvPortalVerts[i][0].color.bcolor[3] = 255;

						vPos = matInv.TransformPointOLD(m_pVisAreaManager->m_lstIndoorActiveOcclVolumes[i]->m_arrvActiveVerts[1]);
						m_pRESky->m_arrvPortalVerts[i][1].xyz = vPos;
						m_pRESky->m_arrvPortalVerts[i][1].color.bcolor[3] = 255;

						vPos = matInv.TransformPointOLD(m_pVisAreaManager->m_lstIndoorActiveOcclVolumes[i]->m_arrvActiveVerts[3]);
						m_pRESky->m_arrvPortalVerts[i][2].xyz = vPos;
						m_pRESky->m_arrvPortalVerts[i][2].color.bcolor[3] = 255;

						vPos = matInv.TransformPointOLD(m_pVisAreaManager->m_lstIndoorActiveOcclVolumes[i]->m_arrvActiveVerts[2]);
						m_pRESky->m_arrvPortalVerts[i][3].xyz = vPos;
						m_pRESky->m_arrvPortalVerts[i][3].color.bcolor[3] = 255;
					}
					else
						memset(m_pRESky->m_arrvPortalVerts[i],0,sizeof(m_pRESky->m_arrvPortalVerts[i]));
				}
			}
			else
				memset(m_pRESky->m_arrvPortalVerts,0,sizeof(m_pRESky->m_arrvPortalVerts));
		}
		else
			memset(m_pRESky->m_arrvPortalVerts,0,sizeof(m_pRESky->m_arrvPortalVerts));

    m_pRESky->m_fTerrainWaterLevel = max(0,m_pTerrain->GetWaterLevel());
		m_pRESky->m_fSkyBoxStretching = m_fSkyBoxStretching;
       
    GetRenderer()->EF_AddEf(0, m_pRESky, pSH, NULL, pObj, 0, NULL,EFSLIST_PREPROCESS);
  }

	// sun lensflares
  if(m_pSHLensFlares && GetCVars()->e_sun==1 )//&& 
//		GetViewCamera().GetPos().z > GetWaterLevel(&GetViewCamera().GetPos()))
  { // no sun flare under water because of conflict with vol fog
    if (m_pSHLensFlares->GetREs()->Num())
    {
      //get the object
      if (!m_SunObject[m_pTerrain->m_nRenderStackLevel])
        m_SunObject[m_pTerrain->m_nRenderStackLevel] = GetRenderer()->EF_GetObject(false, -1);
      else
        GetRenderer()->EF_GetObject(false, m_SunObject[m_pTerrain->m_nRenderStackLevel]->m_Id);	

      Vec3d vTrans = GetSunPosition(false);
			vTrans.z *= m_fSunHeightScale;
      Vec3d vCamDir = -GetViewCamera().GetVCMatrixD3D9().GetOrtZ();
      if(vTrans.Dot(vCamDir)<0)
        return; // sun is behind the camera

      float fDist = vTrans.GetDistance(GetViewCamera().GetPos());
      if(fDist > GetViewCamera().GetZMax()) // sun is behind far cliping plane - move it closer
      {
        float t = GetViewCamera().GetZMax() / fDist;
        vTrans = GetViewCamera().GetPos() + 0.9f*t*(vTrans-GetViewCamera().GetPos());
      }

      m_SunObject[m_pTerrain->m_nRenderStackLevel]->m_Matrix = GetTranslationMat(vTrans);
      m_SunObject[m_pTerrain->m_nRenderStackLevel]->m_ObjFlags |= FOB_DRSUN | FOB_TRANS_TRANSLATE;
      m_SunObject[m_pTerrain->m_nRenderStackLevel]->m_TempVars[2] = 1.0f;
      float fWaterLevel = GetWaterLevel();
      float fCamZ = GetViewCamera().GetPos().z;
      if((fCamZ-fWaterLevel)*(vTrans.z-fWaterLevel)>0)
        m_SunObject[m_pTerrain->m_nRenderStackLevel]->m_SortId = -1000000;
      else
        m_SunObject[m_pTerrain->m_nRenderStackLevel]->m_SortId = 1000000;
      GetRenderer()->EF_AddEf(0, m_pSHLensFlares->GetREs()->Get(0), m_pSHLensFlares, NULL, m_SunObject[m_pTerrain->m_nRenderStackLevel], -1, NULL, EFSLIST_DISTSORT);
    }
  }
}

void C3DEngine::DrawText(float x, float y, const char * format, ...)
{
	char buffer[512];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	ICryFont *pCryFont = GetSystem()->GetICryFont();
	if (!pCryFont)
		return;

	IFFont *pFont = pCryFont->GetFont("console");
	if (!pFont)
		return;

	pFont->UseRealPixels(false);
	pFont->SetSameSize(true);
	pFont->SetCharWidthScale(1);
	pFont->SetSize(vector2f(16, 16));
	pFont->SetColor(color4f(1,1,1,1));
	pFont->SetCharWidthScale(.65f);
	pFont->DrawString( x-pFont->GetCharWidth() * strlen(buffer) * pFont->GetCharWidthScale(), y, buffer );
}

void C3DEngine::DisplayInfo(float & fTextPosX, float & fTextPosY, float & fTextStepY)
{  
  //GetRenderer()->ResetToDefault();
  //GetRenderer()->EnableDepthTest(false);
  GetRenderer()->SetState(GS_NODEPTHTEST);

  static float fCurrentFPS = 0;
	static float fFrameRateSecMin=GetTimer()->GetFrameRate();
	static float fFrameRateSecMax=GetTimer()->GetFrameRate();


	float fFrameRate = 0;
	float fFrameTimeInSec=GetTimer()->GetFrameTime();
	
	if(fFrameTimeInSec>0.0f)									// to prevent divide by zero
		fFrameRate = 1.0f / fFrameTimeInSec;

	if(fFrameRate>999.0f)											// to ensure number is <=999 (for nice printout)
		fFrameRate=999.0f;

  { // current fps
    static float START_TIME = GetCurAsyncTimeSec()*1000;
    static int iFrameRateCount=0;
		static float fFrameRateSum=0;
		static float fFrameRateMin=0;
		static float fFrameRateMax=0;

		float time = GetCurAsyncTimeSec()*1000;

		iFrameRateCount++;
		
		fFrameRateSum += fFrameRate;

		if (fFrameRateMin > fFrameRate) fFrameRateMin = fFrameRate;
		if (fFrameRateMax < fFrameRate) fFrameRateMax = fFrameRate;

   
    float diff = (time-START_TIME);

    if(diff>500) // every second
    {
			if(iFrameRateCount)
				fCurrentFPS = (float)(fFrameRateSum/(float)iFrameRateCount);
			 else 
				 fCurrentFPS=0.0f;

			fFrameRateSecMin = fFrameRateMin;
			fFrameRateSecMax = fFrameRateMax;

			fFrameRateMin = fFrameRate;
			fFrameRateMax = fFrameRate;
			fFrameRateSum = 0;
			iFrameRateCount = 0;
      START_TIME = time;
    }
		else
		if(diff<0)		// timer was reseted
		{
			START_TIME = GetCurAsyncTimeSec()*1000;
			iFrameRateCount=0;
			fFrameRateSum=0;
			fFrameRateMin=0;
			fFrameRateMax=0;
		}
	}

  int nPolygons,nShadowVolPolys;

	GetRenderer()->GetPolyCount(nPolygons,nShadowVolPolys);

  // Cam pos
  Vec3d vPos = GetViewCamera().GetPos();

	// make level name
	char szLevelName[16];

	*szLevelName=0;
	{
		int i;
		for(i=strlen(m_szLevelFolder)-2; i>0; i--)
			if(m_szLevelFolder[i] == '\\' || m_szLevelFolder[i] == '/')
				break;

		if(i>=0)
		{
			strncpy(szLevelName,&m_szLevelFolder[i+1],sizeof(szLevelName));
			szLevelName[sizeof(szLevelName)-1] = 0;

			for(int i=strlen(szLevelName)-1; i>0; i--)
				if(szLevelName[i] == '\\' || szLevelName[i] == '/')
					szLevelName[i]=0;
		}
	}

	fTextPosX = 800-5;
	fTextPosY = -10;
	fTextStepY = 16;

  DrawText( fTextPosX, fTextPosY+=fTextStepY,
		"CamPos=%3d %3d %3d Angl=%3d %2d %3d", 
    (int)vPos.x,(int)vPos.y,(int)vPos.z,
    (int)GetViewCamera().GetAngles().x,
    (int)GetViewCamera().GetAngles().y,
    (int)GetViewCamera().GetAngles().z);

	// get verson
	const SFileVersion & ver = GetSystem()->GetFileVersion();
	char sVersion[128];
	ver.ToString(sVersion);

	DrawText( fTextPosX, fTextPosY+=fTextStepY, "Ver=%s Lev=%s", sVersion,szLevelName);

  // Polys in scene
	if (nShadowVolPolys>0)
		DrawText(fTextPosX, fTextPosY+=fTextStepY,"Polygons %d(SV=%d)", nPolygons,nShadowVolPolys);
	else
		DrawText(fTextPosX, fTextPosY+=fTextStepY,"Polygons %2d,%003d", nPolygons/1000, nPolygons-nPolygons/1000*1000);

  { // Polys per sec
    int per_sec = (int)(nPolygons*fCurrentFPS);
    int m =  per_sec    /1000000;
    int t = (per_sec - m*1000000)/1000;
    int n =  per_sec - m*1000000 - t*1000;      
    DrawText(fTextPosX, fTextPosY+=fTextStepY,"P/Sec %003d,%003d,%003d", m,t,n);
  }

  if(GetCVars()->e_stream_cgf && m_pObjManager)
  {
    int nReady=0, nTotalInStreaming=0, nTotal=0;
    m_pObjManager->GetObjectsStreamingStatus(nReady,nTotalInStreaming,nTotal);
    DrawText(fTextPosX, fTextPosY+=fTextStepY,"Objects streaming: %d/%d/%d", nReady, nTotalInStreaming, nTotal);
  }

  if(GetCVars()->e_stream_areas && m_pTerrain)
  {
    int nReady=0, nTotal=0;

    m_pTerrain->GetStreamingStatus(nReady,nTotal);
    DrawText(fTextPosX, fTextPosY+=fTextStepY,"Terrain streaming: %d/%d", nReady, nTotal);

    m_pVisAreaManager->GetStreamingStatus(nReady,nTotal);
    DrawText(fTextPosX, fTextPosY+=fTextStepY,"Indoor streaming: %d/%d", nReady, nTotal);
  }

  // fps
	const int nStartFrame = -50;
  static int nFrames = nStartFrame;
  static float fStartTime = GetTimer()->gfGetTime();

	float fCurrentTime = GetTimer()->gfGetTime();

	if(fCurrentTime<fStartTime)		// timer was reseted
		fStartTime=fCurrentTime;
  
  if(GetCVars()->e_timedemo_frames && nFrames>=0)
  { // print current and average fps
    if(fStartTime != GetTimer()->gfGetTime())
      DrawText( fTextPosX, fTextPosY+=fTextStepY, "FPS %3d (%3d..%3d) / %3.2f / %3.2f", 
        (int)fCurrentFPS, (int)fFrameRateSecMin, (int)fFrameRateSecMax, fFrameRate,
        float(nFrames)/(GetTimer()->gfGetTime()-fStartTime) );
  }
  else
  { // print current fps
    DrawText( fTextPosX, fTextPosY+=fTextStepY, "FPS %3d (%3d..%3d) / %5.1f",
			(int)fCurrentFPS, (int)fFrameRateSecMin, (int)fFrameRateSecMax, fFrameRate );
  }

  // exit when x frames rendered
  if(nFrames > GetCVars()->e_timedemo_frames && GetCVars()->e_timedemo_frames)
  {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Average FPS = %3.2f", float(nFrames)/(GetTimer()->gfGetTime()-fStartTime));

#ifdef GAMECUBE
		OSReport("- Timedemo result -\n");
#elif (defined (PS2) || defined (_XBOX) || defined(LINUX))
	  snprintf(buffer, sizeof(buffer),"- Timedemo result -");
	  OutputDebugString(buffer);
#else      
    if(GetCVars()->e_timedemo_frames>10)
      MessageBox(0, buffer, "- Timedemo result -", MB_OK);
#endif      

		GetLog()->Log ("\001Timedemo quit");
		GetConsole()->Exit("Timedemo quit 1");
  }

	// Exit unconditionally when x seconds of game passed
	// THis is used for automatic multipass profiling with VTune 6.x
	if (GetCVars()->e_timedemo_milliseconds)
	{
		if (int((GetTimer()->gfGetTime() - fStartTime)*1000) > GetCVars()->e_timedemo_milliseconds)
		{
			GetLog()->Log ("\001Timedemo quit in %dms (started at %g sec, stopped at %g sec) Average %3.2f FPS", GetCVars()->e_timedemo_milliseconds, fStartTime, GetTimer()->gfGetTime(), float(nFrames - nStartFrame)/(GetTimer()->gfGetTime()-fStartTime));
			GetSystem()->Quit();
		}
	}

  nFrames++;

/*  if(GetCVars()->e_video_buffer_stats)
  {
    char * szRendererStatus = GetRenderer()->GetStatusText(eRS_VidBuffer);
    GetRenderer()->TextToScreen( fTextPosX-44, fTextPosY+=fTextStepY, szRendererStatus );
  }*/

  if(GetCVars()->e_particles_debug)
  {
    int nSprites = 0, nFree = 0, nEmitters = 0;
    int nParts = m_pPartManager->Count(&nSprites, &nFree, &nEmitters);
    DrawText( fTextPosX, fTextPosY+=fTextStepY, 
      "Parts=%4d/%4d  Emit=%d  Sprites=%4d", nParts, nFree, nEmitters, nSprites );
  }

	DrawText( fTextPosX, fTextPosY+=fTextStepY, "ViewDist=%d/%.1f", (int)m_fMaxViewDist, m_pObjManager ? m_pObjManager->m_fMaxViewDistanceScale : 0);

  {
    int nActive=0;
    char sLightsList[512]="";
    for(int i=0; i<m_lstDynLights.Count(); i++)
    {
      if(m_lstDynLights[i].m_Id>=0 && m_lstDynLights[i].m_fRadius >= 0.5f )
				if(!(m_lstDynLights[i].m_Flags & DLF_FAKE))
      {
        nActive++;

				if(i<4)
				{
					strcat(sLightsList, m_lstDynLights[i].m_sDebugName);        
					if(i<m_lstDynLights.Count()-1)
						strcat(sLightsList,",");
				}
      }
    }

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

    DrawText( fTextPosX, fTextPosY+=fTextStepY, "DynLights=%s(%d/%d/%d)", sLightsList, nActive, m_nRealLightsNum, m_lstDynLights.Count() );
  }

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

  if(GetCVars()->e_terrain_debug>1)
    DrawText( fTextPosX, fTextPosY+=fTextStepY, m_pTerrain->m_pTexturePool->GetStatusText(m_pTerrain));

  // Render path info
  char * pRenderPath;
  static ICVar *p_r_SM30PATH = GetConsole()->GetCVar("r_SM30PATH");
  static ICVar *p_r_SM2XPATH = GetConsole()->GetCVar("r_SM2XPATH");
  static ICVar *p_r_Quality_BumpMapping = GetConsole()->GetCVar("r_Quality_BumpMapping");
  if(p_r_SM30PATH->GetIVal())
    pRenderPath = "SM3.0 Path";
  else
  if(p_r_SM2XPATH->GetIVal())
    pRenderPath = "SM2.X path";
  else
  if(p_r_Quality_BumpMapping->GetIVal() == 3)
    pRenderPath = "SM2.0 Path";
  else
  if(p_r_Quality_BumpMapping->GetIVal() > 0)
    pRenderPath = "SM1.1 Path";
  else 
    pRenderPath = "FP Path";

  DrawText( fTextPosX, fTextPosY+=fTextStepY, "Render path = %s", pRenderPath );
}

#define DLD_SET_CVAR(VarName,FlagName)\
int VarName = GetCVars()->VarName;\
if(!(DrawFlags & FlagName))\
	GetCVars()->VarName = 0;\

#define DLD_RESTORE_CVAR(VarName) \
GetCVars()->VarName = VarName; \

void C3DEngine::DrawLowDetail(const int & DrawFlags)
{
	if(m_pObjManager)
		m_pObjManager->m_fMaxViewDistanceScale = 1.f;
	m_nRenderFrameID = GetRenderer()->GetFrameID();

	DLD_SET_CVAR(e_shadow_maps,DLD_SHADOW_MAPS);
	DLD_SET_CVAR(e_stencil_shadows,DLD_STENCIL_SHADOWS);
	DLD_SET_CVAR(e_brushes,DLD_STATIC_OBJECTS);
	DLD_SET_CVAR(e_vegetation,DLD_STATIC_OBJECTS);
	DLD_SET_CVAR(e_entities,DLD_ENTITIES);
	DLD_SET_CVAR(e_terrain,DLD_TERRAIN);
	DLD_SET_CVAR(e_player,DLD_FIRST_PERSON_CAMERA_OWNER);

	RenderScene(DrawFlags);

	DLD_RESTORE_CVAR(e_shadow_maps);
	DLD_RESTORE_CVAR(e_stencil_shadows);
	DLD_RESTORE_CVAR(e_brushes);
	DLD_RESTORE_CVAR(e_vegetation);
	DLD_RESTORE_CVAR(e_entities);
	DLD_RESTORE_CVAR(e_terrain);
	DLD_RESTORE_CVAR(e_player);
}

void C3DEngine::DrawTerrainDetailTextureLayers()
{
  GetRenderer()->EnableFog(false);
  
//  float fRainFactor = 1.f;// - 0.75f*GetCVars()->e_rain_amount;
  //float fFogDistance = (m_fFogNearDist*fRainFactor + m_fFogFarDist *fRainFactor)*0.5f;

//  m_pTerrain->DrawDetailTextures(m_fFogNearDist,m_fFogFarDist,true);
  SetupDistanceFog();

//  m_pObjManager->DrawShadows(0); // draw tree shadows
	m_pObjManager->DrawEntitiesShadowSpotsOnTerrain();
}

void C3DEngine::DrawFarTrees()
{
  m_pObjManager->DrawFarObjects(m_fMaxViewDist);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Particles, decals above the water
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void C3DEngine::DrawTerrainParticles(IShader * pShader)
{
  if(pShader == m_pSHTerrainParticles)
  {
    m_pDecalManager->DrawBigDecalsOnTerrain();    
/*    if(!(m_pVisAreaManager->m_pCurArea || m_pVisAreaManager->m_pCurPortal)
			&& IsOutdoorVisible() && m_pVisAreaManager->IsOutdoorAreasVisible())
      m_pBFManager->Render(m_pTerrain);*/
  }
//  else
  //  assert(0);
}

void C3DEngine::RenderTerrainParticles()
{
  if (m_pRETerrainParticles && GetCVars()->e_decals)
  {
    CCObject * pObj = GetRenderer()->EF_GetObject(true, -1);
    pObj->m_Matrix.SetIdentity();
    GetRenderer()->EF_AddEf(0, m_pRETerrainParticles, m_pSHTerrainParticles, NULL, pObj, 0, NULL, EFSLIST_STENCIL | eS_TerrainDetailTextures);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void C3DEngine::SetupDistanceFog()
{
  if(!m_pTerrain)
    return;

	VolumeInfo * pFogVolume = NULL;
	if(GetCVars()->e_use_global_fog_in_fog_volumes && !m_nRenderStackLevel)
	{ // find camera fog volume
		CVisArea * pCurVisArea = m_pVisAreaManager->m_pCurArea ? m_pVisAreaManager->m_pCurArea : m_pVisAreaManager->m_pCurPortal;
		if(pCurVisArea)
		{ // find indoor fog volume with this id
			/*for(int f=0; f<m_pTerrain->m_lstFogVolumes.Count(); f++)
				if(m_pTerrain->m_lstFogVolumes[f].bOcean)
				if(m_pTerrain->m_lstFogVolumes[f].nRendererVolumeID == pCurVisArea->m_nFogVolumeId)
					break;

			if(f<m_pTerrain->m_lstFogVolumes.Count() && m_pTerrain->m_lstFogVolumes[f].vBoxMax.z>GetViewCamera().GetPos().z)
				pFogVolume = &m_pTerrain->m_lstFogVolumes[f];*/
		}
		else
		{ // find outdoor fog volume
			int f;
			for(f=0; f<m_pTerrain->m_lstFogVolumes.Count(); f++)
				if(m_pTerrain->m_lstFogVolumes[f].bOcean)
				if(m_pTerrain->m_lstFogVolumes[f].InsideBBox(GetViewCamera().GetPos()))
					break;

			if(f<m_pTerrain->m_lstFogVolumes.Count())
				pFogVolume = &m_pTerrain->m_lstFogVolumes[f];
		}
	}

  GetRenderer()->SetFog(0, 
		pFogVolume ? pFogVolume->fMaxViewDist*0.01f : m_fFogNearDist, 
		pFogVolume ? pFogVolume->fMaxViewDist : m_fFogFarDist ,  
		GetRenderer()->EF_GetHeatVision() ? Vec3d(0,0,0) : 
		pFogVolume ? pFogVolume->vColor : m_vFogColor, 
		R_FOGMODE_LINEAR);
  GetRenderer()->EnableFog(GetCVars()->e_fog>0);
}

void C3DEngine::RenderOutSpace()
{    
/*  if (m_pREOutSpace && GetCVars()->e_out_space)
  {
    CCObject * pObj = GetRenderer()->EF_GetObject(true, -1);
    pObj->m_Matrix.SetTranslationMat(GetViewCamera().GetPos());   
    GetRenderer()->EF_AddEf(0, m_pREOutSpace, m_pSHOutSpace, NULL, pObj, 0, NULL);
  }*/
}

void C3DEngine::DrawFullScreenQuad(IShader *pShader, bool bRectangle, EF_Sort eSort)
{
  if(m_pRE2DQuad)
  {
    CCObject * pObj = GetRenderer()->EF_GetObject(true, -1);
    pObj->m_Matrix.SetIdentity();
		pObj->m_Color = m_vFogColor;
		pObj->m_Color.a = 1.f;
    pObj->m_nTemplId = bRectangle;
    GetRenderer()->EF_AddEf(0, m_pRE2DQuad, pShader, NULL, pObj, 0, NULL, eSort);
  }
}
/*
void C3DEngine::RenderTerrainShadowMaps()
{
  if(!GetCVars()->e_shadow_maps)
    return;

  CCObject * pRendObject = GetRenderer()->EF_GetObject(true, -1);
  pRendObject->m_Trans.Clear();
  pRendObject->m_Angs.Clear();       
///  m_pRETerrainShadowMap->m_pShadowFrustum = 0;
//  m_pRETerrainShadowMap->m_fAlpha = -1;
//  GetRenderer()->EF_AddEf(0, m_pRETerrainShadowMap, "TerrainShadowMaps", pRendObject, 0, NULL, eS_TerrainDetailTextures);
}	*/

// render fog plane circle if camera is inside volume
void C3DEngine::RenderVolumeFogTopPlane()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );
	if(m_pObjManager->m_nRenderStackLevel)
		return;

  Vec3d vCamPos = GetViewCamera().GetPos();

  CSectorInfo * pSectorInfo = m_pTerrain ? m_pTerrain->GetSectorFromPoint((int)vCamPos.x,(int)vCamPos.y) : 0;
  VolumeInfo * pFogVolume = 0;
  if(!pSectorInfo || !pSectorInfo->m_pFogVolume || vCamPos.z>pSectorInfo->m_pFogVolume->vBoxMax.z)
  {
  }
  else
    pFogVolume = pSectorInfo->m_pFogVolume;

  CVisArea * pCurVisArea = m_pVisAreaManager->m_pCurArea ? m_pVisAreaManager->m_pCurArea : m_pVisAreaManager->m_pCurPortal;

  if(pCurVisArea && pCurVisArea->m_nFogVolumeId)
  {
		int f;
    for(f=0; f<m_pTerrain->m_lstFogVolumes.Count(); f++)
      if(m_pTerrain->m_lstFogVolumes[f].nRendererVolumeID == pCurVisArea->m_nFogVolumeId)
        break;

    if(f<m_pTerrain->m_lstFogVolumes.Count())
      pFogVolume = &m_pTerrain->m_lstFogVolumes[f];
  }

  if(!pFogVolume)
    return;

  if( vCamPos.z<GetWaterLevel() && pSectorInfo && pFogVolume->bOcean )
    return;

	// do not render fog top plane if it is higher than current visarea
	if( pCurVisArea && pFogVolume->vBoxMax.z>pCurVisArea->m_vBoxMax.z )
		return;

	const int nChunksNum = 16;

	if(!m_pFogTopPlane)
	{
		struct_VERTEX_FORMAT_P3F tmp;
		list2<struct_VERTEX_FORMAT_P3F> lstVertices; 
		list2<ushort> lstIndices;
		list2<ushort> lstFirstIdxId;
 
		for(float r1=0, r2=1.f; r2<=nChunksNum; r1+=1.f, r2+=1.f)
		{
			lstFirstIdxId.Add(lstIndices.Count());

			for(int i=0; i<=360; i+=30)
			{
				float rad = (i) * (gf_PI/180);

				tmp.xyz.x = cry_sinf(rad)*(r2);
				tmp.xyz.y = cry_cosf(rad)*(r2);
				tmp.xyz.z = 0;

				lstIndices.Add(lstVertices.Count());
				lstVertices.Add(tmp);

				tmp.xyz.x = cry_sinf(rad)*(r1);
				tmp.xyz.y = cry_cosf(rad)*(r1);
				tmp.xyz.z = 0;

				lstIndices.Add(lstVertices.Count());
				lstVertices.Add(tmp);
			}
		}
	
		lstFirstIdxId.Add(lstIndices.Count());

		m_pFogTopPlane = GetRenderer()->CreateLeafBufferInitialized(
			lstVertices.GetElements(), lstVertices.Count(), VERTEX_FORMAT_P3F, 
			lstIndices.GetElements(), lstIndices.Count(), R_PRIMV_MULTI_STRIPS,
			"VolFogTopCircle", eBT_Dynamic,nChunksNum, 0x1000);

		for(int i=0; i<nChunksNum; i++)
		{
			m_pFogTopPlane->SetChunk(pFogVolume->pShader,
				lstIndices[lstFirstIdxId[i]],lstVertices.Count()/(lstFirstIdxId.Count()-1),
				lstFirstIdxId[i],lstFirstIdxId[i+1]-lstFirstIdxId[i], i);
		}
	}

	CCObject * pObj = GetRenderer()->EF_GetObject(true);	
	pObj->m_Matrix.SetTranslationMat(Vec3d(vCamPos.x, vCamPos.y, pFogVolume->vBoxMax.z+0.0125f));

  float fSize = (pFogVolume->vBoxMax-pFogVolume->vBoxMin).Length();
	float s= (1.f/nChunksNum*fSize)*2;
	pObj->m_Matrix = Matrix33::CreateScale(Vec3d(s,s,s))*pObj->m_Matrix;
  pObj->m_ObjFlags |= FOB_TRANS_TRANSLATE | FOB_TRANS_SCALE;

	if(pFogVolume->nRendererVolumeID)
		m_pFogTopPlane->AddRenderElements(pObj, 0, -1, pFogVolume->nRendererVolumeID);
}

void C3DEngine::MakeHiResScreenShot()
{
#if !defined(LINUX)
	static int nFileId = 0;
	static int nHiResShootCounter = -1;
	static list2<unsigned char*> lstSubImages;

	if(nHiResShootCounter<0)
	{ // start loop
		lstSubImages.Clear();
		nHiResShootCounter = GetCVars()->e_hires_screenshoot*GetCVars()->e_hires_screenshoot;
		GetConsole()->SetScrollMax(0);
		GetTimer()->Enable(false);

		// find free group id
		while(1)
		{
			char sFileName[256];
			snprintf(sFileName, sizeof(sFileName), "HiResScreenShoots\\%dx%d_%d.tga", 
				nFileId, 
				GetRenderer()->GetWidth()*GetCVars()->e_hires_screenshoot, 
				GetRenderer()->GetHeight()*GetCVars()->e_hires_screenshoot);

			FILE * fp = GetSystem()->GetIPak()->FOpen(sFileName,"rb");
			if (!fp)
				break; // file doesn't exist
			GetSystem()->GetIPak()->FClose(fp);
			nFileId++;
		}
	}
	else
	{
		//IVO: I have a special solution for HiResScreenShoots on GameCube!
		#ifndef GAMECUBE  
			CreateDirectory("HiResScreenShoots",0);
			lstSubImages.Add(0);
			lstSubImages.Last() = new unsigned char [GetRenderer()->GetWidth()*GetRenderer()->GetHeight()*4];
			GetRenderer()->ReadFrameBuffer(lstSubImages.Last(), 
			GetRenderer()->GetWidth(), GetRenderer()->GetHeight(), false, true);
		#endif
	
	}

	nHiResShootCounter--;

	if(nHiResShootCounter<0)
	{ // end loop
		// restore initial state
		GetRenderer()->SetViewport(0,0,GetRenderer()->GetWidth(),GetRenderer()->GetHeight());
		GetConsole()->SetScrollMax(300);
		GetTimer()->Enable(true);

		char sFileName[256];
		snprintf(sFileName, sizeof(sFileName), "HiResScreenShoots\\%dx%d_%d.tga", 
			nFileId, 
			GetRenderer()->GetWidth()*GetCVars()->e_hires_screenshoot, 
			GetRenderer()->GetHeight()*GetCVars()->e_hires_screenshoot);
		GetLog()->UpdateLoadingScreen("Writing %s ...", sFileName);

		int nFinSize = lstSubImages.Count()*
			GetRenderer()->GetWidth()*GetRenderer()->GetHeight()*4;

		unsigned char * pFinalImage = NULL;

#ifdef WIN32
		pFinalImage = (unsigned char * )VirtualAlloc(NULL,nFinSize,MEM_COMMIT,PAGE_READWRITE);
#endif // WIN32

		if(pFinalImage)
		{
			// Save final image
			for(int nImageId=0; nImageId<lstSubImages.Count(); nImageId++)
			{
				int pos_X = (lstSubImages.Count()-nImageId-1)%GetCVars()->e_hires_screenshoot*GetRenderer()->GetWidth();
				int pos_Y = (lstSubImages.Count()-nImageId-1)/GetCVars()->e_hires_screenshoot*GetRenderer()->GetHeight();

				for(int x=0; x<GetRenderer()->GetWidth(); x++)
				for(int y=0; y<GetRenderer()->GetHeight(); y++)
				{
					int nFin = ((pos_X+x) + (pos_Y+y)*GetRenderer()->GetWidth()*GetCVars()->e_hires_screenshoot);
					int nSub = ((			 x) + (			 y)*GetRenderer()->GetWidth());
		
					for(int c=0; c<4; c++)
						pFinalImage[nFin*4+c] = lstSubImages[nImageId][nSub*4+c];
				}

				delete [] lstSubImages[nImageId];
				lstSubImages[nImageId]=0;
			}

			GetRenderer()->WriteTGA(pFinalImage,
				GetRenderer()->GetWidth()*GetCVars()->e_hires_screenshoot, 
				GetRenderer()->GetHeight()*GetCVars()->e_hires_screenshoot, 
				sFileName, 32);

#ifdef WIN32
			VirtualFree( pFinalImage,0,MEM_RELEASE );
#endif // WIN32
			
			GetLog()->LogPlus(" done");
		}
		else
			GetLog()->Log("Error making screenshot: memory allocation error (%d MB)", nFinSize/1024/1024);

		GetCVars()->e_hires_screenshoot = 0;
	}
	else
	{
		GetRenderer()->SetViewport( 
			-nHiResShootCounter%GetCVars()->e_hires_screenshoot*GetRenderer()->GetWidth(),
			-nHiResShootCounter/GetCVars()->e_hires_screenshoot*GetRenderer()->GetHeight(),
			GetRenderer()->GetWidth()*GetCVars()->e_hires_screenshoot,
			GetRenderer()->GetHeight()*GetCVars()->e_hires_screenshoot);
	}
#endif
}

void C3DEngine::CaptureFrameBufferToFile()
{
#if !defined(LINUX)
	if(GetCVars()->e_capture_frames>0)
	{
		char * pFolderName = GetCVars()->e_capture_folder->GetString();
		CreateDirectory(pFolderName,0);

		char * pFileFormat = GetCVars()->e_capture_file_format->GetString();
		strlwr(pFileFormat);
		
		// get image
		unsigned char * pImage = new unsigned char [GetRenderer()->GetWidth()*GetRenderer()->GetHeight()*4];
		GetRenderer()->ReadFrameBuffer(pImage, GetRenderer()->GetWidth(), GetRenderer()->GetHeight(), true, true);


		{ // flip up side down
			unsigned char * pImageFlip = new unsigned char [GetRenderer()->GetWidth()*GetRenderer()->GetHeight()*4];
			int nSizeY = GetRenderer()->GetHeight();
			int nSizeX = GetRenderer()->GetWidth();
			for (int i=0; i<nSizeY; i++)
			{
				for (int j=0; j<nSizeX; j++)
				{
					pImageFlip[(i*nSizeX+j)*4+0] = pImage[((nSizeY-1-i)*nSizeX+j)*4+0];
					pImageFlip[(i*nSizeX+j)*4+1] = pImage[((nSizeY-1-i)*nSizeX+j)*4+1];
					pImageFlip[(i*nSizeX+j)*4+2] = pImage[((nSizeY-1-i)*nSizeX+j)*4+2];
					pImageFlip[(i*nSizeX+j)*4+3] = pImage[((nSizeY-1-i)*nSizeX+j)*4+3];
				}
			}
			delete [] pImage;
			pImage = pImageFlip;
		}

		// save to file
		char sFileName[256];
		if(!strcmp(pFileFormat,"jpg"))
		{
			snprintf(sFileName, sizeof(sFileName), "%s\\Frame%06d.jpg", pFolderName, GetCVars()->e_capture_frames-1);	
			GetLog()->UpdateLoadingScreen("Writing %s ...", sFileName);
			GetRenderer()->WriteJPG(pImage,GetRenderer()->GetWidth(), GetRenderer()->GetHeight(), sFileName);
		}
		else if(!strcmp(pFileFormat,"tga"))
		{
			snprintf(sFileName, sizeof(sFileName), "%s\\Frame%06d.tga", pFolderName, GetCVars()->e_capture_frames-1);	
			GetLog()->UpdateLoadingScreen("Writing %s ...", sFileName);
			GetRenderer()->WriteTGA(pImage,GetRenderer()->GetWidth(), GetRenderer()->GetHeight(), sFileName, 32);
		}
		else
			Warning(0,0,"C3DEngine::CaptureFrameBuffer: Unsupported image file format specified: %s", pFileFormat);

		delete [] pImage;

		GetCVars()->e_capture_frames++;
	}
#endif
}

void C3DEngine::DrawShadowSpotOnTerrain(Vec3d vPos, float fRadius)
{
	// calc area
	int x1=int(vPos.x-fRadius*0.75)/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize();
	int y1=int(vPos.y-fRadius*0.75)/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize();
	int x2=int(vPos.x+CTerrain::GetHeightMapUnitSize()+fRadius*0.75)/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize();
	int y2=int(vPos.y+CTerrain::GetHeightMapUnitSize()+fRadius*0.75)/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize();

	// limits
	if(x1<0) x1=0;
	if(y1<0) y1=0;
	if(x2>=CTerrain::GetTerrainSize()) x2=CTerrain::GetTerrainSize();
	if(y2>=CTerrain::GetTerrainSize()) y2=CTerrain::GetTerrainSize();

	// fill buffer and draw
	list2<struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F> verts; 
	struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F tmp;
	tmp.color.dcolor = -1;
	
	float fAlpha = (1.f-(
		m_pObjManager->m_vOutdoorAmbientColor.x+
		m_pObjManager->m_vOutdoorAmbientColor.y+
		m_pObjManager->m_vOutdoorAmbientColor.z)*0.3333f*0.5f);

	float fHeight = vPos.z - GetTerrainElevation(vPos.x,vPos.y) - fRadius;

	if(fHeight>=fRadius)
		return;

	if(fHeight<0)
		fHeight=0;

	fHeight = 1.f-fHeight/(fRadius);

	tmp.color.bcolor[3] = uchar(min(255.f,fAlpha*fHeight*255.f));

	GetRenderer()->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);
	GetRenderer()->SetColorOp(eCO_MODULATE, eCO_MODULATE, eCA_Texture|(eCA_Constant<<3), eCA_Texture|(eCA_Constant<<3));
	GetRenderer()->SetCullMode(R_CULL_DISABLE);
	GetRenderer()->SetTexture(m_nShadowSpotTexId);
	ITexPic * pTexPic = GetRenderer()->EF_GetTextureByID(m_nShadowSpotTexId);
	if(pTexPic)
		pTexPic->SetFilter(FILTER_LINEAR);
	GetRenderer()->SetTexClampMode(true);

	for(int x=x1; x<x2; x+=CTerrain::GetHeightMapUnitSize())
	{
		verts.Clear();
		for(int y=y1; y<=y2; y+=CTerrain::GetHeightMapUnitSize())
		{
			tmp.xyz.x = (float)x;
			tmp.xyz.y = (float)y;
			tmp.xyz.z = Get3DEngine()->GetTerrainZ(x,y)+0.07f;
			tmp.st[0] = (((float)x)-vPos.x)/fRadius+0.5f;
			tmp.st[1] = 1.f-((((float)y)-vPos.y)/fRadius+0.5f);
			verts.Add(tmp);

			tmp.xyz.x = (float)(x+CTerrain::GetHeightMapUnitSize());
			tmp.xyz.y = (float)y;
			tmp.xyz.z = Get3DEngine()->GetTerrainZ((x+CTerrain::GetHeightMapUnitSize()),y)+0.07f;
			tmp.st[0] = (((float)(x+CTerrain::GetHeightMapUnitSize()))-vPos.x)/fRadius+0.5f;
			tmp.st[1] = 1.f-((((float)y)-vPos.y)/fRadius+0.5f);
			verts.Add(tmp);
		}

		if(verts.Count())
			GetRenderer()->DrawTriStrip(&(CVertexBuffer (&verts[0].xyz.x,VERTEX_FORMAT_P3F_COL4UB_TEX2F)),verts.Count());
	}
}

void C3DEngine::SetupClearColor()
{
	bool bCameraInOutdoors = !m_pVisAreaManager->m_pCurArea && !(m_pVisAreaManager->m_pCurPortal && m_pVisAreaManager->m_pCurPortal->m_lstConnections.Count()>1);
	GetRenderer()->SetClearColor(bCameraInOutdoors ? m_vFogColor : Vec3d(0,0,0));

	if(bCameraInOutdoors)
	if(GetViewCamera().GetPos().z<GetWaterLevel() && m_pTerrain)
	{
		CSectorInfo * pSectorInfo = m_pTerrain ? m_pTerrain->GetSectorFromPoint((int)GetViewCamera().GetPos().x,(int)GetViewCamera().GetPos().y) : 0;
		if(!pSectorInfo || !pSectorInfo->m_pFogVolume || GetViewCamera().GetPos().z>pSectorInfo->m_pFogVolume->vBoxMax.z)
		{
			if(GetViewCamera().GetPos().z<GetWaterLevel() && m_pTerrain && 
				m_pTerrain->m_lstFogVolumes.Count() &&
				m_pTerrain->m_lstFogVolumes[0].bOcean)
				GetRenderer()->SetClearColor( m_pTerrain->m_lstFogVolumes[0].vColor );
		}
		//else if( pSectorInfo->m_pFogVolume->bOcean ) // makes problems if there is no skybox
			//GetRenderer()->SetClearColor( pSectorInfo->m_pFogVolume->vColor );
	}
}
