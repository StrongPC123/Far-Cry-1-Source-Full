////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   timedemorecorder.h
//  Version:     v1.00
//  Created:     2/8/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __timedemorecorder_h__
#define __timedemorecorder_h__
#pragma once

class CTimeDemoRecorder : public IFrameProfilePeakCallback
{
public:
	CTimeDemoRecorder( ISystem *pSystem );
	~CTimeDemoRecorder();

	void Update();
	void RenderInfo();

	void Record( bool bEnable );
	void Play( bool bEnable );
	bool IsRecording() const { return m_bRecording; };
	bool IsPlaying() const { return m_bPlaying; };

	//! Get number of frames in record.
	int GetNumFrames() const;
	float GetAverageFrameRate() const;

	void Save( const char *filename );
	void Load(  const char *filename );

	//////////////////////////////////////////////////////////////////////////
	// Implements IFrameProfilePeakCallback interface.
	//////////////////////////////////////////////////////////////////////////
	virtual void OnFrameProfilerPeak( CFrameProfiler *pProfiler,float fPeakTime );

private:
	void RecordFrame();
	void PlayFrame();

	float GetTime();
	// Set Value of console variable.
	void SetConsoleVar( const char *sVarName,float value );
	// Get value of console variable.
	float GetConsoleVar( const char *sVarName );

	void StartSession();
	void StopSession();

	void LogRun();
	void LogInfo( const char *format,... );

	//! This sructure saved for every frame of time demo.
	struct FrameRecord
	{
		Vec3 playerPos;
		Vec3 playerAngles;
		float frameTime; // Immidiate frame rate, for this frame.
		
		// Snapshot of current processing command.
		unsigned int nActionFlags[2];
		float fLeaning;
		int nPolygons; // Polys rendered in this frame.
	};
	typedef std::list<FrameRecord> FrameRecords;
	FrameRecords m_records;

	bool m_bRecording;
	bool m_bPlaying;
	bool m_bPaused;

	//! Current play or record frame.
	int m_currentFrame;
	FrameRecords::iterator m_currFrameIter;

	//////////////////////////////////////////////////////////////////////////
	// Old values of console vars.
	//////////////////////////////////////////////////////////////////////////
	float m_fixedTimeStep;
	float m_oldPeakTolerance;

	//! Timings.
	float m_recordStartTime;
	float m_recordEndTime;
	float m_recordLastFrameTime;
	float m_lastFrameTime;
	float m_totalDemoTime;
	float m_recordedDemoTime;

	// How many polygons per frame where recorded.
	int m_nTotalPolysRecorded;
	// How many polygons per frame where played.
	int m_nTotalPolysPlayed;
	
	float m_lastPlayedTotalTime;
	float m_lastAveFrameRate;
	float m_minFPS;
	float m_maxFPS;
	float m_currFPS;
	int m_minFPS_Frame;
	int m_maxFPS_Frame;

	int m_nCurrPolys;
	int m_nMaxPolys;
	int m_nMinPolys;
	int m_nPolysPerSec;
	int m_nPolysCounter;
	
	// For calculating current last second fps.
	float m_lastFpsTimeRecorded;
	int m_fpsCounter;

	int m_numLoops;
	int m_maxLoops;

	int m_demo_scroll_pause;
	int m_demo_noinfo;
	int m_demo_quit;
	int m_demo_screenshot_frame;

	String m_file;

	ISystem *m_pSystem;
	IGame *m_pGame;
};

#endif // __timedemorecorder_h__
