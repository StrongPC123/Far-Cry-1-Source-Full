////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjconstr.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: loading
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "MeshIdx.h"

void CStatObj::Refresh(int nFlags)
{
	if(nFlags & FRO_GEOMETRY)
	{
		bool bSpritesWasCreated = IsSpritesCreated();
		ShutDown();
		Init();
		bool bRes = Load(m_szFileName, m_szGeomName[0] ? m_szGeomName : 0, m_eVertsSharing, 
			m_bLoadAdditinalInfo, m_bKeepInLocalSpace, false);

		LoadLowLODs(m_eVertsSharing, m_bLoadAdditinalInfo, m_bKeepInLocalSpace, false);

		if(bRes && bSpritesWasCreated)
		{
			Vec3d vColor = Get3DEngine()->GetAmbientColorFromPosition(Vec3d(-1000,-1000,-1000));
			UpdateCustomLightingSpritesAndShadowMaps( vColor, m_nSpriteTexRes, m_fBackSideLevel, m_bCalcLighting );
		}

		if(!bRes)
		{ // load default in case of error
			ShutDown();
			Init();
			Load("Objects\\default.cgf", 0, m_eVertsSharing, m_bLoadAdditinalInfo, m_bKeepInLocalSpace);
			m_bDefaultObject = true;
		}

		return;
	}

	if (nFlags & (FRO_TEXTURES | FRO_SHADERS))
	{
		CLeafBuffer *lb = m_pLeafBuffer;

		for (int i=0; i<lb->m_pMats->Count(); i++)
		{
			IShader *e = (*lb->m_pMats)[i].shaderItem.m_pShader;
			if (e && (*lb->m_pMats)[i].pRE && (*lb->m_pMats)[i].nNumIndices)
				e->Reload(nFlags);
		}
	}
}

bool CStatObj::Load(const char * szFileName, const char * szGeomName, 
										EVertsSharing eVertsSharing,
										bool bLoadAdditinalInfo,
										bool bKeepInLocalSpace,
										bool bUseStreaming,
										bool bMakePhysics)
{ 
	// Tick progress bar if we are now loading a level.
	GetConsole()->TickProgressBar();

	if(!szFileName[0])
		GetSystem()->Error("CStatObj::Load: szFileName not specified");

	if(!GetCVars()->e_ccgf_load)
		return LoadUncompiled(szFileName, szGeomName, eVertsSharing, bLoadAdditinalInfo, bKeepInLocalSpace, false, bMakePhysics);

	// remember file loading parameters
	m_eVertsSharing      = eVertsSharing;
	m_bLoadAdditinalInfo = bLoadAdditinalInfo;
	m_bKeepInLocalSpace  = bKeepInLocalSpace;
	m_bMakePhysics       = bMakePhysics;

	// remember names
	strcpy(m_szFileName,szFileName);
	strcpy(m_szGeomName, szGeomName && szGeomName[0] ? szGeomName : "");
	strcpy(m_szFolderName,szFileName);
	while(m_szFolderName[0])
	{ // make folder name
		if(m_szFolderName[strlen(m_szFolderName)-1] == '\\' || m_szFolderName[strlen(m_szFolderName)-1] == '/')
		{ m_szFolderName[strlen(m_szFolderName)-1]=0; break; }
		m_szFolderName[strlen(m_szFolderName)-1]=0;
	}  

	// Create compiled file if needed
	// This checks will slowdown loading but allows to avoid complex processing of errors during streaming
	if(!CompileInNeeded())
		return false; // unable to build file

	// do not stream subobjects
	if(bUseStreaming && !m_szGeomName[0])
	{ 
		Init();

		// define fake bbox
		m_vBoxMin = Vec3d(-1.f,-1.f,-1.f);
		m_vBoxMax = Vec3d( 1.f, 1.f, 1.f);
		m_vBoxCenter = Vec3d(0,0,0);
		m_fRadiusHors = m_fRadiusVert = 1.f;

		m_nLoadedTrisCount = 0;
		m_bUseStreaming = true;
		return true;
	}

	// load compiled file
	StreamCCGF(true);
	m_bUseStreaming = false;
	return m_eCCGFStreamingStatus == ecss_Ready;
}

bool CStatObj::LoadUncompiled(const char * szFileName, 
											 const char * szGeomName, 
											 EVertsSharing eVertsSharing,
											 bool bLoadAdditinalInfo,
											 bool bKeepInLocalSpace,
											 bool bLoadLater,
											 bool bMakePhysics)
{ 
	if(!szFileName[0]) 
	GetSystem()->Error("CStatObj::LoadUncompiled: szFileName not specified");

	m_eVertsSharing      = eVertsSharing;
	m_bLoadAdditinalInfo = bLoadAdditinalInfo;
	m_bKeepInLocalSpace  = bKeepInLocalSpace;
	m_bUseStreaming			 = false;
	m_bMakePhysics       = bMakePhysics;

	// make folder name
	strcpy(m_szFolderName,szFileName);
	while(m_szFolderName[0])
	{ 
		if(m_szFolderName[strlen(m_szFolderName)-1] == '\\' || m_szFolderName[strlen(m_szFolderName)-1] == '/')
		{ m_szFolderName[strlen(m_szFolderName)-1]=0; break; }
		m_szFolderName[strlen(m_szFolderName)-1]=0;
	}  

	// remember file and geom names
	strcpy(m_szFileName, szFileName);
	strcpy(m_szGeomName, szGeomName && szGeomName[0] ? szGeomName : "");
	
	// load uncompiled 
	m_nLoadedTrisCount = 0;
	m_pTriData = new CIndexedMesh( GetSystem(), szFileName, (szGeomName && szGeomName[0]) ? szGeomName : 0, &m_nLoadedTrisCount, bLoadAdditinalInfo, bKeepInLocalSpace, m_bIgnoreFakeMaterialsInCGF );
	if(!m_nLoadedTrisCount)
		return false;

	// copy helpers
	m_lstHelpers.AddList(*m_pTriData->GetHelpers());

	// copy lsources
	for(int i=0; i<m_pTriData->GetLightSourcesList()->Count(); i++)
	{
		m_lstLSources.Add(*m_pTriData->GetLightSourcesList()->GetAt(i));
		if(m_lstLSources.Last().m_pShader)
			m_lstLSources.Last().m_pShader->AddRef();
		if(m_lstLSources.Last().m_pLightImage)
			m_lstLSources.Last().m_pLightImage->AddRef();
	}

	// set bbox and rediuses
	m_vBoxMin = m_pTriData->m_vBoxMin;
	m_vBoxMax = m_pTriData->m_vBoxMax;
	CalcRadiuses();

	if(bMakePhysics)
		Physicalize(); // can remove some indices/faces

	// create vert buffers
	MakeLeafBuffer(eVertsSharing==evs_ShareAndSortForCache);

	for (int i=0; m_pLeafBuffer && m_pLeafBuffer->m_pMats && i<m_pLeafBuffer->m_pMats->Count(); i++)
		m_lstShaderTemplates.Add(-1);

	if(!GetCVars()->e_stencil_shadows)
		FreeTriData(); // source geometry is needed only for stencil shadows
	else if(GetCVars()->e_stencil_shadows_build_on_load)
	{
		if (!GetShadowVolume())
		{
			if(m_pTriData)
			{
				SetShadowVolume(CStatObj::MakeConnectivityInfo(m_pTriData, Vec3d(0,0,0), this));
				FreeTriData(); // source geometry is needed only for stencil shadows
			}
		}
	}

	return true;
}

void CStatObj::LoadLowLODs(EVertsSharing eVertsSharing,bool bLoadAdditinalInfo,bool bKeepInLocalSpace, bool bLoadLater)
{
	m_nLoadedLodsNum = 1;

	if(!GetCVars()->e_cgf_load_lods)
		return;

	for(int nLodLevel=1; nLodLevel<MAX_STATOBJ_LODS_NUM; nLodLevel++)
	{
		// make lod file name
		char sLodFileName[512];
		strncpy(sLodFileName, m_szFileName, sizeof(m_szFileName));
		sLodFileName[strlen(sLodFileName)-4]=0;
		strcat(sLodFileName,"_lod");
		char sLodNum[8];
		ltoa(nLodLevel,sLodNum,10);
		strcat(sLodFileName,sLodNum);
		strcat(sLodFileName,".cgf");

		// try to load
		FILE * fp = GetSystem()->GetIPak()->FOpen(sLodFileName,"r");
		bool bRes = false;
		if(fp)
		{
			if(!m_arrpLowLODs[nLodLevel])
				m_arrpLowLODs[nLodLevel] = new CStatObj( );
			bRes = m_arrpLowLODs[nLodLevel]->Load(sLodFileName, m_szGeomName, eVertsSharing, bLoadAdditinalInfo, bKeepInLocalSpace, bLoadLater, false);
			GetSystem()->GetIPak()->FClose(fp);
		}

		if(!bRes)
		{
			delete m_arrpLowLODs[nLodLevel];
			m_arrpLowLODs[nLodLevel]=0;
			break;
		}

		if(m_arrpLowLODs[nLodLevel]->m_nLoadedTrisCount > m_nLoadedTrisCount / 1.5f)
		{
#if !defined(LINUX)
			Warning(0, sLodFileName,	
			"CStatObj::LoadLowLODs: Low lod model contains too many polygons comparing to full LOD model [%d agains %d], there is no sense to use it",
			m_arrpLowLODs[nLodLevel]->m_nLoadedTrisCount, m_nLoadedTrisCount);
#endif
		}

		m_nLoadedLodsNum++;
	}
}

void CStatObj::PreloadResources(float fDist, float fTime, int dwFlags)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );
	if(GetLeafBuffer())
		GetRenderer()->EF_PrecacheResource(GetLeafBuffer(),fDist,fTime,0);

	if(m_arrSpriteTexID[0])
	for(int i=0; i<FAR_TEX_COUNT; i++)
	{
		if(m_arrSpriteTexID[i])
		{
			ITexPic * pTexPic = GetRenderer()->EF_GetTextureByID(m_arrSpriteTexID[i]);
			if(pTexPic)
				GetRenderer()->EF_PrecacheResource(pTexPic, 0, 1.f, 0);
		}
	}
}
