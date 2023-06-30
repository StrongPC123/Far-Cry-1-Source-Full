
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// XDemoMgr.cpp: implementation of the CXDemoMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XDemoMgr.h"
#include "TimeDemoRecorder.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CXDemoMgr::CXDemoMgr()
{
	m_pFile=NULL;
	Stop();
}

//////////////////////////////////////////////////////////////////////////
CXDemoMgr::~CXDemoMgr()
{
	Stop();
}

//////////////////////////////////////////////////////////////////////////
void CXDemoMgr::Stop()
{
	m_nFileSize = 0;
	m_fCurrTime = 0;
	m_fGameTime = 0;
	if(m_pFile) fclose(m_pFile);
	m_pFile = NULL;
	bStreamStarted = false;
};

#define DEMOMAGIC1 "CRYDEMO"
#define DEMOMAGIC2 "CRYSTREAM"
#define DEMOVERSION 1

//////////////////////////////////////////////////////////////////////////
// Prepares recording of a new demo.
bool CXDemoMgr::StartRecord(const char *sFileName, CStream &stm)
{
	Stop();
	
	m_pFile=fopen(sFileName,"wb+");
	if(m_pFile==NULL)
		return false;
		
	// header: string + version	+ savegamesize
	if(fwrite(DEMOMAGIC1, strlen(DEMOMAGIC1)+1, 1, m_pFile)!=1)
		return false;
		
	int hd[2] = { DEMOVERSION, (int)stm.GetSize() };
	if(fwrite(&hd, sizeof(hd), 1, m_pFile)!=1)
		return false;	
	
	// savegame, has its own header and version
	if(fwrite(stm.GetPtr(), BITS2BYTES(stm.GetSize()), 1, m_pFile)!=1)
		return false;
	
	// header for the network stream part, just as sanity check
	if(fwrite(DEMOMAGIC2, strlen(DEMOMAGIC2)+1, 1,m_pFile)!=1)
		return false;
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Stores a chunk of demo-data on disk.
bool CXDemoMgr::AddChunk(float fTimestamp, CStream &stm, IEntity *player)
{
	unsigned int nStreamSizeInBytes=BITS2BYTES(stm.GetSize());
	unsigned int nStreamSizeInBits=stm.GetSize();
	
	if(nStreamSizeInBits==0)
		return false;
	if(m_pFile==NULL)
		return false;
	if(fwrite(&fTimestamp,sizeof(fTimestamp),1,m_pFile)!=1)
		return false;
	if(fwrite(&nStreamSizeInBits,sizeof(nStreamSizeInBits),1,m_pFile)!=1)
		return false;
	if(fwrite(stm.GetPtr(),nStreamSizeInBytes,1,m_pFile)!=1)
		return false;
	if(fwrite(&player->GetAngles(),sizeof(Vec3d),1,m_pFile)!=1)
		return false;
		
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Opens a recorded demo for playback.
bool CXDemoMgr::StartDemoPlay(const char *sFileName, CXGame *pGame)
{
	Stop();
	m_pGame = pGame;

	m_pFile=fopen(sFileName,"rb");
	if(m_pFile==NULL)
		return false;
		
	//measure the file size
	fseek(m_pFile,0,SEEK_END);
	m_nFileSize=ftell(m_pFile);
	fseek(m_pFile,0,SEEK_SET);
	
	char magic[32];
	if(fread(magic, strlen(DEMOMAGIC1)+1, 1, m_pFile)!=1 || strcmp(magic, DEMOMAGIC1))
		return false;
		
	int hd[2];
	if(fread(hd, sizeof(hd), 1, m_pFile)!=1 || hd[0]!=DEMOVERSION)
		return false;
		
	int nNumBits = hd[1];
	int nNumBytes = BITS2BYTES(nNumBits);
	
	CDefaultStreamAllocator sa;
	CStream stm(300, &sa); 

	stm.SetSize(nNumBits);
	if(fread(stm.GetPtr(), nNumBytes, 1, m_pFile)!=1)
		return false;

	if(!pGame->LoadFromStream(stm, true))
		return false;

	if(fread(magic, strlen(DEMOMAGIC2)+1, 1, m_pFile)!=1 || strcmp(magic, DEMOMAGIC2))
		return false;

	//read the first chunk
	if(!ReadChunk(m_ChunkToPlay.stm, m_ChunkToPlay.fTimestamp, m_ChunkToPlay.angles))
		return false;
	//set the chunk as non played
	m_ChunkToPlay.bPlayed=false;
	//set the first chunk timestamp as "base time"
	m_fCurrTime=m_ChunkToPlay.fTimestamp;
	
	pGame->GetSystem()->GetIConsole()->ShowConsole(false);
	
	bStreamStarted = true;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Play a recorded demo.
bool CXDemoMgr::PlayChunk(float fCurrentTime, CXClient *pClient)
{
	if(!m_pFile || !bStreamStarted)
		return false;
		
	//the first time this function is called the base timer is initialized
	if(m_fGameTime==0)
		m_fGameTime = fCurrentTime;

	m_fCurrTime += fCurrentTime-m_fGameTime;
	m_fGameTime = fCurrentTime;
	
	if(m_fCurrTime==0 || ((m_fCurrTime>m_ChunkToPlay.fTimestamp) && (!m_ChunkToPlay.bPlayed)))
	{
		m_ChunkToPlay.bPlayed=false;
		m_ChunkToPlay.stm.Seek(0);
		pClient->OnXData(m_ChunkToPlay.stm);
		IEntity *e = m_pGame->GetXSystem()->GetEntity(pClient->GetPlayerId());
		e->SetAngles(m_ChunkToPlay.angles);
		if(!EOD())
		{
			ReadChunk(m_ChunkToPlay.stm, m_ChunkToPlay.fTimestamp, m_ChunkToPlay.angles);
			m_ChunkToPlay.bPlayed = false;
		}
		return true;		
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Reads a demo-chunk from disk.
bool CXDemoMgr::ReadChunk(CStream &stm, float &fTimestamp, Vec3d &angles)
{
	unsigned int nStreamSizeInBytes;
	unsigned int nStreamSizeInBits;
	
	if(m_pFile==NULL)
		return false;
	stm.Reset();
	if(fread(&fTimestamp, sizeof(fTimestamp),1,m_pFile)!=1)
		return false;
	if(fread(&nStreamSizeInBits,sizeof(nStreamSizeInBits),1,m_pFile)!=1)
		return false;
	nStreamSizeInBytes = BITS2BYTES(nStreamSizeInBits);
	
	#ifdef _DEBUG
	if(nStreamSizeInBits>stm.GetAllocatedSize())
		CryError( "<CryGame> (CXDemoMgr::ReadChunk) Invalid stream size" );	
	#endif

	stm.SetSize(nStreamSizeInBits);
	if(fread(stm.GetPtr(), nStreamSizeInBytes, 1, m_pFile)!=1)
		return false;
		
	if(fread(&angles, sizeof(Vec3d), 1, m_pFile)!=1)
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Check if the end of the demo is reached.
bool CXDemoMgr::EOD()
{
	if(m_pFile==NULL)
		return true;
	if(ftell(m_pFile)<int(m_nFileSize))
		return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
//! Start recording a demo.
bool CXGame::StartRecording(const char *sFileName)
{
	g_timedemo_file->Set( sFileName );
	m_pTimeDemoRecorder->Record(true);
	return false;
}

//////////////////////////////////////////////////////////////////////////
//! Stop recording a demo.
void CXGame::StopRecording()
{
	m_pTimeDemoRecorder->Record(false);
	String filename = m_currentLevelFolder + "/" + g_timedemo_file->GetString() + ".tmd";
	m_pTimeDemoRecorder->Save( filename.c_str() );
}

//////////////////////////////////////////////////////////////////////////
//! Start playing a demo.
void CXGame::StartDemoPlay(const char *sFileName)
{
	g_timedemo_file->Set( sFileName );
	String filename = m_currentLevelFolder + "/" + g_timedemo_file->GetString() + ".tmd";
	m_pTimeDemoRecorder->Load( filename.c_str() );
	m_pTimeDemoRecorder->Play(true);
};

//////////////////////////////////////////////////////////////////////////
//! Stop playing a demo.
void CXGame::StopDemoPlay()
{
	if (m_pTimeDemoRecorder->IsRecording())
	{
		StopRecording();
	}
	m_pTimeDemoRecorder->Play(false);
};

//////////////////////////////////////////////////////////////////////////
void CXGame::PlaybackChunk()
{
	m_XDemoMgr.PlayChunk(GetSystem()->GetITimer()->GetCurrTime(), m_pClient);
};

//////////////////////////////////////////////////////////////////////////
//! Adding a chunk to the demo.
bool CXGame::AddDemoChunk(CStream &stm)
{
	IEntity *e = GetXSystem()->GetEntity(m_pClient->GetPlayerId());
	assert(e);
	return m_XDemoMgr.AddChunk(GetSystem()->GetITimer()->GetCurrTime(), stm, e);
}
