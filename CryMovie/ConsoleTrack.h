////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   consoletrack.h
//  Version:     v1.00
//  Created:     12/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __consoletrack_h__
#define __consoletrack_h__
#pragma once

//forward declarations.
#include "IMovieSystem.h"
#include "AnimTrack.h"
#include "AnimKey.h"

/** EntityTrack contains entity keys, when time reach event key, it fires script event or start animation etc...
*/
class CConsoleTrack : public TAnimTrack<IConsoleKey>
{
public:
	//////////////////////////////////////////////////////////////////////////
	// Overrides of IAnimTrack.
	//////////////////////////////////////////////////////////////////////////
	EAnimTrackType GetType() { return ATRACK_CONSOLE; };
	EAnimValue GetValueType() { return AVALUE_CONSOLE; };

	void GetKeyInfo( int key,const char* &description,float &duration );
	void SerializeKey( IConsoleKey &key,XmlNodeRef &keyNode,bool bLoading );
};

#endif // __consoletrack_h__
