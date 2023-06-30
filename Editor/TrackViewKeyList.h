////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackviewkeylist.h
//  Version:     v1.00
//  Created:     23/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __trackviewkeylist_h__
#define __trackviewkeylist_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "TrackViewKeys.h"

/** List of tracks.
*/
class CTrackViewKeyList : public CTrackViewKeys
{
	DECLARE_DYNAMIC(CTrackViewKeyList)
public:
	// public stuff.

	CTrackViewKeyList();
	~CTrackViewKeyList();


protected:
	DECLARE_MESSAGE_MAP()

	void DrawTrack( int item,CDC *dc,CRect &rcItem );
	void DrawKeys( IAnimTrack *track,CDC *dc,CRect &rc,Range &timeRange );

	// Ovverides from CTrackViewKeys.
	int KeyFromPoint( CPoint point );
	void SelectKeys( const CRect &rc );
	int GetItemRect( int item,CRect &rect );
};


#endif // __trackviewkeylist_h__