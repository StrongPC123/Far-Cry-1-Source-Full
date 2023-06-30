////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   booltrack.cpp
//  Version:     v1.00
//  Created:     4/6/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BoolTrack.h"

//////////////////////////////////////////////////////////////////////////
void CBoolTrack::GetKeyInfo( int index,const char* &description,float &duration )
{
	description = 0;
	duration = 0;
}

//////////////////////////////////////////////////////////////////////////
void CBoolTrack::GetValue( float time,bool &value )
{
	value = true; // True by default.

	CheckValid();

	int nkeys = m_keys.size();
	if (nkeys < 1)
		return;

	int key = 0;
	while ((key < nkeys) && (time >= m_keys[key].time)) 
		key++;

	value = !(key & 1);	// True if even key.
}

//////////////////////////////////////////////////////////////////////////
void CBoolTrack::SetValue( float time,const bool &value,bool bDefault )
{
	Invalidate();
}