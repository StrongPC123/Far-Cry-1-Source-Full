////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   consoletrack.cpp
//  Version:     v1.00
//  Created:     12/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MusicTrack.h"

//////////////////////////////////////////////////////////////////////////
void CMusicTrack::SerializeKey( IMusicKey &key,XmlNodeRef &keyNode,bool bLoading )
{
	if (bLoading)
	{
		const char *pStr;
		int nType;
		if (!keyNode->getAttr("type", nType))
			key.eType=eMusicKeyType_SetMood;
		else
			key.eType=(EMusicKeyType)nType;
		pStr=keyNode->getAttr("mood");
		if (pStr)
		{
			strncpy(key.szMood, pStr, sizeof(key.szMood));
			key.szMood[sizeof(key.szMood)-1]=0;
		}else
		{
			key.szMood[0]=0;
		}
		if (!keyNode->getAttr("volramp_time", key.fTime))
			key.fTime=0.0f;
	}
	else
	{
		keyNode->setAttr("type", key.eType);
		if (strlen(key.szMood)>0)
			keyNode->setAttr("mood", key.szMood);
		keyNode->setAttr("volramp_time", key.fTime);
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicTrack::GetKeyInfo( int key,const char* &description,float &duration )
{
	assert( key >= 0 && key < (int)m_keys.size() );
	CheckValid();
	description = 0;
	duration = 0;
	switch (m_keys[key].eType)
	{
		case eMusicKeyType_SetMood:
			duration=0.0f;
			description=m_keys[key].szMood;
			break;
		case eMusicKeyType_VolumeRamp:
			duration=m_keys[key].fTime;
			description="RampDown";
			break;
	}
}
