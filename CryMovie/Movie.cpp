////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   movie.cpp
//  Version:     v1.00
//  Created:     23/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Movie.h"
#include "AnimSplineTrack.h"
#include "AnimSequence.h"
#include "SequenceIt.h"
#include "EntityNode.h"
#include "CVarNode.h"
#include "ScriptVarNode.h"
#include "AnimCameraNode.h"
#include "SceneNode.h"
#include "MaterialNode.h"

#include <ISystem.h>
#include <io.h>
#include <ILog.h>
#include <IConsole.h>
#include <ITimer.h>

int CMovieSystem::m_mov_NoCutscenes = 0;

//////////////////////////////////////////////////////////////////////////
CMovieSystem::CMovieSystem( ISystem *system )
{
	m_system = system;
	m_bRecording = false;
	m_pCallback=NULL;
	m_pUser=NULL;
	m_bPaused = false;
	m_bLastFrameAnimateOnStop = true;
	m_lastGenId = 1;
	m_sequenceStopBehavior = ONSTOP_GOTO_END_TIME;

	system->GetIConsole()->Register( "mov_NoCutscenes",&m_mov_NoCutscenes,0,0,"Disable playing of Cut-Scenes" );
}

//////////////////////////////////////////////////////////////////////////
CMovieSystem::~CMovieSystem()
{
}

//////////////////////////////////////////////////////////////////////////
bool CMovieSystem::Load(const char *pszFile, const char *pszMission)
{
	XmlNodeRef rootNode = m_system->LoadXmlFile(pszFile);
	if (!rootNode)
		return false;
	XmlNodeRef Node=NULL;
	for (int i=0;i<rootNode->getChildCount();i++)
	{
		XmlNodeRef missionNode=rootNode->getChild(i);
		XmlString sName;
		if (!(sName = missionNode->getAttr("Name")))
			continue;
		if (stricmp(sName.c_str(), pszMission))
			continue;
		Node=missionNode;
		break;
	}
	if (!Node)
		return false;
	Serialize(Node, true, true, false);
	return true;
}

//////////////////////////////////////////////////////////////////////////
IAnimNode* CMovieSystem::CreateNode( int nodeType,int nodeId )
{
	CAnimNode *node = NULL;
	if (!nodeId)
	{
		// Make uniq id.
		do {
			nodeId = m_lastGenId++;
		} while (GetNode(nodeId) != 0);
	}
	switch (nodeType)
	{
	case ANODE_ENTITY:
		node = new CAnimEntityNode(this);
		break;
	case ANODE_CAMERA:
		node = new CAnimCameraNode(this);
		break;
	case ANODE_CVAR:
		node = new CAnimCVarNode(this);
		break;
	case ANODE_SCRIPTVAR:
		node = new CAnimScriptVarNode(this);
		break;
	case ANODE_SCENE:
		node = new CAnimSceneNode(this);
		nodeId = 0;
		return node;
		break;
	case ANODE_MATERIAL:
		node = new CAnimMaterialNode(this);
		break;
	}
	if (node)
	{
		node->SetId(nodeId);
		m_nodes[nodeId] = node;
	}
	return node;
}

//////////////////////////////////////////////////////////////////////////
IAnimTrack* CMovieSystem::CreateTrack( EAnimTrackType type )
{
	switch (type)
	{
	case ATRACK_TCB_FLOAT:
		return new CTcbFloatTrack;
	case ATRACK_TCB_VECTOR:
		return new CTcbVectorTrack;
	case ATRACK_TCB_QUAT:
		return new CTcbQuatTrack;
	};
	//ATRACK_TCB_FLOAT,
	//ATRACK_TCB_VECTOR,
	//ATRACK_TCB_QUAT,
	//ATRACK_BOOL,
	// Unknown type of track.
//	CLogFile::WriteLine( "Error: Requesting unknown type of animation track!" );
	assert(0);
	return 0;
}

void CMovieSystem::ChangeAnimNodeId( int nodeId,int newNodeId )
{
	if (nodeId == newNodeId)
		return;
	Nodes::iterator it = m_nodes.find(nodeId);
	if (it != m_nodes.end())
	{
		IAnimNode *node = GetNode( nodeId );
		((CAnimNode*)node)->SetId( newNodeId );
		m_nodes[newNodeId] = node;
		m_nodes.erase(it);
	}

}

//////////////////////////////////////////////////////////////////////////
IAnimSequence* CMovieSystem::CreateSequence( const char *sequenceName )
{
	IAnimSequence *seq = new CAnimSequence( this );
	seq->SetName( sequenceName );
	m_sequences.push_back( seq );
	return seq;
}

//////////////////////////////////////////////////////////////////////////
IAnimSequence* CMovieSystem::LoadSequence( const char *pszFilePath )
{
	XmlNodeRef sequenceNode = m_system->LoadXmlFile( pszFilePath );
	if (sequenceNode)
	{
		return LoadSequence( sequenceNode );
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
IAnimSequence* CMovieSystem::LoadSequence( XmlNodeRef &xmlNode, bool bLoadEmpty )
{
	IAnimSequence *seq = new CAnimSequence( this );
	seq->Serialize( xmlNode,true,bLoadEmpty );
	// Delete previous sequence with the same name.
	IAnimSequence *pPrevSeq = FindSequence( seq->GetName() );
	if (pPrevSeq)
		RemoveSequence( pPrevSeq );
	m_sequences.push_back( seq );
	return seq;
}

//////////////////////////////////////////////////////////////////////////
IAnimSequence* CMovieSystem::FindSequence( const char *sequence )
{
	for (Sequences::iterator it = m_sequences.begin(); it != m_sequences.end(); ++it)
	{
		IAnimSequence *seq = *it;
		if (stricmp(seq->GetName(),sequence) == 0)
		{
			return seq;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
ISequenceIt* CMovieSystem::GetSequences()
{
	CSequenceIt *It=new CSequenceIt();
	for (Sequences::iterator it = m_sequences.begin(); it != m_sequences.end(); ++it)
	{
		It->add( *it );
	}
	return It;
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::RemoveSequence( IAnimSequence *seq )
{
	assert( seq != 0 );
	if (seq)
	{
		IMovieCallback *pCallback=GetCallback();
		SetCallback(NULL);
		StopSequence(seq);

		for (Sequences::iterator it = m_sequences.begin(); it != m_sequences.end(); ++it)
		{
			if (seq == *it)
			{
				m_sequences.erase(it);
				break;
			}
		}
		SetCallback(pCallback);
	}
}

//////////////////////////////////////////////////////////////////////////
IAnimNode* CMovieSystem::GetNode( int nodeId ) const
{
	Nodes::const_iterator it = m_nodes.find(nodeId);
	if (it != m_nodes.end())
		return it->second;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
IAnimNode* CMovieSystem::FindNode( const char *nodeName ) const
{
	for (Nodes::const_iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		IAnimNode *node = it->second;
		// Case insesentivy name comparasion.
		if (stricmp(node->GetName(),nodeName) == 0)
		{
			return node;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::RemoveNode( IAnimNode* node )
{
	assert( node != 0 );

	{
		// Remove this node from all sequences that reference this node.
		for (Sequences::iterator sit = m_sequences.begin(); sit != m_sequences.end(); ++sit)
		{
			(*sit)->RemoveNode( node );
		}
	}

	Nodes::iterator it = m_nodes.find(node->GetId());
	if (it != m_nodes.end())
		m_nodes.erase( it );
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::RemoveAllSequences()
{
	m_bLastFrameAnimateOnStop = false;
	IMovieCallback *pCallback=GetCallback();
	SetCallback(NULL);
	StopAllSequences();
	m_sequences.clear();
	SetCallback(pCallback);
	m_bLastFrameAnimateOnStop = true;
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::RemoveAllNodes()
{
	m_bLastFrameAnimateOnStop = false;
	IMovieCallback *pCallback=GetCallback();
	SetCallback(NULL);
	StopAllSequences();
	m_nodes.clear();
	SetCallback(pCallback);
	m_bLastFrameAnimateOnStop = true;
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::SaveNodes(XmlNodeRef nodesNode)
{
	for (Nodes::iterator It=m_nodes.begin();It!=m_nodes.end();++It)
	{
		XmlNodeRef nodeNode=nodesNode->newChild("Node");
		IAnimNode *pNode=It->second;
		nodeNode->setAttr("Id", pNode->GetId());
		nodeNode->setAttr("Type", pNode->GetType());
		nodeNode->setAttr("Name", pNode->GetName());
		switch (pNode->GetType())
		{
			case ANODE_CAMERA:	// FALL THROUGH
			case ANODE_ENTITY:
				IAnimNode *pTgt=pNode->GetTarget();
				if (pTgt)
					nodeNode->setAttr("TargetId", pTgt->GetId());
				break;
		}
		pNode->Serialize( nodeNode,false );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::PlaySequence( const char *sequenceName,bool bResetFx )
{
	IAnimSequence *seq = FindSequence(sequenceName);
	if (seq)
	{ 
		PlaySequence(seq,bResetFx);
	}
	else
		GetSystem ()->GetILog()->Log ("CMovieSystem::PlaySequence: Error: Sequence \"%s\" not found", sequenceName);
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::PlaySequence( IAnimSequence *seq,bool bResetFx )
{
	assert( seq != 0 );
	if (!seq || IsPlaying(seq))
		return;

	if ((seq->GetFlags() & IAnimSequence::CUT_SCENE) || (seq->GetFlags() & IAnimSequence::NO_HUD))
	{
		// Dont play cut-scene if this console variable set.
		if (m_mov_NoCutscenes != 0)
			return;
	}

	//GetSystem ()->GetILog()->Log ("TEST: Playing Sequence (%s)", seq->GetName());

	// If this sequence is cut scene disable player.
	if (seq->GetFlags() & IAnimSequence::CUT_SCENE)
	{
		if (m_pUser)
			m_pUser->BeginCutScene(seq->GetFlags(),bResetFx);
	}

	seq->Activate();
	PlayingSequence ps;
	ps.sequence = seq;
	ps.time = seq->GetTimeRange().start;
	m_playingSequences.push_back(ps);
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::StopSequence( const char *sequenceName )
{
	IAnimSequence *seq = FindSequence(sequenceName);
	if (seq)
		StopSequence(seq);
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::StopSequence( IAnimSequence *seq )
{
	assert( seq != 0 );
	for (PlayingSequences::iterator it = m_playingSequences.begin(); it != m_playingSequences.end(); ++it)
	{
		if (it->sequence == seq)
		{
			m_playingSequences.erase( it );

			if (m_bLastFrameAnimateOnStop)
			{
				if (m_sequenceStopBehavior == ONSTOP_GOTO_END_TIME)
				{
					SAnimContext ac;
					ac.bSingleFrame = true;
					ac.time = seq->GetTimeRange().end;
					seq->Animate(ac);
				}
				else if (m_sequenceStopBehavior == ONSTOP_GOTO_START_TIME)
				{
					SAnimContext ac;
					ac.bSingleFrame = true;
					ac.time = seq->GetTimeRange().start;
					seq->Animate(ac);
				}
				seq->Deactivate();
			}
			
			// If this sequence is cut scene end it.
			if (seq->GetFlags() & IAnimSequence::CUT_SCENE)
			{
				if (m_pUser)
					m_pUser->EndCutScene();
			}
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::StopAllSequences()
{
	while (!m_playingSequences.empty())
	{
		StopSequence( m_playingSequences.begin()->sequence );
	}
	m_playingSequences.clear();
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::StopAllCutScenes()
{
	PlayingSequences::iterator next;
	for (PlayingSequences::iterator it = m_playingSequences.begin(); it != m_playingSequences.end(); it = next)
	{
		next = it; ++next;
		IAnimSequence *seq = it->sequence;
		if (seq->GetFlags() & IAnimSequence::CUT_SCENE)
			StopSequence( seq );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CMovieSystem::IsPlaying( IAnimSequence *seq ) const
{
	for (PlayingSequences::const_iterator it = m_playingSequences.begin(); it != m_playingSequences.end(); ++it)
	{
		if (it->sequence == seq)
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::Reset( bool bPlayOnReset )
{
	m_bLastFrameAnimateOnStop = false;
	StopAllSequences();
	m_bLastFrameAnimateOnStop = true;

	// Reset all sequences.
	for (Sequences::iterator sit = m_sequences.begin(); sit != m_sequences.end(); ++sit)
	{
		IAnimSequence *seq = *sit;
		seq->Reset();
	}

	// Reset all nodes.
	for (Nodes::const_iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		IAnimNode *node = it->second;
		node->Reset();
	}

	// Force end Cut-Scene on the reset.
/*	if (m_pUser)	// lennert why is this here ??? if there was a cutscene playing it will be stopped above...
	{
		m_pUser->EndCutScene();
	}*/

	if (bPlayOnReset)
	{
		for (Sequences::iterator sit = m_sequences.begin(); sit != m_sequences.end(); ++sit)
		{
			IAnimSequence *seq = *sit;
			if (seq->GetFlags() & IAnimSequence::PLAY_ONRESET)
				PlaySequence(seq);
		}
	}

	// Reset camera.
	SCameraParams CamParams=GetCameraParams();
	CamParams.cameraNode=NULL;
	CamParams.nCameraId=0;
	SetCameraParams(CamParams);
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::PlayOnLoadSequences()
{
	for (Sequences::iterator sit = m_sequences.begin(); sit != m_sequences.end(); ++sit)
	{
		IAnimSequence *seq = *sit;
		if (seq->GetFlags() & IAnimSequence::PLAY_ONRESET)
			PlaySequence(seq);
	}

	// Reset camera.
	SCameraParams CamParams=GetCameraParams();
	CamParams.cameraNode=NULL;
	CamParams.nCameraId=0;
	SetCameraParams(CamParams);
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::Update( float dt )
{
	if (m_bPaused)
		return;

	SAnimContext ac;
	float fps = 60;
	Range timeRange;

	std::vector<IAnimSequence*> stopSequences;

	// cap delta time.
	dt = max( 0,min(0.5f,dt) );
	
	PlayingSequences::iterator next;
	for (PlayingSequences::iterator it = m_playingSequences.begin(); it != m_playingSequences.end(); it = next)
	{
		next = it; ++next;

		PlayingSequence &ps = *it;

		ac.time = ps.time;
		ac.sequence = ps.sequence;
		ac.dt = dt;
		ac.fps = fps;

		// Increase play time.
		ps.time += dt;

		// Check time out of range.
		timeRange = ps.sequence->GetTimeRange();
		if (ps.time > timeRange.end)
		{
			int seqFlags = ps.sequence->GetFlags();
			if (seqFlags & IAnimSequence::ORT_LOOP)
			{
				// Time wrap's back to the start of the time range.
				ps.time = timeRange.start;
			}
			else if (seqFlags & IAnimSequence::ORT_CONSTANT)
			{
				// Time just continues normally past the end of time range.
			}
			else
			{
				// If no out-of-range type specified sequence stopped when time reaches end of range.
				// Que sequence for stopping.
				stopSequences.push_back(ps.sequence);
				continue;
			}
		}

		// Animate sequence. (Can invalidate iterator)
		ps.sequence->Animate( ac );
	}

	// Stop quied sequencs.
	for (int i = 0; i < (int)stopSequences.size(); i++)
	{
		StopSequence( stopSequences[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::Callback(ECallbackReason Reason)
{
	if (!m_pCallback)
		return;
	switch (Reason)
	{
		case CBR_ADDNODE:
			m_pCallback->OnAddNode();
			break;
		case CBR_REMOVENODE:
			m_pCallback->OnRemoveNode();
			break;
		case CBR_CHANGENODE:
			m_pCallback->OnChangeNode();
			break;
		case CBR_REGISTERNODECB:
			m_pCallback->OnRegisterNodeCallback();
			break;
		case CBR_UNREGISTERNODECB:
			m_pCallback->OnUnregisterNodeCallback();
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::Serialize( XmlNodeRef &xmlNode,bool bLoading,bool bRemoveOldNodes,bool bLoadEmpty )
{
	if (bLoading)
	{
		RemoveAllSequences();
		if (bRemoveOldNodes)
		{
			RemoveAllNodes();
		}
		//////////////////////////////////////////////////////////////////////////
		// Load animation nodes from XML.
		//////////////////////////////////////////////////////////////////////////
		XmlNodeRef nodeNode=xmlNode->findChild("NodeData");
		if (nodeNode)
		{
			std::map<int,int> mapNodeTarget;
			for (int i=0;i<nodeNode->getChildCount();i++)
			{
				XmlNodeRef node=nodeNode->getChild(i);
				IAnimNode *pAnimNode = NULL;
				string sTarget;

				int nodeId = atoi(node->getAttr("Id"));
				// If node with such ID already exists. skip it.
				if (!GetNode(nodeId))
				{
					int nodeType = atoi(node->getAttr("Type"));
					pAnimNode = CreateNode( nodeType,nodeId );
					if (pAnimNode)
					{
						pAnimNode->SetName(node->getAttr("Name"));
						int entityId = -1;
						if (node->getAttr("EntityId",entityId))
						{
							pAnimNode->SetEntity(entityId);
						}
						pAnimNode->Serialize( node,true );

						int targetId = -1;
						if (node->getAttr("TargetId",targetId))
						{
							if (!sTarget.empty())
								mapNodeTarget.insert(std::map<int,int>::value_type( pAnimNode->GetId(), targetId ));
						}
					}
				}
			}
			// After all nodes loaded,Bind targets.
			for (std::map<int,int>::iterator It=mapNodeTarget.begin();It!=mapNodeTarget.end();++It)
			{
				IAnimNode *pAnimNode=GetNode(It->first);
				assert(pAnimNode);
				pAnimNode->SetTarget(GetNode(It->second));
			}
		}
		//////////////////////////////////////////////////////////////////////////
		// Load sequences from XML.
		//////////////////////////////////////////////////////////////////////////
		XmlNodeRef seqNode=xmlNode->findChild("SequenceData");
		if (seqNode)
		{
			for (int i=0;i<seqNode->getChildCount();i++)
			{
				if (!LoadSequence(seqNode->getChild(i), bLoadEmpty))
					return;
			}
		}
		//Reset();
	}else
	{
		// Save animation nodes to xml.
		XmlNodeRef nodesNode = xmlNode->newChild("NodeData");
		for (Nodes::iterator nodeIt = m_nodes.begin(); nodeIt != m_nodes.end(); ++nodeIt)
		{
			XmlNodeRef nodeNode = nodesNode->newChild("Node");
			IAnimNode *pNode = nodeIt->second;
			pNode->Serialize( nodeNode,false );
		}

		XmlNodeRef sequencesNode=xmlNode->newChild("SequenceData");
		ISequenceIt *It=GetSequences();
		IAnimSequence *seq=It->first();;
		while (seq)
		{
			XmlNodeRef sequenceNode=sequencesNode->newChild("Sequence");
			seq->Serialize(sequenceNode, false);
			seq=It->next();
		}
		It->Release();
	}
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::SetCameraParams( const SCameraParams &Params )
{
	m_ActiveCameraParams = Params;
	if (m_pUser)
		m_pUser->SetActiveCamera(m_ActiveCameraParams);
	if (m_pCallback)
		m_pCallback->OnSetCamera( m_ActiveCameraParams );
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::SendGlobalEvent( const char *pszEvent )
{
	if (m_pUser)
		m_pUser->SendGlobalEvent(pszEvent);
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::Pause()
{
	if (m_bPaused)
		return;
	m_bPaused = true;

	/*
	PlayingSequences::iterator next;
	for (PlayingSequences::iterator it = m_playingSequences.begin(); it != m_playingSequences.end(); it = next)
	{
		next = it; ++next;
		PlayingSequence &ps = *it;
		ps.sequence->Pause();
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::Resume()
{
	if (!m_bPaused)
		return;

	m_bPaused = false;

	/*
	PlayingSequences::iterator next;
	for (PlayingSequences::iterator it = m_playingSequences.begin(); it != m_playingSequences.end(); it = next)
	{
		next = it; ++next;
		PlayingSequence &ps = *it;
		ps.sequence->Resume();
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CMovieSystem::OnPlaySound( ISound *pSound )
{
	if (m_pUser)
		m_pUser->PlaySubtitles( pSound );
}

float CMovieSystem::GetPlayingTime(IAnimSequence * pSeq)
{
	if (!pSeq)
		return -1;

	if (!IsPlaying(pSeq))
		return -1;

	PlayingSequences::const_iterator itend = m_playingSequences.end();
	for (PlayingSequences::const_iterator it = m_playingSequences.begin(); it != itend; ++it)
	{
		if (it->sequence == pSeq)
			return it->time;
	}

	return -1;
}

bool CMovieSystem::SetPlayingTime(IAnimSequence * pSeq, float fTime)
{
	if (!pSeq)
		return false;

	if (!IsPlaying(pSeq))
		return false;

	PlayingSequences::iterator itend = m_playingSequences.end();
	for (PlayingSequences::iterator it = m_playingSequences.begin(); it != itend; ++it)
	{
		if (it->sequence == pSeq)
			it->time = fTime;
	}


	return false;
}

void CMovieSystem::SetSequenceStopBehavior( ESequenceStopBehavior behavior )
{
	m_sequenceStopBehavior = behavior;
}