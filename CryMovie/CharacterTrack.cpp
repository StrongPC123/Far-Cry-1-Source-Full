////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   charactertrack.h.cpp
//  Version:     v1.00
//  Created:     20/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "CharacterTrack.h"

//////////////////////////////////////////////////////////////////////////
void CCharacterTrack::SerializeKey( ICharacterKey &key,XmlNodeRef &keyNode,bool bLoading )
{
	if (bLoading)
	{
		const char *str;
		
		str = keyNode->getAttr( "anim" );
		strncpy( key.animation,str,sizeof(key.animation) );
		key.animation[sizeof(key.animation)-1] = 0;

		key.duration = 0;
		key.blendTime = 0;
		key.startTime = 0;
		key.bLoop = false;
		key.speed = 1;
		keyNode->getAttr( "length",key.duration );
		keyNode->getAttr( "blend",key.blendTime );
		keyNode->getAttr( "speed",key.speed );
		keyNode->getAttr( "loop",key.bLoop );
		keyNode->getAttr( "unload",key.bUnload );
		keyNode->getAttr( "start",key.startTime );
	}
	else
	{
		if (strlen(key.animation) > 0)
			keyNode->setAttr( "anim",key.animation );
		if (key.duration > 0)
			keyNode->setAttr( "length",key.duration );
		if (key.blendTime > 0)
			keyNode->setAttr( "blend",key.blendTime );
		if (key.speed != 1)
			keyNode->setAttr( "speed",key.speed );
		if (key.bLoop)
			keyNode->setAttr( "loop",key.bLoop );
		if (key.bUnload)
			keyNode->setAttr( "unload",key.bUnload );
		if (key.startTime != 0)
			keyNode->setAttr( "start",key.startTime );
	}
}

void CCharacterTrack::GetKeyInfo( int key,const char* &description,float &duration )
{
	assert( key >= 0 && key < (int)m_keys.size() );
	CheckValid();
	description = 0;
	duration = 0;
	if (strlen(m_keys[key].animation) > 0)
	{
		description = m_keys[key].animation;
		if (m_keys[key].bLoop)
		{
			float lastTime = m_timeRange.end;
			if (key+1 < (int)m_keys.size())
			{
				lastTime = m_keys[key+1].time;
			}
			// duration is unlimited but cannot last past end of track or time of next key on track.
			duration = lastTime - m_keys[key].time;
		}
		else
			duration = (m_keys[key].duration - m_keys[key].startTime) / m_keys[key].speed;
	}
}