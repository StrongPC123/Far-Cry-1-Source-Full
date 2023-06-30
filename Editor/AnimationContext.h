////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   animationcontext.h
//  Version:     v1.00
//  Created:     7/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __animationcontext_h__
#define __animationcontext_h__

#if _MSC_VER > 1000
#pragma once
#endif

struct IAnimSequence;

/**	CAnimationContext stores information about current editable animation sequence.
		Stores information about whenever animation is being recorded know,
		currect sequence, current time in sequence etc.
*/
class CAnimationContext
{
public:
	//////////////////////////////////////////////////////////////////////////
	// Constructors.
	//////////////////////////////////////////////////////////////////////////
	/** Constructor.
	*/
	CAnimationContext();

	//////////////////////////////////////////////////////////////////////////
	// Accessors
	//////////////////////////////////////////////////////////////////////////
	
	/** Return current animation time in active sequence.
		@return Current time.
	*/
	float GetTime() const { return m_currTime; };

	void SetTimeScale(float fScale) { m_fTimeScale=fScale; }

	/** Get currently edited sequence.
	*/
	IAnimSequence* GetSequence() const { return m_sequence; };

	/** Set time markers to play within.
	*/
	void SetMarkers(Range Marker) { m_timeMarker=Marker; }

	/** Get time markers to play within.
	*/
	Range GetMarkers() { return m_timeMarker; }
	
	/** Get time range of active animation sequence.
	*/
	Range GetTimeRange() const { return m_timeRange; }

	/** Returns true if editor is recording animations now.
	*/
	bool IsRecording() const { return m_recording && m_paused==0; };

	/** Returns true if editor is playing animation now.
	*/
	bool IsPlaying() const { return m_playing && m_paused==0; };

	/** Returns true if currently playing or recording is paused.
	*/
	bool IsPaused() const { return m_paused > 0; }

	/** Return if animation context is now in playing mode.
			In difference from IsPlaying function this function not affected by pause state.
	*/
	bool IsPlayMode() const { return m_playing; };

	/** Return if animation context is now in recording mode.
			In difference from IsRecording function this function not affected by pause state.
	*/
	bool IsRecordMode() const { return m_recording; };

	/** Returns true if currently looping as activated.
	*/
	bool IsLoopMode() const { return m_bLooping; }

	/** Enable/Disable looping.
	*/
	void SetLoopMode(bool bLooping) { m_bLooping=bLooping; }
	
	//////////////////////////////////////////////////////////////////////////
	// Operators
	//////////////////////////////////////////////////////////////////////////

	/** Set current animation time in active sequence.
		@param seq New active time.
	*/
	void SetTime( float t );

	/** Set active editing sequence.
		@param seq New active sequence.
	*/
	void SetSequence( IAnimSequence *seq );

	/** Start animation recorduing.
		Automatically stop playing.
		@param recording True to start recording, false to stop.
	*/
	void SetRecording( bool playing );

	/** Enables/Disables automatic recording, sets the time step for each recorded frame.
	*/
	void SetAutoRecording( bool bEnable,float fTimeStep );
	
	//! Check if autorecording enabled.
	bool IsAutoRecording() const { return m_bAutoRecording; };

	/** Start/Stop animation playing.
		Automatically stop recording.
		@param playing True to start playing, false to stop.
	*/
	void SetPlaying( bool playing );

	/** Pause animation playing/recording.
	*/
	void Pause();

	/** Resume animation playing/recording.
	*/
	void Resume();

	/** Called every frame to update all animations if animation should be playing.
	*/
	void Update();

	/** Force animation for current sequence.
	*/
	void ForceAnimation();

	/** Reset all animation sequences to zero time.
	*/
	void ResetAnimations( bool bPlayOnLoad );

	//////////////////////////////////////////////////////////////////////////
	// Enables playback of sequence with encoding to the AVI.
	// Arguments:
	//		bEnable - Enables/Disables writing to the AVI when playing sequence.
	//		aviFilename - Filename of avi file, must be with .avi extension.
	void PlayToAVI( bool bEnable,const char *aviFilename=NULL );
	bool IsPlayingToAVI() const;

private:
	//////////////////////////////////////////////////////////////////////////
	void StartAVIEncoding();
	void StopAVIEncoding();
	void PauseAVIEncoding( bool bPause );

	//! Current time within active animation sequence.
	float m_currTime;

	float m_fTimeScale;

	// Recording time step.
	float m_fRecordingTimeStep;
	float m_fRecordingCurrTime;

	bool m_bAutoRecording;

	//! Time range of active animation sequence.
	Range m_timeRange;

	Range m_timeMarker;

	//! Currently active animation sequence.
	TSmartPtr<IAnimSequence> m_sequence;

	bool m_bLooping;
	
	//! True if editor is recording animations now.
	bool m_recording;

	//! True if editor is plaing animation now.
	bool m_playing;

	//! Stores how many times animation have been paused prior to calling resume.
	int m_paused;

	//////////////////////////////////////////////////////////////////////////
	//! When set encoding to the AVI while playing.
	bool m_bEncodeAVI;
	//! Name of the AVI file.
	CString m_aviFilename;
};

#endif // __animationcontext_h__
