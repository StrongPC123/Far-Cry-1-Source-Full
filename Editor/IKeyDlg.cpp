////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   ikeydlg.cpp
//  Version:     v1.00
//  Created:     21/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////


#include "StdAfx.h"
#include "ikeydlg.h"

#include "TrackViewUndo.h"

IKeyDlg::IKeyDlg(UINT nIDTemplate, CWnd *pParentWnd) :
				 CDialog(nIDTemplate, pParentWnd)
{
	m_bNoReloadKey = false;
}

//////////////////////////////////////////////////////////////////////////
void IKeyDlg::RefreshTrackView()
{
	// Prevent endless recursion, when SetKey calls refersh.
	m_bNoReloadKey = true;
	GetIEditor()->UpdateTrackView(true);
	m_bNoReloadKey = false;
}

//////////////////////////////////////////////////////////////////////////
void IKeyDlg::RecordTrackUndo()
{
	if (m_track)
	{
		CUndo undo("Track Modify");
		CUndo::Record( new CUndoTrackObject(m_track) );
	}
}