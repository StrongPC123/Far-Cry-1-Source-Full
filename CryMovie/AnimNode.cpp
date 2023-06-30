////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   animnode.cpp
//  Version:     v1.00
//  Created:     23/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AnimNode.h"
#include "Animtrack.h"
#include "CharacterTrack.h"
#include "AnimSplineTrack.h"
#include "BoolTrack.h"
#include "SelectTrack.h"
#include "EventTrack.h"
#include "SoundTrack.h"
#include "ExprTrack.h"
#include "ConsoleTrack.h"
#include "MusicTrack.h"

//////////////////////////////////////////////////////////////////////////
// CAnimBlock
//////////////////////////////////////////////////////////////////////////
int CAnimBlock::GetTrackCount() const
{
	return m_tracks.size();
}

//////////////////////////////////////////////////////////////////////////
bool CAnimBlock::GetTrackInfo( int index,int &paramId,IAnimTrack **pTrack ) const
{
	if (index < 0 && index >= m_tracks.size())
	{
		return false;
	}
	paramId = m_tracks[index].paramId;
	if (pTrack)
		*pTrack = m_tracks[index].track;
	return true;
}

const char* CAnimBlock::GetParamName( AnimParamType param ) const
{
	switch (param)
	{
	case APARAM_FOV:
		return "FOV";
	case APARAM_POS:
		return "Position";
	case APARAM_ROT:
		return "Rotation";
	case APARAM_SCL:
		return "Scale";
	case APARAM_VISIBLE:
		return "Visiblity";
	case APARAM_EVENT:
		return "Events";
	case APARAM_CAMERA:
		return "Camera";
	
	// Sound tracks.
	case APARAM_SOUND1:
		return "Sound1";
	case APARAM_SOUND2:
		return "Sound2";
	case APARAM_SOUND3:
		return "Sound3";

	// Character tracks.
	case APARAM_CHARACTER1:
		return "Animation1";
	case APARAM_CHARACTER2:
		return "Animation2";
	case APARAM_CHARACTER3:
		return "Animation3";

	case APARAM_EXPRESSION1:
		return "Expression1";
	case APARAM_EXPRESSION2:
		return "Expression2";
	case APARAM_EXPRESSION3:
		return "Expression3";

	case APARAM_SEQUENCE:
		return "Sequence";
	case APARAM_CONSOLE:
		return "Console";
	case APARAM_MUSIC:
		return "Music";

	case APARAM_FLOAT_1:
		return "Value";
	}
	return "Unknown";
}

IAnimTrack* CAnimBlock::GetTrack( int param ) const
{
	for (int i = 0; i < m_tracks.size(); i++)
	{
		if (m_tracks[i].paramId == param)
			return m_tracks[i].track;
	}
	return 0;
}

void CAnimBlock::SetTrack( int param,IAnimTrack *track )
{
	if (track)
	{
		for (int i = 0; i < m_tracks.size(); i++)
		{
			if (m_tracks[i].paramId == param)
			{
				m_tracks[i].track = track;
				return;
			}
		}
		AddTrack( param,track );
	}
	else
	{
		// Remove track at this id.
		for (int i = 0; i < m_tracks.size(); i++)
		{
			if (m_tracks[i].paramId == param)
			{
				m_tracks.erase( m_tracks.begin() + i );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimBlock::AddTrack( int param,IAnimTrack *track )
{
	TrackDesc td;
	td.paramId = param;
	td.track = track;
	m_tracks.push_back(td);
}

//////////////////////////////////////////////////////////////////////////
bool CAnimBlock::RemoveTrack( IAnimTrack *pTrack )
{
	for (int i = 0; i < m_tracks.size(); i++)
	{
		if (m_tracks[i].track == pTrack)
		{
			m_tracks.erase( m_tracks.begin() + i );
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
IAnimTrack* CAnimBlock::CreateTrack( int paramId,EAnimValue valueType )
{
	IAnimTrack *pTrack = NULL;
	switch (valueType)
	{
	case AVALUE_FLOAT:   pTrack=new CTcbFloatTrack; break;
	case AVALUE_VECTOR:  pTrack=new CTcbVectorTrack; break;
	case AVALUE_QUAT:    pTrack=new CTcbQuatTrack; break;
	case AVALUE_EVENT:   pTrack=new CEventTrack; break;
	case AVALUE_BOOL:    pTrack=new CBoolTrack; break;
	case AVALUE_SELECT:  pTrack=new CSelectTrack; break;
	case AVALUE_SOUND:   pTrack=new CSoundTrack; break;
	case AVALUE_CHARACTER: pTrack=new CCharacterTrack; break;
	case AVALUE_EXPRESSION:pTrack=new CExprTrack; break;
	case AVALUE_CONSOLE: pTrack=new CConsoleTrack; break;
	case AVALUE_MUSIC:   pTrack=new CMusicTrack; break;
	}
	if (pTrack)
		AddTrack( paramId,pTrack );
	return pTrack;
}

//////////////////////////////////////////////////////////////////////////
void CAnimBlock::Serialize( IAnimNode *pNode,XmlNodeRef &xmlNode,bool bLoading, bool bLoadEmptyTracks )
{
	if (bLoading)
	{
		// Delete all tracks.
		m_tracks.clear();

		IAnimNode::SParamInfo info;
		// Loading.
		int paramId = -1;
		int num = xmlNode->getChildCount();
		for (int i = 0; i < num; i++)
		{
			XmlNodeRef trackNode = xmlNode->getChild(i);
			trackNode->getAttr( "ParamId",paramId );
			if (!pNode->GetParamInfoFromId( paramId,info ))
				continue;
			
			IAnimTrack *track = CreateTrack( paramId,info.valueType );
			if (track)
			{
				if (!track->Serialize( trackNode,bLoading,bLoadEmptyTracks ))
				{
					// Boolean tracks must always be loaded even if empty.
					if (track->GetType() != ATRACK_BOOL)
					{
						RemoveTrack(track);
					}
				}
			}
		}
	}
	else
	{
		// Saving.
		for (int i = 0; i < m_tracks.size(); i++)
		{
			IAnimTrack *track = m_tracks[i].track;
			if (track)
			{
				int paramid = m_tracks[i].paramId;
				XmlNodeRef trackNode = xmlNode->newChild( "Track" );
				trackNode->setAttr( "ParamId",m_tracks[i].paramId );
				track->Serialize( trackNode,bLoading );
			}
		}
	}
}

void CAnimBlock::SetTimeRange( Range timeRange )
{
	for (int i = 0; i < m_tracks.size(); i++)
	{
		if (m_tracks[i].track)
		{
			m_tracks[i].track->SetTimeRange( timeRange );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// CAnimNode.
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
CAnimNode::CAnimNode( IMovieSystem *sys )
{
	m_dwSupportedTracks=0;
	m_id = 0;
	m_animBlock = 0;
	m_callback = 0;
	m_parent = 0;
	m_pMovieSystem = sys;
	m_pMovieSystem->Callback(CBR_ADDNODE);
	m_flags = 0;
	strcpy(m_name,"");
}

//////////////////////////////////////////////////////////////////////////
CAnimNode::~CAnimNode()
{
	m_pMovieSystem->Callback(CBR_REMOVENODE);
}

//////////////////////////////////////////////////////////////////////////
void CAnimNode::SetFlags( int flags )
{
	m_flags = flags;
}

//////////////////////////////////////////////////////////////////////////
int CAnimNode::GetFlags() const
{
	return m_flags;
}

//////////////////////////////////////////////////////////////////////////
int CAnimNode::FindTrack(IAnimTrack *pInTrack)
{
	if (!m_animBlock)
		return -1;
	
	int paramCount = m_animBlock->GetTrackCount();
	for (int paramIndex = 0; paramIndex < paramCount; paramIndex++)
	{
		int paramId;
		IAnimTrack *pTrack;
		if (!m_animBlock->GetTrackInfo( paramIndex,paramId,&pTrack ))
			continue;
		if (pTrack == pInTrack)
			return paramId;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
IAnimTrack* CAnimNode::CreateTrack( int paramId )
{
	if (!m_animBlock)
		return 0;
	
	SParamInfo info;
	if (GetParamInfoFromId(paramId,info))
	{
		IAnimTrack *pTrack = m_animBlock->CreateTrack( paramId,info.valueType );
		if (pTrack)
			m_pMovieSystem->Callback(CBR_CHANGENODE);
		return pTrack;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimNode::RemoveTrack( IAnimTrack *pTrack )
{
	if (!m_animBlock)
		return false;
	if (m_animBlock->RemoveTrack( pTrack ))
	{
		m_pMovieSystem->Callback(CBR_CHANGENODE);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
IAnimNode* CAnimNode::GetChild( int i ) const
{
	assert( i >= 0 && i < (int)m_childs.size() );
	return m_childs[i];
}

//////////////////////////////////////////////////////////////////////////
bool CAnimNode::IsChildOf( IAnimNode *node )
{
	CAnimNode *p = m_parent;
	while (p && p != node) {
		p = p->m_parent;
	}
	if (p == node)
		return true;
	return false;

}
	
//////////////////////////////////////////////////////////////////////////
void CAnimNode::AttachChild( IAnimNode* child )
{
	assert( child );
	if (!child)
		return;

	CAnimNode *childNode = (CAnimNode*)child;

	// If not already attached to this node.
	if (childNode->m_parent == this)
		return;

	// Add to child list first to make sure node not get deleted while reattaching.
	m_childs.push_back( childNode );
	if (childNode->m_parent)
		childNode->DetachThis();	// Detach node if attached to other parent.	
	childNode->m_parent = this;	// Assign this node as parent to child node.
}

//////////////////////////////////////////////////////////////////////////
void CAnimNode::DetachAll()
{
	for (Childs::iterator c = m_childs.begin(); c != m_childs.end(); c++)
	{
		(*c)->m_parent = 0;
	}
	m_childs.clear();
}

//////////////////////////////////////////////////////////////////////////
void CAnimNode::DetachThis()
{
	if (m_parent)
	{
		// Copy parent to temp var, erasing child from parent may delete this node if child referenced only from parent.
		CAnimNode* parent = m_parent;
		m_parent = 0;
		parent->RemoveChild( this );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimNode::RemoveChild( CAnimNode *node )
{
	Childs::iterator it = std::find( m_childs.begin(),m_childs.end(),node );
	if (it != m_childs.end())
		m_childs.erase( it );
}

//////////////////////////////////////////////////////////////////////////
void CAnimNode::Animate( SAnimContext &ec )
{
}

//////////////////////////////////////////////////////////////////////////
bool CAnimNode::IsParamValid( int paramId ) const
{
	SParamInfo info;
	if (GetParamInfoFromId(paramId,info))
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimNode::SetParamValue( float time,AnimParamType param,float value )
{
	IAnimBlock *anim = GetAnimBlock();
	if (anim)
	{
		IAnimTrack *track = anim->GetTrack(param);
		if (track && track->GetValueType() == AVALUE_FLOAT)
		{
			// Float track.
			bool bDefault = !(m_pMovieSystem->IsRecording() && (m_flags&ANODE_FLAG_SELECTED)); // Only selected nodes can be recorded
			track->SetValue( time,value,bDefault );
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimNode::GetParamValue( float time,AnimParamType param,float &value )
{
	IAnimBlock *anim = GetAnimBlock();
	if (anim)
	{
		IAnimTrack *track = anim->GetTrack(param);
		if (track && track->GetValueType() == AVALUE_FLOAT)
		{
			// Float track.
			track->GetValue( time,value );
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
IAnimTrack* CAnimNode::GetTrack( int nParamId ) const
{
	if (!m_animBlock)
		return 0;
	return m_animBlock->GetTrack( nParamId );
}

//////////////////////////////////////////////////////////////////////////
void CAnimNode::SetTrack( int nParamId,IAnimTrack *track )
{
	if (!m_animBlock)
		return;
	m_animBlock->SetTrack( nParamId,track );
}

//////////////////////////////////////////////////////////////////////////
void CAnimNode::Serialize( XmlNodeRef &xmlNode,bool bLoading )
{
	if (bLoading)
	{
		xmlNode->getAttr( "Id",m_id );
		const char *name = xmlNode->getAttr("Name");
		if (name)
			SetName(name);
	}
	else
	{
		xmlNode->setAttr( "Id",m_id );
		xmlNode->setAttr("Type", GetType() );
		xmlNode->setAttr("Name", GetName() );
		IAnimNode *pTgt = GetTarget();
		if (pTgt)
		{
			xmlNode->setAttr("TargetId", pTgt->GetId());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimNode::SetAnimBlock( IAnimBlock *block )
{
	m_animBlock = block;
	if (m_animBlock)
		m_animBlock->SetId( m_id );
};