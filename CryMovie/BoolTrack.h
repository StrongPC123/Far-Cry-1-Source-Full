////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   booltrack.h
//  Version:     v1.00
//  Created:     4/6/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __booltrack_h__
#define __booltrack_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "IMovieSystem.h"
#include "AnimTrack.h"

//! Boolean key.
struct IBoolKey : public IKey
{
	IBoolKey() {};
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/** Boolean track, every key on this track negates boolean value.
*/
class CBoolTrack : public TAnimTrack<IBoolKey>
{
public:
	virtual EAnimTrackType GetType() { return ATRACK_BOOL; };
	virtual EAnimValue GetValueType() { return AVALUE_BOOL; };

	virtual void GetValue( float time,bool &value );
	virtual void SetValue( float time,const bool &value,bool bDefault=false );

	void SerializeKey( IBoolKey &key,XmlNodeRef &keyNode,bool bLoading ) {};
	void GetKeyInfo( int key,const char* &description,float &duration );
};

#endif // __booltrack_h__
