////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackviewundo.cpp
//  Version:     v1.00
//  Created:     30/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TrackViewUndo.h"
#include <IMovieSystem.h>

//////////////////////////////////////////////////////////////////////////
CUndoTrackObject::CUndoTrackObject( IAnimTrack *track )
{
	// Stores the current state of this track.
	assert( track != 0 );

	m_pTrack = track;

	// Store undo info.
	m_undo = new CXmlNode("Undo");
	m_pTrack->Serialize( m_undo,false );
}

//////////////////////////////////////////////////////////////////////////
void CUndoTrackObject::Undo( bool bUndo )
{
	if (!m_undo)
		return;

	if (bUndo)
	{
		m_redo = new CXmlNode("Redo");
		m_pTrack->Serialize( m_redo,false );
	}
	// Undo track state.
	m_pTrack->Serialize( m_undo,true );

	if (bUndo)
	{
		// Refresh stuff after undo.
		GetIEditor()->GetAnimation()->ForceAnimation();
		GetIEditor()->UpdateTrackView(true);
	}
}

//////////////////////////////////////////////////////////////////////////
void CUndoTrackObject::Redo()
{
	if (!m_redo)
		return;

	// Redo track state.
	m_pTrack->Serialize( m_redo,true );

	// Refresh stuff after undo.
	GetIEditor()->GetAnimation()->ForceAnimation();
	GetIEditor()->UpdateTrackView(true);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CUndoAnimSequenceObject::CUndoAnimSequenceObject( IAnimSequence *seq )
{
	// Stores the current state of this track.
	assert( seq != 0 );

	m_pSequence = seq;

	// Store undo info.
	m_undo = new CXmlNode("Undo");
	m_pSequence->Serialize( m_undo,false );
}

//////////////////////////////////////////////////////////////////////////
void CUndoAnimSequenceObject::Undo( bool bUndo )
{
	if (!m_undo)
		return;

	if (bUndo)
	{
		m_redo = new CXmlNode("Redo");
		m_pSequence->Serialize( m_redo,false );
	}
	// Undo track state.
	m_pSequence->Serialize( m_undo,true );

	if (bUndo)
	{
		// Refresh stuff after undo.
		GetIEditor()->GetAnimation()->ForceAnimation();
		GetIEditor()->UpdateTrackView(false);
	}
}

//////////////////////////////////////////////////////////////////////////
void CUndoAnimSequenceObject::Redo()
{
	if (!m_redo)
		return;

	// Redo track state.
	m_pSequence->Serialize( m_redo,true );

	// Refresh stuff after undo.
	GetIEditor()->GetAnimation()->ForceAnimation();
	GetIEditor()->UpdateTrackView(false);
}