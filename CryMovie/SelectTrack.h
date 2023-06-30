////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   selecttrack.h
//  Version:     v1.00
//  Created:     20/8/2002 by Lennert.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __selecttrack_h__
#define __selecttrack_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "IMovieSystem.h"
#include "AnimTrack.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/** Boolean track, every key on this track negates boolean value.
*/
class CSelectTrack : public TAnimTrack<ISelectKey>
{
public:
	EAnimTrackType GetType() { return ATRACK_SELECT; };
	EAnimValue GetValueType() { return AVALUE_SELECT; };

	void GetKeyInfo( int key,const char* &description,float &duration );
	void SerializeKey( ISelectKey &key,XmlNodeRef &keyNode,bool bLoading );
};

#endif // __selecttrack_h__