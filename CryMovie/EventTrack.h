////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   entitytrack.h
//  Version:     v1.00
//  Created:     20/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __entitytrack_h__
#define __entitytrack_h__

#if _MSC_VER > 1000
#pragma once
#endif


//forward declarations.
#include "IMovieSystem.h"
#include "AnimTrack.h"
#include "AnimKey.h"

/** EntityTrack contains entity keys, when time reach event key, it fires script event or start animation etc...
*/
class CEventTrack : public TAnimTrack<IEventKey>
{
public:
	//////////////////////////////////////////////////////////////////////////
	// Overrides of IAnimTrack.
	//////////////////////////////////////////////////////////////////////////
	EAnimTrackType GetType() { return ATRACK_EVENT; };
	EAnimValue GetValueType() { return AVALUE_EVENT; };

	void GetKeyInfo( int key,const char* &description,float &duration );
	void SerializeKey( IEventKey &key,XmlNodeRef &keyNode,bool bLoading );
};

#endif // __entitytrack_h__
