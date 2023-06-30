////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   animsequence.cpp
//  Version:     v1.00
//  Created:     29/4/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AnimSequence.h"
#include "SceneNode.h"

//////////////////////////////////////////////////////////////////////////
CAnimSequence::CAnimSequence( IMovieSystem *pMovieSystem )
{
	m_pMovieSystem = pMovieSystem;
	m_flags = 0;
	m_pSceneNode=NULL;
	m_timeRange.Set( 0,10 );
	m_bPaused = false;
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::SetName( const char *name )
{
	m_name = name;
}

//////////////////////////////////////////////////////////////////////////
const char* CAnimSequence::GetName()
{
	return m_name.c_str();
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::SetFlags( int flags )
{
	m_flags = flags;
}

//////////////////////////////////////////////////////////////////////////
int CAnimSequence::GetFlags() const
{
	return m_flags;
}

//////////////////////////////////////////////////////////////////////////
int CAnimSequence::GetNodeCount() const
{
	return m_nodes.size();
}

//////////////////////////////////////////////////////////////////////////
IAnimNode* CAnimSequence::GetNode( int index ) const
{
	assert( index >= 0 && index < (int)m_nodes.size() );
	return m_nodes[index].node;
}

//////////////////////////////////////////////////////////////////////////
IAnimBlock* CAnimSequence::GetAnimBlock( int index ) const
{
	assert( index >= 0 && index < (int)m_nodes.size() );
	return m_nodes[index].anim;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimSequence::AddNode( IAnimNode *node )
{
	assert( node != 0 );

	// Check if this node already in sequence.
	for (int i = 0; i < (int)m_nodes.size(); i++)
	{
		if (node == m_nodes[i].node)
		{
			// Fail to add node second time.
			return false;
		}
	}

	ANode an;
	an.node = node;
	an.anim = new CAnimBlock;//node->CreateAnimBlock();
	an.anim->SetTimeRange( m_timeRange );
	an.node->SetAnimBlock( an.anim );
	m_nodes.push_back( an );

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::RemoveNode( IAnimNode *node )
{
	assert( node != 0 );
	for (int i = 0; i < (int)m_nodes.size(); i++)
	{
		if (node == m_nodes[i].node)
		{
			if (node->GetAnimBlock() == m_nodes[i].anim)
        node->SetAnimBlock( 0 );
			m_nodes.erase( m_nodes.begin()+i );
			if (node && (node==m_pSceneNode))
				m_pSceneNode=NULL;
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
IAnimNode* CAnimSequence::AddSceneNode()
{
	if (m_pSceneNode)
		return NULL;
	m_pSceneNode=new CAnimSceneNode(GetMovieSystem());
	m_pSceneNode->SetName("Scene");
	AddNode(m_pSceneNode);
	return m_pSceneNode;
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::RemoveAll()
{
	m_nodes.clear();
	m_pSceneNode=NULL;
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::Reset()
{
	// Animation sequence should not be animated on Reset.
	Activate();
	
	SAnimContext ec;
	ec.bSingleFrame = true;
	ec.bResetting = true;
	ec.sequence = this;
	ec.time = m_timeRange.start;
	Animate( ec );
	
	Deactivate();
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::Pause()
{
	if (m_bPaused)
		return;
	m_bPaused = true;
	// Detach animation block from all nodes in this sequence.
	for (AnimNodes::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		it->node->Pause();
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::Resume()
{
	if (!m_bPaused)
		return;
	m_bPaused = false;
	// Detach animation block from all nodes in this sequence.
	for (AnimNodes::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		it->node->Resume();
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::Animate( SAnimContext &ec )
{
	ec.sequence = this;
	// Evaluate all animation nodes in sequence.
	for (AnimNodes::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		// Make sure correct animation block is binded to node.
		IAnimBlock *ablock = it->anim;
		if (it->node->GetAnimBlock() != ablock)
		{
			ablock->SetTimeRange( m_timeRange );
			it->node->SetAnimBlock( ablock );
		}
		// Animate node.
		it->node->Animate( ec );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::Activate()
{
	// Assign animation block to all nodes in this sequence.
	for (AnimNodes::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		IAnimBlock *ablock = it->anim;
		ablock->SetTimeRange( m_timeRange );
		it->node->SetAnimBlock( ablock );
	}

	// If this sequence is cut scene disable player.
	if (m_flags & CUT_SCENE)
	{
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::Deactivate()
{
	// Detach animation block from all nodes in this sequence.
	for (AnimNodes::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		it->node->Reset();
		it->node->SetAnimBlock( 0 );
	}
	// If this sequence is cut scene, enable player.
	if (m_flags & CUT_SCENE)
	{
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::Serialize( XmlNodeRef &xmlNode,bool bLoading, bool bLoadEmptyTracks )
{
	if (bLoading)
	{
		// Load.
		RemoveAll();

		Range timeRange;
		const char *name = xmlNode->getAttr( "Name" );
		xmlNode->getAttr( "Flags",m_flags );
		xmlNode->getAttr( "StartTime",timeRange.start );
		xmlNode->getAttr( "EndTime",timeRange.end );

		SetName( name );
		SetTimeRange( timeRange );
		// Loading.
		XmlNodeRef nodes = xmlNode->findChild( "Nodes" );
		if (nodes)
		{
			int id;
			int type;
			for (int i = 0; i < nodes->getChildCount(); i++)
			{
				XmlNodeRef xn = nodes->getChild(i);
				xn->getAttr( "Id",id );
				IAnimNode *node=NULL;
				if (!id)	// scene-node-id
				{
					if (m_pSceneNode)
						node=m_pSceneNode;
					else
						node=AddSceneNode();
				}else
				{
					node = m_pMovieSystem->GetNode(id);
				}
				if (!node)
				{
					if (!xn->getAttr( "Type",type ))
						continue;
					node = m_pMovieSystem->CreateNode( type );
					if (!node)
						continue;
					const char *sName = xn->getAttr( "Name" );
					if (sName)
						node->SetName(sName);
				}

				// Add nodes to sequence and intialize its animation block for this sequence.
				if (node!=m_pSceneNode)
					AddNode(node);

				IAnimBlock *ablock = node->GetAnimBlock();
				if (ablock)
				{
					ablock->Serialize( node,xn,bLoading,bLoadEmptyTracks );
				}
			}
		}
		Deactivate();
		//ComputeTimeRange();
	}
	else
	{
		xmlNode->setAttr( "Name",GetName() );
		xmlNode->setAttr( "Flags",m_flags );
		xmlNode->setAttr( "StartTime",m_timeRange.start );
		xmlNode->setAttr( "EndTime",m_timeRange.end );

		// Save.
		XmlNodeRef nodes = xmlNode->newChild( "Nodes" );
		int num = GetNodeCount();
		for (int i = 0; i < num; i++)
		{
			IAnimNode *node = GetNode(i);
			if (!node)
				continue;
			XmlNodeRef xn = nodes->newChild( "Node" );
			xn->setAttr( "Id",node->GetId() );
			xn->setAttr( "Type",node->GetType() );
			xn->setAttr( "Name",node->GetName() );

			IAnimBlock *ablock = GetAnimBlock(i);
			if (!ablock)
				continue;

			ablock->Serialize( node,xn,bLoading );
		}
		/*
		for (AnimNodes::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
		{
			IAnimNode *node = it->node;
			if (!node)
				continue;
			IAnimBlock *ablock = it->anim;
			XmlNodeRef xn = nodes->newChild( "Node" );
			xn->setAttr( "Id",node->GetId() );
		}
		*/
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::SetTimeRange( Range timeRange )
{
	m_timeRange = timeRange;
	// Set this time range for every track in animation.
		// Set time range to be in range of largest animation track.
	for (AnimNodes::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		IAnimBlock *ablock = it->anim;
		if (!ablock)
			continue;
		ablock->SetTimeRange( timeRange );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::ScaleTimeRange( const Range &timeRange )
{
	// Calculate scale ratio.
	float scale = timeRange.Length() / m_timeRange.Length();
	m_timeRange = timeRange;

	// Set time range to be in range of largest animation track.
	for (AnimNodes::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		IAnimBlock *ablock = it->anim;
		if (!ablock)
			continue;
		
		int paramCount = ablock->GetTrackCount();
		for (int paramIndex = 0; paramIndex < paramCount; paramIndex++)
		{
			int trackType;
			IAnimTrack *pTrack;
			if (!ablock->GetTrackInfo( paramIndex,trackType,&pTrack ))
				continue;
			if (pTrack)
			{
				int nkey = pTrack->GetNumKeys();
				for (int k = 0; k < nkey; k++)
				{
					float keytime = pTrack->GetKeyTime(k);
					keytime = keytime*scale;
					pTrack->SetKeyTime(k,keytime);
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimSequence::ComputeTimeRange()
{
	Range timeRange;
	//timeRange.start = FLT_MAX;
	//timeRange.end = FLT_MIN;

	timeRange = m_timeRange;
	
	// Set time range to be in range of largest animation track.
	for (AnimNodes::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		IAnimBlock *ablock = it->anim;
		if (!ablock)
			continue;
		
		int paramCount = ablock->GetTrackCount();
		for (int paramIndex = 0; paramIndex < paramCount; paramIndex++)
		{
			int trackType;
			IAnimTrack *pTrack;
			if (!ablock->GetTrackInfo( paramIndex,trackType,&pTrack ))
				continue;
			if (pTrack)
			{
				int nkey = pTrack->GetNumKeys();
				if (nkey > 0)
				{
					timeRange.start = min( timeRange.start,pTrack->GetKeyTime(0) );
					timeRange.end = max( timeRange.end,pTrack->GetKeyTime(nkey-1) );
				}
			}
		}
	}

	if (timeRange.start > 0)
		timeRange.start = 0;

	m_timeRange = timeRange;
}