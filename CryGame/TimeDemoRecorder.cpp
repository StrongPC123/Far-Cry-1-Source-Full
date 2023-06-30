////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   timedemorecorder.cpp
//  Version:     v1.00
//  Created:     2/8/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TimeDemoRecorder.h"
#include <CryFile.h>
#include "Game.h"

#if defined(WIN32) && !defined(WIN64)
//#include "Psapi.h"								// PSAPI is not supported on windows9x
//#pragma comment(lib,"Psapi.lib")	// PSAPI is not supported on windows9x
#endif

//////////////////////////////////////////////////////////////////////////
// Brush Export structures.
//////////////////////////////////////////////////////////////////////////
#define TIMEDEMO_FILE_SIGNATURE "CRY"
#define TIMEDEMO_FILE_TYPE 150
#define TIMEDEMO_FILE_VERSION 1

#define FIXED_TIME_STEP 0.02f // Assume runing at 50fps.

#pragma pack(push,1)
struct STimeDemoHeader
{
	char signature[3];	// File signature.
	int filetype;				// File type.
	int	version;				// File version.

	//////////////////////////////////////////////////////////////////////////
	int numFrames;
	float totalTime;
	char reserved[128];
};

//////////////////////////////////////////////////////////////////////////
struct STimeDemoFrame
{
	Vec3 pos;
	Vec3 angles;
	float frametime;

	unsigned int nActionFlags[2];
	float fLeaning;
	int nPolygonsPerFrame;
	char reserved[28];
};
#pragma pack(pop)
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CTimeDemoRecorder::CTimeDemoRecorder( ISystem *pSystem )
{
	m_pSystem = pSystem;
	assert(m_pSystem);
	m_pGame = pSystem->GetIGame();
	assert(m_pGame);

	m_bRecording = false;
	m_bPlaying = false;

	m_currFrameIter = m_records.end();

	m_recordStartTime = 0;
	m_recordEndTime = 0;
	m_lastFrameTime = 0;
	m_totalDemoTime = 0;
	m_recordedDemoTime = 0;

	m_lastAveFrameRate = 0;
	m_lastPlayedTotalTime = 0;

	m_fixedTimeStep = 0;
	m_maxLoops = 1000;
	m_demo_scroll_pause = 1;
	m_demo_noinfo = 1;

	m_bPaused = false;

	// Register demo variables.
	pSystem->GetIConsole()->Register( "demo_num_runs",&m_maxLoops,1000,0,"Number of times to loop timedemo" );
	pSystem->GetIConsole()->Register( "demo_scroll_pause",&m_demo_scroll_pause,1,0,"ScrollLock pauses demo play/record" );
	pSystem->GetIConsole()->Register( "demo_noinfo",&m_demo_noinfo,0,0,"Disable info display during demo playback" );
	pSystem->GetIConsole()->Register( "demo_quit",&m_demo_quit,0,0,"Quit game after demo runs finished" );
	pSystem->GetIConsole()->Register( "demo_screenshot_frame",&m_demo_screenshot_frame,0,0,"Make screenshot on specified frame during demo playback" );
}

//////////////////////////////////////////////////////////////////////////
CTimeDemoRecorder::~CTimeDemoRecorder()
{

}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::Record( bool bEnable )
{
	if (bEnable == m_bRecording)
		return;

	m_bRecording = bEnable;
	m_bPlaying = false;
	if (m_bRecording)
	{
		// Start recording.
		m_records.clear();
		m_recordStartTime = GetTime();
		m_lastFrameTime = m_recordStartTime;

		StartSession();
	}
	else
	{
		// Stop recording.
		m_recordedDemoTime = m_totalDemoTime;
		m_lastFrameTime = GetTime();

		StopSession();
	}
	m_currFrameIter = m_records.end();
	m_currentFrame = 0;
	m_totalDemoTime = 0;
}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::Play( bool bEnable )
{
	if (bEnable == m_bPlaying)
		return;

	if (m_records.empty())
		return;

	m_bRecording = false;
	m_bPlaying = bEnable;

	if (m_bPlaying)
	{
		LogInfo( "==============================================================" );
		LogInfo( "TimeDemo Play Started , (Total Frames: %d, Recorded Time: %.2fs)",(int)m_records.size(),m_recordedDemoTime );

		// Start demo playback.
		m_currFrameIter = m_records.begin();
		m_lastPlayedTotalTime = 0;
		StartSession();
	}
	else
	{
		LogInfo( "TimeDemo Play Ended, (%d Runs Performed)",m_numLoops );
		LogInfo( "==============================================================" );

		// End demo playback.
		m_currFrameIter = m_records.end();
		m_lastPlayedTotalTime = m_totalDemoTime;
		StopSession();
	}
	m_currentFrame = 0;
	m_totalDemoTime = 0;
	m_numLoops = 0;
	m_fpsCounter = 0;
	m_lastFpsTimeRecorded = GetTime();

	m_currFPS = 0;
	m_minFPS = 10000;
	m_maxFPS = -10000;
	m_nMaxPolys = INT_MIN;
	m_nMinPolys = INT_MAX;
}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::Save( const char *filename )
{
	CCryFile file;
	if (!file.Open( filename,"wb" ))
	{
		m_pSystem->Warning( VALIDATOR_MODULE_GAME,VALIDATOR_WARNING,0,filename,"Cannot open time demo file %s",filename );
		return;
	}

	m_file = filename;

	// Save Time demo file.
	STimeDemoHeader hdr;
	memset( &hdr,0,sizeof(hdr) );
	
	memcpy( hdr.signature,TIMEDEMO_FILE_SIGNATURE,3 );
	hdr.filetype = TIMEDEMO_FILE_TYPE;
	hdr.version = TIMEDEMO_FILE_VERSION;

	hdr.numFrames = m_records.size();
	hdr.totalTime = m_recordedDemoTime;
	file.Write( &hdr,sizeof(hdr) );

	for (FrameRecords::iterator it = m_records.begin(); it != m_records.end(); ++it)
	{
		FrameRecord &rec = *it;
		STimeDemoFrame frame;
		frame.angles = rec.playerAngles;
		frame.pos = rec.playerPos;
		frame.frametime = rec.frameTime;
		*frame.nActionFlags = *rec.nActionFlags;
		frame.fLeaning = rec.fLeaning;
		frame.nPolygonsPerFrame = rec.nPolygons;
		file.Write( &frame,sizeof(frame) );
	}	
/*
	XmlNodeRef root = m_pSystem->CreateXmlNode( "TimeDemo" );
	root->setAttr( "TotalTime",m_recordedDemoTime );
	for (FrameRecords::iterator it = m_records.begin(); it != m_records.end(); ++it)
	{
		FrameRecord &rec = *it;
		XmlNodeRef xmlRecord = root->newChild( "Frame" );
		xmlRecord->setAttr( "Pos",rec.playerPos );
		xmlRecord->setAttr( "Ang",rec.playerAngles );
		xmlRecord->setAttr( "Time",rec.frameTime );
	}
	root->saveToFile( filename );
*/
}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::Load(  const char *filename )
{
	m_records.clear();
	m_recordedDemoTime = 0;
	m_totalDemoTime = 0;

	CCryFile file;
	if (!file.Open( filename,"rb" ))
	{
		m_pSystem->Warning( VALIDATOR_MODULE_GAME,VALIDATOR_WARNING,0,filename,"Cannot open time demo file %s",filename );
		return;
	}

	m_file = filename;

	// Load Time demo file.
	STimeDemoHeader hdr;
	file.Read( &hdr,sizeof(hdr) );
	
	m_recordedDemoTime = hdr.totalTime;
	m_totalDemoTime = m_recordedDemoTime;

	for (int i = 0; i < hdr.numFrames && !file.IsEof(); i++)
	{
		STimeDemoFrame frame;
		FrameRecord rec;
		file.Read( &frame,sizeof(frame) );

		rec.playerAngles = frame.angles;
		rec.playerPos = frame.pos;
		rec.frameTime = frame.frametime;
		*rec.nActionFlags = *frame.nActionFlags;
		rec.fLeaning = frame.fLeaning;
		rec.nPolygons = frame.nPolygonsPerFrame;
		m_records.push_back( rec );
	}

	/*
	XmlNodeRef root = m_pSystem->LoadXmlFile( filename );
	if (!root)
	{
		// No such demo file.
		m_pSystem->Warning( VALIDATOR_MODULE_GAME,VALIDATOR_WARNING,0,filename,"Cannot open time demo file %s",filename );
		return;
	}
	root->getAttr( "TotalTime",m_recordedDemoTime );
	m_totalDemoTime = m_recordedDemoTime;
	for (int i = 0; i < root->getChildCount(); i++)
	{
		FrameRecord rec;
		XmlNodeRef xmlRecord = root->getChild(i);
		xmlRecord->getAttr( "Pos",rec.playerPos );
		xmlRecord->getAttr( "Ang",rec.playerAngles );
		xmlRecord->getAttr( "Time",rec.frameTime );
		m_records.push_back( rec );
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::Update()
{
	if (m_bRecording)
	{
		RecordFrame();
	}
	else if (m_bPlaying)
	{
		PlayFrame();
	}
}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::RecordFrame()
{
	float time = GetTime();
	float frameTime = time - m_lastFrameTime;

	if (m_bPaused)
	{
		m_lastFrameTime = time;
		return;
	}

	FrameRecord rec;
	rec.frameTime = frameTime;

	IEntity *pPlayer = NULL;

	pPlayer = GetIXGame( m_pGame )->GetMyPlayer();

	if (pPlayer)
	{
		rec.playerPos = pPlayer->GetPos();
		rec.playerAngles = pPlayer->GetAngles();
	}
	// Record current processing command.
	CXEntityProcessingCmd &cmd = ((CXGame*)m_pGame)->m_pClient->m_PlayerProcessingCmd;
	*rec.nActionFlags = *cmd.m_nActionFlags;
	rec.fLeaning = cmd.GetLeaning();

	m_totalDemoTime += rec.frameTime;

	int nPolygons,nShadowVolPolys;
	m_pSystem->GetIRenderer()->GetPolyCount(nPolygons,nShadowVolPolys);
	rec.nPolygons = nPolygons;

	m_records.push_back( rec );

	m_currentFrame++;
	m_lastFrameTime = GetTime();
}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::PlayFrame()
{
	if (m_currFrameIter == m_records.end()) // can't playback empty records.
		return;

	float time = GetTime();
	float frameTime = time - m_lastFrameTime;

	if (m_bPaused)
	{
		m_lastFrameTime = time;
		return;
	}

	FrameRecord &rec = *m_currFrameIter;

	IEntity *pPlayer = NULL;
	pPlayer = GetIXGame( m_pGame )->GetMyPlayer();

	if (pPlayer)
	{
		pPlayer->SetPos( rec.playerPos );
		pPlayer->SetAngles( rec.playerAngles );

		GetIXGame( m_pGame )->SetViewAngles( rec.playerAngles );
	}
	// Overwrite current processing command.
	CXEntityProcessingCmd &cmd = ((CXGame*)m_pGame)->m_pClient->m_PlayerProcessingCmd;
	*cmd.m_nActionFlags = *rec.nActionFlags;
	cmd.SetLeaning( rec.fLeaning );

	m_totalDemoTime += frameTime;

	int nPolygons,nShadowVolPolys;
	m_pSystem->GetIRenderer()->GetPolyCount(nPolygons,nShadowVolPolys);
	m_nPolysCounter += nPolygons;
	m_nCurrPolys = nPolygons;
	if (nPolygons > m_nMaxPolys)
		m_nMaxPolys = nPolygons;
	if (nPolygons < m_nMinPolys)
		m_nMinPolys = nPolygons;

	m_nTotalPolysRecorded += rec.nPolygons;
	m_nTotalPolysPlayed += nPolygons;

	//////////////////////////////////////////////////////////////////////////
	// Calculate Frame Rates.
	//////////////////////////////////////////////////////////////////////////
	// Skip some frames before calculating frame rates.
	if (time - m_lastFpsTimeRecorded > 1)
	{
		// Skip some frames before recording frame rates.
		if (m_currentFrame > 60)
		{
			m_currFPS = (float)m_fpsCounter / (time - m_lastFpsTimeRecorded);
			if (m_currFPS > m_maxFPS)
			{
				m_maxFPS_Frame = m_currentFrame;
				m_maxFPS = m_currFPS;
			}
			if (m_currFPS < m_minFPS)
			{
				m_minFPS_Frame = m_currentFrame;
				m_minFPS = m_currFPS;
			}
		}
		
		m_nPolysPerSec = (int)(m_nPolysCounter  / (time - m_lastFpsTimeRecorded));
		m_nPolysCounter = 0;

		m_fpsCounter = 0;
		m_lastFpsTimeRecorded = time;
	}
	else
	{
		m_fpsCounter++;
	}
	//////////////////////////////////////////////////////////////////////////

	m_currFrameIter++;
	m_currentFrame++;

	if(m_demo_screenshot_frame && m_currentFrame == m_demo_screenshot_frame)
	{		
/*		ICVar *	p_demo_file = pSystem->GetIConsole()->GetCVar("demo_file");
		char szFileName[256]="";
		sprinyf("demo_%s_%d.jpg",p_demo_file->GetString(),m_currentFrame);*/
		m_pSystem->GetIRenderer()->ScreenShot();
	}

	// Play looped.
	if (m_currFrameIter == m_records.end())
	{
		m_lastPlayedTotalTime = m_totalDemoTime;
		m_lastAveFrameRate = GetAverageFrameRate();

		// Log info to file.
		LogRun();

		m_totalDemoTime = 0;
		m_currFrameIter = m_records.begin();
		m_currentFrame = 0;
		m_numLoops++;
		m_nTotalPolysPlayed = 0;
		m_nTotalPolysRecorded = 0;

		// Stop playing if max runs reached.
		if (m_numLoops > m_maxLoops)
		{
			Play(false);
			if (m_demo_quit)
			{
				// Immidiate game abort after num loops done.
				exit(0);
			}
		}
	}

	m_lastFrameTime = GetTime();
}

//////////////////////////////////////////////////////////////////////////
float CTimeDemoRecorder::GetTime()
{
	// Must be asynchronius time, used for profiling.
	return m_pSystem->GetITimer()->GetAsyncCurTime();
}

//////////////////////////////////////////////////////////////////////////
int CTimeDemoRecorder::GetNumFrames() const
{
	return m_records.size();
}

//////////////////////////////////////////////////////////////////////////
float CTimeDemoRecorder::GetAverageFrameRate() const
{
	float aveFrameTime = m_totalDemoTime / m_currentFrame;
	float aveFrameRate = 1.0f / aveFrameTime;
	return aveFrameRate;
}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::RenderInfo()
{
	if (m_demo_noinfo != 0)
		return;

	const char *sInfo = "";
	m_bPaused = false;
	if (m_bRecording || m_bPlaying)
	{
		if (m_demo_scroll_pause != 0 && GetISystem()->GetIInput())
		{
			bool bPaused = (GetISystem()->GetIInput()->GetKeyState(XKEY_SCROLLLOCK) & 1);
			if (bPaused)
			{
				m_bPaused = true;
				sInfo = " (Paused)";
			}
		}
	}

	IRenderer *pRenderer = m_pSystem->GetIRenderer();
	if (m_bRecording)
	{
		float fColor[4] = {1,0,0,1};
		pRenderer->Draw2dLabel( 1,1, 1.3f, fColor,false,"Recording TimeDemo%s, Frames: %d",sInfo,m_currentFrame );
	}
	else if (m_bPlaying)
	{
		float aveFrameRate = GetAverageFrameRate();
		int numFrames = GetNumFrames();
		float fColor[4] = {0,1,0,1};
		pRenderer->Draw2dLabel( 1,1, 1.3f, fColor,false,"Playing TimeDemo%s, Frame %d of %d (Looped: %d)",sInfo,m_currentFrame,numFrames,m_numLoops );
		pRenderer->Draw2dLabel( 1,1+15, 1.3f, fColor,false,"Last Played Length: %.2fs, FPS: %.2f",m_lastPlayedTotalTime,m_lastAveFrameRate );
		pRenderer->Draw2dLabel( 1,1+30, 1.3f, fColor,false,"Average FPS: %.2f, FPS: %.2f, Polys/Frame: %d",aveFrameRate,m_currFPS,m_nCurrPolys );
		pRenderer->Draw2dLabel( 1,1+45, 1.3f, fColor,false,"Polys Rec/Play Ratio: %.2f",(float)m_nTotalPolysRecorded/m_nTotalPolysPlayed );
	}
}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::SetConsoleVar( const char *sVarName,float value )
{
	ICVar *pVar = m_pSystem->GetIConsole()->GetCVar( sVarName );
	if (pVar)
		pVar->Set( value );
}

//////////////////////////////////////////////////////////////////////////
float CTimeDemoRecorder::GetConsoleVar( const char *sVarName )
{
	ICVar *pVar = m_pSystem->GetIConsole()->GetCVar( sVarName );
	if (pVar)
		return pVar->GetFVal();
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::StartSession()
{
	m_nTotalPolysRecorded = 0;
	m_nTotalPolysPlayed = 0;
	m_lastAveFrameRate = 0;
	m_lastPlayedTotalTime = 0;

	// Register to frame profiler.
	m_pSystem->GetIProfileSystem()->AddPeaksListener( this );
	// Remember old time step, and set constant one.
	m_fixedTimeStep = GetConsoleVar( "fixed_time_step" );
	SetConsoleVar( "fixed_time_step",FIXED_TIME_STEP );

	SetConsoleVar( "ai_ignoreplayer",1 );
	SetConsoleVar( "ai_soundperception",0 );
	SetConsoleVar( "mov_NoCutscenes",1 );

	// Profile
	m_oldPeakTolerance = GetConsoleVar( "profile_peak" );
	SetConsoleVar( "profile_peak",50 );

	m_lastFrameTime = GetTime();
}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::StopSession()
{
	// Set old time step.
	SetConsoleVar( "fixed_time_step",m_fixedTimeStep );
	
	SetConsoleVar( "ai_ignoreplayer",0 );
	SetConsoleVar( "ai_soundperception",1 );
	SetConsoleVar( "mov_NoCutscenes",0 );

	// Profile.
	SetConsoleVar( "profile_peak",m_oldPeakTolerance );
	m_pSystem->GetIProfileSystem()->RemovePeaksListener( this );
}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::LogInfo( const char *format,... )
{
	if (m_demo_noinfo != 0)
		return;

	va_list	ArgList;
	char szBuffer[1024];

	va_start(ArgList, format);
	vsprintf(szBuffer, format, ArgList);
	va_end(ArgList);

	char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	m_pSystem->GetILog()->Log( szBuffer  );

	_splitpath( m_file.c_str(), drive, dir, fname, ext );
	_makepath( path_buffer, drive, dir,fname,"log" );
	FILE *hFile = fopen( path_buffer,"at" );
	if (hFile)
	{
		// Write the string to the file and close it
		fprintf(hFile, "%s\n",szBuffer );
		fclose(hFile);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTimeDemoRecorder::OnFrameProfilerPeak( CFrameProfiler *pProfiler,float fPeakTime )
{
	if (m_bPlaying && !m_bPaused)
	{
		LogInfo( "    -Peak at Frame %d, %.2fms : %s (count: %d)",m_currentFrame,fPeakTime,pProfiler->m_name,pProfiler->m_count );
	}
}

void CTimeDemoRecorder::LogRun()
{
	int numFrames = m_records.size();
	LogInfo( "!TimeDemo Run %d Finished.",m_numLoops );
	LogInfo( "    Play Time: %.2fs, Average FPS: %.2f",m_lastPlayedTotalTime,m_lastAveFrameRate );
	LogInfo( "    Min FPS: %.2f at frame %d, Max FPS: %.2f at frame %d",m_minFPS,m_minFPS_Frame,m_maxFPS,m_maxFPS_Frame );
	LogInfo( "    Average Tri/Sec: %d, Tri/Frame: %d",(int)(m_nTotalPolysPlayed/m_lastPlayedTotalTime),m_nTotalPolysPlayed/numFrames );
	LogInfo( "    Recorded/Played Tris ratio: %.2f",(float)m_nTotalPolysRecorded/m_nTotalPolysPlayed );

#if defined(WIN32) && !defined(WIN64)

	// PSAPI is not supported on window9x
	// so, don't use it

	//PROCESS_MEMORY_COUNTERS pc;
	//HANDLE hProcess = GetCurrentProcess();
	//pc.cb = sizeof(pc);
	//GetProcessMemoryInfo( hProcess,&pc,sizeof(pc) );
	//int MB = 1024*1024;
	//LogInfo( "    Memory Usage: WorkingSet=%dMb, PageFile=%dMb, PageFaults=%d",pc.WorkingSetSize/MB,pc.PagefileUsage/MB,pc.PageFaultCount );

#endif
}