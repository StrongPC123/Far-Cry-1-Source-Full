////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2003.
// -------------------------------------------------------------------------
//  File name:   consoletrack.h
//  Version:     v1.00
//  Created:     30/6/2003 by Lennert Schneider.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __musictrack_h__
#define __musictrack_h__
#pragma once

//forward declarations.
#include "IMovieSystem.h"
#include "AnimTrack.h"
#include "AnimKey.h"

/** MusicTrack contains music keys, when time reach event key, it applies changes to the music-system...
*/
class CMusicTrack : public TAnimTrack<IMusicKey>
{
public:
	//////////////////////////////////////////////////////////////////////////
	// Overrides of IAnimTrack.
	//////////////////////////////////////////////////////////////////////////
	EAnimTrackType GetType() { return ATRACK_MUSIC; };
	EAnimValue GetValueType() { return AVALUE_MUSIC; };
	void GetKeyInfo( int key,const char* &description,float &duration );
	void SerializeKey( IMusicKey &key,XmlNodeRef &keyNode,bool bLoading );
};

#endif // __musictrack_h__
