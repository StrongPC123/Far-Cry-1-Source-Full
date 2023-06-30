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
#include "../RenderDll/Common/shadow_renderer.h"
#include <irenderer.h>
#include <CrySizer.h>


void CStatObj::StreamOnProgress (IReadStream* pStream) 
{
}

void CStatObj::ProcessStreamOnCompleteError()
{ // file was not loaded successfully
	m_eCCGFStreamingStatus = ecss_NotLoaded;

	// try to rebuild compiled file if not tryed yet
/*	if(!m_bCompilingNotAllowed)
	{
		char szCompiledFileName[MAX_PATH_LENGTH];
		MakeCompiledFileName(szCompiledFileName,MAX_PATH_LENGTH);
		bool bCompRes = CompileObject(szCompiledFileName, m_szFileName, m_szGeomName, 
			m_eVertsSharing, m_bLoadAdditinalInfo, m_bKeepInLocalSpace);

		// load new compiled file
		if(bCompRes)
		{
			m_bCompilingNotAllowed=1;
			StreamCCGF(true);
			m_bCompilingNotAllowed=0;
		}

		if(m_eCCGFStreamingStatus == ecss_Ready)
			return; // success
	}
	else if(strstr(m_szFileName,"Objects\\default.cgf"))
		GetSystem()->Error("Error loading default object");

	// still error - make nice default object
	m_eCCGFStreamingStatus = ecss_NotLoaded;
	Load("Objects\\default.cgf","",evs_NoSharing,false,false,false,false);
	assert(	m_eCCGFStreamingStatus == ecss_Ready);
	m_bDefaultObject=true;*/

	assert(0);
	GetSystem()->Error("Error loading CCGF for: %s", m_szFileName);
}

void CStatObj::StreamOnComplete(IReadStream* pStream, unsigned nError)
{
	m_pReadStream = 0;

	if(pStream->IsError())
	{ // file was not loaded successfully
		ProcessStreamOnCompleteError();
		return;
	}

	// load header
	CCGFHeader * pFileHeader = (CCGFHeader *)pStream->GetBuffer();
#if !defined(LINUX)
	assert(pFileHeader->nDataSize == pStream->GetBytesRead()-sizeof(CCGFHeader));
#endif

	if(!pFileHeader->nDataSize)
	{ // should happend only in case of sync loading
		assert(m_szGeomName[0]);
		m_eCCGFStreamingStatus = ecss_GeomNotFound;
		return; // geom name was specified but not found in source sgf during compilation
	}

  int nChecksum = CCGFHeader::GetStructuresCheckSummm();
	if(pFileHeader->nStructuresCheckSummm != nChecksum)
	{ 
		m_eCCGFStreamingStatus = ecss_LoadingError;
		GetConsole()->Exit("Error: CStatObj::StreamOnComplete: version of " RC_EXECUTABLE " is not compatible with engine: %s", m_szFileName);
		return; // geom name was specified but not found in source sgf during compilation
	}

	// load data
	uchar * pData = ((uchar *)pStream->GetBuffer()+sizeof(CCGFHeader));
	int nPos=0;
	Serialize(nPos, pData, false, m_szFolderName);
	assert(nPos == pFileHeader->nDataSize);

	// original tris count
	m_nLoadedTrisCount = pFileHeader->nFacesInCGFNum;
	assert(m_nLoadedTrisCount);

	m_bPhysicsExistInCompiledFile = (pFileHeader->dwFlags & CCGFHF_PHYSICS_EXIST);

	// get bbox
	m_vBoxMin = pFileHeader->vBoxMin;
	m_vBoxMax = pFileHeader->vBoxMax;
	CalcRadiuses();

	PhysicalizeCompiled();

	m_eCCGFStreamingStatus = ecss_Ready;

	if(GetCVars()->e_stream_cgf)
		GetLog()->Log("CStatObj::StreamOnComplete: %s", m_szFileName);
}

void CStatObj::StreamCCGF(bool bFinishNow)
{
//	if(strstr(m_szFileName,"de_bind.cgf"))
	//	int t=0;

	if(m_eCCGFStreamingStatus != ecss_NotLoaded)
		return;

	char szCompiledFileName[MAX_PATH_LENGTH];
	MakeCompiledFileName(szCompiledFileName,MAX_PATH_LENGTH);

	if(bFinishNow)
		GetLog()->UpdateLoadingScreen("\003Load compiled object: %s", szCompiledFileName);
	else
		GetLog()->UpdateLoadingScreen("\003Start streaming compiled object: %s", szCompiledFileName);

	// start streaming
	StreamReadParams params;
	params.dwUserData = 0;
	params.nSize = 0;
	params.pBuffer = NULL;
	params.nLoadTime = 10000;
	params.nMaxLoadTime = 10000;
	params.nFlags |= SRP_FLAGS_ASYNC_PROGRESS;

	m_eCCGFStreamingStatus = ecss_LoadingInProgress;
	m_pReadStream = m_pSys->GetStreamEngine()->StartRead("3DEngine", szCompiledFileName, this, &params);

	if(bFinishNow)
		m_pReadStream->Wait();
}

void CStatObj::CheckLoaded()
{
	if(!m_bUseStreaming)
		return;

	m_nMarkedForStreamingFrameId = GetFrameID()+100;

	if(m_eCCGFStreamingStatus == ecss_NotLoaded)
	{ // load now
		assert(m_bUseStreaming == true);
		m_fStreamingTimePerFrame -= GetTimer()->GetAsyncCurTime();
		bool bRes = Load(m_szFileName, m_szGeomName[0] ? m_szGeomName : 0, m_eVertsSharing, 
			m_bLoadAdditinalInfo, m_bKeepInLocalSpace, false, m_bMakePhysics);
		m_bUseStreaming = true;
		m_fStreamingTimePerFrame += GetTimer()->GetAsyncCurTime();
	}
	else if(m_eCCGFStreamingStatus == ecss_LoadingInProgress)		
	{ // finish now
		m_pReadStream->Wait();
	}
	else
	{ // object is ready
		assert(m_eCCGFStreamingStatus == ecss_Ready);
		assert(m_pLeafBuffer && m_nLoadedTrisCount);
	}
}

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

void CStatObj::MakeCompiledFileName(char * szCompiledFileName, int nMaxLen)
{
	// make ccgf name
	char szFileNameNoExt[512]="";
	strcpy(szFileNameNoExt,m_szFileName);

	// remove extension
	for(int i=strlen(szFileNameNoExt)-1; i>0; i--) if(szFileNameNoExt[i]=='.')
	{	szFileNameNoExt[i]=0; break; }

	// replace slashes in geom name
	char szGeomName[sizeof(m_szGeomName)]="";
	strncpy(szGeomName,m_szGeomName,sizeof(szGeomName));
	int nLen = strlen(szGeomName);
	for(int i=0; i<nLen; i++)
		if (szGeomName[i]=='/' || szGeomName[i]=='\\')
			szGeomName[i] = '_';

	_snprintf(szCompiledFileName, nMaxLen,
		"%s\\%s_%s_%d_%d_%d.ccgf",
		CCGF_CACHE_DIR_NAME, szFileNameNoExt, szGeomName, 
		int(m_eVertsSharing == evs_ShareAndSortForCache), (int)m_bLoadAdditinalInfo, (int)m_bKeepInLocalSpace);
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif
