////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   scenenode.h
//  Version:     v1.00
//  Created:     23/4/2002 by Lennert.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __scenenode_h__
#define __scenenode_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "AnimNode.h"
#include "SoundTrack.h"

#define SCENE_SOUNDTRACKS 3

struct ISound;

class CAnimSceneNode : public CAnimNode
{
public:
	CAnimSceneNode(IMovieSystem *sys);
	~CAnimSceneNode();

	virtual EAnimNodeType GetType() const { return ANODE_SCENE; }

	//////////////////////////////////////////////////////////////////////////
	// Overrides from CAnimNode
	//////////////////////////////////////////////////////////////////////////
	void Animate( SAnimContext &ec );
	void CreateDefaultTracks();

	// ovverided from IAnimNode
//	IAnimBlock* CreateAnimBlock();
	void Reset();
	void Pause();

	//////////////////////////////////////////////////////////////////////////
	virtual int GetParamCount() const;
	virtual bool GetParamInfo( int nIndex, SParamInfo &info ) const;
	virtual bool GetParamInfoFromId( int paramId, SParamInfo &info ) const;

private:
	void ReleaseSounds();
	void ApplyCameraKey( ISelectKey &key,SAnimContext &ec );
	void ApplyEventKey(IEventKey &key, SAnimContext &ec);
	void ApplyConsoleKey(IConsoleKey &key, SAnimContext &ec);
	void ApplySoundKey( IAnimTrack *pTrack,int nCurrKey,int nLayer, ISoundKey &key, SAnimContext &ec);
	void ApplySequenceKey( IAnimTrack *pTrack,int nCurrKey,ISelectKey &key,SAnimContext &ec );
	void ApplyMusicKey(IMusicKey &key, SAnimContext &ec);

	// Cached parameters of node at givven time.
	float m_time;

	IMovieSystem *m_pMovie;

	bool m_bActive;

	bool m_bMusicMoodSet;

	//! Last animated key in track.
	int m_lastCameraKey;
	int m_lastEventKey;
	int m_lastConsoleKey;
	int m_lastMusicKey;
	int m_lastSequenceKey;
	int m_sequenceCameraId;
	SSoundInfo m_SoundInfo[SCENE_SOUNDTRACKS];
};

#endif // __entitynode_h__
