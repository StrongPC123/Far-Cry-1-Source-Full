////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackviewgraph.h
//  Version:     v1.00
//  Created:     23/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __trackviewgraph_h__
#define __trackviewgraph_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "TrackViewKeys.h"

/** CTrackViewGraph dialog.
		Placed at the same position as tracks dialog, and display spline graphs of track.
*/
class CTrackViewGraph : public CTrackViewKeys
{
	DECLARE_DYNAMIC(CTrackViewGraph)
public:
	struct Item
	{
		IAnimTrack *track;
		IAnimNode *node;
		int paramId;
		Item() { track = 0; node = 0; paramId = 0; }
		Item( IAnimNode *pNode,int nParamId,IAnimTrack *pTrack )
		{
			track = pTrack; paramId = nParamId; node = pNode;
		}
	};

	CTrackViewGraph();
	virtual ~CTrackViewGraph();

protected:
	DECLARE_MESSAGE_MAP()

	//////////////////////////////////////////////////////////////////////////
	// CTrackViewKeys overrides.
	//////////////////////////////////////////////////////////////////////////
	int GetItemRect( int item,CRect &rect );
	int KeyFromPoint( CPoint point );
	int ItemFromPoint( CPoint pnt );
	void SelectKeys( const CRect &rc );

	void DrawTrack( int item,CDC *dc,CRect &rcItem );
	void DrawKeys( IAnimTrack *track,CDC *dc,CRect &rc,Range &timeRange );
	void DrawGraph( IAnimTrack *track,CDC *dc,CRect &rcItem );
	void DrawGraphAxis( CDC *dc,CRect &rcItem,int y );
	void SetCurSel( int sel );

private:
	void FitGraphToRect( IAnimTrack *track,const CRect &rcItem );


	float m_fValueScale;
	float m_fMaxValue;
	float m_fMinValue;
};

#endif // __trackviewgraph_h__