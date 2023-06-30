////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   ikeydlg.h
//  Version:     v1.00
//  Created:     21/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ikeydlg_h__
#define __ikeydlg_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "IMovieSystem.h"

class IKeyDlg : public CDialog
{
public:
	explicit IKeyDlg(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	
	virtual void SetKey(IAnimNode *node, IAnimTrack *track, int key) = 0;
	IAnimNode* GetNode() const { return m_node; };
	IAnimTrack* GetTrack() const { return m_track; };
	int GetKey() const { return m_key; };

	// refresh keys displayed in track view.
	void RefreshTrackView();
	void RecordTrackUndo();

	bool CanReloadKey() const { return !m_bNoReloadKey; };

protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	IAnimTrack* m_track;
	IAnimNode* m_node;
	int m_key;

	bool m_bNoReloadKey;
};

#endif // __ikeydlg_h__