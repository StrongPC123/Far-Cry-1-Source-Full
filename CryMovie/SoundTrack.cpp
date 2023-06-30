////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   selecttrack.cpp
//  Version:     v1.00
//  Created:     20/8/2002 by Lennert Schneider.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SoundTrack.h"

//////////////////////////////////////////////////////////////////////////
void CSoundTrack::SerializeKey( ISoundKey &key,XmlNodeRef &keyNode,bool bLoading )
{
	if (bLoading)
	{
		const char *desc;
		desc=keyNode->getAttr( "filename");
		strncpy(key.pszFilename,desc,sizeof(key.pszFilename));
		key.pszFilename[sizeof(key.pszFilename)-1] = 0;
		keyNode->getAttr( "volume",key.nVolume );
		keyNode->getAttr( "pan",key.nPan );
		keyNode->getAttr( "duration",key.fDuration );
		keyNode->getAttr( "InRadius",key.inRadius );
		keyNode->getAttr( "OutRadius",key.outRadius );
		keyNode->getAttr( "Stream",key.bStream );
		keyNode->getAttr( "Is3D",key.b3DSound );
		keyNode->getAttr( "Loop",key.bLoop );
		desc = keyNode->getAttr( "desc" );
		strncpy( key.description,desc,sizeof(key.description) );
		key.description[sizeof(key.description)-1] = 0;
	}
	else
	{
		keyNode->setAttr( "filename", key.pszFilename);
		keyNode->setAttr( "volume",key.nVolume );
		keyNode->setAttr( "pan",key.nPan );
		keyNode->setAttr( "duration",key.fDuration );
		keyNode->setAttr( "desc",key.description );
		keyNode->setAttr( "InRadius",key.inRadius );
		keyNode->setAttr( "OutRadius",key.outRadius );
		keyNode->setAttr( "Stream",key.bStream );
		keyNode->setAttr( "Is3D",key.b3DSound );
		keyNode->setAttr( "Loop",key.bLoop );
	}
}

//////////////////////////////////////////////////////////////////////////
void CSoundTrack::GetKeyInfo( int key,const char* &description,float &duration )
{
	assert( key >= 0 && key < (int)m_keys.size() );
	CheckValid();
	description = 0;
	duration = m_keys[key].fDuration;
	//if (strlen(m_keys[key].description) > 0)
	//	description = m_keys[key].description;
	if (strlen(m_keys[key].pszFilename) > 0)
		description=m_keys[key].pszFilename;
}