////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   animationcontext.cpp
//  Version:     v1.00
//  Created:     7/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AnimationContext.h"

#include "IMovieSystem.h"
#include "ITimer.h"
#include "GameEngine.h"
#include "Objects\SelectionGroup.h"
#include "IObjectManager.h"

#include "Viewport.h"
#include "ViewManager.h"

//using namespace std;

//////////////////////////////////////////////////////////////////////////
CAnimationContext::CAnimationContext()
{
	m_paused = 0;
	m_playing = false;
	m_recording = false;
	m_timeRange.Set(0,0);
	m_timeMarker.Set(0,0);
	m_currTime = 0;
	m_fTimeScale = 1.0f;
	m_sequence = 0;
	m_bLooping = false;
	m_bAutoRecording = false;
	m_fRecordingTimeStep = 0;
	m_bEncodeAVI = false;
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::SetSequence( IAnimSequence *seq )
{
	if (m_sequence)
	{
		SetTime(0);
		m_sequence->Deactivate();
	}
	m_sequence = seq;

	if (m_sequence)
	{
		m_timeRange = m_sequence->GetTimeRange();
		m_timeMarker=m_timeRange;
		m_sequence->Activate();
		ForceAnimation();
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::SetTime( float t )
{
	if (t < m_timeRange.start)
		t = m_timeRange.start;

	if (t > m_timeRange.end)
		t = m_timeRange.end;

	if (fabs(m_currTime-t) < 0.001f)
		return;

	m_currTime = t;
	m_fRecordingCurrTime = t;
	ForceAnimation();
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::Pause()
{
	assert( m_paused >= 0 );
	m_paused++;
	
	if (m_recording)
		GetIEditor()->GetMovieSystem()->SetRecording(false);
	
	GetIEditor()->GetMovieSystem()->Pause();
	if (m_sequence)
		m_sequence->Pause();

	if (m_bEncodeAVI)
		PauseAVIEncoding(true);
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::Resume()
{
	assert( m_paused > 0 );
	m_paused--;
	if (m_recording && m_paused == 0)
		GetIEditor()->GetMovieSystem()->SetRecording(true);
	GetIEditor()->GetMovieSystem()->Resume();
	if (m_sequence)
		m_sequence->Resume();

	if (m_bEncodeAVI)
		PauseAVIEncoding(false);
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::SetRecording( bool recording )
{
	if (recording == m_recording)
		return;
	m_paused = 0;
	m_recording = recording;
	m_playing = false;

	if (!recording && m_fRecordingTimeStep != 0)
		SetAutoRecording( false,0 );

	// If started recording, assume we have modified the document.
	GetIEditor()->SetModifiedFlag();

	GetIEditor()->GetMovieSystem()->SetRecording(recording);

	if (m_bEncodeAVI)
		StopAVIEncoding();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CAnimationContext::SetPlaying( bool playing )
{
	if (playing == m_playing)
		return;

	m_paused = 0;
	m_playing = playing;
	m_recording = false;
	GetIEditor()->GetMovieSystem()->SetRecording(false);

	if (playing)
	{
		GetIEditor()->GetMovieSystem()->Resume();
		if (m_sequence)
			m_sequence->Resume();

		if (m_bEncodeAVI)
			StartAVIEncoding();
	}
	else
	{
		GetIEditor()->GetMovieSystem()->Pause();
		GetIEditor()->GetMovieSystem()->SetSequenceStopBehavior( IMovieSystem::ONSTOP_GOTO_START_TIME );
		GetIEditor()->GetMovieSystem()->StopAllSequences();
		GetIEditor()->GetMovieSystem()->SetSequenceStopBehavior( IMovieSystem::ONSTOP_GOTO_END_TIME );
		if (m_sequence)
			m_sequence->Pause();

		if (m_bEncodeAVI)
			StopAVIEncoding();
	}
	/*
	if (!playing && m_sequence != 0)
		m_sequence->Reset();
	*/
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::Update()
{
	if (m_paused > 0 || !(m_playing || m_bAutoRecording))
		return;

	ITimer *pTimer = GetIEditor()->GetSystem()->GetITimer();

	if (!m_bAutoRecording)
	{
		if (m_sequence != NULL)
		{
			SAnimContext ac;
			ac.dt = 0;
			ac.fps = pTimer->GetFrameRate();
			ac.time = m_currTime;
			ac.bSingleFrame = (!m_playing) || (m_fTimeScale!=1.0f);
			m_sequence->Animate( ac );
		}
		float dt = pTimer->GetFrameTime();
		m_currTime += dt * m_fTimeScale;

		if (!m_recording)
			GetIEditor()->GetMovieSystem()->Update(dt);
	}
	else
	{
		float dt = pTimer->GetFrameTime();
		m_fRecordingCurrTime += dt*m_fTimeScale;
		if (fabs(m_fRecordingCurrTime-m_currTime) > m_fRecordingTimeStep)
			m_currTime += m_fRecordingTimeStep;
	}

	if (m_currTime > m_timeMarker.end)
	{
		if (m_bAutoRecording)
		{
			SetAutoRecording(false,0);
		}
		else
		{
			if (m_bLooping)
				m_currTime = m_timeMarker.start;
			else
				SetPlaying(false);
		}
	}

	if (m_bAutoRecording)
	{
		// This is auto recording mode.
		// Send sync with physics event to all selected entities.
		GetIEditor()->GetSelection()->SendEvent( EVENT_PHYSICS_GETSTATE );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::ForceAnimation()
{
	// Before animating node, pause recording.
	Pause();
	if (m_sequence)
	{
		SAnimContext ac;
		ac.dt = 0;
		ac.fps = GetIEditor()->GetSystem()->GetITimer()->GetFrameRate();
		ac.time = m_currTime;
		ac.bSingleFrame = true;
		m_sequence->Animate( ac );
	}
	Resume();
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::ResetAnimations( bool bPlayOnLoad )
{
	if (m_sequence)
	{
		SetTime(0);
	}
	GetIEditor()->GetMovieSystem()->Reset(bPlayOnLoad);
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::SetAutoRecording( bool bEnable,float fTimeStep )
{
	if (bEnable)
	{
		m_bAutoRecording = true;
		m_fRecordingTimeStep = fTimeStep;
		// Turn on fixed time step.
		ICVar *fixed_time_step = GetIEditor()->GetSystem()->GetIConsole()->GetCVar( "fixed_time_step" );
		if (fixed_time_step)
		{
			//fixed_time_step->Set( m_fRecordingTimeStep );
		}
		// Enables physics/ai.
		GetIEditor()->GetGameEngine()->SetSimulationMode(true);
		SetRecording(bEnable);
	}
	else
	{
		m_bAutoRecording = false;
		m_fRecordingTimeStep = 0;
		// Turn off fixed time step.
		ICVar *fixed_time_step = GetIEditor()->GetSystem()->GetIConsole()->GetCVar( "fixed_time_step" );
		if (fixed_time_step)
		{
			//fixed_time_step->Set( 0 );
		}
		// Disables physics/ai.
		GetIEditor()->GetGameEngine()->SetSimulationMode(false);
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::PlayToAVI( bool bEnable,const char *aviFilename )
{
	if (bEnable && aviFilename)
	{
		m_bEncodeAVI = true;
		m_aviFilename = aviFilename;
	}
	else
	{
		m_bEncodeAVI = false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CAnimationContext::IsPlayingToAVI() const
{
	return m_bEncodeAVI;
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::StartAVIEncoding()
{
	CViewport* pViewport = GetIEditor()->GetViewManager()->GetActiveViewport();
	if (pViewport)
	{
		pViewport->StartAVIRecording( m_aviFilename );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::StopAVIEncoding()
{
	CViewport* pViewport = GetIEditor()->GetViewManager()->GetActiveViewport();
	if (pViewport)
	{
		pViewport->StopAVIRecording();
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimationContext::PauseAVIEncoding( bool bPause )
{
	CViewport* pViewport = GetIEditor()->GetViewManager()->GetActiveViewport();
	if (pViewport)
	{
		pViewport->PauseAVIRecording( bPause );
	}
}