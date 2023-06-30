////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   charactertrack.h
//  Version:     v1.00
//  Created:     20/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __charactertrack_h__
#define __charactertrack_h__

#if _MSC_VER > 1000
#pragma once
#endif

//forward declarations.
#include "IMovieSystem.h"
#include "AnimTrack.h"

/** CCharacterTrack contains entity keys, when time reach event key, it fires script event or start animation etc...
*/
class CCharacterTrack : public TAnimTrack<ICharacterKey>
{
public:
	//////////////////////////////////////////////////////////////////////////
	// Overrides of IAnimTrack.
	//////////////////////////////////////////////////////////////////////////
	EAnimTrackType GetType() { return ATRACK_CHARACTER; };
	EAnimValue GetValueType() { return AVALUE_CHARACTER; };

	void GetKeyInfo( int key,const char* &description,float &duration );
	void SerializeKey( ICharacterKey &key,XmlNodeRef &keyNode,bool bLoading );
};

#endif // __charactertrack_h__
