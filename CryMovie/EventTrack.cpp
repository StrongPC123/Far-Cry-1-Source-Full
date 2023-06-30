////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   entitytrack.cpp
//  Version:     v1.00
//  Created:     20/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EventTrack.h"

//////////////////////////////////////////////////////////////////////////
void CEventTrack::SerializeKey( IEventKey &key,XmlNodeRef &keyNode,bool bLoading )
{
	if (bLoading)
	{
		const char *str;
		str = keyNode->getAttr( "event" );
		strncpy( key.event,str,sizeof(key.event) );
		key.event[sizeof(key.event)-1] = 0;
			
		str = keyNode->getAttr( "anim" );
		strncpy( key.animation,str,sizeof(key.animation) );
		key.animation[sizeof(key.animation)-1] = 0;

		key.duration = 0;
		keyNode->getAttr( "length",key.duration );
	}
	else
	{
		if (strlen(key.event) > 0)
			keyNode->setAttr( "event",key.event );
		if (strlen(key.animation) > 0)
			keyNode->setAttr( "anim",key.animation );
		if (key.duration > 0)
			keyNode->setAttr( "length",key.duration );
	}
}

void CEventTrack::GetKeyInfo( int key,const char* &description,float &duration )
{
	assert( key >= 0 && key < (int)m_keys.size() );
	CheckValid();
	description = 0;
	duration = 0;
	if (strlen(m_keys[key].event) > 0)
		description = m_keys[key].event;
}