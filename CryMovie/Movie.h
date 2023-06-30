////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   movie.h
//  Version:     v1.00
//  Created:     23/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __movie_h__
#define __movie_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "IMovieSystem.h"

/**	This is descirption of currently playing sequence.
*/
struct PlayingSequence
{
	//! Sequence playnig.
	TSmartPtr<IAnimSequence> sequence;
	//! Current playing time for this sequence.
	float time;
};

struct string_less : public std::binary_function<string,string,bool> 
{
	bool operator()( const string &s1,const string &s2 ) const
	{
		return stricmp(s1.c_str(),s2.c_str()) < 0;
	}
};


class CMovieSystem : public IMovieSystem
{
public:
	CMovieSystem( ISystem *system );
	~CMovieSystem();

	void Release() { delete this; };

	void SetUser(IMovieUser *pUser) { m_pUser=pUser; }
	IMovieUser* GetUser() { return m_pUser; }

	bool Load(const char *pszFile, const char *pszMission);

	ISystem* GetSystem() { return m_system; }

	IAnimNode* CreateNode( int nodeType,int nodeId=0 );
	IAnimTrack* CreateTrack( EAnimTrackType type );

	void ChangeAnimNodeId( int nodeId,int newNodeId );

	IAnimSequence* CreateSequence( const char *sequence );
	IAnimSequence* LoadSequence( const char *pszFilePath );
	IAnimSequence* LoadSequence( XmlNodeRef &xmlNode, bool bLoadEmpty=true );

	void RemoveSequence( IAnimSequence *seq );
	IAnimSequence* FindSequence( const char *sequence );
	ISequenceIt* GetSequences();

	void RemoveAllSequences();
	void RemoveAllNodes();

	void SaveNodes(XmlNodeRef nodesNode);

	//////////////////////////////////////////////////////////////////////////
	// Sequence playback.
	//////////////////////////////////////////////////////////////////////////
	void PlaySequence( const char *sequence,bool bResetFX=true );
	void PlaySequence( IAnimSequence *seq,bool bResetFX=true );
	void PlayOnLoadSequences();

	void StopSequence( const char *sequence );
	void StopSequence( IAnimSequence *seq );
	void StopAllSequences();
	void StopAllCutScenes();
	void Pause( bool bPause );

	void Reset( bool bPlayOnReset=true );
	void Update( float dt );

	bool IsPlaying( IAnimSequence *seq ) const;

	virtual void Pause();
	virtual void Resume();

	//////////////////////////////////////////////////////////////////////////
	// Nodes.
	//////////////////////////////////////////////////////////////////////////
	void RemoveNode( IAnimNode* node );
	IAnimNode* GetNode( int nodeId ) const;
	IAnimNode* FindNode( const char *nodeName ) const;

	void SetRecording( bool recording ) { m_bRecording = recording; };
	bool IsRecording() const { return m_bRecording; };

	void SetCallback( IMovieCallback *pCallback ) { m_pCallback=pCallback; }
	IMovieCallback* GetCallback() { return m_pCallback; }
	void Callback(ECallbackReason Reason);

	void Serialize( XmlNodeRef &xmlNode,bool bLoading, bool bRemoveOldNodes=false, bool bLoadEmpty=true );

	const SCameraParams& GetCameraParams() const { return m_ActiveCameraParams; }
	void SetCameraParams( const SCameraParams &Params );

	void SendGlobalEvent( const char *pszEvent );
	void OnPlaySound( ISound *pSound );
	void SetSequenceStopBehavior( ESequenceStopBehavior behavior );

private:
	ISystem*	m_system;

	IMovieUser *m_pUser;
	IMovieCallback *m_pCallback;

	int m_lastGenId;

	typedef std::vector<TSmartPtr<IAnimSequence> > Sequences;
	Sequences m_sequences;

	typedef std::map<int,TSmartPtr<IAnimNode> > Nodes;
	Nodes m_nodes;

	typedef std::list<PlayingSequence> PlayingSequences;
	PlayingSequences m_playingSequences;
	
	bool m_bRecording;
	bool m_bPaused;

	bool m_bLastFrameAnimateOnStop;

	SCameraParams m_ActiveCameraParams;

	ESequenceStopBehavior m_sequenceStopBehavior;

	static int m_mov_NoCutscenes;
public:
	float GetPlayingTime(IAnimSequence * pSeq);
	bool SetPlayingTime(IAnimSequence * pSeq, float fTime);
};

#endif // __movie_h__
